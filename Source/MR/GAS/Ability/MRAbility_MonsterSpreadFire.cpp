// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAbility_MonsterSpreadFire.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Actor.h"
#include "MRGameplayTags.h"
#include "MRAttributeSetBase.h"
#include "MREffect_AttackDamage.h"
#include "Engine/OverlapResult.h"

UMRAbility_MonsterSpreadFire::UMRAbility_MonsterSpreadFire()
{
	// 어빌리티 식별 태그
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(MRGameplayTags::Ability_Monster_SpreadFire);
	SetAssetTags(AssetTags);

	// 어빌리티 활성 중 Character.State.Attacking 태그 자동 부여/해제
	ActivationOwnedTags.AddTag(MRGameplayTags::Character_State_Attacking);

	ActivationBlockedTags.AddTag(MRGameplayTags::Character_State_Dead);

	DamageEffectClass = UMREffect_AttackDamage::StaticClass();
}

void UMRAbility_MonsterSpreadFire::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterSpreadFire] AttackMontage is not set. Cancelling."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, AttackMontage, PlayRate);

	MontageTask->OnCompleted.AddDynamic(this, &UMRAbility_MonsterSpreadFire::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UMRAbility_MonsterSpreadFire::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UMRAbility_MonsterSpreadFire::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UMRAbility_MonsterSpreadFire::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	// DamageStartDelay 후 반복 데미지 타이머 시작
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			DamageDelayTimer,
			this,
			&UMRAbility_MonsterSpreadFire::StartDamageTimer,
			DamageStartDelay,
			false);
	}
}

void UMRAbility_MonsterSpreadFire::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(DamageDelayTimer);
		World->GetTimerManager().ClearTimer(DamageTickTimer);
	}

	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMRAbility_MonsterSpreadFire::OnMontageCompleted()
{
	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UMRAbility_MonsterSpreadFire::OnMontageCancelled()
{
	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UMRAbility_MonsterSpreadFire::StartDamageTimer()
{
	// 즉시 첫 번째 데미지 판정 후 DamageTickInterval마다 반복
	ApplyConeDamage();

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			DamageTickTimer,
			this,
			&UMRAbility_MonsterSpreadFire::ApplyConeDamage,
			DamageTickInterval,
			true);
	}
}

void UMRAbility_MonsterSpreadFire::ApplyConeDamage()
{
	AActor* Owner = GetAvatarActorFromActorInfo();
	if (!Owner)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC || !DamageEffectClass)
	{
		return;
	}

	const UMRAttributeSetBase* AttrSet = SourceASC->GetSet<UMRAttributeSetBase>();
	const float AttackPower = AttrSet ? AttrSet->GetAttackPower() : 1.f;
	const float FinalDamage = -(AttackPower * DamageMultiplier);

	// 콘 범위 계산을 위한 방향/각도 사전 계산
	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector Forward = Owner->GetActorForwardVector();
	const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(ConeHalfAngleDeg));

	// 구 형태로 주변 Pawn 오버랩 감지 후 콘 내부 필터링
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	UWorld* World = Owner->GetWorld();
	if (!World)
	{
		return;
	}

	World->OverlapMultiByChannel(
		Overlaps,
		OwnerLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(ConeRange),
		QueryParams);

	int32 HitCount = 0;
	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor)
		{
			continue;
		}

		// 콘 범위 내에 있는지 내적으로 확인
		const FVector ToTarget = HitActor->GetActorLocation() - OwnerLocation;
		if (ToTarget.IsNearlyZero())
		{
			continue;
		}

		const float DotProduct = FVector::DotProduct(Forward, ToTarget.GetSafeNormal());
		if (DotProduct < CosHalfAngle)
		{
			continue;
		}

		// 대상이 ASC를 보유하는지 확인
		IAbilitySystemInterface* TargetASCOwner = Cast<IAbilitySystemInterface>(HitActor);
		if (!TargetASCOwner)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = TargetASCOwner->GetAbilitySystemComponent();
		if (!TargetASC)
		{
			continue;
		}

		// 데미지 GE 적용 (기존 MonsterAttack 패턴과 동일)
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(this);

		FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_Damage, FinalDamage);
			SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
			++HitCount;
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("[MonsterSpreadFire] ConeDamage applied to %d targets. Damage=%.1f"),
		HitCount, FinalDamage);
}
