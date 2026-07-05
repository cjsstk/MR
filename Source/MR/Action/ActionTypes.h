// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionTypes.generated.h"

/**
 * 게임에서 사용되는 액션 타입.
 * 새 액션 추가 시 여기에 열거값을 추가하고 Action.h에 DECLARE_ACTION_PAYLOAD를 추가한다.
 */
UENUM(BlueprintType)
enum class EActionType : uint8
{
	SetHealth,
	SetStamina,

	// ─── 인벤토리 ──────────────────────────────────────────────────────────────
	AddInventoryItem,
	RemoveInventoryItem,
	UseInventoryItem,
	SyncInventorySlots,
	ShowCarveResult,

	// ─── Store 알림 전용 (실제로 Dispatch되지 않고 Store::Notify()에서만 쓰임) ────────
	// 여러 액션이 하나의 알림으로 모이는 경우(예: Add/Remove/UseInventoryItem이 전부
	// 슬롯 변경 알림 하나로 모임), 그 알림 자체를 위한 전용 타입.
	InventorySlotsChanged,
	InventoryCarveResultChanged,
};

/**
 * 액션 페이로드 struct -> EActionType 매핑.
 * DECLARE_ACTION_PAYLOAD 매크로가 각 페이로드 정의부에서 템플릿 특수화로 채운다.
 * UActionDispatcher::Dispatch/BindAction이 페이로드 타입만으로 EActionType을 추론하는 데 쓰인다.
 */
template<typename TPayload>
struct TActionTypeOf;

/**
 * ActionName(EActionType의 열거값 이름)만으로 페이로드 struct(FAction_ActionName)를 선언하고
 * 대응하는 EActionType과 연결하는 매크로. 새 액션 추가 시 이 매크로 한 번으로
 * 페이로드 타입 선언 + 라우팅 등록이 끝난다.
 * 이름 있는 팩토리 함수는 따로 만들 필요 없이 MakeAction<TPayload>(...)으로 바로 생성한다
 * (필드 선언 순서대로 애그리게잇 초기화됨).
 * 필드 목록에 콤마가 들어가면 매크로 인자 분리가 깨지므로, 한 줄에 필드 하나씩 세미콜론으로 끝낸다.
 *
 * 사용 예:
 *   DECLARE_ACTION_PAYLOAD(SetHealth,
 *       float Current = 0.f;
 *       float Max = 100.f;
 *   );
 *   ...
 *   Dispatcher->Dispatch(MakeAction<FAction_SetHealth>(80.f, 100.f));
 */
#define DECLARE_ACTION_PAYLOAD(ActionName, ...) \
	struct FAction_##ActionName \
	{ \
		__VA_ARGS__ \
	}; \
	template<> \
	struct TActionTypeOf<FAction_##ActionName> \
	{ \
		static constexpr EActionType Value = EActionType::ActionName; \
	};

/**
 * Store::Handle*() / 위젯 핸들러를 ActionName만으로 선언하는 매크로.
 * FAction_ActionName 타입명을 매번 찾아 쓸 필요 없이, DECLARE_ACTION_PAYLOAD에 쓴
 * 이름 그대로 핸들러를 선언할 수 있다. 실제 정의(.cpp)는 그대로 FAction_ActionName을
 * 파라미터 타입으로 써서 구현한다 — 여기서 감춘 이름을 알아야 하니, 선언부 바로 위에서
 * 확인하면 된다.
 *
 * 사용 예:
 *   // .h
 *   DECLARE_ACTION_HANDLER(HandleHealthChanged, SetHealth);
 *   // .cpp
 *   void UMyWidget::HandleHealthChanged(const FAction_SetHealth& Action) { ... }
 */
#define DECLARE_ACTION_HANDLER(HandlerName, ActionName) \
	void HandlerName(const FAction_##ActionName& Action)
