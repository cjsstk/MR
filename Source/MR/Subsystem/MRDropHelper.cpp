// Fill out your copyright notice in the Description page of Project Settings.

#include "MRDropHelper.h"

FMRDropResult UMRDropHelper::RollOnce(const FDropTableRow& DropTable)
{
	if (DropTable.Entries.IsEmpty())
	{
		return FMRDropResult{};
	}

	// 전체 가중치 합산
	int32 TotalWeight = 0;
	for (const FDropEntry& Entry : DropTable.Entries)
	{
		TotalWeight += FMath::Max(Entry.Weight, 1);
	}

	// 가중치 범위 [1, TotalWeight] 에서 랜덤 값 선택
	const int32 Roll = FMath::RandRange(1, TotalWeight);

	// 누적 가중치로 당첨 항목 결정
	int32 Accumulated = 0;
	for (const FDropEntry& Entry : DropTable.Entries)
	{
		Accumulated += FMath::Max(Entry.Weight, 1);
		if (Roll <= Accumulated)
		{
			FMRDropResult Result;
			Result.ItemId = Entry.ItemId;
			Result.Count  = FMath::RandRange(
				FMath::Max(Entry.MinCount, 1),
				FMath::Max(Entry.MaxCount, Entry.MinCount));

			UE_LOG(LogTemp, Log, TEXT("UMRDropHelper::RollOnce: ItemId=%d x%d (Roll=%d, Total=%d)"),
				Result.ItemId, Result.Count, Roll, TotalWeight);

			return Result;
		}
	}

	// 부동소수 오차 방어 — 마지막 항목 반환
	const FDropEntry& Last = DropTable.Entries.Last();
	FMRDropResult Result;
	Result.ItemId = Last.ItemId;
	Result.Count  = FMath::RandRange(FMath::Max(Last.MinCount, 1), FMath::Max(Last.MaxCount, Last.MinCount));
	return Result;
}
