// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "GameplayTagContainer.h"
#include "AuraPlayerController.generated.h"


class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnemyInterface;
class UAuraInputConfig;
class UAuraAbilitySystemComponent;
class USplineComponent;

/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAuraPlayerController();
	virtual void PlayerTick(float DeltaTime) override;


protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	
	//人物移动
	void Move(const FInputActionValue& InputActionValue);

	//光标追踪
	void CursorTrace();

	IEnemyInterface* LastActor;
	IEnemyInterface* ThisActor;


	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	UAuraAbilitySystemComponent* GetASC();

	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;            //跟随时间

	float ShortPressThreshold = 0.5f;   //多长时间为短触

	bool bAutoRunning = false;       //是否是自动行走
	bool bTargeting = false;

	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;   //离目的地的距离

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;

	void AutoRun();

};
