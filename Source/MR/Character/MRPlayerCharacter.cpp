// Fill out your copyright notice in the Description page of Project Settings.

#include "MRPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Component/MRCharacterMovementComponent.h"
#include "MRInventoryComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "MRAbility_Walk.h"
#include "MRAbility_Sprint.h"
#include "MRAbility_Dodge.h"
#include "MRAbility_Attack.h"
#include "MRAbility_LockOn.h"
#include "GAS/Ability/MRAbility_Gather.h"
#include "Interface/MRGatherable.h"
#include "MRMonster.h"
#include "MRGameplayTags.h"
#include "MRAttributeSetBase.h"
#include "Action/Action.h"
#include "Sugar.h"

AMRPlayerCharacter::AMRPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMRCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	InventoryComponent = ObjectInitializer.CreateDefaultSubobject<UMRInventoryComponent>(this, TEXT("InventoryComponent"));

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
	DefaultCameraOffset = CameraBoom->SocketOffset;
	LinkWeaponAnimLayer(CurrentWeaponType);
}

void AMRPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!AbilitySystemComponent || !CameraBoom)
	{
		return;
	}

	const bool bIsTargeting = AbilitySystemComponent->HasMatchingGameplayTag(MRGameplayTags::Character_State_LockOn)
	                       || AbilitySystemComponent->HasMatchingGameplayTag(MRGameplayTags::Character_State_Aiming);

	const FVector TargetOffset = bIsTargeting ? TargetingCameraOffset : DefaultCameraOffset;
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetOffset, DeltaSeconds, CameraOffsetInterpSpeed);
}

void AMRPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController); // InitAbilityActorInfo + DefaultAbilities/Effects 처리

	// Walk/Sprint 어빌리티는 핸들 관리를 위해 별도 부여
	if (AbilitySystemComponent)
	{
		UClass* WalkClass   = WalkAbilityClass   ? WalkAbilityClass.Get()   : UMRAbility_Walk::StaticClass();
		UClass* SprintClass = SprintAbilityClass ? SprintAbilityClass.Get() : UMRAbility_Sprint::StaticClass();
		UClass* DodgeClass  = DodgeAbilityClass  ? DodgeAbilityClass.Get()  : UMRAbility_Dodge::StaticClass();
		UClass* LockOnClass = LockOnAbilityClass ? LockOnAbilityClass.Get() : UMRAbility_LockOn::StaticClass();
		WalkAbilityHandle   = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(WalkClass,   1, INDEX_NONE, this));
		SprintAbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(SprintClass, 1, INDEX_NONE, this));
		DodgeAbilityHandle  = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DodgeClass,  1, INDEX_NONE, this));
		LockOnAbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(LockOnClass, 1, INDEX_NONE, this));

		if (GatherAbilityClass)
		{
			GatherAbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GatherAbilityClass, 1, INDEX_NONE, this));
		}

		SwapWeaponAbilities(CurrentWeaponType);

		// 초기 체력/스태미나 값을 Store에 반영
		if (const UMRAttributeSetBase* AttrSet = AbilitySystemComponent->GetSet<UMRAttributeSetBase>())
		{
			if (UActionDispatcher* Dispatcher = GetActionDispatcher(this))
			{
				Dispatcher->Dispatch(MakeAction<FAction_SetHealth>(AttrSet->GetHealth(), AttrSet->GetMaxHealth()));
				Dispatcher->Dispatch(MakeAction<FAction_SetStamina>(AttrSet->GetStamina(), AttrSet->GetMaxStamina()));
			}
		}

		// 이후 변경사항 구독
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UMRAttributeSetBase::GetHealthAttribute()).AddUObject(this, &AMRPlayerCharacter::OnHealthChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UMRAttributeSetBase::GetStaminaAttribute()).AddUObject(this, &AMRPlayerCharacter::OnStaminaChanged);
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
		}

		if (LookAction)
		{
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

		if (AttackAction)
		{
			EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &AMRPlayerCharacter::OnAttackInput);
		}

		if (HeavyAttackAction)
		{
			EIC->BindAction(HeavyAttackAction, ETriggerEvent::Started,   this, &AMRPlayerCharacter::OnHeavyAttackStarted);
			EIC->BindAction(HeavyAttackAction, ETriggerEvent::Completed, this, &AMRPlayerCharacter::OnHeavyAttackCompleted);
		}

		if (SpecialAction)
		{
			EIC->BindAction(SpecialAction, ETriggerEvent::Started, this, &AMRPlayerCharacter::OnSpecialInput);
		}

		if (DodgeAction)
		{
			EIC->BindAction(DodgeAction, ETriggerEvent::Started, this, &AMRPlayerCharacter::OnDodgeInput);
		}

		if (LockOnAction)
		{
			EIC->BindAction(LockOnAction, ETriggerEvent::Started, this, &AMRPlayerCharacter::OnLockOnInput);
		}

		if (InteractAction)
		{
			EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AMRPlayerCharacter::OnInteractInput);
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
	LinkWeaponAnimLayer(NewWeaponType);
	SwapWeaponAbilities(NewWeaponType);

	// 활로 전환하면 록온 어빌리티 강제 종료 (활은 Aim 어빌리티가 록온 역할 담당)
	if (NewWeaponType == EMRWeaponType::Bow && AbilitySystemComponent)
	{
		FGameplayAbilitySpec* LockOnSpec = AbilitySystemComponent->FindAbilitySpecFromHandle(LockOnAbilityHandle);
		if (LockOnSpec && LockOnSpec->IsActive())
		{
			AbilitySystemComponent->CancelAbility(LockOnSpec->Ability);
		}
	}
}

