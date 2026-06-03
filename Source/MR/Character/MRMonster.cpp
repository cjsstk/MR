// Fill out your copyright notice in the Description page of Project Settings.

#include "MRMonster.h"
#include "AbilitySystemComponent.h"
#include "CMSSubsystem.h"
#include "MREffect_MonsterStats.h"
#include "MRGameplayTags.h"
#include "MRAIController.h"
#include "MRAttributeSetBase.h"
#include "MRMonsterHealthBarWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"

static TAutoConsoleVariable<int32> CVarShowMonsterHealthBar(
	TEXT("mr.ShowMonsterHealthBar"),
	1,
	TEXT("몬스터 머리 위 체력바 표시 (0=숨김, 1=표시)"),
	ECVF_Default
);

TArray<TWeakObjectPtr<AMRMonster>> AMRMonster::AliveMonsters;

AMRMonster::AMRMonster(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 월드에 배치되거나 런타임에 스폰될 때 자동으로 AI 컨트롤러가 빙의
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AMRAIController::StaticClass();

	HealthBarWidgetComponent = ObjectInitializer.CreateDefaultSubobject<UWidgetComponent>(this, TEXT("HealthBarWidgetComponent"));
	HealthBarWidgetComponent->SetupAttachment(RootComponent);
	HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMRMonster::BeginPlay()
{
	Super::BeginPlay();

	// AI 컨트롤러 빙의 이전에 스폰된 경우를 위한 ASC 초기화 폴백
	if (AbilitySystemComponent && !AbilitySystemComponent->GetAvatarActor())
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitializeAbilities();
		InitializeEffects();
	}

	// 체력바 위치: 캡슐 상단 + 여유 오프셋
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		HealthBarWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, HalfHeight + 20.f));
	}

	// CVar 초기값에 따른 가시성 설정
	HealthBarWidgetComponent->SetVisibility(CVarShowMonsterHealthBar.GetValueOnGameThread() != 0);

	// CVar 변경 콜백 등록 (전체 인스턴스 중 최초 1회)
	static bool bCVarCallbackRegistered = false;
	if (!bCVarCallbackRegistered)
	{
		CVarShowMonsterHealthBar.AsVariable()->SetOnChangedCallback(
			FConsoleVariableDelegate::CreateStatic(&AMRMonster::OnShowHealthBarCVarChanged));
		bCVarCallbackRegistered = true;
	}

	// 정적 레지스트리에 등록
	AliveMonsters.Add(this);

	// ASC Health 변경 델리게이트 구독
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UMRAttributeSetBase::GetHealthAttribute()
		).AddUObject(this, &AMRMonster::OnHealthChanged);
	}

	// 위젯 인스턴스 초기화 및 캐싱
	HealthBarWidgetComponent->InitWidget();
	if (UUserWidget* Widget = HealthBarWidgetComponent->GetWidget())
	{
		HealthBarWidget = Cast<UMRMonsterHealthBarWidget>(Widget);
		if (HealthBarWidget && AttributeSetBase)
		{
			HealthBarWidget->UpdateHealth(AttributeSetBase->GetHealth(), AttributeSetBase->GetMaxHealth());
		}
	}
}

void AMRMonster::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AliveMonsters.RemoveAll([this](const TWeakObjectPtr<AMRMonster>& Ptr) { return Ptr.Get() == this; });

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UMRAttributeSetBase::GetHealthAttribute()
		).RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AMRMonster::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeMonsterStats();
}

void AMRMonster::InitializeMonsterStats()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	UCMSSubsystem* CMS = GetGameInstance()->GetSubsystem<UCMSSubsystem>();
	if (!CMS)
	{
		return;
	}

	const FMonsterTableRow* Row = CMS->GetMonsterRow(MonsterType);
	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRMonster] MonsterType %d not found in DataTable"), MonsterType);
		return;
	}

	// BaseStat * LevelScale^(Level-1). Level=1이면 배율 1.
	const float Scale       = FMath::Pow(Row->LevelScale, static_cast<float>(FMath::Max(MonsterLevel - 1, 0)));
	const float FinalHealth = Row->MaxHealth  * Scale;
	const float FinalAttack = Row->BaseAttack * Scale;
	const float FinalDefense= Row->BaseDefense* Scale;

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(UMREffect_MonsterStats::StaticClass(), 1.0f, Context);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	FGameplayEffectSpec& Spec = *SpecHandle.Data;
	Spec.SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_MaxHealth,    FinalHealth);
	Spec.SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_AttackPower,  FinalAttack);
	Spec.SetSetByCallerMagnitude(MRGameplayTags::SetByCaller_DefensePower, FinalDefense);

	ASC->ApplyGameplayEffectSpecToSelf(Spec);

	UE_LOG(LogTemp, Log, TEXT("[MRMonster] Type=%d Lv=%d | HP=%.0f ATK=%.0f DEF=%.0f"),	
		MonsterType, MonsterLevel, FinalHealth, FinalAttack, FinalDefense);
}

void AMRMonster::HandleDeath()
{
	// 사망 즉시 체력바 숨김 (사망 애니메이션 재생 중에도 표시되지 않도록)
	if (HealthBarWidgetComponent)
	{
		HealthBarWidgetComponent->SetVisibility(false);
	}

	Super::HandleDeath();

	// MRMonsterSpawner는 OnDestroyed 이벤트로 재스폰을 처리하므로
	// 사망 애니메이션 재생 시간 후 Destroy()만 호출하면 된다.
	FTimerHandle DestroyTimerHandle;
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AMRMonster::DestroyAfterDeath, DeathDestroyDelay, false);
}

void AMRMonster::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (HealthBarWidget && AttributeSetBase)
	{
		HealthBarWidget->UpdateHealth(Data.NewValue, AttributeSetBase->GetMaxHealth());
	}
}

void AMRMonster::OnShowHealthBarCVarChanged(IConsoleVariable* CVar)
{
	const bool bShow = CVar->GetInt() != 0;

	for (int32 i = AliveMonsters.Num() - 1; i >= 0; --i)
	{
		if (AMRMonster* Monster = AliveMonsters[i].Get())
		{
			Monster->HealthBarWidgetComponent->SetVisibility(bShow);
		}
		else
		{
			AliveMonsters.RemoveAtSwap(i);
		}
	}
}

void AMRMonster::DestroyAfterDeath()
{
	Destroy();
}
