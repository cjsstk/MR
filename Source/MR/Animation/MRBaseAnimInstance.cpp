// Fill out your copyright notice in the Description page of Project Settings.

#include "MRBaseAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMRBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<ACharacter>(GetOwningActor());
}

void UMRBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter.IsValid())
	{
		return;
	}

	const UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
	const float TargetSpeed = Movement->Velocity.Size2D();
	Speed = FMath::FInterpTo(Speed, TargetSpeed, DeltaSeconds, SpeedInterpSpeed);
	bIsFalling = Movement->IsFalling();
}
