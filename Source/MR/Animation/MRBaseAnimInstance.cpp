// Fill out your copyright notice in the Description page of Project Settings.

#include "MRBaseAnimInstance.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UMRBaseAnimInstance::HasCharacterTag(FGameplayTag Tag) const
{
	return CachedASC && CachedASC->HasMatchingGameplayTag(Tag);
}

void UMRBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<ACharacter>(GetOwningActor());

	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwningActor()))
	{
		CachedASC = ASI->GetAbilitySystemComponent();
	}
}

void UMRBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter.IsValid())
	{
		return;
	}

	const UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
	Velocity = Movement->Velocity;
	TargetSpeed = Velocity.Size2D();
	Speed = FMath::FInterpTo(Speed, TargetSpeed, DeltaSeconds, SpeedInterpSpeed);
	bIsFalling = Movement->IsFalling();
	bShouldMove = !Movement->GetCurrentAcceleration().IsNearlyZero() && Speed > 3.0f;
}
