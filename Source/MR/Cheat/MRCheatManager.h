// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "MRCheatManager.generated.h"

/**
 * 프로젝트 전용 치트 매니저. 콘솔(`~`)에서 Exec 함수를 커맨드로 실행할 수 있다.
 * DefaultEngine.ini의 [/Script/Engine.PlayerController] CheatClass로 등록됨.
 */
UCLASS()
class MR_API UMRCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	/** 원킬 모드 토글. 활성화 시 플레이어의 공격이 명중한 몬스터를 즉시 사망시킨다. */
	UFUNCTION(Exec)
	void OneHitKill();

	bool IsOneHitKillEnabled() const { return bOneHitKillEnabled; }

private:
	bool bOneHitKillEnabled = false;
};