FMRPlayerPersistData AMRPlayerCharacter::ExtractPersistData() const
{
	FMRPlayerPersistData Data;
	Data.WeaponType = CurrentWeaponType;

	if (AbilitySystemComponent)
	{
		Data.Health    = AbilitySystemComponent->GetNumericAttribute(UMRAttributeSetBase::GetHealthAttribute());
		Data.MaxHealth = AbilitySystemComponent->GetNumericAttribute(UMRAttributeSetBase::GetMaxHealthAttribute());
		Data.Stamina    = AbilitySystemComponent->GetNumericAttribute(UMRAttributeSetBase::GetStaminaAttribute());
		Data.MaxStamina = AbilitySystemComponent->GetNumericAttribute(UMRAttributeSetBase::GetMaxStaminaAttribute());
	}

	return Data;
}

void AMRPlayerCharacter::ApplyPersistData(const FMRPlayerPersistData& Data)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetNumericAttributeBase(UMRAttributeSetBase::GetHealthAttribute(),    Data.Health);
		AbilitySystemComponent->SetNumericAttributeBase(UMRAttributeSetBase::GetMaxHealthAttribute(), Data.MaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UMRAttributeSetBase::GetStaminaAttribute(),    Data.Stamina);
		AbilitySystemComponent->SetNumericAttributeBase(UMRAttributeSetBase::GetMaxStaminaAttribute(), Data.MaxStamina);
	}

	SetWeaponType(Data.WeaponType);
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
	// 록온 중에는 카메라가 태스크에 의해 자동 제어되므로 마우스 Look 입력 차단
	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(MRGameplayTags::Character_State_LockOn))
	{
		return;
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMRPlayerCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (UActionDispatcher* Dispatcher = GetActionDispatcher(this))
	{
		const float MaxHealth = AbilitySystemComponent->GetNumericAttribute(UMRAttributeSetBase::GetMaxHealthAttribute());
		Dispatcher->Dispatch(MakeAction<FAction_SetHealth>(Data.NewValue, MaxHealth));
	}
}

void AMRPlayerCharacter::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (UActionDispatcher* Dispatcher = GetActionDispatcher(this))
	{
		const float MaxStamina = AbilitySystemComponent->GetNumericAttribute(UMRAttributeSetBase::GetMaxStaminaAttribute());
		Dispatcher->Dispatch(MakeAction<FAction_SetStamina>(Data.NewValue, MaxStamina));
	}
}

