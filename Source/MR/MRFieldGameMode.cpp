// Fill out your copyright notice in the Description page of Project Settings.

#include "MRFieldGameMode.h"
#include "Travel/MRTravelSubsystem.h"
#include "Character/MRPlayerCharacter.h"
#include "Engine/GameInstance.h"

AMRFieldGameMode::AMRFieldGameMode()
{
}

void AMRFieldGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	AMRPlayerCharacter* Player = Cast<AMRPlayerCharacter>(NewPlayer->GetPawn());
	if (!Player)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	UMRTravelSubsystem* TravelSubsystem = GI->GetSubsystem<UMRTravelSubsystem>();
	if (TravelSubsystem)
	{
		TravelSubsystem->RestorePlayerState(Player);
	}
}
