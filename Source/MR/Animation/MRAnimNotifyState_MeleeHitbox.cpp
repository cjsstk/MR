// Fill out your copyright notice in the Description page of Project Settings.

#include "MRAnimNotifyState_MeleeHitbox.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MRGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"

#if ENABLE_DRAW_DEBUG
static TAutoConsoleVariable<int32> CVarMeleeHitboxDebug(
	TEXT("mr.MeleeHitboxDebug"),
	0,
	TEXT("근접 공격 히트박스 디버그 표시 (0=꺼짐, 1=켜짐). 초록=판정 구간, 빨강=히트 순간"),
	ECVF_Default
);
#endif

void UMRAnimNotifyState_MeleeHitbox::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// 이 스윙의 히트 셋을 초기화. 이전 NotifyEnd가 호출되지 않은 경우에도 안전하게 리셋
	HitActorsMap.FindOrAdd(MeshComp).Reset();
}

void UMRAnimNotifyState_MeleeHitbox::NotifyTick(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner || !Owner->GetWorld() || !Cast<IAbilitySystemInterface>(Owner))
	{
		return;
	}

	FVector TraceOrigin;
	if (!BoneName.IsNone() && MeshComp->DoesSocketExist(BoneName))
	{
		TraceOrigin = MeshComp->GetSocketLocation(BoneName);
	}
	else
	{
		TraceOrigin = Owner->GetActorLocation();
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	Owner->GetWorld()->OverlapMultiByChannel(
		Overlaps,
		TraceOrigin,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(TraceRadius),
		Params
	);

	TSet<TWeakObjectPtr<AActor>>& HitActors = HitActorsMap.FindOrAdd(MeshComp);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || !Cast<IAbilitySystemInterface>(HitActor))
		{
			continue;
		}

		// 이 스윙 윈도우에서 이미 히트한 타겟은 스킵
		if (HitActors.Contains(HitActor))
		{
			continue;
		}

		HitActors.Add(HitActor);

		FGameplayEventData EventData;
		EventData.Instigator = Owner;
		EventData.Target     = HitActor;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			Owner,
			MRGameplayTags::Event_Attack_Hit,
			EventData
		);

#if ENABLE_DRAW_DEBUG
		if (CVarMeleeHitboxDebug.GetValueOnGameThread() != 0)
		{
			// 히트 순간: 빨강으로 잠깐 표시
			DrawDebugSphere(Owner->GetWorld(), TraceOrigin, TraceRadius, 12,
				FColor::Red, false, 0.2f);
		}
#endif
	}

#if ENABLE_DRAW_DEBUG
	if (CVarMeleeHitboxDebug.GetValueOnGameThread() != 0)
	{
		// 판정 구간 전체: 초록으로 매 틱 표시 (다음 틱까지 유지)
		DrawDebugSphere(Owner->GetWorld(), TraceOrigin, TraceRadius, 12,
			FColor::Green, false, FrameDeltaTime * 1.5f);
	}
#endif
}

void UMRAnimNotifyState_MeleeHitbox::NotifyEnd(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	HitActorsMap.Remove(MeshComp);
}
