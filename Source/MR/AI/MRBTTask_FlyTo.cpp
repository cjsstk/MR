// Fill out your copyright notice in the Description page of Project Settings.

#include "MRBTTask_FlyTo.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Pawn.h"
#include "MRAIController.h"

UMRBTTask_FlyTo::UMRBTTask_FlyTo()
{
	NodeName = TEXT("Fly To");
	bNotifyTick = true;

	// Actor 키(플레이어 추적)와 Vector 키(둥지 등 고정 위치) 모두 허용
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMRBTTask_FlyTo, TargetKey), AActor::StaticClass());
	TargetKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UMRBTTask_FlyTo, TargetKey));
}

void UMRBTTask_FlyTo::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BBAsset);
	}
}

uint16 UMRBTTask_FlyTo::GetInstanceMemorySize() const
{
	return sizeof(FMRFlyToTaskMemory);
}

EBTNodeResult::Type UMRBTTask_FlyTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	// Actor 키와 Vector 키 모두 처리
	FVector GoalLocation = FVector::ZeroVector;
	if (TargetKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
		if (!TargetActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[FlyTo] TargetKey(%s) Actor is null"), *TargetKey.SelectedKeyName.ToString());
			return EBTNodeResult::Failed;
		}
		GoalLocation = TargetActor->GetActorLocation();
	}
	else if (TargetKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		GoalLocation = BB->GetValueAsVector(TargetKey.SelectedKeyName);
		if (GoalLocation == FAISystem::InvalidLocation)
		{
			UE_LOG(LogTemp, Warning, TEXT("[FlyTo] TargetKey(%s) Vector is invalid"), *TargetKey.SelectedKeyName.ToString());
			return EBTNodeResult::Failed;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[FlyTo] TargetKey type not supported"));
		return EBTNodeResult::Failed;
	}

	// 비행 고도 오프셋 적용
	GoalLocation.Z += FlightHeight;

	FMRFlyToTaskMemory* Memory = reinterpret_cast<FMRFlyToTaskMemory*>(NodeMemory);
	Memory->GoalLocation = GoalLocation;

	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(GoalLocation);
	MoveReq.SetAcceptanceRadius(AcceptableRadius);
	MoveReq.SetUsePathfinding(false);
	MoveReq.SetAllowPartialPath(false);
	MoveReq.SetProjectGoalLocation(false);

	const FPathFollowingRequestResult Result = AIC->MoveTo(MoveReq);

	UE_LOG(LogTemp, Log, TEXT("[FlyTo] Key=%s Goal=%s Height=%.0f Radius=%.0f Code=%d"),
		*TargetKey.SelectedKeyName.ToString(), *GoalLocation.ToString(), FlightHeight, AcceptableRadius, (int32)Result.Code);

	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}
	else if (Result.Code == EPathFollowingRequestResult::RequestSuccessful)
	{
		Memory->MoveRequestID = Result.MoveId;
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UMRBTTask_FlyTo::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FMRFlyToTaskMemory* Memory = reinterpret_cast<FMRFlyToTaskMemory*>(NodeMemory);
	const float Dist = FVector::Dist(Pawn->GetActorLocation(), Memory->GoalLocation);

	if (Dist <= AcceptableRadius)
	{
		AIC->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	UPathFollowingComponent* PathComp = AIC->GetPathFollowingComponent();
	if (PathComp && PathComp->GetStatus() == EPathFollowingStatus::Idle)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FlyTo] PathFollowing Idle but Dist=%.0f > Radius=%.0f. Failed."),
			Dist, AcceptableRadius);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

EBTNodeResult::Type UMRBTTask_FlyTo::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (AIC)
	{
		AIC->StopMovement();
	}

	UE_LOG(LogTemp, Log, TEXT("[FlyTo] Task Aborted."));
	return EBTNodeResult::Aborted;
}

FString UMRBTTask_FlyTo::GetStaticDescription() const
{
	return FString::Printf(TEXT("Fly To: %s\nHeight: %.0f  Radius: %.0f"),
		*TargetKey.SelectedKeyName.ToString(), FlightHeight, AcceptableRadius);
}
