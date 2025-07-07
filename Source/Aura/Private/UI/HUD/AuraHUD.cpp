// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"

UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);

		OverlayWidgetController->BindCallbacksToDependencies();

	}
	return OverlayWidgetController;
}

void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("OverlayWidgetClass 已初始化，请填写BP_AuraHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("OverlayWidgetControllerClass 已初始化，请填写BP_AuraHUD"));

	//1.创建覆盖层小部件
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UAuraUserWidget>(Widget);

	//2.创建小部件控制器
	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	//获取覆盖层小部件控制器指针
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);
	//将参数赋值给小部件控制器
	OverlayWidget->SetWidgetController(WidgetController);

	//3.将覆盖层小部件控制器添加到覆盖层小部件
	//OverlayWidget->AddWidgetController(OverlayWidgetController);

	//4.广播初始值
	WidgetController->BroadcastInitivalValues();
	//将覆盖层小部件打印到屏幕
	Widget->AddToViewport();
}
