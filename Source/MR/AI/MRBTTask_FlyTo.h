// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "MRBTTask_FlyTo.generated.h"

struct FMRFlyToTaskMemory
{
	FAIRequestID MoveRequestID;
	FVector GoalLocation;
};

/**
 * MOVE_Flying 상태의 몬스터를 지정한 BB 키 위치로 NavMesh 없이 직선 비행시키는 BT Task.
 * TargetKey에 Actor 키(플레이어 추적)와 Vector 키(둥지 등 고정 지점) 모두 사용 가능.
 * 타겟 위치에 FlightHeight 오프셋을 더해 공중에서 접근하고,
 * AcceptableRadius 이내 도달 시 Succeeded, PathFollowing이 멈추면 Failed.
 */
UCLASS()
class MR_API UMRBTTask_FlyTo : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMRBTTask_FlyTo();

	/** 이동 목표. Actor 키(TargetActor 등)와 Vector 키(HomeLocation 등) 모두 지원. */
	UPROPERTY(EditAnywhere, Category = "FlyTo")
	FBlackboardKeySelector TargetKey;

	/** 도착 판정 반경 */
	UPROPERTY(EditAnywhere, Category = "FlyTo", meta = (ClampMin = "50.0"))
	float AcceptableRadius = 300.f;

	/** 타겟 위치 기준 비행 고도 오프셋 (Z) */
	UPROPERTY(EditAnywhere, Category = "FlyTo", meta = (ClampMin = "0.0"))
	float FlightHeight = 600.f;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};
