// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

struct AuraDamageStatics
{
	
	//DECLARE和DEFINE是配套使用的
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);// 声明捕获定义
	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet,Armor,Target,false);// 定义捕获规则
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
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC =  ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC =  ExecutionParams.GetTargetAbilitySystemComponent();
	
	const AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	const AActor* TargetActor = TargetASC ? TargetASC->GetAvatarActor() : nullptr;
	
	
	
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	
	FAggregatorEvaluateParameters EvaluationParameters;  //聚合器评估参数
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;
	
	float Armor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef,EvaluationParameters,Armor);
	Armor = FMath::Max<float>(0.f,Armor);
	++Armor;
	
	FGameplayModifierEvaluatedData EvaluationData(DamageStatics().ArmorProperty,EGameplayModOp::Additive,Armor);
	OutExecutionOutput.AddOutputModifier(EvaluationData);
}
