// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRStore/StoreBase.h"
#include "CharacterStore.generated.h"

/** CharacterStore 필드명 상수. 오타 방지를 위해 raw FName 리터럴 대신 사용한다. */
namespace CharacterStoreFields
{
	inline const FName Health  = TEXT("Health");
	inline const FName Stamina = TEXT("Stamina");
}

/**
 * FCharacterState
 *
 * HUD에 표시되는 캐릭터 상태 스냅샷 (HP, 스태미나 등).
 * 순수 값 타입(POD)으로 유지한다 — UObject 참조 금지.
 */
USTRUCT(BlueprintType)
struct FCharacterState
{
	GENERATED_BODY()

	/** 현재 HP */
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float CurrentHealth = 0.f;

	/** 최대 HP */
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float MaxHealth = 100.f;

	/** 현재 스태미나 */
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float CurrentStamina = 0.f;

	/** 최대 스태미나 */
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float MaxStamina = 100.f;
};

/**
 * UCharacterStore
 *
 * 캐릭터 HP, 스태미나 등의 상태 데이터를 보유하는 Store.
 * UHUDStore가 생성하고 소유한다.
 *
 * 접근 방법:
 *   GetHUDStore(this)->GetCharacterStore()
 */
UCLASS()
class MR_API UCharacterStore : public UStoreBase
{
	GENERATED_BODY()

public:
	/** 현재 캐릭터 상태 스냅샷을 반환한다. */
	const FCharacterState& GetState() const { return State; }

	// --- Actions (Dispatch 메서드) ---

	/**
	 * HP를 갱신한다.
	 * Current는 [0, Max] 범위로 클램핑된다.
	 * CharacterStoreFields::Health로 변경 알림 발송.
	 */
	UFUNCTION(BlueprintCallable, Category = "Character|Actions")
	void DispatchSetHealth(float Current, float Max);

	/**
	 * 스태미나를 갱신한다.
	 * Current는 [0, Max] 범위로 클램핑된다.
	 * CharacterStoreFields::Stamina로 변경 알림 발송.
	 */
	UFUNCTION(BlueprintCallable, Category = "Character|Actions")
	void DispatchSetStamina(float Current, float Max);

private:
	UPROPERTY()
	FCharacterState State;
};
