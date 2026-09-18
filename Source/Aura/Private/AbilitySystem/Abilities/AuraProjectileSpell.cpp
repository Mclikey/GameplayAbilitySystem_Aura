// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "interaction/CombatInterface.h"
#include "Aura/Public/AuraGameplayTags.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;
	
	ICombatInterface *CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();
		
		FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
		Rotation.Pitch = 0.0f;
		
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		
		SpawnTransform.SetRotation(Rotation.Quaternion());
		
		
		//第一步：用 SpawnActorDeferred 创建半成品投掷物
		//SpawnActorDeferred延迟生成
		AAuraProjectile* Projectile =  GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,                               // 1. 要生成的资产类（比如你的火球术蓝图）
			SpawnTransform,                                // 2. 生成时的位置和旋转（坐标、朝向）
			GetOwningActorFromActorInfo(),                 // 3. 所有者 (Owner)：谁拥有它
			Cast<APawn>(GetOwningActorFromActorInfo()),    // 4. 施法者/煽动者 (Instigator)：通常是哪个 Pawn 发起的攻击
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn // 5. 碰撞冲突处理：即使生成的位置卡在墙里，也强行生成，不取消
		);
				
		
		const UAbilitySystemComponent* SourceASC =  UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		EffectContextHandle.SetAbility(this);
		EffectContextHandle.AddSourceObject(Projectile);
		TArray<TWeakObjectPtr<AActor>> Actors;
		Actors.Add(Projectile);	
		EffectContextHandle.AddActors(Actors);
		FHitResult HitResult;
		HitResult.Location = ProjectileTargetLocation;
		EffectContextHandle.AddHitResult(HitResult);
		
		//通过配置好的伤害效果类（DamageEffectClass）创建一个 GameplayEffect 的实例蓝图（SpecHandle）。此时这个 Spec 里面还不知道具体的伤害数值是多少（它可能是一个空的模板）。
		const FGameplayEffectSpecHandle SpecHandle =  SourceASC->MakeOutgoingSpec(DamageEffectClass,GetAbilityLevel(),EffectContextHandle);
		
		const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
		//在数据包（SpecHandle）里贴上一个名叫 Damage 的标签，并把它的数值设为 50.f
		//因为同一个伤害效果类（比如 GE_Damage）可以用来做火球术、普攻、冰锥术等。通过 Set by Caller，
		//你可以让同一个 GameplayEffect 在不同技能里打出不同的伤害数字（火球术 50 点，普攻 20 点），
		//而不需要为每个技能单独去创建几十个不同的 GameplayEffect 资产
		
		for (auto& Pair: DamageTypes)
		{
			const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle,Pair.Key,ScaledDamage);
		}
		
		//把这个已经塞了 Damage = 50.f 标签数据的完整数据包赋值给生成的投掷物 (AAuraProjectile)。
		//当这个火球砸中敌人时，投掷物会把这个 SpecHandle 应用到敌人的 AbilitySystemComponent 上。
		// 第二步：在它正式启动前，把前面带标签的伤害数据包安全地塞给它！
		Projectile->DamageEffectSpecHandle = SpecHandle;
		
		// 第三步：数据都配置好了，调用这个让投掷物正式“出生”并开始运行 BeginPlay
		//让这个投掷物正式注册到世界（World）中。触发它的 BeginPlay()。让它开始正常的逻辑（比如根据初始速度和方向开始向前飞行）。
		Projectile->FinishSpawning(SpawnTransform);
	}
	
}

