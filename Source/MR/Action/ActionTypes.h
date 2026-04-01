// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionTypes.generated.h"

/**
 * 게임에서 사용되는 액션 타입.
 * 새 액션 추가 시 여기에 열거값을 추가하고 Action.h의 팩토리 함수를 추가한다.
 */
UENUM(BlueprintType)
enum class EActionType : uint8
{
	SetHealth,
	SetStamina,
};
