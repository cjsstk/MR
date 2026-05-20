// Fill out your copyright notice in the Description page of Project Settings.

#include "MRPlayerAnimInstance.h"
#include "MRGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

void UMRPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ACharacter* Owner = Cast<ACharacter>(GetOwningActor());
	if (!Owner)
	{
		return;
	}

	bIsTargeting = HasCharacterTag(MRGameplayTags::Character_State_LockOn)
	            || HasCharacterTag(MRGameplayTags::Character_State_Aiming);

	const UCharacterMovementComponent* MovComp = Owner->GetCharacterMovement();
	const float MaxSpeed = MovComp->MaxWalkSpeed;
	if (MaxSpeed <= 0.f)
	{
		VelocityForward = 0.f;
		VelocityRight   = 0.f;
		return;
	}

	// 월드 속도를 캐릭터 로컬 좌표로 변환
	const FVector LocalVel = Owner->GetActorTransform().InverseTransformVector(MovComp->Velocity);
	const FVector2D Vel2D(LocalVel.X, LocalVel.Y);
	const float Len2D = Vel2D.Size();

	if (Len2D > KINDA_SMALL_NUMBER)
	{
		// L∞ 정규화: 대각선 이동 시에도 주축 성분이 100에 도달하도록 스케일
		// 예) 대각선 Dir=(0.707, 0.707) → InfNorm=0.707 → ScaledDir=(1, 1)
		const FVector2D Dir = Vel2D / Len2D;
		const float InfNorm = FMath::Max(FMath::Abs(Dir.X), FMath::Abs(Dir.Y));
		const float SpeedRatio = FMath::Clamp(Len2D / MaxSpeed, 0.f, 1.f);
		VelocityForward = Dir.X / InfNorm * SpeedRatio * 100.f;
		VelocityRight   = Dir.Y / InfNorm * SpeedRatio * 100.f;
	}
	else
	{
		VelocityForward = 0.f;
		VelocityRight   = 0.f;
	}
	
	// 에임 오프셋: 컨트롤러 회전 기준 상하(Pitch) / 좌우(Yaw) 이탈각
	if (APlayerController* PC = Owner->GetController<APlayerController>())
	{
		const FRotator ControlRot = PC->GetControlRotation();
		const FRotator ActorRot   = Owner->GetActorRotation();
		AimPitch = FMath::ClampAngle(FRotator::NormalizeAxis(ControlRot.Pitch), -90.f, 90.f);
		AimYaw   = FMath::ClampAngle(FRotator::NormalizeAxis(ControlRot.Yaw - ActorRot.Yaw), -90.f, 90.f);
	}

	//UE_LOG(LogTemp, Warning, TEXT("VelocityForward = %.1f, VelocityRight = %.1f"), VelocityForward, VelocityRight);
	//UE_LOG(LogTemp, Warning, TEXT("AimPitch = %.1f, AimYaw = %.1f"), AimPitch, AimYaw);
}
