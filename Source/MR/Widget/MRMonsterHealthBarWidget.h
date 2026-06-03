// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MRMonsterHealthBarWidget.generated.h"

class UProgressBar;

UCLASS()
class MR_API UMRMonsterHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateHealth(float CurrentHealth, float MaxHealth);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
};
