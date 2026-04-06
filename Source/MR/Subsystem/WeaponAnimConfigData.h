// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MREnum.h"
#include "WeaponAnimConfigData.generated.h"

class UBlendSpace;
class UAnimSequence;

/**
 * 무기 타입별 애니메이션 에셋 세트.
 * UWeaponAnimConfigData::WeaponAnimConfigs에서 EMRWeaponType을 키로 설정한다.
 */
USTRUCT(BlueprintType)
struct MR_API FWeaponAnimConfig
{
	GENERATED_BODY()

	/** 이동 방향+속도 기반 블렌드 스페이스 (보행~달리기 통합) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UBlendSpace> LocomotionBlendSpace;

	/** 대기 상태 애니메이션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimSequence> IdleAnimation;

	/** 점프 시작 (지면에서 발이 떨어지는 순간) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Jump")
	TSoftObjectPtr<UAnimSequence> JumpStartAnimation;

	/** 공중 체공 루프 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Jump")
	TSoftObjectPtr<UAnimSequence> JumpLoopAnimation;

	/** 착지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Jump")
	TSoftObjectPtr<UAnimSequence> JumpEndAnimation;
};

/**
 * 무기 타입별 애니메이션 설정을 담는 DataAsset.
 * BP_GameResource의 WeaponAnimConfigData에 지정한다.
 */
UCLASS(BlueprintType)
class MR_API UWeaponAnimConfigData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponAnimation")
	TMap<EMRWeaponType, FWeaponAnimConfig> WeaponAnimConfigs;
};
