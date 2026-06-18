// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MRAIController.generated.h"

class AMRMonster;
class UBehaviorTree;

/**
 * 몬스터 전용 AIController.
 * BehaviorTree를 실행하고 Blackboard 키를 초기화한다.
 * BP 서브클래스(BP_AIController_Monster)에서 BehaviorTree 에셋을 할당한다.
 */
UCLASS()
class MR_API AMRAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMRAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Blackboard TargetActor 키 이름. BTTask/BTService에서 이 상수를 사용한다. */
	static const FName BBKey_TargetActor;

	/** Blackboard HomeLocation 키 이름. 배회 복귀 지점. */
	static const FName BBKey_HomeLocation;

	/** Blackboard IsFlying 키 이름. 비행 상태 동기화. */
	static const FName BBKey_IsFlying;

	/** Blackboard NestLocation 키. 둥지 위치 — 스포너에서 초기화. */
	static const FName BBKey_NestLocation;

	/** BP 서브클래스에서 할당. null이면 BT를 실행하지 않는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	AMRMonster* GetMonster() const;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
};
