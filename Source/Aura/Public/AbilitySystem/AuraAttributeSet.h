// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AuraAttributeSet.generated.h"


//能够在PostGameplayEffectExecute中获取到的信息，放到一个结构体中方便获取
USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()
	FEffectProperties() {}
	FGameplayEffectContextHandle EffectContextHandle;
	//Source 代表这个Effect是从哪个Actor释放的,Target代表自身(拥有该AttributeSet的角色)

	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;
	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;
	UPROPERTY()
	AController* SourceController = nullptr;
	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;
	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;
	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;
	UPROPERTY()
	AController* TargetController = nullptr;
	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
 	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
 	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
 	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
 	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


// typedef is specific to the FGameplayAttribute() signature, but TstaticFunptr is generic to any signature chosen
//typedef TBasestaticDelegateInstance<FGameplayAttribute(), FefaultDelegateUserPolicy>::FFuncPtr FAttributeFuncPtr,
template<class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

/**
 * 
 */
UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	//属性访问器有4种： 
	// 获取属性（GAMEPLAYATTRIBUTE_PROPERTY_GETTER）、 
	// 获取属性值（GAMEPLAYATTRIBUTE_PROPERTY_VALUE_GETTER）、 
	// 初始化值（GAMEPLAYATTRIBUTE_PROPERTY_VALUE_INITTER）、 
	// 修改属性值（GAMEPLAYATTRIBUTE_PROPERTY_VALUE_SETTER）

	UAuraAttributeSet();

	//指定那些属性需要在服务器和客户端之间同步，并定义每个属性的复制条件（Replication Condition）
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	//在GameplayEffect被添加时的回调
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;


	//给AS添加一个变量属性，类型为Map，key为Tag标签，Value为对应的委托,
	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;


	/*
		Primary Attributes
	*/
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Primary Attributrs")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Strength);//自动生成对应的宏

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence, Category = "Primary Attributrs")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Intelligence);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Resilience, Category = "Primary Attributrs")
	FGameplayAttributeData Resilience;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Resilience);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Vigor, Category = "Primary Attributrs")
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Vigor);


	/*
		Secondary Attributes

		Armor 防御，基于Resilience 韧性属性计算， 降低所受伤害
		ArmorPenetration 护甲穿透，基于Resilience 韧性属性计算，降低敌人的防御，增加暴击率
		BlockChance 格挡率 ，基于Armor 防御属性计算，增加格挡伤害概率，触发时，降低一半所受伤害
		CriticalHitChance 暴击率，基于ArmorPenetration 护甲穿透属性计算，增加触发暴击伤害的概率
		CriticalHitDamage 暴击伤害，基于ArmorPenetration 护甲穿透属性计算，触发暴击时基于增加的伤害量
		CriticalHitResistance 暴击抵抗，基于Armor 防御属性计算，降低敌人的暴击概率
		HealthRegeneration 血量自动恢复，基于Vigor 体力属性计算，每秒自动恢复一定血量
		ManaRegeneration 蓝量自动恢复，基于Intelligence 智力属性，每秒自动恢复蓝量
		MaxHealth 血量上限，基于Vigor 体力属性计算
		MaxMana 蓝量上限，基于Intelligence 智力属性

	*/

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "Secondary Attributrs")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Armor);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ArmorPenetration, Category = "Secondary Attributrs")
	FGameplayAttributeData ArmorPenetration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ArmorPenetration);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BlockChance, Category = "Secondary Attributrs")
	FGameplayAttributeData BlockChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, BlockChance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitChance, Category = "Secondary Attributrs")
	FGameplayAttributeData CriticalHitChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitChance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitDamage, Category = "Secondary Attributrs")
	FGameplayAttributeData CriticalHitDamage;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitDamage);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitResistance, Category = "Secondary Attributrs")
	FGameplayAttributeData CriticalHitResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegeneration, Category = "Secondary Attributrs")
	FGameplayAttributeData HealthRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, HealthRegeneration);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ManaRegeneration, Category = "Secondary Attributrs")
	FGameplayAttributeData ManaRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ManaRegeneration);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vital Attributrs")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Vital Attributrs")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana);


	/*
		Vital Attributes
	*/
	
	//生命值
	//ReplicatedUsing为当服务端修改Health数值是的回调，拥有Replicated属性时，其值会从服务器自动复制到客户端
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital Attributrs")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health);
	//法力值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Vital Attributrs")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana);

	


	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;


	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;
	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;
	UFUNCTION()
	void OnRep_Resilience(const FGameplayAttributeData& OldResilience) const;
	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;


	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;

	UFUNCTION()
	void OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const;

	UFUNCTION()
	void OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const;

	UFUNCTION()
	void OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const;

	UFUNCTION()
	void OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const;

	UFUNCTION()
	void OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const;

	UFUNCTION()
	void OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const;

	UFUNCTION()
	void OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

private:
	//从PostGameplayEffectExecute的Data中获取所有的属性,包括ContextHandle,Actor,Character等等
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props);

	
};



/*
经过了长达几分钟的思考，主要属性为一下几项：

Strength 力量
Intelligence 智力
Resilience 韧性
Vigor 体力


而次要属性里面我们设计了多项数值，依托于上面的主要属性，主要用于战斗中

MaxHealth 血量上限，基于Vigor 体力属性计算
MaxMana 蓝量上限，基于Intelligence 智力属性
Armor 防御，基于Resilience 韧性属性计算， 降低所受伤害
ArmorPenetration 护甲穿透，基于Resilience 韧性属性计算，降低敌人的防御，增加暴击率
BlockChance 格挡率 ，基于Armor 防御属性计算，增加格挡伤害概率，触发时，降低一半所受伤害
CriticalHitChance 暴击率，基于ArmorPenetration 护甲穿透属性计算，增加触发暴击伤害的概率
CriticalHitDamage 暴击伤害，基于ArmorPenetration 护甲穿透属性计算，触发暴击时基于增加的伤害量
CriticalHitResistance 暴击抵抗，基于Armor 防御属性计算，降低敌人的暴击概率
HealthRegeneration 血量自动恢复，基于Vigor 体力属性计算，每秒自动恢复一定血量
ManaRegeneration 蓝量自动恢复，基于Intelligence 智力属性，每秒自动恢复蓝量
*/