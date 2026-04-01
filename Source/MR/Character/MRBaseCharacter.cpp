// Fill out your copyright notice in the Description page of Project Settings.

#include "MRBaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "MRAttributeSetBase.h"

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
