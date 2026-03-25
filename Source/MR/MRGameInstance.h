// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MRGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MR_API UMRGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	static UMRGameInstance* Get() { return Singleton; }

	// GameInstance Created -> Init() -> StartGameInstance()
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;

private:
	static UMRGameInstance* Singleton;

};