void AMRPlayerCharacter::OnAttackInput(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// 활성 중인 공격 어빌리티가 있으면 태그 상태와 무관하게 콤보 버퍼링
	// (방패 공격 시작 후 ShieldMode 태그가 제거되어도 콤보가 유지되어야 함)
	auto TryBuffer = [](UAbilitySystemComponent* ASC, FGameplayAbilitySpecHandle Handle) -> bool
	{
		FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
		if (Spec && Spec->IsActive())
		{
			if (UMRAbility_Attack* Attack = Cast<UMRAbility_Attack>(Spec->GetPrimaryInstance()))
			{
				Attack->BufferComboInput();
			}
			return true;
		}
		return false;
	};

	if (TryBuffer(AbilitySystemComponent, ShieldLightAbilityHandle)) return;
	if (TryBuffer(AbilitySystemComponent, AimedAttackAbilityHandle)) return;
	if (TryBuffer(AbilitySystemComponent, AttackAbilityHandle)) return;

	// 새로 시작
	if (CurrentWeaponType == EMRWeaponType::Bow)
	{
		const bool bAiming = AbilitySystemComponent->HasMatchingGameplayTag(MRGameplayTags::Character_State_Aiming);
		const FGameplayAbilitySpecHandle& BowAttackHandle = (bAiming && AimedAttackAbilityHandle.IsValid())
			? AimedAttackAbilityHandle
			: AttackAbilityHandle;
		AbilitySystemComponent->TryActivateAbility(BowAttackHandle);
	}
	else
	{
		// 한손검/양손검 등: 방패 모드면 방패 약공격, 아니면 일반 약공격
		const bool bShieldMode = AbilitySystemComponent->HasMatchingGameplayTag(MRGameplayTags::Character_State_ShieldMode);
		AbilitySystemComponent->TryActivateAbility(bShieldMode ? ShieldLightAbilityHandle : AttackAbilityHandle);
	}
}

void AMRPlayerCharacter::OnHeavyAttackStarted(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	auto TryBuffer = [](UAbilitySystemComponent* ASC, FGameplayAbilitySpecHandle Handle) -> bool
	{
		FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
		if (Spec && Spec->IsActive())
		{
			if (UMRAbility_Attack* Attack = Cast<UMRAbility_Attack>(Spec->GetPrimaryInstance()))
			{
				Attack->BufferComboInput();
			}
			return true;
		}
		return false;
	};

	if (CurrentWeaponType == EMRWeaponType::Bow)
	{
		// 활: TryBuffer/ShieldMode 분기 없이 바로 조준 어빌리티 활성화
		AbilitySystemComponent->TryActivateAbility(HeavyAttackAbilityHandle);
		return;
	}

	if (TryBuffer(AbilitySystemComponent, ShieldHeavyAbilityHandle)) return;
	if (TryBuffer(AbilitySystemComponent, HeavyAttackAbilityHandle)) return;

	// 새로 시작 — 방패 모드면 방패 강공격, 아니면 일반 강공격
	const bool bShieldMode = AbilitySystemComponent->HasMatchingGameplayTag(MRGameplayTags::Character_State_ShieldMode);
	AbilitySystemComponent->TryActivateAbility(bShieldMode ? ShieldHeavyAbilityHandle : HeavyAttackAbilityHandle);
}

void AMRPlayerCharacter::OnHeavyAttackCompleted(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// 활만 RMB 홀드 방식 — 다른 무기는 Completed 이벤트 무시
	if (CurrentWeaponType != EMRWeaponType::Bow)
	{
		return;
	}

	// 조준 어빌리티 취소
	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(HeavyAttackAbilityHandle);
	if (Spec && Spec->IsActive())
	{
		AbilitySystemComponent->CancelAbility(Spec->Ability);
	}
}

