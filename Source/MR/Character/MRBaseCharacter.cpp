// Fill out your copyright notice in the Description page of Project Settings.

#include "MRBaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "MRAttributeSetBase.h"
#include "MRGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

AMRBaseCharacter::AMRBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// ASC가 서브오브젝트로 생성된 AttributeSet을 자동으로 등록한다.
	AttributeSetBase = CreateDefaultSubobject<UMRAttributeSetBase>(TEXT("AttributeSetBase"));
}

UAbilitySystemComponent* AMRBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AMRBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		// 싱글 플레이어: OwnerActor와 AvatarActor 모두 이 캐릭터로 설정
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	InitializeAbilities();
	InitializeEffects();
}

void AMRBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(
			MRGameplayTags::Character_State_Dead,
			EGameplayTagEventType::NewOrRemoved
		).AddUObject(this, &AMRBaseCharacter::OnDeadTagChanged);
	}
}

void AMRBaseCharacter::OnDeadTagChanged(const FGameplayTag /*Tag*/, int32 NewCount)
{
	if (NewCount > 0 && !bIsDead)
	{
		bIsDead = true;
		HandleDeath();
	}
}

void AMRBaseCharacter::HandleDeath()
{
	// 진행 중인 모든 어빌리티 취소
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
	}

	// 이동 정지 및 낙하 방지
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	// 캡슐 충돌 비활성화 (다른 캐릭터에 물리적으로 영향 주지 않도록)
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	OnDeath.Broadcast();
}

void AMRBaseCharacter::InitializeAbilities()
{
	if (bAbilitiesInitialized || !AbilitySystemComponent)
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
		}
	}

	bAbilitiesInitialized = true;
}

void AMRBaseCharacter::InitializeEffects()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (const TSubclassOf<UGameplayEffect>& EffectClass : DefaultEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}
