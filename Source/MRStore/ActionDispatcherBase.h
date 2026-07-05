// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"

/**
 * TActionDispatcher
 *
 * 액션 타입(TActionType)별로 핸들러를 등록하고, 액션 발생 시 해당 타입에 등록된
 * 핸들러들을 호출하는 제네릭 액션 디스패처.
 *
 * 액션마다 페이로드 struct 타입이 달라도 된다(Bind/Dispatch가 각각 템플릿).
 * 같은 TActionType 값에 대해 Bind/Dispatch에 서로 다른 페이로드 타입을 쓰면
 * 핸들러가 호출되지 않거나 잘못된 캐스팅이 발생하므로, 타입과 페이로드를 1:1로
 * 묶어 호출부에 노출하는 것은 사용하는 쪽(예: MR 모듈의 DECLARE_ACTION_PAYLOAD)의 책임이다.
 *
 * UObject가 아닌 순수 C++ 템플릿이라 프로젝트별 액션 타입을 그대로 재사용할 수 있다.
 * 사용 방법: 프로젝트의 UGameInstanceSubsystem 등이 이 클래스를 멤버로 감싸서 사용한다.
 */
template<typename TActionType>
class TActionDispatcher
{
public:
	/** 특정 액션 타입에 핸들러를 바인딩한다. 반환된 FDelegateHandle로 UnbindAction 호출 시 해제한다. */
	template<typename TPayload>
	FDelegateHandle BindAction(TActionType Type, TDelegate<void(const TPayload&)>&& Delegate)
	{
		FDelegateHandle Handle(FDelegateHandle::GenerateNewHandle);
		const void* Owner = Delegate.GetUObject();
		ActionBindings.FindOrAdd(Type).Add(FBinding
		{
			Handle,
			Owner,
			[BoundDelegate = MoveTemp(Delegate)](const void* Payload)
			{
				BoundDelegate.ExecuteIfBound(*static_cast<const TPayload*>(Payload));
			}
		});
		return Handle;
	}

	/** 특정 액션 타입에서 핸들러를 해제한다. */
	void UnbindAction(TActionType Type, FDelegateHandle Handle)
	{
		if (TArray<FBinding>* Bindings = ActionBindings.Find(Type))
		{
			Bindings->RemoveAll([Handle](const FBinding& B) { return B.Handle == Handle; });
		}
	}

	/**
	 * 특정 오너(CreateUObject로 바인딩할 때 쓴 this)가 등록한 모든 액션 타입의 핸들러를
	 * 한 번에 해제한다. UE의 TMulticastDelegate::RemoveAll(UserObject)과 같은 역할이라,
	 * Store가 액션 타입/핸들 하나하나를 직접 추적하지 않아도 된다.
	 */
	void UnbindAllForOwner(const void* Owner)
	{
		for (auto& Pair : ActionBindings)
		{
			Pair.Value.RemoveAll([Owner](const FBinding& B) { return B.Owner == Owner; });
		}
	}

	/** 액션을 디스패치한다. 해당 타입에 바인딩된 모든 핸들러가 호출된다. */
	template<typename TPayload>
	void Dispatch(TActionType Type, const TPayload& Payload)
	{
		TArray<FBinding>* Bindings = ActionBindings.Find(Type);
		if (!Bindings)
		{
			return;
		}

		// 복사본으로 순회 — 핸들러 내부에서 바인딩이 변경될 경우를 방어
		TArray<FBinding> BindingsCopy = *Bindings;
		for (const FBinding& Binding : BindingsCopy)
		{
			Binding.Invoke(&Payload);
		}
	}

private:
	struct FBinding
	{
		FDelegateHandle Handle;
		const void* Owner = nullptr;
		TFunction<void(const void*)> Invoke;
	};

	TMap<TActionType, TArray<FBinding>> ActionBindings;
};

/**
 * 멤버 함수 포인터로부터 페이로드 타입을 추론해 델리게이트를 만드는 헬퍼.
 * BindAction 호출부에서 델리게이트 타입을 직접 쓰지 않아도 되게 해준다.
 *
 * 사용 예: Dispatcher->BindAction(MakeActionDelegate(this, &UMyStore::HandleFoo));
 */
template<typename TOwner, typename TPayload>
TDelegate<void(const TPayload&)> MakeActionDelegate(TOwner* Owner, void (TOwner::* Handler)(const TPayload&))
{
	return TDelegate<void(const TPayload&)>::CreateUObject(Owner, Handler);
}

/**
 * 페이로드 struct를 애그리게잇 초기화로 생성하는 헬퍼.
 * 액션마다 이름 있는 팩토리 함수를 손으로 만들지 않아도, 페이로드 타입만 지정하면
 * 필드 선언 순서대로 인자를 넘겨 생성할 수 있다.
 *
 * 사용 예: Dispatcher->Dispatch(MakeAction<FAction_SetHealth>(80.f, 100.f));
 */
template<typename TPayload, typename... TArgs>
TPayload MakeAction(TArgs&&... Args)
{
	return TPayload{ Forward<TArgs>(Args)... };
}
