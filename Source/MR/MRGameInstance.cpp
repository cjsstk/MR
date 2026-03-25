// Fill out your copyright notice in the Description page of Project Settings.


#include "MRGameInstance.h"

UMRGameInstance* UMRGameInstance::Singleton = nullptr;

void UMRGameInstance::Init()
{
	Super::Init();
}

void UMRGameInstance::Shutdown()
{
	Super::Shutdown();
}

void UMRGameInstance::StartGameInstance()
{
	Super::StartGameInstance();
}
