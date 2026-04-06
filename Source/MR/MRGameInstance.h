// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MRGameInstance.generated.h"

class UMRGameResource;

/**
 *
 */
UCLASS()
class MR_API UMRGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	static UMRGameInstance* Get() { return Singleton; }

	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;

	UMRGameResource* GetGameResource() const { return GameResource; }

private:
	static UMRGameInstance* Singleton;

	/** 사용할 GameResource 클래스. BP_MRGameInstance에서 BP_GameResource로 지정한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Resource")
	TSubclassOf<UMRGameResource> GameResourceClass;

	UPROPERTY()
	TObjectPtr<UMRGameResource> GameResource;
};
