// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterStore.h"
#include "Action/Action.h"

void UCharacterStore::RegisterActionHandlers(UActionDispatcher* Dispatcher)
{
	HealthHandle  = Dispatcher->BindAction(EActionType::SetHealth,
		FActionHandlerDelegate::CreateUObject(this, &UCharacterStore::HandleSetHealth));

	StaminaHandle = Dispatcher->BindAction(EActionType::SetStamina,
		FActionHandlerDelegate::CreateUObject(this, &UCharacterStore::HandleSetStamina));
}

void UCharacterStore::UnregisterActionHandlers(UActionDispatcher* Dispatcher)
{
	Dispatcher->UnbindAction(EActionType::SetHealth,  HealthHandle);
	Dispatcher->UnbindAction(EActionType::SetStamina, StaminaHandle);
}

void UCharacterStore::HandleSetHealth(const FAction& Action)
{
	State.MaxHealth     = FMath::Max(Action.Value2, 0.f);
	State.CurrentHealth = FMath::Clamp(Action.Value1, 0.f, State.MaxHealth);
	NotifyStateChanged(CharacterStoreFields::Health);
}

void UCharacterStore::HandleSetStamina(const FAction& Action)
{
	State.MaxStamina     = FMath::Max(Action.Value2, 0.f);
	State.CurrentStamina = FMath::Clamp(Action.Value1, 0.f, State.MaxStamina);
	NotifyStateChanged(CharacterStoreFields::Stamina);
}
