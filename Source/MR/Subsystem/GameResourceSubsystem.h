// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/StreamableManager.h"
#include "GameResourceSubsystem.generated.h"

/**
 * FStreamableManager 기반 비동기 에셋 로딩 및 캐싱을 담당하는 서브시스템.
 *
 * 사용 예:
 *   GetGameResource(this)->AsyncLoad(MeshPath, [](UObject* Loaded)
 *   {
 *       UStaticMesh* Mesh = Cast<UStaticMesh>(Loaded);
 *   });
 */
UCLASS()
class MR_API UGameResourceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * 에셋을 비동기 로드한다.
	 * 이미 캐시된 경우 즉시 콜백을 호출한다.
	 * 동일 경로에 대해 로딩이 진행 중이더라도 중복 요청이 가능하며, 완료 시 모두 콜백된다.
	 * @param Path      로드할 에셋의 Soft Object Path
	 * @param Callback  로드 완료 시 호출되는 함수. 로드 실패 시 nullptr이 전달된다.
	 */
	void AsyncLoad(const FSoftObjectPath& Path, TFunction<void(UObject*)> Callback);

	/**
	 * 캐시에서 에셋을 즉시 반환한다.
	 * 아직 로드되지 않은 경우 nullptr를 반환한다.
	 */
	UObject* GetCached(const FSoftObjectPath& Path) const;

	/** 캐시 및 진행 중인 핸들에서 해당 에셋을 제거한다. */
	void Release(const FSoftObjectPath& Path);

private:
	FStreamableManager StreamableManager;

	/** 로드 완료된 에셋 캐시. GC 방지를 위해 TObjectPtr 대신 TWeakObjectPtr + 핸들로 관리. */
	TMap<FSoftObjectPath, TWeakObjectPtr<UObject>> CachedAssets;

	/** 로딩 중인 핸들. 완료 전 GC되지 않도록 TSharedPtr로 보관. */
	TMap<FSoftObjectPath, TSharedPtr<FStreamableHandle>> ActiveHandles;
};
