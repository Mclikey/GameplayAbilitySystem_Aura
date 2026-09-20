// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "interaction/CombatInterface.h"

struct AuraDamageStatics
{
	
	//DECLARE和DEFINE是配套使用的
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);// 声明捕获定义，护甲
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance); //格挡
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration); //护甲穿透
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance); //暴击率
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance); //暴击抵抗
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage); //暴击伤害
	
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance); 
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance); 
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance); 
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance); 
	
	TMap<FGameplayTag,FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;
	
	
	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet,Armor,Target,false);// 定义捕获规则
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet,BlockChance,Target,false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet,ArmorPenetration,Source,false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet,CriticalHitChance,Source,false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet,CriticalHitResistance,Target,false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet,CriticalHitDamage,Source,false);
		
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet,FireResistance,Target,false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet,LightningResistance,Target,false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet,ArcaneResistance,Target,false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet,PhysicalResistance,Target,false);
		
		
		//添加映射表
		const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor,ArmorDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance,BlockChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration,ArmorPenetrationDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitChance,CriticalHitChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitResistance,CriticalHitResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitDamage,CriticalHitDamageDef);
		
		
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire,FireResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning,LightningResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane,ArcaneResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical,PhysicalResistanceDef);
		
		
		
	}
};


//C++ 经典的静态局部变量单例模式（Static Singleton）：整个游戏运行期间，这个结构体只在内存中初始化一次。以后不管触发多少次伤害，大家直接共用这一份定义，既干净又高效。
static const AuraDamageStatics& DamageStatics() 
{
	static AuraDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
	
	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC =  ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC =  ExecutionParams.GetTargetAbilitySystemComponent();
	
	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetActor = TargetASC ? TargetASC->GetAvatarActor() : nullptr;
	ICombatInterface* SourceCombatInterface = Cast<ICombatInterface>(SourceAvatar);
	ICombatInterface* TargetCombatInterface = Cast<ICombatInterface>(TargetActor);
	
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	
	FAggregatorEvaluateParameters EvaluationParameters;  //聚合器评估参数
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;
	
	//计算Damage
	// float Damage = Spec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Damage);		
	
	float Damage = 0.f;
	for (const TTuple<FGameplayTag, FGameplayTag>& Pair  : FAuraGameplayTags::Get().DamageTypesToResistance)
	{
		const FGameplayTag DamageTypeTag = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;
		
		checkf(AuraDamageStatics().TagsToCaptureDefs.Contains(ResistanceTag),TEXT("TagsToCaptureDefs does not contain Tag : [%s] in  ExecCalc_Damage"), *ResistanceTag.ToString());
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = AuraDamageStatics().TagsToCaptureDefs[ResistanceTag];
		
		float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key);
		
		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef,EvaluationParameters,Resistance);
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);
		
		DamageTypeValue *= (100.f - Resistance) /100.f;
		
		Damage += DamageTypeValue;
	}
	
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef,EvaluationParameters,TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(0.f,TargetBlockChance);
	
	
	FGameplayEffectContextHandle EffectContextHandle =  Spec.GetContext();
	//是否格挡
	const bool bBlocked = FMath::RandRange(1,100) < TargetBlockChance;
	UAuraAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle,bBlocked);

	
	Damage = bBlocked ? Damage / 2.f : Damage;
	
	//护甲
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef,EvaluationParameters,TargetArmor);
	TargetArmor = FMath::Max<float>(0.f,TargetArmor);
	
	//护甲穿透
	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef,EvaluationParameters,SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(0.f,SourceArmorPenetration);
	
	const UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	
	//护甲穿透系数
	const FRealCurve* ArmorPenetrationCurve =  CharacterClassInfo->DamageCalculationCoefficient->FindCurve(FName("ArmorPenetration"),FString());
	const float ArmorPenetrationCoefficient =  ArmorPenetrationCurve->Eval(SourceCombatInterface->GetPlayerLevel());
	const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100.f;
	
	//护甲系数值
	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficient->FindCurve(FName("EffectiveArmor"),FString());
	const float EffectiveArmorCoefficient =  EffectiveArmorCurve->Eval(TargetCombatInterface->GetPlayerLevel());
	Damage *=  (100 - EffectiveArmor * EffectiveArmorCoefficient) / 100.f;
	
	//暴击率
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef,EvaluationParameters,SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(0.f,SourceCriticalHitChance);
	
	//暴击抵抗
	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef,EvaluationParameters,TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(0.f,TargetCriticalHitResistance);
	
	//暴击伤害
	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef,EvaluationParameters,SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(0.f,SourceCriticalHitDamage);
	
	
	
	//暴击最后伤害计算
	//读曲线表获取CriticalHitResistance暴击抵抗值
	const FRealCurve* CriticalHitResistanceCurve =  CharacterClassInfo->DamageCalculationCoefficient->FindCurve(FName("CriticalHitResistance"),FString());
	const float CriticalHitResistanceCoefficient =  CriticalHitResistanceCurve->Eval(TargetCombatInterface->GetPlayerLevel());
	
	const float EffectiveCritialHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCoefficient;
	//是否暴击
	const bool bCriticalHit = FMath::RandRange(1,100) < EffectiveCritialHitChance;
	UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle,bCriticalHit);
	Damage = bCriticalHit ? Damage *2.f + SourceCriticalHitDamage: Damage;
	
	
	FGameplayModifierEvaluatedData EvaluationData(UAuraAttributeSet::GetIncomingDamageAttribute(),EGameplayModOp::Additive,Damage);
	OutExecutionOutput.AddOutputModifier(EvaluationData);
}
