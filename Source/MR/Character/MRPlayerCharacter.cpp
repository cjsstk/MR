// Fill out your copyright notice in the Description page of Project Settings.

#include "MRPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Animation/BlendSpace.h"
#include "Camera/CameraComponent.h"
#include "Component/MRCharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "MRAbility_Walk.h"
#include "MRAbility_Sprint.h"
#include "MRPlayerAnimInstance.h"
#include "GameResourceSubsystem.h"
#include "Animation/BlendSpace.h"
#include "Sugar.h"

AMRPlayerCharacter::AMRPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMRCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	GetCharacterMovement<UMRCharacterMovementComponent>()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
}

void AMRPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	LoadAndApplyWeaponAnims(CurrentWeaponType);
}

void AMRPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController); // InitAbilityActorInfo + DefaultAbilities/Effects 처리

	// Walk/Sprint 어빌리티는 핸들 관리를 위해 별도 부여
	if (AbilitySystemComponent)
	{
		WalkAbilityHandle = AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(UMRAbility_Walk::StaticClass(), 1, INDEX_NONE, this));
		SprintAbilityHandle = AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(UMRAbility_Sprint::StaticClass(), 1, INDEX_NONE, this));
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

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMRPlayerCharacter::OnMoveInputTriggered);
			EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &AMRPlayerCharacter::OnMoveInputCompleted);
			
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMRPlayerCharacter::OnLook);
		}

		if (JumpAction)
		{
			EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		
		if (SprintAction)
		{
			EIC->BindAction(SprintAction, ETriggerEvent::Started,    this, &AMRPlayerCharacter::OnSprintStarted);
			EIC->BindAction(SprintAction, ETriggerEvent::Completed,  this, &AMRPlayerCharacter::OnSprintCompleted);
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
	LoadAndApplyWeaponAnims(NewWeaponType);
}

void AMRPlayerCharacter::LoadAndApplyWeaponAnims(EMRWeaponType WeaponType)
{
	UMRGameResource* GameRes = GetGameResource(this);
	if (!GameRes)
	{
		return;
	}

	TWeakObjectPtr<AMRPlayerCharacter> WeakThis(this);

	// Idle과 LocomotionBS를 각각 비동기 로드하여 AnimInstance에 바로 반영
	GameRes->AsyncLoadWeaponIdleAnim(WeaponType, [WeakThis](UAnimSequence* Idle)
	{
		if (!WeakThis.IsValid()) return;
		if (UMRPlayerAnimInstance* Anim = Cast<UMRPlayerAnimInstance>(WeakThis->GetMesh()->GetAnimInstance()))
		{
			Anim->IdleAnimation = Idle;
		}
	});

	GameRes->AsyncLoadWeaponLocomotionBS(WeaponType, [WeakThis](UBlendSpace* BS)
	{
		if (!WeakThis.IsValid()) return;
		if (UMRPlayerAnimInstance* Anim = Cast<UMRPlayerAnimInstance>(WeakThis->GetMesh()->GetAnimInstance()))
		{
			Anim->LocomotionBlendSpace = BS;
		}
	});

	GameRes->AsyncLoadWeaponJumpAnims(WeaponType, [WeakThis](FWeaponJumpAnims Anims)
	{
		if (!WeakThis.IsValid()) return;
		if (UMRPlayerAnimInstance* Anim = Cast<UMRPlayerAnimInstance>(WeakThis->GetMesh()->GetAnimInstance()))
		{
			Anim->JumpStartAnimation = Anims.Start;
			Anim->JumpLoopAnimation  = Anims.Loop;
			Anim->JumpEndAnimation   = Anims.End;
		}
	});
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
			// Spec->Ability는 CDO이므로 CancelAbility가 올바르게 매칭함
			AbilitySystemComponent->CancelAbility(Spec->Ability);
		}
	}
}

void AMRPlayerCharacter::OnSprintStarted(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbility(SprintAbilityHandle);
	}
}

void AMRPlayerCharacter::OnSprintCompleted(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(SprintAbilityHandle);
		if (Spec && Spec->IsActive())
		{
			// Spec->Ability는 CDO이므로 CancelAbility가 올바르게 매칭함
			AbilitySystemComponent->CancelAbility(Spec->Ability);
		}
	}
}

void AMRPlayerCharacter::OnLook(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
