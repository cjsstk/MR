// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRBaseCharacter.h"
#include "MREnum.h"
#include "InputActionValue.h"
#include "AbilitySystemComponent.h"
#include "Travel/MRTravelTypes.h"
#include "MRPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UMRGameplayAbility;
class UMRAbility_Attack;
class UMRAbility_Dodge;

/** 무기 타입 하나에 필요한 모든 어빌리티·애니메이션 설정. BP에서 WeaponConfigs TMap에 등록한다. */
USTRUCT(BlueprintType)
struct FMRWeaponAbilityConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMRGameplayAbility> LightAttackClass;

	/** 한손검/양손검: HeavyAttack 어빌리티, 활: Aim 어빌리티 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMRGameplayAbility> HeavyAttackClass;

	/** 방패 모드 약공격 (한손검 전용, 없으면 무시) */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMRGameplayAbility> ShieldLightClass;

	/** 방패 모드 강공격 (한손검 전용, 없으면 무시) */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMRGameplayAbility> ShieldHeavyClass;

	/** 조준 중 공격 (활 전용). 미설정 시 LightAttackClass 사용. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMRGameplayAbility> AimedAttackClass;

	/** 특수 액션: 한손검=ShieldMode 토글, 활=근접 공격 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMRGameplayAbility> SpecialClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAnimInstance> AnimLayerClass;
};

/**
 * 플레이어가 조작하는 캐릭터.
 *
 * 이동 흐름:
 *  1. Enhanced Input (MoveAction) Triggered → OnMoveInputTriggered
 *  2. AddMovementInput 호출 + WalkAbility 활성화 ("Character.State.Moving" 태그 관리)
 *  3. Completed → OnMoveInputCompleted → WalkAbility Cancel
 *
 * 애니메이션 연동:
 *  - BeginPlay / SetWeaponType에서 LinkWeaponAnimLayer 호출
 *  - 무기별 Linked Anim Layer(ABP_OneHandedSword 등)가 Idle/Locomotion/Jump 담당
 */
UCLASS()
class MR_API AMRPlayerCharacter : public AMRBaseCharacter
{
	GENERATED_BODY()

public:
	AMRPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ─── 무기 ──────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	EMRWeaponType GetWeaponType() const { return CurrentWeaponType; }

	/** 록온 태스크에서 카메라 방향 계산에 사용 */
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** 무기 타입 변경. 이동 중이면 즉시 새 BlendSpace 로드를 트리거한다. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWeaponType(EMRWeaponType NewWeaponType);

	/** 현재 상태를 FMRPlayerPersistData로 추출한다. TravelSubsystem에서 저장용으로 호출. */
	FMRPlayerPersistData ExtractPersistData() const;

	/** FMRPlayerPersistData를 적용해 레벨 이동 후 상태를 복원한다. */
	void ApplyPersistData(const FMRPlayerPersistData& Data);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** 타겟팅(록온/조준) 중 카메라 오프셋. 양수 Y = 카메라 오른쪽 이동 → 캐릭터 화면 왼쪽 배치. */
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Targeting")
	FVector TargetingCameraOffset = FVector(0.f, 80.f, 0.f);

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Targeting")
	float CameraOffsetInterpSpeed = 8.f;

	// ─── 입력 에셋 (Blueprint에서 설정) ────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	/** 무기의 특수 키(R). 한손검: 방패 토글, 활: 근접 공격 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SpecialAction;

	/** RMB. 한손검/양손검: 강공격, 활: 조준(홀드) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> HeavyAttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DodgeAction;

	/** Tab 키 등 — 록온 대상 토글 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LockOnAction;

	/** 무기 타입별 어빌리티·애니메이션 설정. BP에서 각 EMRWeaponType 키에 맞춰 등록. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TMap<EMRWeaponType, FMRWeaponAbilityConfig> WeaponConfigs;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

private:
	void OnMoveInputTriggered(const FInputActionValue& Value);
	void OnMoveInputCompleted(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnSprintStarted(const FInputActionValue& Value);
	void OnSprintCompleted(const FInputActionValue& Value);
	void OnAttackInput(const FInputActionValue& Value);

	/** RMB 입력 — 한손검/양손검: 강공격 콤보, 활: 조준 시작 */
	void OnHeavyAttackStarted(const FInputActionValue& Value);

	/** RMB 해제 — 활: 조준 종료 */
	void OnHeavyAttackCompleted(const FInputActionValue& Value);

	/** R 키 입력 — 무기 타입에 따라 방패 토글 또는 근접 공격 */
	void OnSpecialInput(const FInputActionValue& Value);

	void OnDodgeInput(const FInputActionValue& Value);
	void OnLockOnInput(const FInputActionValue& Value);

	/** WeaponConfigs에서 해당 무기 타입 설정을 읽어 모든 어빌리티 핸들을 교체 */
	void SwapWeaponAbilities(EMRWeaponType WeaponType);

	/** WeaponConfigs에서 AnimLayerClass를 읽어 메시에 연결 */
	void LinkWeaponAnimLayer(EMRWeaponType WeaponType);

	/** ASC 어트리뷰트 변경 → Store 동기화 */
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnStaminaChanged(const FOnAttributeChangeData& Data);

	FVector DefaultCameraOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	EMRWeaponType CurrentWeaponType = EMRWeaponType::OneHandedSword;

	/** BP에서 지정할 Walk 어빌리티 클래스 (미지정 시 C++ 기본값 사용) */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UMRGameplayAbility> WalkAbilityClass;

	/** BP에서 지정할 Sprint 어빌리티 클래스 (미지정 시 C++ 기본값 사용) */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UMRGameplayAbility> SprintAbilityClass;

	/** BP에서 지정할 Dodge 어빌리티 클래스 (미지정 시 C++ 기본값 사용) */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UMRAbility_Dodge> DodgeAbilityClass;

	/** BP에서 지정할 LockOn 어빌리티 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UMRGameplayAbility> LockOnAbilityClass;

	FGameplayAbilitySpecHandle WalkAbilityHandle;
	FGameplayAbilitySpecHandle SprintAbilityHandle;
	FGameplayAbilitySpecHandle DodgeAbilityHandle;
	FGameplayAbilitySpecHandle LockOnAbilityHandle;

	/** 현재 무기 타입에 해당하는 AttackAbility 스펙 핸들 */
	FGameplayAbilitySpecHandle AttackAbilityHandle;

	/** 활 조준 공격 스펙 핸들 (활 전용) */
	FGameplayAbilitySpecHandle AimedAttackAbilityHandle;

	/** 현재 무기 타입에 해당하는 HeavyAttackAbility 스펙 핸들 */
	FGameplayAbilitySpecHandle HeavyAttackAbilityHandle;

	/** 방패 모드 약공격 스펙 핸들 */
	FGameplayAbilitySpecHandle ShieldLightAbilityHandle;

	/** 방패 모드 강공격 스펙 핸들 */
	FGameplayAbilitySpecHandle ShieldHeavyAbilityHandle;

	/** 현재 무기 타입에 해당하는 SpecialAbility 스펙 핸들 */
	FGameplayAbilitySpecHandle SpecialAbilityHandle;
};
