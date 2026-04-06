// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRBaseAnimInstance.h"
#include "MRMonsterAnimInstance.generated.h"

/**
 * 몬스터 전용 AnimInstance.
 * 공통 Speed는 베이스에서 처리되며, 몬스터 전용 상태 변수를 여기에 추가한다.
 */
UCLASS()
class MR_API UMRMonsterAnimInstance : public UMRBaseAnimInstance
{
	GENERATED_BODY()
};
