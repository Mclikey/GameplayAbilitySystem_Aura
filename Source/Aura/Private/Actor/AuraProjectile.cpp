// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Aura.h"
#include "Components/AudioComponent.h"
#include "GameFramework/PlayerState.h"	// 阵营判定需要 APlayerState
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	/**
	 * 阵营。目前只区分"玩家方 / AI 方"，用于"不打自己人"。
	 * 以后要做多队伍(红蓝对抗)，把这里换成真实 TeamId，再比较 Attitude 即可。
	 */
	enum class EAuraFaction : uint8
	{
		None,	// 没有 ASC 的 Actor（场景道具、触发器等），不参与阵营判定
		Player,	// 玩家方
		Enemy	// AI 方
	};

	/**
	 * 用「ASC 的宿主」反推阵营：
	 *   - 玩家角色的 ASC 寄生在 PlayerState 上 → AuraCharacter::InitAbilityActorInfo(AuraPlayerState, this)
	 *   - 敌人的 ASC 宿主就是自身              → AuraEnemy::InitAbilityActorInfo(this, this)
	 * 所以"ASC->GetOwnerActor() 是不是 APlayerState"就等价于"这具身体属于玩家方"。
	 * 这样判定不依赖 Controller/Pawn 是否就绪，服务器上永远成立。
	 */
	EAuraFaction ResolveFaction(AActor* Actor)
	{
		const UAbilitySystemComponent* ASC = IsValid(Actor)
			? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor)
			: nullptr;

		if (ASC == nullptr)
		{
			return EAuraFaction::None;
		}

		return Cast<APlayerState>(ASC->GetOwnerActor()) != nullptr ? EAuraFaction::Player : EAuraFaction::Enemy;
	}
}

// Sets default values
AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; //客户端可以复制
	
	Sphere = CreateDefaultSubobject<USphereComponent>(FName("Sphere"));
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic,ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic,ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("Projectile"));
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSpan);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);
	 
	LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound,GetRootComponent());
	
}

void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 施法者守卫：Owner / Instigator 都是真正施法的角色，会复制到客户端，两端判定一致
	if (OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	// 命中判定只归服务器：客户端不参与判定、也不本机播表现，完全等服务器的多播
	if (!HasAuthority())
	{
		return;
	}
	
	if (!UAuraAbilitySystemLibrary::IsNotFriend(DamageEffectSpecHandle.Data.Get()->GetContext().GetEffectCauser(),OtherActor))
	{
		return;
	}

	// 同一颗火球只处理一次权威命中
	if (bHit)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

	// 友军守卫：同阵营之间不产生任何效果（玩家不打玩家、AI 不打 AI）。
	// 这里直接 return —— 不置 bHit、不播命中表现、不销毁，火球会"穿过去"继续飞。
	// 施法者没有 ASC 时（None）不做拦截，避免把非 ASC 来源的投射物误伤为无敌。
	const EAuraFaction SourceFaction = ResolveFaction(GetOwner());
	if (SourceFaction != EAuraFaction::None && SourceFaction == ResolveFaction(OtherActor))
	{
		return;
	}

	bHit = true;

	// 爆点取真实接触点；拿不到扫掠信息时退回火球当前位置与反向前向
	// 注意：FHitResult::ImpactPoint/ImpactNormal 是 FVector_NetQuantize(Normal)，直接与 FVector 做三元会因"双向都能转换"产生二义性，需显式统一为 FVector
	const FVector ImpactLocation = bFromSweep ? FVector(SweepResult.ImpactPoint) : GetActorLocation();
	const FVector ImpactNormal = bFromSweep ? FVector(SweepResult.ImpactNormal) : -GetActorForwardVector();

	// 服务器算一次爆点，广播出去，服务器与所有客户端播同一份表现
	Multicast_HandleImpact(FVector_NetQuantize(ImpactLocation), FVector_NetQuantizeNormal(ImpactNormal));

	if (TargetASC != nullptr && DamageEffectSpecHandle.Data.IsValid())
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
	}

	Destroy();
}

void AAuraProjectile::Multicast_HandleImpact_Implementation(FVector_NetQuantize ImpactLocation, FVector_NetQuantizeNormal ImpactNormal)
{
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, ImpactLocation, FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, ImpactLocation, ImpactNormal.Rotation());
	if (LoopingSoundComponent) LoopingSoundComponent->Stop();
}


