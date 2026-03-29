// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/CharacterStatusWidget.h"
#include "Components/ProgressBar.h"
#include "Store/CharacterStore.h"
#include "Sugar.h"

void UCharacterStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindStore(GetCharacterStore(this));
}

void UCharacterStatusWidget::OnStoreStateChanged(UStoreBase* Store, FName FieldName)
{
	UCharacterStore* CharacterStore = Cast<UCharacterStore>(Store);
	if (!CharacterStore)
	{
		return;
	}

	const FCharacterState& State = CharacterStore->GetState();

	if (FieldName == NAME_None || FieldName == CharacterStoreFields::Health)
	{
		RefreshHealth(State);
	}

	if (FieldName == NAME_None || FieldName == CharacterStoreFields::Stamina)
	{
		RefreshStamina(State);
	}
}

void UCharacterStatusWidget::RefreshHealth(const FCharacterState& State)
{
	if (HealthBar)
	{
		const float Percent = State.MaxHealth > 0.f ? State.CurrentHealth / State.MaxHealth : 0.f;
		HealthBar->SetPercent(Percent);
	}
}

void UCharacterStatusWidget::RefreshStamina(const FCharacterState& State)
{
	if (StaminaBar)
	{
		const float Percent = State.MaxStamina > 0.f ? State.CurrentStamina / State.MaxStamina : 0.f;
		StaminaBar->SetPercent(Percent);
	}
}
