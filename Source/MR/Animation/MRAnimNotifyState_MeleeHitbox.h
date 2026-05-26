// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MRAnimNotifyState_MeleeHitbox.generated.h"

/**
 * 근접 공격 히트 판정 AnimNotifyState.
 *
 * 구간 동안 매 틱 BoneName 위치에서 SphereOverlap을 수행한다.
 * ASC를 가진 새 타겟이 감지되면 Event.Attack.Hit GameplayEvent를 소유자에게 딱 한 번 발송한다.
 * 중복 히트 방지는 이 클래스가 담당하므로 어빌리티 쪽에 별도 HitActors 셋이 필요 없다.
 *
 * CDO 공유 문제 해결:
 * 같은 몬타주를 여러 인스턴스가 동시 재생해도 MeshComp 키로 인스턴스별 히트 셋을 분리한다.
 */
UCLASS()
class MR_API UMRAnimNotifyState_MeleeHitbox : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	/** 히트 판정 구체 반경 */
	UPROPERTY(EditAnywhere, Category = "MeleeHitbox", meta = (ClampMin = "1.0"))
	float TraceRadius = 80.f;

	/** 히트 판정 기준 본/소켓 이름. None이면 액터 루트 위치 사용 */
	UPROPERTY(EditAnywhere, Category = "MeleeHitbox")
	FName BoneName = NAME_None;


	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

private:
	/** MeshComp별 이미 히트한 타겟 목록. CDO 공유 문제를 해결하기 위한 인스턴스 분리 구조 */
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, TSet<TWeakObjectPtr<AActor>>> HitActorsMap;
};
