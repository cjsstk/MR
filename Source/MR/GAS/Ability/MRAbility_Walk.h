// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "MRAbility_Walk.generated.h"

class UBlendSpace;

/**
 * 플레이어 이동(걷기/달리기) 어빌리티.
 *
 * 역할:
 *  - 활성화 시 "Character.State.Moving" 태그를 ASC에 부착 → AnimBP가 이동 상태 인식
 *  - 현재 무기 타입에 맞는 LocomotionBlendSpace를 CMS + GameResourceSubsystem으로 비동기 로드
 *  - 로드 완료 후 PlayerCharacter에 BlendSpace 전달 → AnimBP가 이를 사용
 *  - 이동 입력이 끊기면 (MRPlayerCharacter가 CancelAbilitySpec 호출) 태그를 제거하고 종료
 *
 * 애니메이션 연동:
 *  - AnimBP에서 AMRPlayerCharacter::GetLocomotionBlendSpace()를 읽어 사용한다.
 *  - "Character.State.Moving" 태그 유무로 이동/대기 상태를 전환한다.
 */
UCLASS()
class MR_API UMRAbility_Walk : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_Walk();
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	/** 현재 무기 타입에 맞는 LocomotionBlendSpace를 CMS에서 조회 후 비동기 로드 */
	void RequestWeaponAnimLoad();

	/** 로드 완료된 BlendSpace를 캐싱하고 PlayerCharacter에 전달 */
	void ApplyLocomotionBlendSpace(UBlendSpace* BlendSpace);

	UPROPERTY()
	TObjectPtr<UBlendSpace> CachedLocomotionBS;
};
