// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MRBaseCharacter.h"
#include "MRMonster.generated.h"

class UWidgetComponent;
class UMRMonsterHealthBarWidget;
class UMRAbility_Carve;
class USphereComponent;
struct FOnAttributeChangeData;

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

	/**
	 * 사체에서 소재를 박리한다. 드롭 계산 + 인벤토리 지급 + 결과 액션 디스패치.
	 * MRAbility_Carve::OnMontageCompleted에서 호출된다.
	 */
	void PerformCarve(UMRAbility_Carve* CarveAbility);

	/** 남은 박리 횟수 */
	int32 GetRemainingCarves() const { return RemainingCarves; }

	/** 박리 가능 상태인지 (사망 후 RemainingCarves > 0) */
	bool CanBeCarved() const { return IsDead() && RemainingCarves > 0; }

	/** 박리 인터랙션 볼륨. 사망 시 활성화되어 플레이어 접근을 감지한다. */
	UPROPERTY(VisibleAnywhere, Category = "Carving")
	TObjectPtr<USphereComponent> CarveInteractionVolume;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void HandleDeath() override;

private:
	/** DataTable에서 MonsterType/MonsterLevel에 맞는 스탯을 조회해 GE로 적용한다. */
	void InitializeMonsterStats();

	void DestroyAfterDeath();

	/** 남은 박리 횟수. HandleDeath에서 FMonsterTableRow::CarveCount로 초기화된다. */
	int32 RemainingCarves = 0;

	/** 박리 완료 또는 MaxCorpseLifetime 초과 시 소멸 타이머 */
	FTimerHandle CorpseDestroyTimerHandle;

	/** 사체 최대 유지 시간 (초). 박리 전이라도 이 시간 후 소멸. */
	UPROPERTY(EditDefaultsOnly, Category = "Carving", meta = (ClampMin = "10"))
	float MaxCorpseLifetime = 120.f;

	/** 박리 완료 후 소멸까지 대기 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Carving", meta = (ClampMin = "1"))
	float PostCarveDestroyDelay = 5.f;

	UFUNCTION()
	void OnCarveVolumeOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnCarveVolumeOverlapEnd(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	void StartCorpseDestroyTimer(float Delay);
	void DestroyCorpse();

	// 머리 위 체력바 위젯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> HealthBarWidgetComponent;

	// 캐시된 체력바 위젯 포인터 (매 프레임 Cast 방지)
	UPROPERTY()
	TObjectPtr<UMRMonsterHealthBarWidget> HealthBarWidget;

	void OnHealthChanged(const FOnAttributeChangeData& Data);

	// CVar 토글 시 전체 몬스터 가시성 일괄 변경용 정적 레지스트리
	static TArray<TWeakObjectPtr<AMRMonster>> AliveMonsters;
	static void OnShowHealthBarCVarChanged(IConsoleVariable* CVar);
};
