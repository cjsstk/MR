// Fill out your copyright notice in the Description page of Project Settings.

#include "MRCheatManager.h"
#include "Engine/Engine.h"

void UMRCheatManager::OneHitKill()
{
	bOneHitKillEnabled = !bOneHitKillEnabled;

	const FString Message = FString::Printf(TEXT("[Cheat] OneHitKill: %s"), bOneHitKillEnabled ? TEXT("ON") : TEXT("OFF"));
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, Message);
	}
}
