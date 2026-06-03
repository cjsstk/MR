// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "MRBTDecorator_IsInRange.generated.h"

/**
 * Blackboard의 TargetActor와 Pawn 사이 거리가 AcceptableRadius 이하인지 판단한다.
 * 타겟이 없으면 false를 반환한다.
 */
UCLASS()
class MR_API UMRBTDecorator_IsInRange : public UBTDecorator
{
	GENERATED_BODY()

public:
	UMRBTDecorator_IsInRange();

	/** 공격 등 근접 행동을 허용할 최대 거리 */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0.0"))
	float AcceptableRadius = 200.f;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
