// Fill out your copyright notice in the Description page of Project Settings.

#include "MRPlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMRPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ACharacter* Owner = Cast<ACharacter>(GetOwningActor());
	if (!Owner)
	{
		return;
	}

	const UCharacterMovementComponent* MovComp = Owner->GetCharacterMovement();
	const float MaxSpeed = MovComp->MaxWalkSpeed;
	if (MaxSpeed <= 0.f)
	{
		VelocityForward = 0.f;
		VelocityRight   = 0.f;
		return;
	}

	// 월드 속도를 캐릭터 로컬 좌표로 변환 후 MaxWalkSpeed 기준 정규화
	const FVector LocalVel = Owner->GetActorTransform().InverseTransformVector(MovComp->Velocity);
	VelocityForward = FMath::Clamp(LocalVel.X / MaxSpeed, -1.f, 1.f);
	VelocityRight   = FMath::Clamp(LocalVel.Y / MaxSpeed, -1.f, 1.f);
}
