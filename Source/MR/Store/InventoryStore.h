// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRStoreBase.h"
#include "Component/MRInventoryComponent.h"
#include "Subsystem/MRDropHelper.h"
#include "InventoryStore.generated.h"

class UActionDispatcher;
struct FAction_AddInventoryItem;
struct FAction_RemoveInventoryItem;
struct FAction_UseInventoryItem;
struct FAction_SyncInventorySlots;
struct FAction_ShowCarveResult;
struct FAction_InventorySlotsChanged;
struct FAction_InventoryCarveResultChanged;

USTRUCT(BlueprintType)
struct MR_API FInventoryState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FMRInventorySlot> Slots;

	/** 가장 최근 박리/획득 결과. 팝업 표시 후 자동 클리어. */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMRDropResult> LastAcquiredItems;
};

/**
 * UInventoryStore
 *
 * 플레이어 인벤토리 슬롯 상태와 최근 획득 결과를 보유하는 Store.
 * MRInventoryComponent가 ActionDispatcher를 통해 상태를 업데이트하고,
 * 인벤토리 UI 위젯은 이 Store를 구독해 표시를 갱신한다.
 *
 * 접근 방법:
 *   GetInventoryStore(this)   // Sugar.h
 */
UCLASS()
class MR_API UInventoryStore : public UMRStoreBase
{
	GENERATED_BODY()

public:
	virtual void RegisterActionHandlers(UActionDispatcher* Dispatcher) override;
	virtual void UnregisterActionHandlers(UActionDispatcher* Dispatcher) override;

	const FInventoryState& GetState() const { return State; }

private:
	DECLARE_ACTION_HANDLER(HandleAddInventoryItem, AddInventoryItem);
	DECLARE_ACTION_HANDLER(HandleRemoveInventoryItem, RemoveInventoryItem);
	DECLARE_ACTION_HANDLER(HandleUseInventoryItem, UseInventoryItem);
	DECLARE_ACTION_HANDLER(HandleSyncInventorySlots, SyncInventorySlots);
	DECLARE_ACTION_HANDLER(HandleShowCarveResult, ShowCarveResult);

	FInventoryState State;

	FTimerHandle CarveResultClearHandle;
};
