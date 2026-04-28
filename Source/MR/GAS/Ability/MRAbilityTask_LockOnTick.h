#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "MRAbilityTask_LockOnTick.generated.h"

class AMRPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTargetLost);

/**
 * 록온 대상을 매 틱마다 추적하며 카메라를 부드럽게 회전시키는 AbilityTask.
 *
 * 흐름:
 *   CreateTask → Activate → TickTask(매 프레임 카메라 보간)
 *   대상 유효성 실패(사망/거리 초과/삭제) 시 OnTargetLost 브로드캐스트 후 EndTask
 */
UCLASS()
class MR_API UMRAbilityTask_LockOnTick : public UAbilityTask
{
	GENERATED_BODY()

public:
	/**
	 * 팩토리 함수.
	 * @param InTarget         추적할 대상 액터 (보통 AMRMonster)
	 * @param InMaxDistance    이 거리를 초과하면 록온 해제
	 * @param InCameraInterpSpeed  카메라 회전 보간 속도 (RInterpTo)
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UMRAbilityTask_LockOnTick* CreateTask(
		UGameplayAbility* OwningAbility,
		AActor* InTarget,
		float InMaxDistance,
		float InCameraInterpSpeed);

	/** 대상이 유효하지 않아진 경우 브로드캐스트 */
	UPROPERTY(BlueprintAssignable)
	FOnTargetLost OnTargetLost;

	/** 현재 추적 중인 대상 반환 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks")
	AActor* GetLockedTarget() const { return LockedTarget.Get(); }

protected:
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	bool IsTargetValid() const;

	TWeakObjectPtr<AActor> LockedTarget;
	float MaxDistance = 2000.f;
	float CameraInterpSpeed = 10.f;
};
