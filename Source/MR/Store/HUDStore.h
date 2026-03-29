// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HUDStore.generated.h"

class UCharacterStore;

/**
 * UHUDStore
 *
 * HUD에 필요한 콘텐츠 Store들을 생성하고 소유하는 매니저.
 * UStoreBase를 상속하지 않으며, 데이터를 직접 보유하지 않는다.
 * 각 콘텐츠 Store는 UStoreBase를 상속한 별도 클래스로 구현된다.
 *
 * 접근 방법:
 *   GetHUDStore(WorldContextObject)   // Sugar.h
 *   GetGameInstance()->GetSubsystem<UHUDStore>()
 *
 * 새로운 HUD 데이터가 필요할 경우 UStoreBase를 상속하는 Store를 추가하고
 * 이 클래스에 멤버와 Getter를 추가한다.
 */
UCLASS()
class MR_API UHUDStore : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// --- UGameInstanceSubsystem ---

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Store Getters ---

	/** 캐릭터 HP, 스태미나 등의 상태를 담고 있는 Store */
	UFUNCTION(BlueprintCallable, Category = "HUD|Store")
	UCharacterStore* GetCharacterStore() const { return CharacterStore; }

private:
	UPROPERTY()
	TObjectPtr<UCharacterStore> CharacterStore;
};
