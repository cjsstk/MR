// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MRStore/ActionDispatcherBase.h"
#include "ActionTypes.h"
#include "Component/MRInventoryComponent.h"
#include "Subsystem/MRDropHelper.h"
#include "Action.generated.h"

// ─── 액션 페이로드 ───────────────────────────────────────────────────────────
// 새 액션 추가 시: DECLARE_ACTION_PAYLOAD 한 번으로 끝난다.
// 생성은 MakeAction<FAction_Xxx>(필드값...)으로, 순서대로 애그리게잇 초기화된다.
// 예: Dispatcher->Dispatch(MakeAction<FAction_SetHealth>(80.f, 100.f));

DECLARE_ACTION_PAYLOAD(SetHealth,
	float Current = 0.f;
	float Max = 100.f;
);

DECLARE_ACTION_PAYLOAD(SetStamina,
	float Current = 0.f;
	float Max = 100.f;
);

// ─── 인벤토리 ──────────────────────────────────────────────────────────────

DECLARE_ACTION_PAYLOAD(AddInventoryItem,
	int32 ItemId = 0;
	int32 Count = 0;
	int32 SlotIndex = INDEX_NONE;
);

DECLARE_ACTION_PAYLOAD(RemoveInventoryItem,
	int32 ItemId = 0;
	int32 Count = 0;
);

DECLARE_ACTION_PAYLOAD(UseInventoryItem,
	int32 SlotIndex = INDEX_NONE;
);

DECLARE_ACTION_PAYLOAD(SyncInventorySlots);

DECLARE_ACTION_PAYLOAD(ShowCarveResult,
	int32 ItemId = 0;
	int32 Count = 0;
);

// InventoryStore 알림 전용 (Add/Remove/UseInventoryItem/SyncInventorySlots가 전부 이 알림 하나로 모인다)
DECLARE_ACTION_PAYLOAD(InventorySlotsChanged,
	TArray<FMRInventorySlot> Slots;
);

DECLARE_ACTION_PAYLOAD(InventoryCarveResultChanged,
	TArray<FMRDropResult> Items;
);

/**
 * UActionDispatcher
 *
 * 액션을 받아 해당 타입에 등록된 Store 핸들러들을 호출하는 중앙 디스패처.
 * GameInstanceSubsystem으로 동작하며 게임 전체에서 접근 가능하다.
 * 실제 등록/호출 로직은 재사용 가능한 TActionDispatcher(MRStore)에 위임한다.
 *
 * 액션마다 페이로드 struct 타입이 다르므로 Bind/Dispatch는 템플릿으로 동작하며,
 * DECLARE_ACTION_PAYLOAD로 등록해둔 EActionType을 페이로드 타입에서 자동으로 추론한다.
 * (템플릿 함수는 UFUNCTION/BlueprintCallable로 노출할 수 없다 — 모든 호출부가 C++이라 문제없음)
 *
 * 흐름:
 *   Caller → Dispatch(Payload) → Store::Handle*() → Store의 필드별 델리게이트 Broadcast → Widget
 *
 * 접근 방법:
 *   GetActionDispatcher(WorldContextObject)   // Sugar.h
 *   GetGameInstance()->GetSubsystem<UActionDispatcher>()
 */
UCLASS()
class MR_API UActionDispatcher : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 액션을 디스패치한다. 페이로드 타입에 대응하는 EActionType으로 바인딩된 모든 핸들러가 호출된다. */
	template<typename TPayload>
	void Dispatch(const TPayload& Payload)
	{
		Impl.Dispatch(TActionTypeOf<TPayload>::Value, Payload);
	}

	/**
	 * 특정 페이로드 타입에 핸들러를 바인딩한다. MakeActionDelegate()로 만든 델리게이트를 넘기면 된다.
	 * 반환된 FDelegateHandle로 UnbindAction 호출 시 해제한다.
	 */
	template<typename TPayload>
	FDelegateHandle BindAction(TDelegate<void(const TPayload&)>&& Delegate)
	{
		return Impl.BindAction(TActionTypeOf<TPayload>::Value, MoveTemp(Delegate));
	}

	/** 특정 페이로드 타입에서 핸들러를 해제한다. */
	template<typename TPayload>
	void UnbindAction(FDelegateHandle Handle)
	{
		Impl.UnbindAction(TActionTypeOf<TPayload>::Value, Handle);
	}

	/**
	 * 특정 오너(BindAction에 넘긴 CreateUObject의 this)가 등록한 모든 핸들러를 한 번에 해제한다.
	 * Store가 액션 타입별로 FDelegateHandle을 직접 들고 있을 필요 없이,
	 * UnregisterActionHandlers에서 이 한 줄로 정리할 수 있게 해준다.
	 */
	void UnbindAll(const void* Owner)
	{
		Impl.UnbindAllForOwner(Owner);
	}

private:
	TActionDispatcher<EActionType> Impl;
};
