// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "MRBTComposite_WeightedRandom.generated.h"

/**
 * 가중치 기반 랜덤 자식 선택 Composite 노드.
 * Selector와 달리 자식 인덱스별 가중치에 비례해 무작위 순서로 실행한다.
 * 한 사이클 내 이미 시도한 자식은 건너뛰어 중복 실행을 방지한다.
 * 모든 자식이 실패하면 Composite 자체가 Failed를 반환한다.
 */
UCLASS()
class MR_API UMRBTComposite_WeightedRandom : public UBTCompositeNode
{
	GENERATED_BODY()

public:
	UMRBTComposite_WeightedRandom();

	/**
	 * 자식 인덱스별 선택 가중치. 값이 클수록 선택 확률이 높아진다.
	 * 배열이 자식 수보다 짧으면 나머지는 1.0 기본값으로 취급한다.
	 */
	UPROPERTY(EditAnywhere, Category = "WeightedRandom", meta = (ClampMin = "0.01"))
	TArray<float> ChildWeights;

	virtual int32 GetNextChildHandler(
		struct FBehaviorTreeSearchData& SearchData,
		int32 PrevChild,
		EBTNodeResult::Type LastResult) const override;

	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void InitializeMemory(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTMemoryInit::Type InitType) const override;

	virtual void CleanupMemory(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTMemoryClear::Type CleanupType) const override;
};
