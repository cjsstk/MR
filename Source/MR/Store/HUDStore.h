// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MRStore/StoreBase.h"
#include "CharacterStore.h"
#include "InventoryStore.h"
#include "HUDStore.generated.h"

class UActionDispatcher;

/**
 * UHUDStore
 *
 * HUD에 필요한 콘텐츠 Store들을 생성·소유하는 매니저.
 * Initialize에서 UActionDispatcher에 각 Store의 핸들러를 등록한다.
 *
 * ── Store 추가 방법 ───────────────────────────────────────────
 *  1. UStoreBase를 상속하는 UXxxStore 클래스를 생성한다.
 *  2. ActionTypes.h에 액션 타입을 추가하고 Action.h에 팩토리 함수를 추가한다.
 *  3. UXxxStore에 RegisterActionHandlers / UnregisterActionHandlers를 구현한다.
 *  4. Initialize에서 RegisterStore<UXxxStore>() 호출 후 핸들러를 등록한다.
 * ──────────────────────────────────────────────────────────────
 */
UCLASS()
class MR_API UHUDStore : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Store Getters ---

	UCharacterStore* GetCharacterStore() const { return GetStore<UCharacterStore>(); }
	UInventoryStore*     GetInventoryStore()     const { return GetStore<UInventoryStore>(); }

	template<typename T>
	T* GetStore() const
	{
		static_assert(TIsDerivedFrom<T, UStoreBase>::Value, "T must derive from UStoreBase");
		TObjectPtr<UStoreBase> const* Found = Stores.Find(T::StaticClass());
		return Found ? Cast<T>(Found->Get()) : nullptr;
	}

private:
	template<typename T>
	T* RegisterStore()
	{
		static_assert(TIsDerivedFrom<T, UStoreBase>::Value, "T must derive from UStoreBase");
		T* Store = NewObject<T>(this);
		Stores.Add(T::StaticClass(), Store);
		return Store;
	}

	UPROPERTY()
	TMap<UClass*, TObjectPtr<UStoreBase>> Stores;
};
