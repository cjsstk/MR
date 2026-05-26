// Fill out your copyright notice in the Description page of Project Settings.

#include "MRMonsterAnimInstance.h"
#include "MRGameplayTags.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

void UMRMonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const ACharacter* Character = Cast<ACharacter>(GetOwningActor());
	if (!Character)
	{
		return;
	}

	// 이동 방향 계산 (캐릭터 정면 기준 -180~180도)
	const FVector Vel = Character->GetVelocity();
	if (!Vel.IsNearlyZero())
	{
		const FRotator CharRot = Character->GetActorRotation();
		Direction = UKismetMathLibrary::NormalizedDeltaRotator(
			Vel.ToOrientationRotator(), CharRot).Yaw;
	}
	else
	{
		Direction = 0.f;
	}

	bIsFlying    = HasCharacterTag(MRGameplayTags::Character_State_Flying);
	bIsDead      = HasCharacterTag(MRGameplayTags::Character_State_Dead);
	bIsAttacking = HasCharacterTag(MRGameplayTags::Character_State_Attacking);
}
