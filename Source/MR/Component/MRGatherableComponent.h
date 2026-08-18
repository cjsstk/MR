// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MRGatherableComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MR_API UMRGatherableComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UMRGatherableComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	/** 인터랙션 볼륨. 플레이어 접근을 감지해 프롬프트를 띄운다. */
	UPROPERTY(VisibleAnywhere, Category = "Gather")
	TObjectPtr<USphereComponent> InteractionVolume;

	UPROPERTY(VisibleAnywhere, Category = "Gather")
	TObjectPtr<UWidgetComponent> InteractionWidget;

	/** 캐시된 테이블 Row. CMS의 LoadedTables는 게임 수명 동안 유지되므로 포인터 캐시가 안전하다. */
	const FGatherableTableRow* CachedRow = nullptr;

	/** 남은 채집 횟수 */
	int32 RemainingGathers = 0;

	/** 소진되어 숨김/리스폰 대기 중인지 */
	bool bDepleted = false;

	FTimerHandle RespawnTimerHandle;

	UFUNCTION()
	void OnGatherVolumeOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnGatherVolumeOverlapEnd(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	/** 채집 소진 처리: 숨김 + 충돌 해제 + 범위 내 플레이어 프롬프트 정리 + 리스폰 타이머 예약 */
	void EnterDepletedAndScheduleRespawn();

	/** 리스폰: 상태/횟수 복원 + 표시 + 충돌 재활성 + 이미 범위 내 플레이어 재감지 */
	void Respawn();
};
