// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ModMagCalc/MMC_MaxHealth.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "interaction/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();//设置需要获取的属性对象
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;//设置拾取对象为GE的应用目标
	VigorDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(VigorDef);//添加到捕获属性数值，只有添加到列表，才会去获取属性值,核心：把这个配置塞进白名单列表！
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{

	//  从 source 和 target 获取 Tag
	// 1. 从技能效果包（Spec）里提取出施法者和受击者身上的标签容器
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	//FAggregatorEvaluateParameters作用是：在计算或提取某个属性（比如体力 Vigor）时，把双方身上当前的 Gameplay Tags（标签）打包传递进去，作为“修饰条件”或者“环境上下文”
	//2. 创建一个评估参数结构体，并把标签装进去
	FAggregatorEvaluateParameters EvaluationParameters;  //聚合器评估参数
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	//获取体力值
	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameters, Vigor);
	Vigor = FMath::Max<float>(Vigor, 0.f);

	//获取等级
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());
	const int32 PlayerLevel = CombatInterface->GetPlayerLevel();

	//计算最大血量
	return 80.f + Vigor * 2.5f + PlayerLevel * 10.f;
}

