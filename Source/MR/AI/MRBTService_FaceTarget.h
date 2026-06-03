// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "MRBTService_FaceTarget.generated.h"

/**
 * Blackboard의 TargetActor 방향으로 Pawn을 부드럽게 회전시킨다.
 * Yaw만 보간하며, Pitch/Roll은 유지한다.
 */
UCLASS()
class MR_API UMRBTService_FaceTarget : public UBTService
{
	GENERATED_BODY()

public:
	UMRBTService_FaceTarget();

	/** 회전 보간 속도. 값이 클수록 빠르게 타겟을 향한다. */
	UPROPERTY(EditAnywhere, Category = "FaceTarget", meta = (ClampMin = "0.0"))
	float InterpSpeed = 5.f;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
