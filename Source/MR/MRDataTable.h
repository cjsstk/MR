// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MREnum.h"
#include "MRDataTable.generated.h"

/**
 * 드롭 풀 항목 하나. FDropTableRow의 Entries 배열 원소로 사용된다.
 * 확률은 Weight 합산 후 비율로 계산한다 (e.g. Weight=70+30 → 70%, 30%).
 */
USTRUCT(BlueprintType)
struct MR_API FDropEntry
{
	GENERATED_BODY()

	/** FItemTableRow의 RowName. 미입력 시 해당 항목 무시. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
	FName ItemId;

	/** 상대 가중치. 전체 Weight 합 대비 비율로 확률 결정. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
	int32 Weight = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
	int32 MaxCount = 1;
};

/**
 * 드롭 테이블 Row. RowName을 FMonsterTableRow에서 참조한다.
 * 일반 박리 / 포획 보상 / 부위 파괴 보상 각각 별도 Row로 관리한다.
 */
USTRUCT(BlueprintType)
struct MR_API FDropTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
	TArray<FDropEntry> Entries;
};

// ─── 레시피 ────────────────────────────────────────────────────────────────

/** 제작·강화에 필요한 소재 하나. FRecipeTableRow의 MaterialCosts 배열 원소. */
USTRUCT(BlueprintType)
struct MR_API FMaterialCost
{
	GENERATED_BODY()

	/** FItemTableRow의 RowName */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe", meta = (ClampMin = "1"))
	int32 Count = 1;
};

/**
 * 제작·강화 레시피 Row. RowName을 FWeaponTableRow/FArmorTableRow에서 RecipeId로 참조한다.
 * 무기 강화와 방어구 제작 모두 이 테이블을 공유한다.
 */
USTRUCT(BlueprintType)
struct MR_API FRecipeTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	TArray<FMaterialCost> MaterialCosts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe", meta = (ClampMin = "0"))
	int32 ZenyCost = 0;
};

// ─── 스킬 ──────────────────────────────────────────────────────────────────

/** 방어구 파츠 하나가 기여하는 스킬 포인트. FArmorTableRow::Skills 배열 원소. */
USTRUCT(BlueprintType)
struct MR_API FSkillContribution
{
	GENERATED_BODY()

	/** FSkillTableRow의 RowName */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName SkillId;

	/** 이 파츠가 기여하는 포인트. 동일 스킬을 합산해 레벨 결정. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "1"))
	int32 Points = 1;
};

/** 스킬 정의 Row. RowName = 스킬 ID. */
USTRUCT(BlueprintType)
struct MR_API FSkillTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "1"))
	int32 MaxLevel = 3;

	/** 레벨별 효과 설명. 인덱스 0 = Lv1. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	TArray<FText> LevelDescriptions;
};

// ─── 아이템 ────────────────────────────────────────────────────────────────

/** 소모품·소재·장식주 공용 아이템 Row. RowName = 아이템 ID. */
USTRUCT(BlueprintType)
struct MR_API FItemTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	EMRItemType ItemType = EMRItemType::Material;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
	int32 MaxStack = 99;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1", ClampMax = "10"))
	int32 Rarity = 1;

	/**
	 * 소모품 전용 효과량. 용도는 ItemType에 따라 다름.
	 * (e.g. Consumable + 회복약 → 회복 체력량, 고기 → 스태미나 상한 증가량)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Effect", meta = (EditCondition = "ItemType == EMRItemType::Consumable"))
	float EffectValue = 0.f;
};

/** 장식주 Row. ItemType == Decoration인 FItemTableRow와 1:1 대응. RowName 동일. */
USTRUCT(BlueprintType)
struct MR_API FDecorationTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** FSkillTableRow의 RowName */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decoration")
	FName SkillId;

	/** 삽입 가능한 슬롯 최소 크기 (1~3) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decoration", meta = (ClampMin = "1", ClampMax = "3"))
	int32 SlotSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decoration", meta = (ClampMin = "1"))
	int32 SkillPoints = 1;
};

// ─── 무기 ──────────────────────────────────────────────────────────────────

/**
 * 무기 Row. RowName = 무기 ID.
 * 강화 트리는 ParentWeaponId로 부모를 참조하는 방향 비순환 그래프로 표현한다.
 */
