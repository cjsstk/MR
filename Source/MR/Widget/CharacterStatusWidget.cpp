// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/CharacterStatusWidget.h"
#include "Components/ProgressBar.h"
#include "Store/CharacterStore.h"
#include "Action/Action.h"
#include "Sugar.h"

void UCharacterStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCharacterStore* Store = GetCharacterStore(this))
	{
		Subscribe<EActionType::SetHealth>(Store, &UCharacterStatusWidget::HandleHealthChanged);
		Subscribe<EActionType::SetStamina>(Store, &UCharacterStatusWidget::HandleStaminaChanged);

		const FCharacterState& State = Store->GetState();
		HandleHealthChanged(FAction_SetHealth{ State.CurrentHealth, State.MaxHealth });
		HandleStaminaChanged(FAction_SetStamina{ State.CurrentStamina, State.MaxStamina });
	}
}

void UCharacterStatusWidget::HandleHealthChanged(const FAction_SetHealth& Action)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(Action.Max > 0.f ? Action.Current / Action.Max : 0.f);
	}
}

void UCharacterStatusWidget::HandleStaminaChanged(const FAction_SetStamina& Action)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(Action.Max > 0.f ? Action.Current / Action.Max : 0.f);
	}
}
