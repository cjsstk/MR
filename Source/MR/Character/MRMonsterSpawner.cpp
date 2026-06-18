// Fill out your copyright notice in the Description page of Project Settings.

#include "MRMonsterSpawner.h"
#include "MRMonster.h"
#include "CMSSubsystem.h"
#include "MRDataTable.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MRAIController.h"

AMRMonsterSpawner::AMRMonsterSpawner(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMRMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		SpawnAll();
	}
}

void AMRMonsterSpawner::SpawnAll()
{
	for (int32 i = 0; i < SpawnEntries.Num(); ++i)
	{
		const FMRSpawnEntry& Entry = SpawnEntries[i];

		// 현재 살아있는 수를 세어 부족한 만큼만 스폰 — 중복 호출에도 안전
		int32 AliveCount = 0;
		for (const auto& [Key, EntryIdx] : MonsterEntryMap)
		{
			if (EntryIdx == i)
			{
				++AliveCount;
			}
		}

		const int32 ToSpawn = FMath::Max(0, Entry.Count - AliveCount);
		for (int32 j = 0; j < ToSpawn; ++j)
		{
			SpawnOneMonster(i);
		}
	}
}

void AMRMonsterSpawner::SpawnOneMonster(int32 EntryIndex)
{
	if (!SpawnEntries.IsValidIndex(EntryIndex))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FMRSpawnEntry& Entry = SpawnEntries[EntryIndex];

	// DataTable에서 MonsterType에 맞는 클래스 조회. 없으면 폴백 MonsterClass 사용.
	TSubclassOf<AMRMonster> ClassToSpawn = MonsterClass;
	if (UCMSSubsystem* CMS = GetGameInstance()->GetSubsystem<UCMSSubsystem>())
	{
		if (const FMonsterTableRow* Row = CMS->GetMonsterRow(Entry.MonsterType))
		{
			if (!Row->MonsterClass.IsNull())
			{
				ClassToSpawn = Row->MonsterClass.LoadSynchronous();
			}
		}
	}

	if (!ClassToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMRMonsterSpawner [%s]: MonsterType=%d에 해당하는 클래스가 없습니다. DataTable 또는 MonsterClass를 확인하세요."),
			*GetName(), Entry.MonsterType);
		return;
	}

	const FTransform SpawnTransform = GetSpawnTransform();

	// BeginPlay 이전에 MonsterType을 설정할 수 있도록 Deferred 스폰 사용
	AMRMonster* Monster = World->SpawnActorDeferred<AMRMonster>(
		ClassToSpawn,
		SpawnTransform,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);
	if (!Monster)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMRMonsterSpawner [%s]: 몬스터 스폰 실패 (EntryIndex=%d)"), *GetName(), EntryIndex);
		return;
	}

	Monster->MonsterType = Entry.MonsterType;
	Monster->FinishSpawning(SpawnTransform);

	Monster->OnDestroyed.AddDynamic(this, &AMRMonsterSpawner::OnMonsterDestroyed);
	MonsterEntryMap.Add(TObjectKey<AMRMonster>(Monster), EntryIndex);

	// 둥지 위치가 설정된 경우 BB에 주입 (BT Phase2 시퀀스에서 사용)
	if (!Entry.NestLocationOffset.IsZero())
	{
		if (AAIController* AIC = Cast<AAIController>(Monster->GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				const FVector NestWorldLocation = GetActorLocation() + Entry.NestLocationOffset;
				BB->SetValueAsVector(AMRAIController::BBKey_NestLocation, NestWorldLocation);
				UE_LOG(LogTemp, Log, TEXT("[MRMonsterSpawner] Monster(%s) NestLocation set: %s"),
					*Monster->GetName(), *NestWorldLocation.ToString());
			}
		}
	}

	OnMonsterSpawned(Monster, EntryIndex);
}

FTransform AMRMonsterSpawner::GetSpawnTransform() const
{
	const float Angle  = FMath::RandRange(0.f, 2.f * PI);
	const float Dist   = FMath::RandRange(0.f, SpawnRadius);
	const FVector Offset(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
	const FRotator Rotation(0.f, FMath::RandRange(-180.f, 180.f), 0.f);

	return FTransform(Rotation, GetActorLocation() + Offset);
}

void AMRMonsterSpawner::OnMonsterSpawned(AMRMonster* Monster, int32 EntryIndex)
{
	// 서브클래스에서 오버라이드 — 기본 구현 없음
}

void AMRMonsterSpawner::OnMonsterDestroyed(AActor* DestroyedActor)
{
	AMRMonster* Monster = Cast<AMRMonster>(DestroyedActor);
	if (!Monster)
	{
		return;
	}

	const int32* EntryIndexPtr = MonsterEntryMap.Find(TObjectKey<AMRMonster>(Monster));
	if (!EntryIndexPtr)
	{
		return;
	}

	const int32 EntryIndex = *EntryIndexPtr;
	MonsterEntryMap.Remove(TObjectKey<AMRMonster>(Monster));

	if (!SpawnEntries.IsValidIndex(EntryIndex))
	{
		return;
	}

	const float RespawnDelay = SpawnEntries[EntryIndex].RespawnDelay;
	if (RespawnDelay <= 0.f)
	{
		return;
	}

	// 타이머 핸들을 보관하지 않아도 발동은 보장됨.
	// 스포너 소멸 시 GetWorldTimerManager()가 이 오브젝트의 모든 타이머를 자동 정리한다.
	FTimerHandle TimerHandle;
	FTimerDelegate Delegate;
	Delegate.BindUObject(this, &AMRMonsterSpawner::SpawnOneMonster, EntryIndex);
	GetWorldTimerManager().SetTimer(TimerHandle, Delegate, RespawnDelay, false);
}
