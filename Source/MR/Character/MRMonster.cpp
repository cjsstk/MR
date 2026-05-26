// Fill out your copyright notice in the Description page of Project Settings.

#include "MRMonster.h"
#include "AbilitySystemComponent.h"
#include "CMSSubsystem.h"
#include "MREffect_MonsterStats.h"
#include "MRGameplayTags.h"
#include "MRAIController.h"

AMRMonster::AMRMonster(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 월드에 배치되거나 런타임에 스폰될 때 자동으로 AI 컨트롤러가 빙의
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AMRAIController::StaticClass();
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
	InitializeMonsterStats();
}

void AMRMonster::InitializeMonsterStats()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	UCMSSubsystem* CMS = GetGameInstance()->GetSubsystem<UCMSSubsystem>();
	if (!CMS)
	{
		return;
	}

	const FMonsterTableRow* Row = CMS->GetMonsterRow(MonsterType);
	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRMonster] MonsterType %d not found in DataTable"), MonsterType);
		return;
	}

	// BaseStat * LevelScale^(Level-1). Level=1이면 배율 1.
	const float Scale       = FMath::Pow(Row->LevelScale, static_cast<float>(FMath::Max(MonsterLevel - 1, 0)));
	const float FinalHealth = Row->MaxHealth  * Scale;
	const float FinalAttack = Row->BaseAttack * Scale;
	const float FinalDefense= Row->BaseDefense* Scale;

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(UMREffect_MonsterStats::StaticClass(), 1.0f, Context);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	FGameplayEffectSpec& Spec = *SpecHandle.Data;
	Spec.SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_MaxHealth,    FinalHealth);
	Spec.SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_AttackPower,  FinalAttack);
	Spec.SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_DefensePower, FinalDefense);

	ASC->ApplyGameplayEffectSpecToSelf(Spec);

	UE_LOG(LogTemp, Log, TEXT("[MRMonster] Type=%d Lv=%d | HP=%.0f ATK=%.0f DEF=%.0f"),	
		MonsterType, MonsterLevel, FinalHealth, FinalAttack, FinalDefense);
}

void AMRMonster::HandleDeath()
{
	Super::HandleDeath();

	// MRMonsterSpawner는 OnDestroyed 이벤트로 재스폰을 처리하므로
	// 사망 애니메이션 재생 시간 후 Destroy()만 호출하면 된다.
	FTimerHandle DestroyTimerHandle;
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AMRMonster::DestroyAfterDeath, DeathDestroyDelay, false);
}

void AMRMonster::DestroyAfterDeath()
{
	Destroy();
}
