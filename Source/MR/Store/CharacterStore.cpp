// Fill out your copyright notice in the Description page of Project Settings.

#include "Store/CharacterStore.h"

void UCharacterStore::DispatchSetHealth(float Current, float Max)
{
	State.MaxHealth     = FMath::Max(Max, 0.f);
	State.CurrentHealth = FMath::Clamp(Current, 0.f, State.MaxHealth);
	NotifyStateChanged(CharacterStoreFields::Health);
}

void UCharacterStore::DispatchSetStamina(float Current, float Max)
{
	State.MaxStamina     = FMath::Max(Max, 0.f);
	State.CurrentStamina = FMath::Clamp(Current, 0.f, State.MaxStamina);
	NotifyStateChanged(CharacterStoreFields::Stamina);
}
