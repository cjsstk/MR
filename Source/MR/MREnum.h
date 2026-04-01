// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MREnum.generated.h"

// ─── 무기 타입 ─────────────────────────────────────────────────────────────

/**
 * 무기 타입 열거형. AnimSet DataTable의 RowName과 1:1 매칭된다.
 * 새 무기 추가 시 여기에 값을 추가하고 DataTable에 Row를 추가한다.
 */
UENUM(BlueprintType)
enum class EMRWeaponType : uint8
{
	OneHandedSword	UMETA(DisplayName = "One-Handed Sword"),
	TwoHandedSword	UMETA(DisplayName = "Two-Handed Sword"),
	Bow				UMETA(DisplayName = "Bow"),
};
