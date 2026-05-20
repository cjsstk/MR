// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MRFieldGameMode.generated.h"

/**
 * 필드(Field/Hunting Ground) 레벨 전용 GameMode.
 * 플레이어 스폰 후 UMRTravelSubsystem::RestorePlayerState를 호출해
 * 마을에서 보존된 플레이어 상태를 복원한다.
 * Phase 2에서 FFieldTableRow::MonsterTypes 기반 몬스터 스폰 로직이 추가된다.
 */
UCLASS()
class MR_API AMRFieldGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMRFieldGameMode();

protected:
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
};
