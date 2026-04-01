// Fill out your copyright notice in the Description page of Project Settings.

#include "Action.h"

void UActionDispatcher::Dispatch(const FAction& Action)
{
	TArray<FActionBinding>* Bindings = ActionBindings.Find(Action.Type);
	if (!Bindings)
	{
		return;
	}

	// 복사본으로 순회 — 핸들러 내부에서 바인딩이 변경될 경우를 방어
	TArray<FActionBinding> BindingsCopy = *Bindings;
	for (const FActionBinding& Binding : BindingsCopy)
	{
		Binding.Delegate.ExecuteIfBound(Action);
	}
}

FDelegateHandle UActionDispatcher::BindAction(EActionType Type, FActionHandlerDelegate&& Delegate)
{
	FDelegateHandle Handle(FDelegateHandle::GenerateNewHandle);
	ActionBindings.FindOrAdd(Type).Add({ Handle, MoveTemp(Delegate) });
	return Handle;
}

void UActionDispatcher::UnbindAction(EActionType Type, FDelegateHandle Handle)
{
	TArray<FActionBinding>* Bindings = ActionBindings.Find(Type);
	if (Bindings)
	{
		Bindings->RemoveAll([Handle](const FActionBinding& B) { return B.Handle == Handle; });
	}
}
