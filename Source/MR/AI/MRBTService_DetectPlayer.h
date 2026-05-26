// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "MRBTService_DetectPlayer.generated.h"

/**
 * 일정 반경 내 플레이어를 감지해 Blackboard의 TargetActor 키를 업데이트한다.
 * DetectRadius 이내 진입 시 타겟 설정, LoseTargetRadius 밖으로 이탈 시 타겟 해제.
 * DetectRadius <= 거리 <= LoseTargetRadius 구간에서는 기존 상태를 유지한다 (히스테리시스).
 */
UCLASS()
class MR_API UMRBTService_DetectPlayer : public UBTService
{
	GENERATED_BODY()

public:
	UMRBTService_DetectPlayer();

	/** 플레이어 최초 감지 반경 */
	UPROPERTY(EditAnywhere, Category = "Detection", meta = (ClampMin = "0.0"))
	float DetectRadius = 1500.f;

	/** 기존 타겟을 잃는 반경. DetectRadius보다 크게 설정해 경계 진동 방지. */
	UPROPERTY(EditAnywhere, Category = "Detection", meta = (ClampMin = "0.0"))
	float LoseTargetRadius = 2000.f;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
