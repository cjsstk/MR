// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MREnum.h"
#include "MRTravelTypes.generated.h"

/** 레벨 간 이동 시 보존할 플레이어 상태 */
USTRUCT()
struct FMRPlayerPersistData
{
	GENERATED_BODY()

	UPROPERTY()
	float Health = 0.f;

	UPROPERTY()
	float MaxHealth = 100.f;

	UPROPERTY()
	float Stamina = 0.f;

	UPROPERTY()
	float MaxStamina = 100.f;

	UPROPERTY()
	EMRWeaponType WeaponType = EMRWeaponType::OneHandedSword;

	/** false면 복원 건너뛰고 GameMode 기본값 사용 */
	UPROPERTY()
	bool bIsValid = false;
};