USTRUCT(BlueprintType)
struct MR_API FWeaponTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EMRWeaponType WeaponType = EMRWeaponType::OneHandedSword;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "1", ClampMax = "10"))
	int32 Rarity = 1;

	// ─── 스탯 ────────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Stats")
	float BaseAttack = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Element")
	EMRElement Element = EMRElement::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Element")
	float ElementDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Status")
	EMRStatusEffect StatusEffect = EMRStatusEffect::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Status")
	float StatusValue = 0.f;

	// ─── 슬롯 & 트리 ─────────────────────────────────────────────────────

	/** 장식주 슬롯 크기 목록. e.g. {3,1} = 3슬롯 1개 + 1슬롯 1개 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<int32> JewelSlotSizes;

	/** 강화 트리 부모 무기 RowName. 루트 무기는 비워둔다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Tree")
	FName ParentWeaponId;

	/** 이 무기 제작·강화에 필요한 레시피. FRecipeTableRow의 RowName. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Tree")
	FName RecipeId;
};

// ─── 방어구 ────────────────────────────────────────────────────────────────

/** 방어구 파츠 Row. 부위(Slot)별 파츠를 각각 Row로 등록한다. RowName = 파츠 ID. */
USTRUCT(BlueprintType)
struct MR_API FArmorTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor")
	EMRArmorSlot Slot = EMRArmorSlot::Head;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor", meta = (ClampMin = "1", ClampMax = "10"))
	int32 Rarity = 1;

	// ─── 방어 스탯 ───────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor|Stats")
	float BaseDefense = 20.f;

	/** 양수 = 내성, 음수 = 약점 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor|Resistance")
	float FireResistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor|Resistance")
	float IceResistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor|Resistance")
	float ThunderResistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor|Resistance")
	float WaterResistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor|Resistance")
	float PoisonResistance = 0.f;

	// ─── 스킬 & 슬롯 ─────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor|Skill")
	TArray<FSkillContribution> Skills;

	/** 장식주 슬롯 크기 목록. e.g. {2} = 2슬롯 1개 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor")
	TArray<int32> JewelSlotSizes;

	/** 이 파츠 제작에 필요한 레시피. FRecipeTableRow의 RowName. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor|Tree")
	FName RecipeId;
};

/** 몬스터 데이터 테이블 Row. RowName = 몬스터 ID (e.g. "Rathalos") */
USTRUCT(BlueprintType)
struct MR_API FMonsterTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/**
	 * 몬스터 고유 타입 번호. 다른 테이블에서 몬스터를 참조할 때 이 값을 사용한다.
	 * 중복되면 로드 시 경고가 출력된다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	int32 Type = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	FText DisplayName;

	// ─── 기본 스탯 ─────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Stats")
	float MaxHealth = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Stats")
	float BaseAttack = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Stats")
	float BaseDefense = 30.f;

	/**
	 * 레벨당 스탯 배율. FinalStat = BaseStat * LevelScale^(Level-1).
	 * 기본값 1.15 → 레벨 5 몬스터는 기본 스탯의 약 1.75배.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Stats", meta = (ClampMin = "1.0"))
	float LevelScale = 1.15f;

	// ─── 속성 ──────────────────────────────────────────────────────────────

	/** 약점 속성. 이 속성 공격 시 데미지 배율 증가. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Element")
	EMRElement WeakElement = EMRElement::None;

	/** 자신이 사용하는 속성 공격 타입. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Element")
	EMRElement AttackElement = EMRElement::None;

	// ─── 드롭 ──────────────────────────────────────────────────────────────

	/** 일반 사냥(박리) 드롭 풀. FDropTableRow의 RowName. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Drop")
	FName NormalDropTableId;

	/** 포획 보상 드롭 풀. FDropTableRow의 RowName. 비어있으면 NormalDropTableId 사용. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Drop")
	FName CaptureDropTableId;
};

/**
 * 프로젝트 전용 DataTable 베이스 클래스.
 * 테이블별 Row 구조체는 이 파일에 FTableRowBase 서브구조체로 정의한다.
 */
UCLASS()
class MR_API UMRDataTable : public UDataTable
{
	GENERATED_BODY()
};
