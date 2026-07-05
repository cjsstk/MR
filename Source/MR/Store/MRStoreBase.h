// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRStore/StoreBase.h"
#include "MRStore/ActionDispatcherBase.h"
#include "Action/ActionTypes.h"
#include <type_traits>
#include "MRStoreBase.generated.h"

/**
 * UMRStoreBase
 *
 * 이 프로젝트 전용 Store 베이스. 재사용 모듈인 UStoreBase는 EActionType을 몰라야 해서,
 * EActionType 기반 Subscribe/Notify는 여기(MR 모듈)에 둔다.
 *
 * 페이로드 타입(DECLARE_ACTION_PAYLOAD로 선언한 FAction_Xxx)에서 TActionTypeOf로
 * EActionType을 자동 추론해 라우팅한다 — 액션 핸들러 등록과 완전히 같은 방식이라
 * enum 값이라는 안정적인 키를 쓰고, 정적 변수 주소 같은 걸 쓸 필요가 없다.
 *
 * 여러 액션이 알림 하나로 모이는 경우(예: InventoryStore의 슬롯 변경)는, 그 알림 전용
 * EActionType/FAction_Xxx를 하나 선언해서 쓴다 — 실제로 Dispatch되지 않고
 * Notify() 전용으로만 쓰여도 된다.
 */
UCLASS(Abstract)
class MR_API UMRStoreBase : public UStoreBase
{
	GENERATED_BODY()

public:
	/**
	 * 이 Store가 Type 액션으로 Notify()를 호출할 때 불릴 핸들러를 구독한다.
	 * 어떤 액션을 구독하는지 호출부에 명시적으로 남기기 위해 Type을 직접 지정한다.
	 * Handler의 페이로드 타입이 Type과 다르면 컴파일 에러로 잡힌다.
	 *
	 * 사용 예: Store->Subscribe<EActionType::SetHealth>(this, &UMyWidget::HandleHealthChanged);
	 */
	template<auto Type, typename TOwner, typename TPayload>
	void Subscribe(TOwner* Owner, void (TOwner::* Handler)(const TPayload&))
	{
		static_assert(std::is_same_v<decltype(Type), EActionType>, "Subscribe<Type>: Type은 EActionType 값이어야 한다.");
		static_assert(TActionTypeOf<TPayload>::Value == Type, "Subscribe<Type>: Handler의 페이로드 타입이 Type과 일치하지 않는다.");
		Notifications.BindAction(Type, TDelegate<void(const TPayload&)>::CreateUObject(Owner, Handler));
	}

	virtual void UnsubscribeAll(const void* Owner) override
	{
		Notifications.UnbindAllForOwner(Owner);
	}

protected:
	/** 상태 변경 후 호출한다. 같은 페이로드 타입을 구독한 핸들러가 전부 호출된다. */
	template<typename TPayload>
	void Notify(const TPayload& Payload)
	{
		Notifications.Dispatch(TActionTypeOf<TPayload>::Value, Payload);
	}

private:
	TActionDispatcher<EActionType> Notifications;
};
