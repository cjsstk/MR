// Fill out your copyright notice in the Description page of Project Settings.

#include "MRVillageGameMode.h"
#include "Travel/MRTravelSubsystem.h"
#include "Character/MRPlayerCharacter.h"
#include "Engine/GameInstance.h"

AMRVillageGameMode::AMRVillageGameMode()
{
}

void AMRVillageGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (NewPlayer)
	{
		if (AMRPlayerCharacter* Player = Cast<AMRPlayerCharacter>(NewPlayer->GetPawn()))
		{
			if (UGameInstance* GI = GetGameInstance())
			{
				if (UMRTravelSubsystem* TravelSubsystem = GI->GetSubsystem<UMRTravelSubsystem>())
				{
					TravelSubsystem->RestorePlayerState(Player);
				}
			}
		}
	}
}
