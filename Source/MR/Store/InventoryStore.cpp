// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryStore.h"
#include "Action/Action.h"
#include "Component/MRInventoryComponent.h"

void UInventoryStore::RegisterActionHandlers(UActionDispatcher* Dispatcher)
{
	// 슬롯 배열을 MaxSlots 크기로 초기화
	State.Slots.SetNum(UMRInventoryComponent::MaxSlots);

	Dispatcher->BindAction(MakeActionDelegate(this, &UInventoryStore::HandleAddInventoryItem));
	Dispatcher->BindAction(MakeActionDelegate(this, &UInventoryStore::HandleRemoveInventoryItem));
	Dispatcher->BindAction(MakeActionDelegate(this, &UInventoryStore::HandleUseInventoryItem));
	Dispatcher->BindAction(MakeActionDelegate(this, &UInventoryStore::HandleSyncInventorySlots));
	Dispatcher->BindAction(MakeActionDelegate(this, &UInventoryStore::HandleShowGatherResult));
}

void UInventoryStore::UnregisterActionHandlers(UActionDispatcher* Dispatcher)
{
	Dispatcher->UnbindAll(this);
}

void UInventoryStore::HandleAddInventoryItem(const FAction_AddInventoryItem& Action)
{
	if (!State.Slots.IsValidIndex(Action.SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryStore::HandleAddInventoryItem: 유효하지 않은 SlotIndex %d"), Action.SlotIndex);
		return;
	}

	State.Slots[Action.SlotIndex].ItemId = Action.ItemId;
	State.Slots[Action.SlotIndex].Count  = Action.Count;

	Notify(FAction_InventorySlotsChanged{ State.Slots });
}

void UInventoryStore::HandleRemoveInventoryItem(const FAction_RemoveInventoryItem& Action)
{
	int32 Remaining = Action.Count;

	for (FMRInventorySlot& Slot : State.Slots)
	{
		if (Remaining <= 0)
		{
			break;
		}

		if (Slot.ItemId == Action.ItemId && Slot.Count > 0)
		{
			const int32 Removing = FMath::Min(Remaining, Slot.Count);
			Slot.Count -= Removing;
			Remaining  -= Removing;

			if (Slot.Count <= 0)
			{
				Slot.ItemId = 0;
				Slot.Count  = 0;
			}
		}
	}

	Notify(FAction_InventorySlotsChanged{ State.Slots });
}

void UInventoryStore::HandleUseInventoryItem(const FAction_UseInventoryItem& Action)
{
	// 슬롯 상태(Count 감소)는 UseItem이 DispatchSlotUpdate → AddInventoryItem 경로로 이미 동기화했다.
	// 여기서 다시 Count를 건드리면 이중 감소가 발생하므로 알림만 발행한다.
	// (위젯에서 "아이템 사용" 이벤트를 AddItem과 별도로 구분하고 싶을 때 이 알림을 구독)
	Notify(FAction_InventorySlotsChanged{ State.Slots });
}

void UInventoryStore::HandleSyncInventorySlots(const FAction_SyncInventorySlots& Action)
{
	// 페이로드 없음 — UI 강제 리프레시만 수행
	Notify(FAction_InventorySlotsChanged{ State.Slots });
}

void UInventoryStore::HandleShowGatherResult(const FAction_ShowGatherResult& Action)
{
	FMRDropResult Result;
	Result.ItemId = Action.ItemId;
	Result.Count  = Action.Count;

	State.LastAcquiredItems.Add(Result);
	Notify(FAction_InventoryGatherResultChanged{ State.LastAcquiredItems });

	UE_LOG(LogTemp, Log, TEXT("UInventoryStore::HandleShowGatherResult: ItemId=%d x%d"),
		Result.ItemId, Result.Count);

	// 3초 후 자동 클리어 → 팝업 위젯이 사라진 뒤 상태 정리
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(GatherResultClearHandle, [this]()
		{
			State.LastAcquiredItems.Empty();
			Notify(FAction_InventoryGatherResultChanged{ State.LastAcquiredItems });
		}, 3.0f, false);
	}
}
