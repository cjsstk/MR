// Fill out your copyright notice in the Description page of Project Settings.

#include "Store/HUDStore.h"
#include "Store/CharacterStore.h"

void UHUDStore::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CharacterStore = NewObject<UCharacterStore>(this);
}

void UHUDStore::Deinitialize()
{
	CharacterStore = nullptr;
	Super::Deinitialize();
}
