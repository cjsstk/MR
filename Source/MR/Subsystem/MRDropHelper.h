// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MRDataTable.h"
#include "MRDropHelper.generated.h"

USTRUCT(BlueprintType)
struct MR_API FMRDropResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ItemId = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Count = 0;
};

/**
 * 드롭 테이블 기반 가중치 랜덤 아이템 결정 유틸리티.
 * 박리, 포획 보상, 퀘스트 보상 등 모든 드롭 계산에 재사용.
 */
UCLASS()
class MR_API UMRDropHelper : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * FDropTableRow의 Entries에서 가중치 랜덤으로 아이템 1개를 결정한다.
	 * 박리 1회당 이 함수를 1회 호출한다.
	 * Entries가 비어있으면 빈 FMRDropResult 반환.
	 */
	static FMRDropResult RollOnce(const FDropTableRow& DropTable);
};
