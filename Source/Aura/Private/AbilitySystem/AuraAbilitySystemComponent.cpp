// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"


void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);
	
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		//FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		//GiveAbility(AbilitySpec); //只应用不激活
		//GiveAbilityAndActivateOnce(AbilitySpec); //应用技能并激活一次

		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAuraGameplayAbility* AbilityBase = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(AbilityBase->StartupInputTag);
			GiveAbility(AbilitySpec); //只应用不激活
			// GiveAbilityAndActivateOnce(AbilitySpec); //应用技能并激活一次
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (auto AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{

	if (!InputTag.IsValid()) return;

	for (auto AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}



void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	//GEngine->AddOnScreenDebugMessage(1, 8.f, FColor::Blue, FString("Effect Applied!"));
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}
