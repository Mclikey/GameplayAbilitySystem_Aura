// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

void UOverlayWidgetController::BroadcastInitivalValues()
{
	//将属性集转换为Aura属性集并且可以访问这些属性，使用属性访问器来设置
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);

	//将初始属性值 广播给订阅委托的代码，希望在属性发生变化时做出响应
	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth());
	OnManaChanged.Broadcast(AuraAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributeSet->GetMaxMana());
	
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	//将属性集转换为Aura属性集并且可以访问这些属性，使用属性访问器来设置
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributeSet->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributeSet->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributeSet->GetManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnManaChanged.Broadcast(Data.NewValue);
			}
		);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributeSet->GetMaxManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxManaChanged.Broadcast(Data.NewValue);
			}
		);

	//AddLambda 绑定匿名函数
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->EffectAssetTags.AddLambda(
		[this](const FGameplayTagContainer& AssetTags)//中括号添加this是为了保证内部能够获取类的对象
	{
		
		/*
		* for (const FGameplayTag& Tag : AssetTags)
		{
			//将tag广播给Widget Controller
			const FString Msg = FString::Printf(TEXT("GE Tag in Widget Controller: %s"), *Tag.ToString()); //获取Asset Tag
			GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan, Msg); //打印到屏幕上 -1 不会被覆盖
		}
		*/

		for (const FGameplayTag& Tag : AssetTags)
		{

			//对标签进行检测，如果不是信息标签，将无法进行广播
			FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
			// "A.1".MatchesTag("A") will return True, "A".MatchesTag("A.1") will return False
			if (Tag.MatchesTag(MessageTag))
			{
				FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
				if (Row)
				{
					MessageWidgetRowDelegate.Broadcast(*Row); //前面加*取消指针引用
				}
			}
		}
		

	}
	);

}


