// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "WeaponAnimConfigData.h"
#include "GameResourceSubsystem.generated.h"

class UBlendSpace;
class UAnimSequence;

/** AsyncLoadWeaponJumpAnims 콜백용. 세 애셋이 모두 로드된 뒤 한 번 호출된다. */
USTRUCT()
struct MR_API FWeaponJumpAnims
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UAnimSequence> Start;

	UPROPERTY()
	TObjectPtr<UAnimSequence> Loop;

	UPROPERTY()
	TObjectPtr<UAnimSequence> End;
};

/**
 * 게임에서 쓰이는 애니메이션, 사운드 등 에셋의 로드/캐시/해제를 담당한다.
 * UMRGameInstance가 소유하며, GetGameResource(this)로 어디서든 접근한다.
 * BP 서브클래스(BP_GameResource)에서 DataAsset을 지정하고 필요 시 새 설정을 추가한다.
 *
 * 사용 예:
 *   GetGameResource(this)->AsyncLoad(MeshPath, [](UObject* Loaded)
 *   {
 *       UStaticMesh* Mesh = Cast<UStaticMesh>(Loaded);
 *   });
 */
UCLASS(Blueprintable)
class MR_API UMRGameResource : public UObject
{
	GENERATED_BODY()

public:
	/** GameInstance::Shutdown에서 호출. 진행 중인 로딩 요청을 모두 취소한다. */
	void Deinitialize();

	/**
	 * 에셋을 비동기 로드한다.
	 * 이미 캐시된 경우 즉시 콜백을 호출한다.
	 * @param Path      로드할 에셋의 Soft Object Path
	 * @param Callback  로드 완료 시 호출. 실패 시 nullptr 전달.
	 */
	void AsyncLoad(const FSoftObjectPath& Path, TFunction<void(UObject*)> Callback);

	/** 캐시에서 에셋을 즉시 반환. 미로드 시 nullptr. */
	UObject* GetCached(const FSoftObjectPath& Path) const;

	/** 캐시 및 진행 중인 핸들에서 해당 에셋을 제거한다. */
	void Release(const FSoftObjectPath& Path);

	/** WeaponType에 해당하는 LocomotionBlendSpace를 비동기 로드한다. */
	void AsyncLoadWeaponLocomotionBS(EMRWeaponType WeaponType, TFunction<void(UBlendSpace*)> Callback);

	/** WeaponType에 해당하는 IdleAnimation을 비동기 로드한다. */
	void AsyncLoadWeaponIdleAnim(EMRWeaponType WeaponType, TFunction<void(UAnimSequence*)> Callback);

	/**
	 * 무기 타입에 해당하는 점프 애니메이션 3종(Start/Loop/End)을 비동기 로드한다.
	 * 셋 모두 로드 완료된 시점에 콜백이 한 번 호출된다.
	 */
	void AsyncLoadWeaponJumpAnims(EMRWeaponType WeaponType, TFunction<void(FWeaponJumpAnims)> Callback);

	// ─── 설정 (BP_GameResource에서 지정, 필요 시 새 DataAsset 추가) ──────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UWeaponAnimConfigData> WeaponAnimConfigData;

private:
	FStreamableManager StreamableManager;

	/** 로드 완료된 에셋 캐시. GC 방지용 핸들과 함께 관리. */
	TMap<FSoftObjectPath, TWeakObjectPtr<UObject>> CachedAssets;

	/** 로딩 중인 핸들. 완료 전 GC되지 않도록 TSharedPtr로 보관. */
	TMap<FSoftObjectPath, TSharedPtr<FStreamableHandle>> ActiveHandles;
};
