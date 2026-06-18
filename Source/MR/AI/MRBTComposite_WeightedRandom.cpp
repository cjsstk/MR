// Fill out your copyright notice in the Description page of Project Settings.

#include "MRBTComposite_WeightedRandom.h"
#include "BehaviorTree/BehaviorTreeTypes.h"

UMRBTComposite_WeightedRandom::UMRBTComposite_WeightedRandom()
{
	NodeName = TEXT("Weighted Random");
}

int32 UMRBTComposite_WeightedRandom::GetNextChildHandler(
	FBehaviorTreeSearchData& SearchData,
	int32 PrevChild,
	EBTNodeResult::Type LastResult) const
{
	uint32& TriedMask = *GetNodeMemory<uint32>(SearchData);
	const int32 NumChildren = GetChildrenNum();

	if (PrevChild == BTSpecialChild::NotInitialized)
	{
		// 첫 호출 — 새 사이클 시작, 마스크 초기화
		TriedMask = 0;
	}
	else if (LastResult != EBTNodeResult::Failed)
	{
		// 이전 자식이 성공(또는 Aborted) — 결과를 부모에게 그대로 전달
		return BTSpecialChild::ReturnToParent;
	}
	else
	{
		// 이전 자식이 실패 — 해당 인덱스를 시도 완료로 표시 (최대 32자식 지원)
		if (PrevChild >= 0 && PrevChild < 32)
		{
			TriedMask |= (1u << PrevChild);
		}
	}

	// 남은 자식(아직 시도하지 않은 자식)의 가중치 합산
	float TotalWeight = 0.f;
	for (int32 i = 0; i < NumChildren; ++i)
	{
		if (!(TriedMask & (1u << i)))
		{
			const float W = ChildWeights.IsValidIndex(i) ? ChildWeights[i] : 1.f;
			TotalWeight += W;
		}
	}

	if (TotalWeight <= 0.f)
	{
		// 시도할 자식이 없으면 Composite 실패로 반환
		return BTSpecialChild::ReturnToParent;
	}

	// 가중치 기반 랜덤 선택
	float Rand = FMath::FRandRange(0.f, TotalWeight);
	for (int32 i = 0; i < NumChildren; ++i)
	{
		if (!(TriedMask & (1u << i)))
		{
			const float W = ChildWeights.IsValidIndex(i) ? ChildWeights[i] : 1.f;
			Rand -= W;
			if (Rand <= 0.f)
			{
				return i;
			}
		}
	}

	// 부동소수점 오차로 선택이 안 된 경우 안전 폴백
	return BTSpecialChild::ReturnToParent;
}

uint16 UMRBTComposite_WeightedRandom::GetInstanceMemorySize() const
{
	return sizeof(uint32);
}

void UMRBTComposite_WeightedRandom::InitializeMemory(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
	*(uint32*)NodeMemory = 0;
}

void UMRBTComposite_WeightedRandom::CleanupMemory(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	// uint32 POD 타입 — 별도 정리 불필요
	Super::CleanupMemory(OwnerComp, NodeMemory, CleanupType);
}

FString UMRBTComposite_WeightedRandom::GetStaticDescription() const
{
	if (ChildWeights.IsEmpty())
	{
		return TEXT("Weights: (all equal)");
	}

	FString Desc = TEXT("Weights: [");
	for (int32 i = 0; i < ChildWeights.Num(); ++i)
	{
		if (i > 0)
		{
			Desc += TEXT(", ");
		}
		Desc += FString::Printf(TEXT("%.2f"), ChildWeights[i]);
	}
	Desc += TEXT("]");
	return Desc;
}
