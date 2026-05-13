#include "MRAbility_LockOn.h"
#include "MRAbilityTask_LockOnTick.h"
#include "MRGameplayTags.h"
#include "MRPlayerCharacter.h"
#include "MRMonster.h"
#include "MREnum.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"

UMRAbility_LockOn::UMRAbility_LockOn()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(MRGameplayTags::Ability_LockOn);
	SetAssetTags(AssetTags);

	// 사망 중, 활 조준 중에는 록온 불가
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Dead);
	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Aiming);

	// 록온 시작 시 스프린트 중단
	CancelAbilitiesWithTag.AddTag(MRGameplayTags::Ability_Sprint);

	InstancingPolicy  = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UMRAbility_LockOn::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 활 무기는 록온 불가 — 조준(Aim) 어빌리티가 담당
	AMRPlayerCharacter* Player = GetPlayerCharacter();
	if (!Player)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (Player->GetWeaponType() == EMRWeaponType::Bow)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 탐색 범위 내 최적 대상 선택
	AActor* Target = FindBestTarget();
	if (!Target)
	{
		UE_LOG(LogTemp, Log, TEXT("MRAbility_LockOn: 록온 가능한 대상이 없습니다."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC)
	{
		ASC->AddLooseGameplayTag(MRGameplayTags::Character_State_LockOn);
	}

	// 록온 중에는 캐릭터가 카메라 방향을 바라보도록 회전 설정 변경
	UCharacterMovementComponent* MovComp = Player->GetCharacterMovement();
	OriginalMaxWalkSpeed = MovComp->MaxWalkSpeed;
	MovComp->MaxWalkSpeed = LockOnMoveSpeed;
	Player->bUseControllerRotationYaw = true;
	MovComp->bOrientRotationToMovement = false;

	// 매 틱 카메라 보간 태스크 시작
	LockOnTask = UMRAbilityTask_LockOnTick::CreateTask(this, Target, MaxLockOnDistance, CameraInterpSpeed, LockOnTargetHeightOffset);
	LockOnTask->OnTargetLost.AddDynamic(this, &UMRAbility_LockOn::OnTargetLost);
	LockOnTask->ReadyForActivation();

	UE_LOG(LogTemp, Log, TEXT("MRAbility_LockOn: [%s] 록온 시작"), *Target->GetName());
}

void UMRAbility_LockOn::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 록온 상태 태그 제거
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTag(MRGameplayTags::Character_State_LockOn);
	}

	// 회전 설정 및 이동속도 복원
	if (AMRPlayerCharacter* Player = GetPlayerCharacter())
	{
		Player->bUseControllerRotationYaw = false;
		UCharacterMovementComponent* MovComp = Player->GetCharacterMovement();
		MovComp->bOrientRotationToMovement = true;
		MovComp->MaxWalkSpeed = OriginalMaxWalkSpeed;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

AActor* UMRAbility_LockOn::FindBestTarget() const
{
	AMRPlayerCharacter* Player = GetPlayerCharacter();
	if (!Player)
	{
		return nullptr;
	}

	UCameraComponent* Camera = Player->GetFollowCamera();
	if (!Camera)
	{
		return nullptr;
	}

	const FVector PlayerLocation  = Player->GetActorLocation();
	const FVector CameraForward   = Camera->GetForwardVector();
	const float   MaxDistSq       = FMath::Square(MaxLockOnDistance);

	AActor* BestTarget  = nullptr;
	float   BestScore   = -1.f;

	UWorld* World = Player->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AMRMonster> It(World); It; ++It)
	{
		AMRMonster* Monster = *It;
		if (!IsValid(Monster))
		{
			continue;
		}

		// 사망한 몬스터 제외
		if (UAbilitySystemComponent* MonsterASC = Monster->GetAbilitySystemComponent())
		{
			if (MonsterASC->HasMatchingGameplayTag(MRGameplayTags::Character_State_Dead))
			{
				continue;
			}
		}

		// 거리 필터
		const FVector ToMonster = Monster->GetActorLocation() - PlayerLocation;
		const float   DistSq   = ToMonster.SizeSquared();
		if (DistSq > MaxDistSq)
		{
			continue;
		}

		// 카메라 정면 기준 각도 필터
		const FVector ToMonsterNorm = ToMonster.GetSafeNormal();
		const float   DotProduct    = FVector::DotProduct(CameraForward, ToMonsterNorm);
		const float   AngleDeg      = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.f, 1.f)));
		if (AngleDeg > MaxTargetAngle)
		{
			continue;
		}

		// 스코어: 각도가 작을수록, 거리가 가까울수록 높은 점수
		// AngleScore: 0(MaxTargetAngle) ~ 1(정면)
		// DistanceScore: 0(MaxLockOnDistance) ~ 1(플레이어 위치)
		const float AngleScore    = 1.f - (AngleDeg / MaxTargetAngle);
		const float DistanceScore = 1.f - (FMath::Sqrt(DistSq) / MaxLockOnDistance);
		const float Score         = 0.7f * AngleScore + 0.3f * DistanceScore;

		if (Score > BestScore)
		{
			BestScore  = Score;
			BestTarget = Monster;
		}
	}

	return BestTarget;
}

void UMRAbility_LockOn::OnTargetLost()
{
	UE_LOG(LogTemp, Log, TEXT("MRAbility_LockOn: 록온 대상 소실 — 어빌리티 종료"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
