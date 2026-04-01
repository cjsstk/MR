// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MRGameplayAbility.generated.h"

class AMRBaseCharacter;
class AMRPlayerCharacter;

/**
 * 프로젝트 전용 GameplayAbility 베이스.
 * 싱글 플레이어 기본값 설정 및 공용 헬퍼를 제공한다.
 */
UCLASS(Abstract)
class MR_API UMRGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UMRGameplayAbility();

protected:
	/** AvatarActor를 AMRBaseCharacter로 캐스팅해서 반환. 없으면 nullptr. */
	AMRBaseCharacter* GetBaseCharacter() const;

	/** AvatarActor를 AMRPlayerCharacter로 캐스팅해서 반환. 없으면 nullptr. */
	AMRPlayerCharacter* GetPlayerCharacter() const;
};
