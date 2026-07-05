// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDStore.h"
#include "CharacterStore.h"
#include "InventoryStore.h"
#include "Action/Action.h"

void UHUDStore::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// ActionDispatcher가 먼저 초기화되도록 보장
	UActionDispatcher* Dispatcher = Collection.InitializeDependency<UActionDispatcher>();

	UCharacterStore* CS = RegisterStore<UCharacterStore>();
	CS->RegisterActionHandlers(Dispatcher);

	UInventoryStore* PS = RegisterStore<UInventoryStore>();
	PS->RegisterActionHandlers(Dispatcher);
}

void UHUDStore::Deinitialize()
{
	if (UActionDispatcher* Dispatcher = GetGameInstance()->GetSubsystem<UActionDispatcher>())
	{
		for (auto& [Class, Store] : Stores)
		{
			if (Store)
			{
				Store->UnregisterActionHandlers(Dispatcher);
			}
		}
	}

	Stores.Empty();
	Super::Deinitialize();
}
