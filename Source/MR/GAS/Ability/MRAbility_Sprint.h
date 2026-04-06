// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRGameplayAbility.h"
#include "MRAbility_Sprint.generated.h"

/**
 * 스프린트 어빌리티.
 * 입력을 누르는 동안 MaxWalkSpeed를 높이고 "Character.State.Sprinting" 태그를 유지한다.
 * AnimBP는 이 태그로 이동 애니메이션을 전환할 수 있다.
 */
UCLASS()
class MR_API UMRAbility_Sprint : public UMRGameplayAbility
{
	GENERATED_BODY()

public:
	UMRAbility_Sprint();

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

	/** 스프린트 중 MaxWalkSpeed 값 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	float SprintSpeed = 900.f;

private:
	/** 스프린트 전 원래 속도 저장 (EndAbility에서 복원) */
	float OriginalMaxWalkSpeed = 0.f;
};
