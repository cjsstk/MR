// Fill out your copyright notice in the Description page of Project Settings.

#include "MRMonsterHealthBarWidget.h"
#include "Components/ProgressBar.h"

void UMRMonsterHealthBarWidget::UpdateHealth(float CurrentHealth, float MaxHealth)
{
	if (HealthBar && MaxHealth > 0.f)
	{
		HealthBar->SetPercent(CurrentHealth / MaxHealth);
	}
}
