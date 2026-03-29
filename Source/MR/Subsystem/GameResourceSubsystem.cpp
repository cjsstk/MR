// Fill out your copyright notice in the Description page of Project Settings.


#include "GameResourceSubsystem.h"

void UGameResourceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGameResourceSubsystem::Deinitialize()
{
	// 진행 중인 모든 로딩 요청을 취소
	for (auto& [Path, Handle] : ActiveHandles)
	{
		if (Handle.IsValid())
		{
			Handle->CancelHandle();
		}
	}
	ActiveHandles.Empty();
	CachedAssets.Empty();

	Super::Deinitialize();
}

void UGameResourceSubsystem::AsyncLoad(const FSoftObjectPath& Path, TFunction<void(UObject*)> Callback)
{
	if (!Path.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UGameResourceSubsystem::AsyncLoad: 유효하지 않은 경로입니다."));
		Callback(nullptr);
		return;
	}

	// 캐시 히트: 즉시 콜백
	if (const TWeakObjectPtr<UObject>* Cached = CachedAssets.Find(Path))
	{
		if (Cached->IsValid())
		{
			Callback(Cached->Get());
			return;
		}
		// 유효하지 않은 WeakPtr이면 캐시에서 제거하고 재로드
		CachedAssets.Remove(Path);
	}

	// 비동기 로드 요청
	TSharedPtr<FStreamableHandle>& Handle = ActiveHandles.FindOrAdd(Path);
	Handle = StreamableManager.RequestAsyncLoad(Path,
		[this, Path, Callback = MoveTemp(Callback)]()
		{
			UObject* Loaded = Path.ResolveObject();
			if (Loaded)
			{
				CachedAssets.Add(Path, Loaded);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("UGameResourceSubsystem: 에셋 로드 실패 (%s)"), *Path.ToString());
			}

			Callback(Loaded);
			ActiveHandles.Remove(Path);
		}
	);
}

UObject* UGameResourceSubsystem::GetCached(const FSoftObjectPath& Path) const
{
	const TWeakObjectPtr<UObject>* Found = CachedAssets.Find(Path);
	return (Found && Found->IsValid()) ? Found->Get() : nullptr;
}

void UGameResourceSubsystem::Release(const FSoftObjectPath& Path)
{
	if (TSharedPtr<FStreamableHandle>* Handle = ActiveHandles.Find(Path))
	{
		if (Handle->IsValid())
		{
			(*Handle)->CancelHandle();
		}
		ActiveHandles.Remove(Path);
	}
	CachedAssets.Remove(Path);
}