void AMRPlayerCharacter::OnSpecialInput(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(SpecialAbilityHandle);
	if (!Spec)
	{
		return;
	}

	if (Spec->IsActive())
	{
		// 이미 활성 중(방패 모드 등)이면 취소하여 토글 오프
		AbilitySystemComponent->CancelAbility(Spec->Ability);
	}
	else
	{
		AbilitySystemComponent->TryActivateAbility(SpecialAbilityHandle);
	}
}

void AMRPlayerCharacter::SwapWeaponAbilities(EMRWeaponType WeaponType)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	auto ClearHandle = [this](FGameplayAbilitySpecHandle& Handle)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->ClearAbility(Handle);
			Handle = FGameplayAbilitySpecHandle();
		}
	};

	auto GiveAbility = [this](FGameplayAbilitySpecHandle& Handle, UClass* AbilityClass)
	{
		if (AbilityClass)
		{
			Handle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
		}
	};

	ClearHandle(AttackAbilityHandle);
	ClearHandle(AimedAttackAbilityHandle);
	ClearHandle(HeavyAttackAbilityHandle);
	ClearHandle(ShieldLightAbilityHandle);
	ClearHandle(ShieldHeavyAbilityHandle);
	ClearHandle(SpecialAbilityHandle);

	const FMRWeaponAbilityConfig* Config = WeaponConfigs.Find(WeaponType);
	if (!Config)
	{
		return;
	}

	GiveAbility(AttackAbilityHandle,      Config->LightAttackClass.Get());
	GiveAbility(AimedAttackAbilityHandle, Config->AimedAttackClass.Get());
	GiveAbility(HeavyAttackAbilityHandle, Config->HeavyAttackClass.Get());
	GiveAbility(ShieldLightAbilityHandle, Config->ShieldLightClass.Get());
	GiveAbility(ShieldHeavyAbilityHandle, Config->ShieldHeavyClass.Get());
	GiveAbility(SpecialAbilityHandle,     Config->SpecialClass.Get());
}

void AMRPlayerCharacter::OnDodgeInput(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbility(DodgeAbilityHandle);
	}
}

void AMRPlayerCharacter::OnLockOnInput(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(LockOnAbilityHandle);
	if (!Spec)
	{
		return;
	}

	// 이미 록온 중이면 취소(토글 오프), 아니면 활성화(토글 온)
	if (Spec->IsActive())
	{
		AbilitySystemComponent->CancelAbility(Spec->Ability);
	}
	else
	{
		AbilitySystemComponent->TryActivateAbility(LockOnAbilityHandle);
	}
}

void AMRPlayerCharacter::LinkWeaponAnimLayer(EMRWeaponType WeaponType)
{
	const FMRWeaponAbilityConfig* Config = WeaponConfigs.Find(WeaponType);
	if (Config && Config->AnimLayerClass)
	{
		GetMesh()->LinkAnimClassLayers(Config->AnimLayerClass);
	}
}

void AMRPlayerCharacter::OnInteractInput(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	IMRGatherable* Gatherable = GetNearestGatherable();
	if (!Gatherable || !Gatherable->CanBeGathered())
	{
		return;
	}

	// 어빌리티 인스턴스에 채집 대상을 전달한 뒤 활성화
	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(GatherAbilityHandle);
	if (Spec)
	{
		if (UMRAbility_Gather* GatherAbility = Cast<UMRAbility_Gather>(Spec->GetPrimaryInstance()))
		{
			GatherAbility->SetTargetGatherable(NearestGatherable.Get());
		}
	}

	AbilitySystemComponent->TryActivateAbility(GatherAbilityHandle);
}

void AMRPlayerCharacter::SetGatherable(AActor* Gatherable)
{
	NearestGatherable = Gatherable;
}

void AMRPlayerCharacter::ClearGatherable(AActor* Gatherable)
{
	// 현재 대상과 같을 때만 클리어 (다른 대상 오버랩 종료로 유효한 대상을 지우지 않도록)
	if (NearestGatherable.Get() == Gatherable)
	{
		NearestGatherable = nullptr;
	}
}
