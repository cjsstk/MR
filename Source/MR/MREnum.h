// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MREnum.generated.h"

// ─── 속성 ──────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EMRElement : uint8
{
	None		UMETA(DisplayName = "None"),
	Fire		UMETA(DisplayName = "Fire"),
	Ice			UMETA(DisplayName = "Ice"),
	Thunder		UMETA(DisplayName = "Thunder"),
	Water		UMETA(DisplayName = "Water"),
	Poison		UMETA(DisplayName = "Poison"),
};

// ─── 방어구 부위 ───────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EMRArmorSlot : uint8
{
	Head	UMETA(DisplayName = "Head"),
	Chest	UMETA(DisplayName = "Chest"),
	Arms	UMETA(DisplayName = "Arms"),
	Waist	UMETA(DisplayName = "Waist"),
	Legs	UMETA(DisplayName = "Legs"),
};

// ─── 아이템 타입 ────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EMRItemType : uint8
{
	Consumable	UMETA(DisplayName = "Consumable"),
	Material	UMETA(DisplayName = "Material"),
	Decoration	UMETA(DisplayName = "Decoration"),
};

// ─── 상태이상 ───────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EMRStatusEffect : uint8
{
	None		UMETA(DisplayName = "None"),
	Poison		UMETA(DisplayName = "Poison"),
	Sleep		UMETA(DisplayName = "Sleep"),
	Paralysis	UMETA(DisplayName = "Paralysis"),
	Stun		UMETA(DisplayName = "Stun"),
	Blast		UMETA(DisplayName = "Blast"),
};

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

// ─── 채집 이동 정책 ─────────────────────────────────────────────────────────

/**
 * 채집 중 캐릭터의 이동 처리 방식.
 * Stationary       : 전신 몽타주 + 이동 입력 차단 (몬스터 박리 등)
 * MovingUpperBody  : 상체 슬롯 몽타주 + 이동 허용 (광석/식물 채집 등)
 */
UENUM(BlueprintType)
enum class EMRGatherMovementPolicy : uint8
{
	Stationary       UMETA(DisplayName = "Stationary (Full Body)"),
	MovingUpperBody  UMETA(DisplayName = "Moving (Upper Body)"),
};

// ─── 채집 대상 타입 ─────────────────────────────────────────────────────────

/** 채집 대상 종류. 데이터/사후처리 분기(소멸 vs 리스폰)에 사용. */
UENUM(BlueprintType)
enum class EMRGatherType : uint8
{
	Monster UMETA(DisplayName = "Monster"),
	Ore     UMETA(DisplayName = "Ore"),
	Plant   UMETA(DisplayName = "Plant"),
};
