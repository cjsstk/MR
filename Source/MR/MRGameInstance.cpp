// Fill out your copyright notice in the Description page of Project Settings.

#include "MRGameInstance.h"
#include "Subsystem/GameResourceSubsystem.h"

UMRGameInstance* UMRGameInstance::Singleton = nullptr;

void UMRGameInstance::Init()
{
	Super::Init();
	Singleton = this;

	// GameResourceClass가 지정된 경우 인스턴스 생성 (CDO 값 포함)
	if (GameResourceClass)
	{
		GameResource = NewObject<UMRGameResource>(this, GameResourceClass);
	}
}

void UMRGameInstance::Shutdown()
{
	if (GameResource)
	{
		GameResource->Deinitialize();
	}
	Singleton = nullptr;
	Super::Shutdown();
}

void UMRGameInstance::StartGameInstance()
{
	Super::StartGameInstance();
}
