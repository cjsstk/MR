// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRStruct.generated.h"

/**
 * 특정 시스템에 종속되지 않는 범용 구조체 모음.
 * 여러 모듈/파일에서 공통으로 쓰는 작은 struct는 여기에 추가한다.
 * (DataTable Row 자체는 MRDataTable.h에 둔다 — 여기는 Row 내부 배열 원소 등
 * 재사용되는 보조 구조체용)
 */

/**
 * 드롭 풀 항목 하나. FDropTableRow의 Entries 배열 원소로 사용된다.
 * 확률은 Weight 합산 후 비율로 계산한다 (e.g. Weight=70+30 → 70%, 30%).
 */
USTRUCT(BlueprintType)
struct MR_API FDropEntry
{
	GENERATED_BODY()

	/** FItemTableRow의 RowName. 미입력 시(0) 해당 항목 무시. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
	int32 ItemId = 0;

	/** 상대 가중치. 전체 Weight 합 대비 비율로 확률 결정. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
	int32 Weight = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
	int32 MaxCount = 1;
};

/** 제작·강화에 필요한 소재 하나. FRecipeTableRow의 MaterialCosts 배열 원소. */
USTRUCT(BlueprintType)
struct MR_API FMaterialCost
{
	GENERATED_BODY()

	/** FItemTableRow의 RowName */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	int32 ItemId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe", meta = (ClampMin = "1"))
	int32 Count = 1;
};

/** 방어구 파츠 하나가 기여하는 스킬 포인트. FArmorTableRow::Skills 배열 원소. */
USTRUCT(BlueprintType)
struct MR_API FSkillContribution
{
	GENERATED_BODY()

	/** FSkillTableRow의 RowName */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	int32 SkillId = 0;

	/** 이 파츠가 기여하는 포인트. 동일 스킬을 합산해 레벨 결정. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "1"))
	int32 Points = 1;
};
