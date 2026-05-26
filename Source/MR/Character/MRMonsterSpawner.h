// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MRMonsterSpawner.generated.h"

class AMRMonster;

/** 스포너 하나에 등록할 몬스터 종류·수·리스폰 설정 */
USTRUCT(BlueprintType)
struct FMRSpawnEntry
{
	GENERATED_BODY()

	/** CMS FMonsterTableRow::Type 값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawn")
	int32 MonsterType = 0;

	/** 동시에 유지할 최대 마리 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawn", meta=(ClampMin=1))
	int32 Count = 1;

	/** 사망 후 재스폰 대기 시간(초). 0이면 재스폰 없음. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawn", meta=(ClampMin=0.f))
	float RespawnDelay = 30.f;
};

/**
 * 필드 레벨에 배치해 몬스터를 스폰·관리하는 액터.
 *
 * 기본 동작:
 *  - BeginPlay 시 SpawnEntries 기준으로 몬스터를 SpawnRadius 내 무작위 위치에 스폰
 *  - 몬스터 사망 감지 후 RespawnDelay 초 뒤 해당 자리를 보충
 *
 * 확장 포인트 (서브클래스 오버라이드):
 *  - GetSpawnTransform() : 스폰 위치·회전 커스터마이징 (NavMesh 샘플링 등)
 *  - OnMonsterSpawned()  : 스폰 직후 추가 초기화
 */
UCLASS()
class MR_API AMRMonsterSpawner : public AActor
{
	GENERATED_BODY()

public:
	AMRMonsterSpawner(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * 각 엔트리에서 현재 부족한 만큼만 스폰한다.
	 * BeginPlay 외에 외부에서 호출해도 중복 스폰되지 않는다.
	 */
	UFUNCTION(BlueprintCallable, Category="Spawner")
	void SpawnAll();

protected:
	virtual void BeginPlay() override;

	/**
	 * 스폰 위치·회전을 반환한다.
	 * 기본 구현: SpawnRadius 안 무작위 XY + 스포너 Z, 무작위 Yaw.
	 * NavMesh 샘플링 등이 필요하면 서브클래스에서 교체한다.
	 */
	virtual FTransform GetSpawnTransform() const;

	/**
	 * 몬스터가 월드에 등장한 직후 호출된다.
	 * 서브클래스에서 AI 컨트롤러 교체, 추가 태그 부여 등을 처리할 수 있다.
	 */
	virtual void OnMonsterSpawned(AMRMonster* Monster, int32 EntryIndex);

	/** 스폰할 몬스터 BP 클래스. BP 서브클래스의 Class Defaults에서 지정. */
	UPROPERTY(EditDefaultsOnly, Category="Spawner")
	TSubclassOf<AMRMonster> MonsterClass;

	/** 스폰 엔트리 목록. 레벨 배치 후 인스턴스별로 설정. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Spawner")
	TArray<FMRSpawnEntry> SpawnEntries;

	/** BeginPlay 시 자동 스폰 여부 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Spawner")
	bool bSpawnOnBeginPlay = true;

	/** 스포너 위치를 중심으로 몬스터를 배치할 반경 (cm) */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Spawner", meta=(ClampMin=0.f))
	float SpawnRadius = 300.f;

private:
	/** EntryIndex 엔트리의 몬스터 한 마리를 스폰한다. */
	void SpawnOneMonster(int32 EntryIndex);

	/** 몬스터 사망(Destroyed) 콜백 */
	UFUNCTION()
	void OnMonsterDestroyed(AActor* DestroyedActor);

	/** 현재 살아있는 몬스터 → 엔트리 인덱스 역참조 테이블 */
	TMap<TObjectKey<AMRMonster>, int32> MonsterEntryMap;
};
