// Fill out your copyright notice in the Description page of Project Settings.

#include "MRPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Animation/BlendSpace.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Controller.h"
#include "MRAbility_Walk.h"

AMRPlayerCharacter::AMRPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AMRPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController); // InitAbilityActorInfo + DefaultAbilities/Effects 처리

	// Walk 어빌리티는 핸들 관리를 위해 별도 부여
	if (AbilitySystemComponent)
	{
		WalkAbilityHandle = AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(UMRAbility_Walk::StaticClass(), 1, INDEX_NONE, this));
	}
}

void AMRPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// MappingContext 등록
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// 이동 입력 바인딩
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMRPlayerCharacter::OnMoveInputTriggered);
			EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &AMRPlayerCharacter::OnMoveInputCompleted);
		}
	}
}

void AMRPlayerCharacter::SetWeaponType(EMRWeaponType NewWeaponType)
{
	if (CurrentWeaponType == NewWeaponType)
	{
		return;
	}

	CurrentWeaponType = NewWeaponType;
	CurrentLocomotionBS = nullptr;

	// 이동 중이라면 새 무기의 BlendSpace 즉시 재로드
	if (AbilitySystemComponent)
	{
		FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(WalkAbilityHandle);
		if (Spec && Spec->IsActive())
		{
			// 기존 어빌리티 인스턴스 취소 후 재활성화하여 새 무기 애니메이션 로드
			if (UGameplayAbility* Instance = Spec->GetPrimaryInstance())
			{
				AbilitySystemComponent->CancelAbility(Instance);
				AbilitySystemComponent->TryActivateAbility(WalkAbilityHandle);
			}
		}
	}
}

void AMRPlayerCharacter::SetLocomotionBlendSpace(UBlendSpace* BlendSpace)
{
	CurrentLocomotionBS = BlendSpace;
}

void AMRPlayerCharacter::OnMoveInputTriggered(const FInputActionValue& Value)
{
	const FVector2D InputVec = Value.Get<FVector2D>();

	// 컨트롤러 Yaw 기준으로 이동 방향 변환
	if (Controller)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDir, InputVec.Y);
		AddMovementInput(RightDir,   InputVec.X);
	}

	// Walk 어빌리티 활성화 (이미 활성 중이면 TryActivate가 무시됨)
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbility(WalkAbilityHandle);
	}
}

void AMRPlayerCharacter::OnMoveInputCompleted(const FInputActionValue& Value)
{
	// Walk 어빌리티 취소 → EndAbility에서 Moving 태그 제거
	if (AbilitySystemComponent)
	{
		FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(WalkAbilityHandle);
		if (Spec && Spec->IsActive())
		{
			if (UGameplayAbility* Instance = Spec->GetPrimaryInstance())
			{
				AbilitySystemComponent->CancelAbility(Instance);
			}
		}
	}
}
