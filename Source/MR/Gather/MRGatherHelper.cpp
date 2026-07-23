// Fill out your copyright notice in the Description page of Project Settings.

#include "Gather/MRGatherHelper.h"
#include "MRPlayerCharacter.h"
#include "Component/MRInventoryComponent.h"
#include "Subsystem/CMSSubsystem.h"
#include "Action/Action.h"
#include "Engine/GameInstance.h"

FMRDropResult UMRGatherHelper::GrantDropToPlayer(AMRPlayerCharacter* Player, FDropTableId DropTableId)
{
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRGatherHelper] GrantDropToPlayer 실패: Player가 nullptr"));
		return FMRDropResult();
	}

	UGameInstance* GI = Player->GetGameInstance();
	if (!GI)
	{
		return FMRDropResult();
	}

	// CMS에서 드롭 테이블 조회
	UCMSSubsystem* CMS = GI->GetSubsystem<UCMSSubsystem>();
	if (!CMS)
	{
		return FMRDropResult();
	}

	const FDropTableRow* DropTable = CMS->GetDropTableRow(DropTableId);
	if (!DropTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRGatherHelper] DropTableId=%d 테이블 없음"), DropTableId.Value);
		return FMRDropResult();
	}

	// 가중치 랜덤으로 아이템 결정
	const FMRDropResult Result = UMRDropHelper::RollOnce(*DropTable);
	if (Result.ItemId == 0 || Result.Count <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRGatherHelper] RollOnce 결과 없음 (DropTableId=%d)"), DropTableId.Value);
		return FMRDropResult();
	}

	// 플레이어 인벤토리에 아이템 지급
	if (UMRInventoryComponent* Inventory = Player->GetInventoryComponent())
	{
		Inventory->AddItem(FItemId(Result.ItemId), Result.Count);
	}

	// 채집 결과 팝업 표시를 위한 액션 디스패치
	if (UActionDispatcher* Dispatcher = GI->GetSubsystem<UActionDispatcher>())
	{
		Dispatcher->Dispatch(MakeAction<FAction_ShowGatherResult>(Result.ItemId, Result.Count));
	}

	UE_LOG(LogTemp, Log, TEXT("[MRGatherHelper] 채집: ItemId=%d x%d (DropTableId=%d)"),
		Result.ItemId, Result.Count, DropTableId.Value);

	return Result;
}
