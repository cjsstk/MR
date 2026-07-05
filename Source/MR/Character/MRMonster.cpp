// Fill out your copyright notice in the Description page of Project Settings.

#include "MRMonster.h"
#include "AbilitySystemComponent.h"
#include "CMSSubsystem.h"
#include "MREffect_MonsterStats.h"
#include "MRGameplayTags.h"
#include "MRAIController.h"
#include "MRAttributeSetBase.h"
#include "MRMonsterHealthBarWidget.h"
#include "MRAbility_Carve.h"
#include "MRInventoryComponent.h"
#include "MRDropHelper.h"
#include "MRPlayerCharacter.h"
#include "Action/Action.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
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

	// 박리 인터랙션 볼륨. 사망 전까지 충돌 비활성화 상태 유지.
	CarveInteractionVolume = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("CarveInteractionVolume"));
	CarveInteractionVolume->SetupAttachment(GetRootComponent());
	CarveInteractionVolume->SetSphereRadius(200.f);
	CarveInteractionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CarveInteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	CarveInteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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

	// DataTable에서 BehaviorTree를 읽어 AIController에 주입한다.
	// OnPossess(AIController)보다 먼저 호출되므로 BT가 즉시 반영된다.
	if (AMRAIController* AIC = Cast<AMRAIController>(NewController))
	{
		if (UCMSSubsystem* CMS = GetGameInstance()->GetSubsystem<UCMSSubsystem>())
		{
			if (const FMonsterTableRow* Row = CMS->GetMonsterRow(MonsterType))
			{
				if (!Row->BehaviorTree.IsNull())
				{
					AIC->BehaviorTree = Row->BehaviorTree.LoadSynchronous();
					UE_LOG(LogTemp, Log, TEXT("[MRMonster] Type=%d BehaviorTree=%s"),
						MonsterType, *AIC->BehaviorTree->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[MRMonster] Type=%d has no BehaviorTree in DataTable"), MonsterType);
				}
			}
		}
	}

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

	// BehaviorTree 중단 — 사망 후 AI 로직이 계속 실행되지 않도록
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AIController->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Death"));
			UE_LOG(LogTemp, Log, TEXT("[MRMonster] %s AI logic stopped on death."), *GetName());
		}
	}

	// CMS에서 박리 횟수 초기화
	if (UCMSSubsystem* CMS = GetGameInstance()->GetSubsystem<UCMSSubsystem>())
	{
		if (const FMonsterTableRow* Row = CMS->GetMonsterRow(MonsterType))
		{
			RemainingCarves = Row->CarveCount;
			UE_LOG(LogTemp, Log, TEXT("[MRMonster] %s 박리 횟수 초기화: %d"), *GetName(), RemainingCarves);
		}
	}

	// 박리 횟수가 있을 때만 상호작용 볼륨 활성화 및 오버랩 이벤트 바인딩
	if (CarveInteractionVolume && RemainingCarves > 0)
	{
		CarveInteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CarveInteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AMRMonster::OnCarveVolumeOverlapBegin);
		CarveInteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AMRMonster::OnCarveVolumeOverlapEnd);
	}

	Super::HandleDeath();

	// DestroyAfterDeath 타이머 대신 사체 최대 유지 시간 타이머를 시작한다.
	// MRMonsterSpawner는 OnDestroyed 이벤트로 재스폰을 처리하므로
	// DestroyCorpse → Destroy() 호출로 충분하다.
	StartCorpseDestroyTimer(MaxCorpseLifetime);
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

void AMRMonster::PerformCarve(UMRAbility_Carve* CarveAbility)
{
	if (!CanBeCarved())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRMonster] PerformCarve 실패: 박리 불가 상태 (IsDead=%d, RemainingCarves=%d)"),
			IsDead(), RemainingCarves);
		return;
	}

	if (!CarveAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRMonster] PerformCarve 실패: CarveAbility가 nullptr"));
		return;
	}

	// CMS에서 드롭 테이블 조회
	UCMSSubsystem* CMS = GetGameInstance()->GetSubsystem<UCMSSubsystem>();
	if (!CMS)
	{
		return;
	}

	const FMonsterTableRow* MonsterRow = CMS->GetMonsterRow(MonsterType);
	if (!MonsterRow || MonsterRow->NormalDropTableId == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRMonster] PerformCarve: MonsterType=%d 드롭 테이블 없음"), MonsterType);
		return;
	}

	const FDropTableRow* DropTable = CMS->GetDropTableRow(FDropTableId(MonsterRow->NormalDropTableId));
	if (!DropTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRMonster] PerformCarve: DropTableId=%d 테이블 없음"),
			MonsterRow->NormalDropTableId);
		return;
	}

	// 가중치 랜덤으로 아이템 결정
	const FMRDropResult Result = UMRDropHelper::RollOnce(*DropTable);
	if (Result.ItemId == 0 || Result.Count <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MRMonster] PerformCarve: RollOnce 결과 없음"));
		return;
	}

	// 플레이어 인벤토리에 아이템 지급
	AMRPlayerCharacter* Player = Cast<AMRPlayerCharacter>(CarveAbility->GetAvatarActorFromActorInfo());
	if (Player)
	{
		if (UMRInventoryComponent* Inventory = Player->FindComponentByClass<UMRInventoryComponent>())
		{
			Inventory->AddItem(FItemId(Result.ItemId), Result.Count);
		}
	}

	// 박리 결과 팝업 표시를 위한 액션 디스패치
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UActionDispatcher* Dispatcher = GI->GetSubsystem<UActionDispatcher>())
		{
			Dispatcher->Dispatch(MakeAction<FAction_ShowCarveResult>(Result.ItemId, Result.Count));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[MRMonster] %s 박리: ItemId=%d x%d (남은 횟수 %d → %d)"),
		*GetName(), Result.ItemId, Result.Count, RemainingCarves, RemainingCarves - 1);

	RemainingCarves--;

	// 박리 횟수 소진 시 볼륨 비활성화 후 단기 소멸 타이머로 전환
	if (RemainingCarves <= 0)
	{
		if (CarveInteractionVolume)
		{
			CarveInteractionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		StartCorpseDestroyTimer(PostCarveDestroyDelay);
	}
}

void AMRMonster::OnCarveVolumeOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!CanBeCarved())
	{
		return;
	}

	AMRPlayerCharacter* Player = Cast<AMRPlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	Player->ShowCarvePrompt(this);
	Player->SetCarvableMonster(this);
}

void AMRMonster::OnCarveVolumeOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	AMRPlayerCharacter* Player = Cast<AMRPlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	Player->HideCarvePrompt();
	Player->ClearCarvableMonster(this);
}

void AMRMonster::StartCorpseDestroyTimer(float Delay)
{
	// 기존 타이머 클리어 후 새 타이머 설정 (박리 완료 시 남은 MaxCorpseLifetime 타이머를 단축)
	GetWorldTimerManager().ClearTimer(CorpseDestroyTimerHandle);
	GetWorldTimerManager().SetTimer(CorpseDestroyTimerHandle, this, &AMRMonster::DestroyCorpse, Delay, false);
}

void AMRMonster::DestroyCorpse()
{
	Destroy();
}
