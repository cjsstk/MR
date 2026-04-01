// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDStore.h"
#include "CharacterStore.h"
#include "Action/Action.h"

void UHUDStore::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// ActionDispatcher가 먼저 초기화되도록 보장
	UActionDispatcher* Dispatcher = Collection.InitializeDependency<UActionDispatcher>();

	UCharacterStore* CS = RegisterStore<UCharacterStore>();
	CS->RegisterActionHandlers(Dispatcher);
	// 새 Store 추가 시:
	// UXxxStore* XS = RegisterStore<UXxxStore>();
	// XS->RegisterActionHandlers(Dispatcher);
}

void UHUDStore::Deinitialize()
{
	if (UActionDispatcher* Dispatcher = GetGameInstance()->GetSubsystem<UActionDispatcher>())
	{
		for (auto& [Class, Store] : Stores)
		{
			if (UCharacterStore* CS = Cast<UCharacterStore>(Store))
			{
				CS->UnregisterActionHandlers(Dispatcher);
			}
		}
	}

	Stores.Empty();
	Super::Deinitialize();
}
