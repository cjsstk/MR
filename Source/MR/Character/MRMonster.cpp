// Fill out your copyright notice in the Description page of Project Settings.

#include "MRMonster.h"
#include "AbilitySystemComponent.h"

AMRMonster::AMRMonster(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 월드에 배치되거나 런타임에 스폰될 때 자동으로 AI 컨트롤러가 빙의
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AMRMonster::BeginPlay()
{
	Super::BeginPlay();

	// AI 컨트롤러 빙의 이전에 스폰된 경우를 위한 ASC 초기화 폴백
	if (AbilitySystemComponent && !AbilitySystemComponent->GetAvatarActor())
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitializeAbilities();
		InitializeEffects();
	}
}

void AMRMonster::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// MonsterType/MonsterLevel 기반 속성 스케일링 GE 적용은 여기서 추가
	// e.g., CMS에서 FMonsterTableRow 조회 후 동적 GE로 MaxHealth 설정
}
