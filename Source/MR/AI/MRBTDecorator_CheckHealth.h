// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "MRBTDecorator_CheckHealth.generated.h"

/**
 * 몬스터의 HP 비율을 기준으로 조건을 판단하는 BT Decorator.
 * bCheckBelow=true이면 HP% <= HealthThreshold, false이면 HP% > HealthThreshold.
 */
UCLASS()
class MR_API UMRBTDecorator_CheckHealth : public UBTDecorator
{
	GENERATED_BODY()

public:
	UMRBTDecorator_CheckHealth();

	/** HP 비율 임계값 (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthThreshold = 0.5f;

	/** true: HP% <= Threshold일 때 통과 (HP 낮을 때), false: HP% > Threshold일 때 통과 */
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bCheckBelow = true;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
