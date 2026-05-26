// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MRMonster.h"

const FName AMRAIController::BBKey_TargetActor  = TEXT("TargetActor");
const FName AMRAIController::BBKey_HomeLocation = TEXT("HomeLocation");
const FName AMRAIController::BBKey_IsFlying     = TEXT("IsFlying");

AMRAIController::AMRAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AMRAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!BehaviorTree)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRAIController] BehaviorTree is not set. Assign it in BP_AIController_Monster."));
		return;
	}

	UBlackboardComponent* BB = nullptr;
	if (UseBlackboard(BehaviorTree->BlackboardAsset, BB))
	{
		// 스폰 위치를 홈 포지션으로 저장 (배회·복귀 지점)
		BB->SetValueAsVector(BBKey_HomeLocation, InPawn->GetActorLocation());
		// 초기 비행 상태는 항상 false로 초기화
		BB->SetValueAsBool(BBKey_IsFlying, false);
	}

	RunBehaviorTree(BehaviorTree);
}

void AMRAIController::OnUnPossess()
{
	if (UBrainComponent* Brain = GetBrainComponent())
	{
		Brain->StopLogic(TEXT("UnPossess"));
	}

	Super::OnUnPossess();
}

AMRMonster* AMRAIController::GetMonster() const
{
	return Cast<AMRMonster>(GetPawn());
}
