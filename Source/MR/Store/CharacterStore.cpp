// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterStore.h"
#include "Action/Action.h"

void UCharacterStore::RegisterActionHandlers(UActionDispatcher* Dispatcher)
{
	Dispatcher->BindAction(MakeActionDelegate(this, &UCharacterStore::HandleSetHealth));
	Dispatcher->BindAction(MakeActionDelegate(this, &UCharacterStore::HandleSetStamina));
}

void UCharacterStore::UnregisterActionHandlers(UActionDispatcher* Dispatcher)
{
	Dispatcher->UnbindAll(this);
}

void UCharacterStore::HandleSetHealth(const FAction_SetHealth& Action)
{
	State.MaxHealth     = FMath::Max(Action.Max, 0.f);
	State.CurrentHealth = FMath::Clamp(Action.Current, 0.f, State.MaxHealth);

	// 클램프된 실제 State 값으로 알린다 — Action의 원본 값(클램프 전)을 그대로
	// 넘기면 구독자가 Store에 실제 반영된 값과 다른 값을 받을 수 있다.
	Notify(FAction_SetHealth{ State.CurrentHealth, State.MaxHealth });
}

void UCharacterStore::HandleSetStamina(const FAction_SetStamina& Action)
{
	State.MaxStamina     = FMath::Max(Action.Max, 0.f);
	State.CurrentStamina = FMath::Clamp(Action.Current, 0.f, State.MaxStamina);
	Notify(FAction_SetStamina{ State.CurrentStamina, State.MaxStamina });
}
