// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"

#include "InputActionValue.h" //输入映射Value值的头文件
#include "EnhancedInputComponent.h" //增强映射的头文件
#include "EnhancedInputSubsystems.h" //增强子系统的头文件
#include "EnhancedInputLibrary.h"
#include "interaction/EnemyInterface.h"
#include "Input/AuraInputComponent.h"
#include "Components/SplineComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"  

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;//是否将数据传送服务器更新

	LastActor = nullptr;
	ThisActor = nullptr;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	//鼠标位置追踪是否悬停在敌人身上
	CursorTrace();

	//自动寻路
	AutoRun();
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		//找到距离样条最近的位置
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		//获取这个位置在样条上的方向
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}

}

void AAuraPlayerController::CursorTrace()
{

	//FHitResult 结构来存储碰撞检测后的相关数据, 在此简单记录一下该结构中各个成员的含义.
	FHitResult CursorHit;

	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);//获取可视的鼠标命中结果
	if (!CursorHit.bBlockingHit) return;//如果未命中直接返回
	
	LastActor = ThisActor;
	ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());

	if (LastActor == nullptr)
	{
		if (ThisActor != nullptr)
		{
			ThisActor->HighlightActor();
		}
	}
	else
	{
		if (ThisActor == nullptr)
		{
			LastActor->UnHighlightActor();
		}
		else
		{
			if (ThisActor != LastActor)
			{
				LastActor->UnHighlightActor();
				ThisActor->HighlightActor();
			}
		}
	}
	
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		bTargeting = ThisActor ? true : false;
		bAutoRunning = false;
		FollowTime = 0.f; //重置统计的时间
	}
	

}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
		return;
	}

	if (bTargeting)
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}

	}
	else
	{
		APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints(); //清除样条内现有的点

				const TArray<FVector>& PathPoints = NavPath->PathPoints;  // 修改这里
				for (const FVector& PointLoc : PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World); //将新的位置添加到样条曲线中
					//DrawDebugSphere(GetWorld(), PointLoc, 8.f, 8, FColor::Orange, false, 5.f); //点击后debug调试
				}

				//自动寻路将最终目的地设置为导航的终点，方便停止导航
				CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
				bAutoRunning = true; //设置当前正常自动寻路状态，将在tick中更新位置
			}
			
			FollowTime = 0.f;
			bTargeting = false;
		}
	}

}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{

	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;
	}

	if (bTargeting)
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		
	}
	else 
	{
		FollowTime += GetWorld()->GetDeltaSeconds(); //统计悬停时间来判断是否为点击
		FHitResult Hit;
		if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
		{
			CachedDestination = Hit.ImpactPoint;
		}

		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
		
	}

}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent =  Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}


void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	//从本地角色身上获取到它的子系统
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		//将自定义的操作映射上下文添加到子系统中
		Subsystem->AddMappingContext(AuraContext, 0);//可以存在多个操作映射，根据优先级触发
	}

	bShowMouseCursor = true;//游戏中是否显示鼠标光标
	DefaultMouseCursor = EMouseCursor::Default;//鼠标光标的样式

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);//将鼠标锁定在视口内
	InputModeData.SetHideCursorDuringCapture(false);//鼠标被捕获时是否隐藏
	SetInputMode(InputModeData);//设置给控制器
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);//获取到增强输入组件

	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
	

}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();//获取输入操作的2维向量值
	const FRotator Rotation = GetControlRotation(); //获取控制器旋转
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);//通过控制器的垂直朝向创建一个旋转值，忽略上下朝向和左右朝向

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);//获取世界坐标系下向前的值，-1到1
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);//获取世界坐标系下向右的值，-1到1

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

