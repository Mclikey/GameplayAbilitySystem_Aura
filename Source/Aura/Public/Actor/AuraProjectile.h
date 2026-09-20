// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Engine/NetSerialization.h"	// FVector_NetQuantize / FVector_NetQuantizeNormal
#include "AuraProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;

UCLASS()
class AURA_API AAuraProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AAuraProjectile();
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	
	UPROPERTY(BlueprintReadWrite,meta = (ExposeOnSpawn = true))
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 命中表现多播：只有服务器调用，服务器与所有客户端在同一个爆点播同一份表现 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HandleImpact(FVector_NetQuantize ImpactLocation, FVector_NetQuantizeNormal ImpactNormal);

private:
	
	UPROPERTY(EditAnywhere)
	float LifeSpan = 15.f;
	
	/** 服务器专用：本颗火球是否已经完成过一次权威命中判定（客户端不再读写它） */
	bool bHit = false;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> ImpactEffect;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> ImpactSound;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> LoopingSound;
	
	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopingSoundComponent;

};
