// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MRProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

/**
 * 활 발사체 액터.
 *
 * AnimNotify_SpawnProjectile이 소켓 위치에서 스폰하고 InitProjectile을 호출한다.
 * 충돌 시 소유자 ASC에 Event.Attack.Hit 이벤트를 전송하여 BowAttack 어빌리티가
 * 데미지를 적용하도록 한다. 동일 타겟 중복 히트는 HitActors 셋으로 방지한다.
 */
UCLASS(Blueprintable)
class MR_API AMRProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMRProjectile(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * 스폰 직후 호출. 발사 방향과 속도를 설정하고 수명 타이머를 시작한다.
	 * @param InInstigatorActor  발사한 캐릭터 (자기 자신과의 충돌을 무시)
	 * @param LaunchDirection    단위 벡터 기준 발사 방향
	 * @param Speed              발사 속도 (cm/s)
	 * @param GravityScale       중력 배율 (0 = 직선, 1 = 기본 중력)
	 */
	void InitProjectile(AActor* InInstigatorActor, const FVector& LaunchDirection, float Speed, float GravityScale);

	/** BP에서 시각적 메시를 지정한다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** 루트 콜리전 구체 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	/** 발사체 이동 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** 발사체 최대 수명 (초). 지나면 자동 소멸. */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile", meta = (ClampMin = "0.1"))
	float LifeSpan = 5.f;

private:
	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	/** 히트 처리: 타겟 ASC에 이벤트를 전송하고 발사체를 소멸시킨다. */
	void HandleImpact(AActor* HitActor);

	/** 이번 발사에서 이미 히트한 액터 목록 (중복 히트 방지) */
	TSet<TWeakObjectPtr<AActor>> HitActors;
};
