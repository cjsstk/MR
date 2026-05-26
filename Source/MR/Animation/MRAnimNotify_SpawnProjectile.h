// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MRProjectile.h"
#include "MRAnimNotify_SpawnProjectile.generated.h"

/**
 * 활 발사체를 소켓 위치에서 스폰하는 AnimNotify.
 *
 * 조준(Character.State.Aiming) 여부에 따라 발사 방향을 다르게 계산한다.
 *  - 조준 중: 카메라 → LineTrace → 목표점 방향으로 발사
 *  - 비조준:  캐릭터 전방 벡터를 UnamedLaunchPitchOffset만큼 상향 보정하여 발사
 */
UCLASS()
class MR_API UMRAnimNotify_SpawnProjectile : public UAnimNotify
{
	GENERATED_BODY()

public:
	/** 스폰할 발사체 클래스. BP에서 AMRProjectile 서브클래스 지정. */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AMRProjectile> ProjectileClass;

	/** 발사 소켓 이름. 스켈레탈 메시에 해당 소켓이 있어야 한다. */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	FName SpawnSocketName = TEXT("arrow_start");

	/** 발사체 속도 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (ClampMin = "100.0"))
	float LaunchSpeed = 5000.f;

	/** 발사체 중력 배율 (0 = 직선, 1 = 기본 중력) */
	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float GravityScale = 0.3f;

	/** 비조준 발사 시 전방 벡터를 위로 기울이는 각도 (도) */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	float UnamedLaunchPitchOffset = 10.f;

	/** 조준 모드 LineTrace 최대 거리 (cm) */
	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (ClampMin = "100.0"))
	float AimTraceDistance = 15000.f;

	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
