// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MRTravelSettings.generated.h"

/**
 * Travel 서브시스템 설정.
 * Project Settings > MR > Travel 에서 편집할 수 있다.
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Travel", CategoryName="MR"))
class MR_API UMRTravelSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category="Travel")
	FString VillageMapName = TEXT("Village");

	UPROPERTY(config, EditAnywhere, Category="Travel")
	float FadeDuration = 0.8f;

	static const UMRTravelSettings* Get() { return GetDefault<UMRTravelSettings>(); }
};
