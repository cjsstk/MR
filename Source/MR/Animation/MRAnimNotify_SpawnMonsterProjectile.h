// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MRProjectile.h"
#include "MRAnimNotify_SpawnMonsterProjectile.generated.h"

/**
 * 몬스터 발사체를 소켓 위치에서 스폰하는 AnimNotify.
 * AIController의 Blackboard에서 TargetActor를 읽어 해당 방향으로 발사한다.
 * 타겟이 없으면 캐릭터 전방으로 발사한다.
 */
UCLASS()
class MR_API UMRAnimNotify_SpawnMonsterProjectile : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AMRProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	FName SpawnSocketName = TEXT("jaw_01");

	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (ClampMin = "100.0"))
	float LaunchSpeed = 3000.f;

	/** 발사체 중력 배율. 파이어볼은 거의 직선이므로 낮게 설정 */
	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float GravityScale = 0.05f;

	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
