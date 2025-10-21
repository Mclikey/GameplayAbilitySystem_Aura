// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/AuraInputConfig.h"
#include "InputAction.h"

const UInputAction* UAuraInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FAuraInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("can't find AbilityInputAction for InputTag [%s], on Inputconfig [%s]"), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;

}
