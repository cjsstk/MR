// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MRVillageGameMode.generated.h"

/**
 * 마을(Village) 레벨 전용 GameMode.
 * 플레이어 스폰 후 UMRTravelSubsystem::RestorePlayerState를 호출해
 * 이전 필드에서 보존된 플레이어 상태를 복원한다.
 */
UCLASS()
class MR_API AMRVillageGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMRVillageGameMode();

protected:
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
};
