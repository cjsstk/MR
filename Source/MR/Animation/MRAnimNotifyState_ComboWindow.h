// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MRAnimNotifyState_ComboWindow.generated.h"

class UMRAbility_Attack;

/**
 * 콤보 입력 수신 가능 구간을 정의하는 AnimNotifyState.
 * 몽타주의 원하는 구간에 배치하면 Open/CloseComboWindow가 자동 호출된다.
 *
 * 사용법:
 *   각 콤보 몽타주에서 '다음 입력을 받을 수 있는 구간'에 이 NotifyState를 배치한다.
 */
UCLASS()
class MR_API UMRAnimNotifyState_ComboWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

private:
	/** MeshComp의 Owner에서 현재 활성화된 UMRAbility_Attack 인스턴스를 탐색 */
	UMRAbility_Attack* FindAttackAbility(USkeletalMeshComponent* MeshComp) const;
};
