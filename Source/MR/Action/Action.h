// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MRStore/ActionDispatcherBase.h"
#include "ActionTypes.h"
// 아래 ActionList.inl의 페이로드가 참조하는 타입 정의 — .generated.h 앞, 페이로드 생성 전에 필요하다.
#include "Component/MRInventoryComponent.h"	// FMRInventorySlot
#include "Subsystem/MRDropHelper.h"			// FMRDropResult

// ─── 액션 페이로드 ───────────────────────────────────────────────────────────
// 페이로드 struct(FAction_Xxx)와 TActionTypeOf 매핑은 ActionList.inl 하나에서
// EActionType 열거값과 함께 자동 생성된다 — 새 액션은 ActionList.inl에 ACTION(...)
// 항목 하나만 추가하면 되고, enum과 페이로드가 서로 어긋날 수 없다.
// 생성은 MakeAction<FAction_Xxx>(필드값...)으로, 필드 선언 순서대로 애그리게잇 초기화된다.
// 예: Dispatcher->Dispatch(MakeAction<FAction_SetHealth>(80.f, 100.f));
//
// 페이로드는 일반 struct(비-USTRUCT)이므로 UHT 리플렉션이 필요 없다. 다만 UActionDispatcher가
// UCLASS라 .generated.h가 필요하고, UHT는 .generated.h를 마지막 include로 강제하므로
// 페이로드 생성은 반드시 .generated.h include '앞'에서 한다.
// (TArray<FMRInventorySlot> / FMRDropResult 등 페이로드가 참조하는 타입이 위에서
//  include 되어 있어야 하므로 enum(ActionTypes.h)과 달리 페이로드 생성은 여기서 한다.)

#define ACTION(Name, ...) DECLARE_ACTION_PAYLOAD(Name, __VA_ARGS__)
#include "ActionList.inl"
#undef ACTION

#include "Action.generated.h"

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
