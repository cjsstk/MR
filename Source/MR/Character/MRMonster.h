// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRBaseCharacter.h"
#include "MRMonster.generated.h"

/**
 * AI가 조작하는 몬스터 캐릭터.
 * MonsterType으로 CMS DataTable(FMonsterTableRow)과 연결하고,
 * MonsterLevel을 기반으로 GameplayEffect를 통해 속성을 스케일링한다.
 */
UCLASS()
class MR_API AMRMonster : public AMRBaseCharacter
{
	GENERATED_BODY()

public:
	AMRMonster(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// FMonsterTableRow::Type과 매칭되는 몬스터 고유 타입 번호
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	int32 MonsterType = 0;

	// GE를 통한 속성 스케일링에 사용되는 레벨
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	int32 MonsterLevel = 1;

	/** 사망 후 액터가 Destroy()되기까지의 대기 시간 (사망 애니메이션 재생 시간에 맞춰 조정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster", meta = (ClampMin = "0.0"))
	float DeathDestroyDelay = 3.f;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void HandleDeath() override;

private:
	/** DataTable에서 MonsterType/MonsterLevel에 맞는 스탯을 조회해 GE로 적용한다. */
	void InitializeMonsterStats();

	void DestroyAfterDeath();
};
