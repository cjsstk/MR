// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRStoreBase.h"
#include "Action/ActionTypes.h"
#include "CharacterStore.generated.h"

class UActionDispatcher;
struct FAction_SetHealth;
struct FAction_SetStamina;

USTRUCT(BlueprintType)
struct FCharacterState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float CurrentHealth = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float CurrentStamina = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float MaxStamina = 100.f;
};

/**
 * UCharacterStore
 *
 * 캐릭터 HP, 스태미나 상태를 보유하는 Store.
 * 상태 변경은 직접 호출이 아닌 UActionDispatcher를 통해 이루어진다.
 *
 * 접근 방법:
 *   GetCharacterStore(this)   // Sugar.h
 */
UCLASS()
class MR_API UCharacterStore : public UMRStoreBase
{
	GENERATED_BODY()

public:
	const FCharacterState& GetState() const { return State; }

	/** UHUDStore::Initialize에서 호출. 액션 타입별 핸들러를 Dispatcher에 등록한다. */
	virtual void RegisterActionHandlers(UActionDispatcher* Dispatcher) override;

	/** UHUDStore::Deinitialize에서 호출. 등록된 모든 핸들러를 해제한다. */
	virtual void UnregisterActionHandlers(UActionDispatcher* Dispatcher) override;

private:
	DECLARE_ACTION_HANDLER(HandleSetHealth, SetHealth);
	DECLARE_ACTION_HANDLER(HandleSetStamina, SetStamina);

	UPROPERTY()
	FCharacterState State;
};
