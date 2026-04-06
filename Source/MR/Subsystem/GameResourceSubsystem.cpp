// Fill out your copyright notice in the Description page of Project Settings.

#include "GameResourceSubsystem.h"
#include "Animation/BlendSpace.h"
#include "WeaponAnimConfigData.h"

void UMRGameResource::Deinitialize()
{
	for (auto& [Path, Handle] : ActiveHandles)
	{
		if (Handle.IsValid())
		{
			Handle->CancelHandle();
		}
	}
	ActiveHandles.Empty();
	CachedAssets.Empty();
}

void UMRGameResource::AsyncLoad(const FSoftObjectPath& Path, TFunction<void(UObject*)> Callback)
{
	if (!Path.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UMRGameResource::AsyncLoad: 유효하지 않은 경로입니다."));
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
		CachedAssets.Remove(Path);
	}

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
				UE_LOG(LogTemp, Warning, TEXT("UMRGameResource: 에셋 로드 실패 (%s)"), *Path.ToString());
			}

			Callback(Loaded);
			ActiveHandles.Remove(Path);
		}
	);
}

UObject* UMRGameResource::GetCached(const FSoftObjectPath& Path) const
{
	const TWeakObjectPtr<UObject>* Found = CachedAssets.Find(Path);
	return (Found && Found->IsValid()) ? Found->Get() : nullptr;
}

void UMRGameResource::Release(const FSoftObjectPath& Path)
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

void UMRGameResource::AsyncLoadWeaponIdleAnim(EMRWeaponType WeaponType, TFunction<void(UAnimSequence*)> Callback)
{
	const FWeaponAnimConfig* Config = WeaponAnimConfigData ? WeaponAnimConfigData->WeaponAnimConfigs.Find(WeaponType) : nullptr;
	if (!Config || Config->IdleAnimation.IsNull())
	{
		Callback(nullptr);
		return;
	}

	AsyncLoad(Config->IdleAnimation.ToSoftObjectPath(), [Callback = MoveTemp(Callback)](UObject* Loaded)
	{
		Callback(Cast<UAnimSequence>(Loaded));
	});
}

void UMRGameResource::AsyncLoadWeaponJumpAnims(EMRWeaponType WeaponType, TFunction<void(FWeaponJumpAnims)> Callback)
{
	const FWeaponAnimConfig* Config = WeaponAnimConfigData ? WeaponAnimConfigData->WeaponAnimConfigs.Find(WeaponType) : nullptr;
	if (!Config)
	{
		Callback(FWeaponJumpAnims{});
		return;
	}

	struct FLoadState
	{
		FWeaponJumpAnims Result;
		int32 Remaining = 3;
		TFunction<void(FWeaponJumpAnims)> Callback;
	};
	TSharedRef<FLoadState> State = MakeShared<FLoadState>();
	State->Callback = MoveTemp(Callback);

	auto LoadOrSkip = [&](const TSoftObjectPtr<UAnimSequence>& SoftPtr, TObjectPtr<UAnimSequence>* OutFieldPtr)
	{
		if (SoftPtr.IsNull())
		{
			if (--State->Remaining == 0)
			{
				State->Callback(State->Result);
			}
			return;
		}
		AsyncLoad(SoftPtr.ToSoftObjectPath(), [State, OutFieldPtr](UObject* Loaded)
		{
			*OutFieldPtr = Cast<UAnimSequence>(Loaded);
			if (--State->Remaining == 0)
			{
				State->Callback(State->Result);
			}
		});
	};

	LoadOrSkip(Config->JumpStartAnimation, &State->Result.Start);
	LoadOrSkip(Config->JumpLoopAnimation,  &State->Result.Loop);
	LoadOrSkip(Config->JumpEndAnimation,   &State->Result.End);
}

void UMRGameResource::AsyncLoadWeaponLocomotionBS(EMRWeaponType WeaponType, TFunction<void(UBlendSpace*)> Callback)
{
	const FWeaponAnimConfig* Config = WeaponAnimConfigData ? WeaponAnimConfigData->WeaponAnimConfigs.Find(WeaponType) : nullptr;
	if (!Config || Config->LocomotionBlendSpace.IsNull())
	{
		Callback(nullptr);
		return;
	}

	AsyncLoad(Config->LocomotionBlendSpace.ToSoftObjectPath(), [Callback = MoveTemp(Callback)](UObject* Loaded)
	{
		Callback(Cast<UBlendSpace>(Loaded));
	});
}
