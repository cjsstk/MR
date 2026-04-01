// Fill out your copyright notice in the Description page of Project Settings.


#include "Sugar.h"
#include "Subsystem/CMSSubsystem.h"
#include "Subsystem/GameResourceSubsystem.h"
#include "Store/HUDStore.h"
#include "Store/CharacterStore.h"
#include "Action/Action.h"
#include "Engine/GameInstance.h"

namespace
{
	UGameInstance* GetGameInstance(const UObject* InObject)
	{
		if (!InObject)
		{
			return nullptr;
		}
		const UWorld* World = InObject->GetWorld();
		return World ? World->GetGameInstance() : nullptr;
	}
}

UCMSSubsystem* GetCMS(const UObject* InObject)
{
	UGameInstance* GI = GetGameInstance(InObject);
	return GI ? GI->GetSubsystem<UCMSSubsystem>() : nullptr;
}

UGameResourceSubsystem* GetGameResource(const UObject* InObject)
{
	UGameInstance* GI = GetGameInstance(InObject);
	return GI ? GI->GetSubsystem<UGameResourceSubsystem>() : nullptr;
}

UHUDStore* GetHUDStore(const UObject* InObject)
{
	if (!InObject)
	{
		return nullptr;
	}

	const UWorld* World = InObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	return GI->GetSubsystem<UHUDStore>();
}

UCharacterStore* GetCharacterStore(const UObject* InObject)
{
	UHUDStore* HUDStore = GetHUDStore(InObject);
	return HUDStore ? HUDStore->GetCharacterStore() : nullptr;
}

UActionDispatcher* GetActionDispatcher(const UObject* InObject)
{
	UGameInstance* GI = GetGameInstance(InObject);
	return GI ? GI->GetSubsystem<UActionDispatcher>() : nullptr;
}
