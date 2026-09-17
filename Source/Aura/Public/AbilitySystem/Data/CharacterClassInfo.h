// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterClassInfo.generated.h"

class UGameplayEffect;
class UGameplayAbility;

UENUM(BlueprintType)
enum class ECharacterClass:	uint8
{
	Elementalist,
	Warrior,
	Ranger
};

USTRUCT(BlueprintType)
struct FCharacterClassDefaultInfo
{
	GENERATED_BODY() //为结构体启用虚幻的反射和垃圾回收机制。
	
	UPROPERTY(EditDefaultsOnly,Category = "Class Default")
	TSubclassOf<UGameplayEffect> PrimaryAttribute;
	
};

/**
 * 
 */
UCLASS()
class AURA_API UCharacterClassInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditDefaultsOnly,Category = "Character Class Default")
	TMap<ECharacterClass, FCharacterClassDefaultInfo> CharacterClassInformation; //把枚举和对应的结构体数据关联起来，方便通过职业类型直接查到对应的配置
	
	UPROPERTY(EditDefaultsOnly,Category = "Common Class Default")
	TSubclassOf<UGameplayEffect> SecondaryAttributes;
	
	UPROPERTY(EditDefaultsOnly,Category = "Common Class Default")
	TSubclassOf<UGameplayEffect> VitalAttributes;
	
	UPROPERTY(EditDefaultsOnly,Category = "Common Class Default")
	TArray<TSubclassOf<UGameplayAbility>> CommonAbilities;
	
	FCharacterClassDefaultInfo GetClassDefaultInfo(ECharacterClass CharacterClass);
	
	
};
