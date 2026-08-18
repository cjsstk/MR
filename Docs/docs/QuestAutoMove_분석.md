# 퀘스트 자동이동(Quest Auto-Move) 구현 분석

퀘스트 UI에서 퀘스트/목표를 선택했을 때 목표 지점까지 자동으로 길찾기 이동하는 기능의 클라이언트(Unreal) 구현을 코드 흐름 순서대로 정리한 문서.

---

## 1. 요약

- 자동이동은 여러 UI(퀘스트 창, 미니 퀘스트 리스트, 퀘스트 게시판 등)의 **"자동이동" 버튼** 클릭에서 시작된다.
- 최종적으로 `AWorldPlayerController`가 `FAutoMoveInfo` 상태를 세팅하고, 매 틱 `TickAutoMove()`에서 목표 좌표를 향해 `AUnit::SetMoveTo()`를 호출한다.
- 실제 경로 탐색은 언리얼 `UNavigationSystemV1` + `UPathFollowingComponent`(정확히는 `UCrowdFollowingComponent`)로 수행되며, 자동이동일 때는 **도로(NavArea_Road) 우선 쿼리 필터**를 사용한다.
- 목표 좌표는 퀘스트 CMS 데이터(`FCMSWorldLocationRow`)에서 결정된다.
- 도착·사용자 입력·전투·마운트 사용 등 다양한 사유로 중단되며, 중단 사유는 `EMoveStopReason`으로 표현된다.

전체 파이프라인:

```
[UI 버튼 클릭]
   └─ UQuestStore::TryQuestAutoMove()              // 진입 게이트 검사
        └─ AWorldPlayerController::SetAutoMoveByQuest()   // 도달 가능성/좌표 결정
             ├─ FQuest::GetQuestWorldLocationId()   // 목표 WorldLocationId 결정 (CMS)
             ├─ SetAutoMoveInternal()               // 선행 조건 + 상태 전환(AutoMoving)
             ├─ FAutoMoveInfo::StartByQuest()       // 자동이동 상태 세팅
             └─ ACTION_DISPATCH_AutoMove()          // UI 갱신 (Flux)
                  └─ UQuestStore IMPLEMENT_ACTION_HANDLER(AutoMove)

[매 프레임 Tick]
   └─ AWorldPlayerController::TickAutoMove()
        └─ AUnit::SetMoveTo()                       // 목표 좌표 등록
             └─ (Tick) AUnit::TickMovement...()
                  └─ AWorldPlayerController::MoveToLocation()
                       └─ UQ7BlueprintLibrary::MoveToLocation()   // NavMesh 경로 탐색 + RequestMove
                            └─ [도착] AUnit::OnArrivedAtTarget() → StopMove(Arrived)
```

---

## 2. 진입점 (UI 버튼 → 자동이동 요청)

여러 위젯이 동일한 진입 함수(`TryQuestAutoMove` 또는 `SetAutoMoveByQuest`)를 호출한다.

| 위젯 / 위치 | 클릭 핸들러 | 파일:라인 |
|---|---|---|
| 퀘스트 게시판 (`UQuestBoardWidget`) | `OnAutoMoveButtonClicked()` → `TryQuestAutoMove(SelectedQuestRow)` | `Widget/QuestWidgets.cpp:1014` |
| 퀘스트 창 (`UQuestWidget`) | `OnAutoMoveButtonClicked()` → `TryQuestAutoMove(SelectedQuestRow)` | `Widget/QuestWidgets.cpp:2277` |
| 사이드 퀘스트 (`USideQuestWidget`) | `OnAutoMoveButtonClicked()` → `TryQuestAutoMove(...)` | `Widget/QuestWidgets.cpp:2619` |
| 미니 퀘스트 리스트 (`UMiniQuestListWidget`) | → `WPC->SetAutoMoveByQuest(...)` | `Widget/WorldWidgets.cpp:4060` |
| 미니 퀘스트 위젯 (`UMiniQuestWidget`) | → `WPC->SetAutoMoveByQuest(...)` | `Widget/WorldWidgets.cpp:4892` |

버튼 바인딩 예시 (`QuestWidgets.cpp:49`, `1518`):

```cpp
UButton* AutoMoveButton = CastChecked<UButton>(GetWidgetFromName("BtnAutoMove"));
AutoMoveButton->OnClicked.AddUniqueDynamic(this, &UQuestBoardWidget::OnAutoMoveButtonClicked);
```

```cpp
// Widget/QuestWidgets.cpp:1014
void UQuestBoardWidget::OnAutoMoveButtonClicked()
{
    if (!GetHUDStore().GetQuestStore().TryQuestAutoMove(SelectedQuestRow))
    {
        // 실패 처리(토스트 등은 하위 함수에서 수행)
    }
}
```

> 참고: 퀘스트 트래커의 텔레포트 버튼은 별도 흐름(`AWorldHUD::OpenQuestTeleportConfirmPopup`, `HUD/WorldHUD.cpp:1945`)이다. 자동이동과 목표 위치 결정 로직(`GetQuestWorldLocationId`)은 공유하지만, 이동 대신 순간이동 패킷을 보낸다.

---

## 3. 1단계: 진입 게이트 검사 — `UQuestStore::TryQuestAutoMove`

`HUDStore/QuestStore.cpp:863`

```cpp
bool UQuestStore::TryQuestAutoMove(FCMSQuestRow const* QuestRow) const
{
    if (!QuestRow) return false;

    FQuestType const QuestType = QuestRow->CmsType();
    FQuest const* Quest = FindQuest(QuestType);

    if (!Quest || Quest->Stage == EQuestStage::None)         return false;   // 진행 중이 아님
    if (Quest->Stage == EQuestStage::CanBeRewarded &&
        !Quest->IsReadyToCompleteNpc())                      return false;   // 보상 대기 & 완료 NPC 대상 아님
    if (Quest->IsAutoMoving())                               return false;   // 이미 자동이동 중

    return GetCheckedWorldPlayerController(this)
        ->SetAutoMoveByQuest(Quest->GetQuestWorldLocationId(), QuestRow);
}
```

역할: **퀘스트 상태 유효성**을 검사하고, 목표 위치 ID를 계산해 `WorldPlayerController`로 위임한다.

---

## 4. 목표 위치 결정 — `FQuest::GetQuestWorldLocationId`

`HUDStore/QuestStore.cpp:192`

```cpp
FWorldLocationId FQuest::GetQuestWorldLocationId() const
{
    if (Stage == EQuestStage::None) return WorldLocationIdInvalid;

    if (IsReadyToCompleteNpc())                       // 완료 보고 단계면
        return FWorldLocationId(Row->GoalWorldLocationId);   //  → 완료 NPC 위치

    // 아직 진행 중이면, 미완료된 첫 번째 조건의 목표 위치
    const TArray<FCMSQuestConditionRow const*>& CondRows = Row->QuestConditionIdRows;
    for (int32 i = 0; i < CondRows.Num(); ++i)
    {
        if (GetProgressCond(i) < CondRows[i]->Count)
            return FWorldLocationId(CondRows[i]->WorldLocationId);
    }
    return WorldLocationIdInvalid;
}
```

- 퀘스트 진행 단계에 따라 **목표가 동적으로 바뀐다.**
  - 완료 보고 단계 → 퀘스트 완료 NPC 위치(`GoalWorldLocationId`)
  - 진행 중 → 아직 달성 못한 **첫 번째 조건**의 목표 위치(`QuestConditionIdRow.WorldLocationId`)
- 반환값은 좌표가 아니라 **WorldLocation ID**이며, 실제 좌표는 CMS 테이블(`FCMSWorldLocationRow.Location`)에서 조회한다(다음 단계).

---

## 5. 2단계: 도달 가능성 검사 & 좌표 확정 — `SetAutoMoveByQuest`

`PlayerController/WorldPlayerController.cpp:5125`

주요 검사 및 처리 순서:

1. **CMS 조회 및 자동이동 지원 여부**
   ```cpp
   const FCMSWorldLocationRow* LocationRow = CMS->GetWorldLocationRow(InWorldLocationId);
   if (!LocationRow || !InQuestRow->AutoMove)   // AutoMove 미지원 퀘스트
   {
       ShowToastMessage(..., TEXT("ThisQuestNotSupportAutoMove"));
       return false;
   }
   ```
   → 퀘스트 CMS 행의 `AutoMove` 플래그로 자동이동 허용 여부를 결정.

2. **도달 불가 케이스 → `UnreachableByAutoMove` 토스트 후 실패**
   - 목표가 하리하라(Harihara) 던전인데 미개방(`5153`)
   - 현재/목표 던전이 서로 다르고, 둘 중 하나라도 "자동이동 가능 던전"이 아닐 때(`5165`, `IsQuestAutoMoveDungeon`)
   - 현재 존이 공성전 내부(`SiegeWarfareInner`)인데 다른 존으로 가야 할 때(`5182`)
   - 현재 존이 길드 하우스인데 목표가 다른 존일 때(`5190`)

3. **선행 조건 검사 & 상태 전환** — `SetAutoMoveInternal(EMoveStopReason::StartAutoMoveQuest)` (`5202`)

4. **목표 좌표/도달 거리 확정 및 상태 세팅**
   ```cpp
   const FVector TargetLocation = LocationRow->Location;              // 실제 좌표
   const float ReachedDistance  = GetQ7Variable()->QuestReachedDistance;

   // 이미 목표 반경 안이면 "도착 시 자동행동"을 강제
   const float DistSquared = FVector::DistSquaredXY(GetPawn()->GetActorLocation(), TargetLocation);
   const bool bAutoActionForcedWhenArrived = DistSquared <= FMath::Square(ReachedDistance);

   AutoMoveInfo.StartByQuest(InQuestRow->CmsType(), TargetLocation, ReachedDistance, bAutoActionForcedWhenArrived);
   ACTION_DISPATCH_AutoMove(AutoMoveInfo);    // UI 갱신 (Flux)
   return true;
   ```

> 자동이동 요청자(Requester)는 퀘스트 외에도 `Map`(월드맵 클릭), `MinimapNpcList`(미니맵 NPC), `SiegeHUD`(공성 게이트), `Comeback`(복귀)가 있으며, 각각 `SetAutoMoveByMap` / `SetAutoMoveByNpcList` / `SetAutoMoveToSiegeGate` / `SetAutoMoveToComebackLocation` 진입 함수를 가진다. 이후 이동 로직은 동일하게 공유된다.

---

## 6. 선행 조건 & 상태 전환 — `SetAutoMoveInternal`

`PlayerController/WorldPlayerController.cpp:5084`

```cpp
bool AWorldPlayerController::SetAutoMoveInternal(EMoveStopReason InStopReason, const bool bStopAutoAction /*=true*/)
{
    if (HasMovementInput())            return false;   // 사용자가 이동 입력 중이면 거부
    if (CanMove() == ECanMove::Nothing) return false;  // 이동 불가 상태(CC 등)면 거부

    if (bSiegeMode) ReqSiegeMode(false);               // 공성 모드 해제

    AbortAllAction(InStopReason);                      // 기존 액션(전투/스킬 등) 중단
    if (bAutoAction && bStopAutoAction)
        SetAutoActionEnabled(false);                   // 자동사냥 끄기

    SetMovingState(EMovingState::AutoMoving);          // 이동 상태를 AutoMoving 으로
    return true;
}
```

역할: 자동이동 시작 전에 **입력/이동 가능 여부를 확인**하고, 충돌하는 기존 액션(전투·자동사냥·공성)을 정리한 뒤 `EMovingState::AutoMoving` 상태로 전환.

---

## 7. 자동이동 상태 데이터 — `FAutoMoveInfo`

정의: `Q7Struct.h:3451` / 구현: `Q7Struct.cpp:146~225`

```cpp
struct FAutoMoveInfo
{
    bool                bAutoMoving = false;                 // 이동 중 여부
    EAutoMoveRequester  Requester   = EAutoMoveRequester::None; // 요청 주체
    FQuestType          QuestType;                           // (퀘스트 요청 시) 대상 퀘스트
    FNpcType            NpcType;                             // (미니맵 NPC 요청 시)
    FVector             Location;                            // 목표 좌표
    float               ReachedDistance = -1;                // 도달 판정 거리
    EMoveStopReason     StopReason;                          // 마지막 중단 사유
    bool                bAutoActionForcedWhenArrived = false;// 도착 시 자동행동 강제

    bool IsMoving() const { return bAutoMoving; }
    void Stopping();   // bAutoMoving=false, 나머지 정보 유지 (일시 중단)
    void Stop();       // 완전 종료 + Requester/플래그 리셋
    void StartByQuest(...);  // 퀘스트용 시작 세팅
    ...
};
```

`StartByQuest` (`Q7Struct.cpp:170`):
```cpp
void FAutoMoveInfo::StartByQuest(const FQuestType& InQuestType, const FVector& InLocation,
                                 float InReachedDistance, bool bInAutoActionForcedWhenArrived)
{
    Reset();
    bAutoMoving = true;
    Requester   = EAutoMoveRequester::Quest;
    QuestType   = InQuestType;
    Location    = InLocation;
    ReachedDistance = InReachedDistance;
    bAutoActionForcedWhenArrived = bInAutoActionForcedWhenArrived;
}
```

관련 enum:
- `EAutoMoveRequester` (`Q7Enum.h:1345`): `None, Map, Quest, MinimapNpcList, SiegeHUD, Comeback`
- `EMoveStopReason` (`Q7Enum.h:1356`): `Arrived, Input, WeaponChanged, MountUsed, StartAttack, Dead, CrowdControl, StartTeleport, StopAutoMoveQuest, ...` 등 다수

---

## 8. UI 갱신 — `ACTION_DISPATCH_AutoMove` → `UQuestStore` 핸들러

`HUDStore/QuestStore.cpp:2687` (`IMPLEMENT_ACTION_HANDLER(UQuestStore, AutoMove)`)

Flux 패턴으로, `SetAutoMoveByQuest`가 발행한 `AutoMove` 액션을 `UQuestStore`가 수신해 퀘스트별 자동이동 표시 상태를 갱신한다.

```cpp
IMPLEMENT_ACTION_HANDLER(UQuestStore, AutoMove)
{
    auto const& AutoMoveInfo = ACTION_PARSE_AutoMove(InAction)->GetVal();

    // 퀘스트/복귀 외 요청이면 → 모든 퀘스트 자동이동 표시 해제
    if (AutoMoveInfo.Requester != EAutoMoveRequester::Quest &&
        AutoMoveInfo.Requester != EAutoMoveRequester::Comeback)
    {
        for (auto& Elem : Quests) { ... Quest->SetAutoMoving(false); }
        ActionDispatchHelper::Dispatch<EHSActionType::AutoQuestPlayStateChange>(false);
        return true;
    }

    FQuest* Quest = GetQuest(AutoMoveInfo.QuestType);
    if (!Quest) return true;

    Quest->SetAutoMoving(AutoMoveInfo.bAutoMoving);

    // 도착 시: 사냥 퀘스트가 아니거나, 자동행동 강제/옵션이 켜져 있으면 해당 퀘스트를 Active 로
    const bool bStartAutoMoveForQuest = AutoMoveInfo.IsMoving();
    const bool bStopAutoMoveWhenArrived =
        !AutoMoveInfo.IsMoving() && AutoMoveInfo.StopReason == EMoveStopReason::Arrived &&
        (AutoMoveInfo.bAutoActionForcedWhenArrived
         || !GetCMS()->IsHuntQuest(*Quest->Row)
         || GetQ7SaveGame()->GetOptionSetting().GetToggleOptionValue(
                EOptionSettingType::AutoActionWhenHuntQuestMovementToggle));

    SetActiveQuest(AutoMoveInfo.QuestType, bStartAutoMoveForQuest || bStopAutoMoveWhenArrived);
    return true;
}
```

- `bAutoActionForcedWhenArrived`, `IsHuntQuest`, `AutoActionWhenHuntQuestMovementToggle` 옵션의 조합으로 **도착 후 자동사냥/자동행동 진입 여부**가 결정된다.

---

## 9. 3단계: 매 틱 실제 이동 처리 — `TickAutoMove`

`PlayerController/WorldPlayerController.cpp:1662` (Tick에서 `675`번 라인 `TickAutoMove()` 호출)

```cpp
void AWorldPlayerController::TickAutoMove()
{
    if (!AutoMoveInfo.IsMoving()) return;

    if (!GetUnit()->HasValidTargetLocation())
    {
        // 아직 목표 좌표가 유닛에 등록되지 않았으면 등록
        if (GetUnit()->SetMoveTo(AutoMoveInfo.Location, AutoMoveInfo.ReachedDistance))
            UpdateUnmountedMoveTime();
        return;
    }

    // 미니맵 NPC 자동이동은 NPC가 시야에 들어오면 NPC 추적으로 전환
    if (AutoMoveInfo.Requester != EAutoMoveRequester::MinimapNpcList) return;
    if (const ANpc* Npc = GetClientWorld()->GetNpc(AutoMoveInfo.NpcType))
        if (!IsOutOfSightActor(Npc)) { SetUnitMoveToNpc(Npc); UpdateUnmountedMoveTime(); }
}
```

즉, 자동이동의 핵심은 **`AutoMoveInfo.Location`을 유닛의 이동 목표로 등록**하는 것이다.

---

## 10. 4단계: 유닛 이동 목표 등록 — `AUnit::SetMoveTo`

`Unit/Unit.cpp:3190`

```cpp
bool AUnit::SetMoveTo(const FVector& InLocation, float InReachedDistance /*=-1*/, bool ...)
{
    ECanMove CanMove = GetMyLocalController()->CanMove();
    if (CanMove == ECanMove::Nothing)  return false;
    if (CanMove == ECanMove::RotateOnly) { RotateTo(InLocation); return false; }

    if (InReachedDistance == -1.f)
        InReachedDistance = IsShip() ? BoundRadius * 2.0f : BoundRadius;   // 선박 보정
    else if (IsShip())
        InReachedDistance += BoundRadius * 2.0f;

    if (InLocation == TargetLocation && InReachedDistance == TargetReachedDistance)
        return false;   // 변화 없음

    TargetLocation        = InLocation;
    TargetReachedDistance = InReachedDistance;
    bTargetLocationDirty  = true;    // → 다음 이동 틱에서 경로 재요청
    return true;
}
```

`bTargetLocationDirty`가 켜지면 유닛의 이동 틱(`TickMovement...`)에서 실제 경로를 요청한다.

---

## 11. 5단계: NavMesh 경로 탐색 & 이동 요청

### 5-1. 유닛 이동 틱에서 경로 요청 — `AUnit::TickMovement...`
`Unit/Unit.cpp:3495` (지상), `3562` (선박)

```cpp
if (!LocalPC->IsFollowingAPath() || bTargetLocationDirty)
{
    UnitUtil::ProjectPointToNavigation(this, TargetLocation);   // Z 보정
    const FPathFollowingRequestResult Result = LocalPC->MoveToLocation(TargetLocation, TargetReachedDistance);
    bTargetLocationDirty = false;

    if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
        OnArrivedAtTarget();                                    // 이미 도착
    ...
}
```

### 5-2. 컨트롤러 래퍼 — `AWorldPlayerController::MoveToLocation`
`PlayerController/WorldPlayerController.cpp:5294`

```cpp
FPathFollowingRequestResult AWorldPlayerController::MoveToLocation(const FVector& GoalLocation, float AcceptanceRadius)
{
    const bool bAutoMove = AutoMoveInfo.IsMoving();
    const auto Result = UQ7BlueprintLibrary::MoveToLocation(this, GoalLocation, AcceptanceRadius, bAutoMove);

    // 자동이동(복귀 제외)이 성공하면 경로 시각화 시작
    if (bAutoMove && AutoMoveInfo.Requester != EAutoMoveRequester::Comeback
        && Result.Code == EPathFollowingRequestResult::RequestSuccessful)
        if (NavPathVisualizer) NavPathVisualizer->StartTracking();

    return Result;
}
```

### 5-3. 실제 NavMesh 탐색 — `UQ7BlueprintLibrary::MoveToLocation`
`Q7BlueprintLibrary.cpp:152`

핵심:
- `UPathFollowingComponent`(`UCrowdFollowingComponent`)를 얻어 기존 이동을 정리(한 번에 하나의 이동 요청만 유지).
- **자동이동 여부에 따라 다른 쿼리 필터** 사용:
  ```cpp
  // Q7BlueprintLibrary.cpp:95  GetQueryFilter()
  if (bAutomove)  // 자동이동이면 도로(NavArea_Road) 우선 경로
      return UNavigationQueryFilter::GetQueryFilter(NavData, Querier, UQ7NavigationQueryFilter_Road::StaticClass());
  return UNavigationQueryFilter::GetQueryFilter(NavData, Querier, UQ7NavigationQueryFilter::StaticClass());
  ```
- 자동이동은 경로 비용 상한(`GQ7NavCostLimit`)을 적용해 너무 먼 경로는 거부할 수 있다.
- `FindPathSync` 성공 시 `RequestMove(MoveReq, Result.Path)`로 실제 이동 시작.
  ```cpp
  const FPathFindingResult Result = NavSys->FindPathSync(Query);
  if (Result.IsSuccessful())
  {
      FAIMoveRequest MoveReq(GoalLocation);
      MoveReq.SetAcceptanceRadius(AcceptanceRadius)
             .SetReachTestIncludesAgentRadius(false)
             .SetReachTestIncludesGoalRadius(false);
      ResultData.MoveId = PFollowComp->RequestMove(MoveReq, Result.Path);
      ResultData.Code   = EPathFollowingRequestResult::RequestSuccessful;
  }
  ```
- 또한 자동이동일 때 군중 시뮬레이션(`CrowdFollowingComponent`)을 `Enabled`로 켠다(`188`).

> 자동이동은 일반 캐릭터 이동과 동일한 NavMesh/PathFollowing 파이프라인을 쓰되, **도로 우선 필터 + 비용 상한 + 경로 시각화 + 군중 회피**가 추가되는 점이 다르다.

---

## 12. 도착 판정 — `AUnit::OnArrivedAtTarget`

`Unit/Unit.cpp:3645`. 지상/선박 이동 틱에서 목표 반경 도달(`DistSquared <= ReachedDistance^2`) 또는 `AlreadyAtGoal` 시 호출.

```cpp
void AUnit::OnArrivedAtTarget()
{
    if (AWorldPlayerController* LocalPC = GetMyLocalController())
        LocalPC->OnMoveEnd();
    StopMove(EMoveStopReason::Arrived);   // 중단 사유 = Arrived
}
```

`StopMove(Arrived)`는 최종적으로 `AutoMoveInfo`를 중단시키고 `ACTION_DISPATCH_AutoMove`를 재발행하며, 이때 8절의 핸들러가 `StopReason == Arrived`를 보고 도착 후 자동행동/활성 퀘스트 처리를 수행한다.

---

## 13. 자동이동 중단(Stop) 흐름

### 13-1. 중단 진입 함수
- `StopMovement()` (`WorldPlayerController.cpp:5311`): 이동 컴포넌트 정지 + `AutoMoveInfo.Stopping()`/`Stop()` + 경로 시각화 종료 + 터치 이펙트 숨김.
- `StopMovementWithReason(EMoveStopReason)` (`5381`): 중단 사유를 기록하고 `ACTION_DISPATCH_StopMoveWithReason` 발행.
- `StopQuestAutoMove()` (`1891`) → `StopAction(EMoveStopReason::StopAutoMoveQuest, ...)`.
- `StopSiegeHUDAutoMove()` (`1896`).

### 13-2. `FAutoMoveInfo`의 두 종류 정지
```cpp
void FAutoMoveInfo::Stopping() { bAutoMoving = false; }          // 상태만 멈춤(정보 유지)
void FAutoMoveInfo::Stop()     { bAutoMoving = false;            // 완전 종료
                                 Requester = None;
                                 bAutoActionForcedWhenArrived = false; }
```

### 13-3. 주요 중단 사유 (`EMoveStopReason`)
- `Arrived` — 목표 도달
- `Input` — 사용자 이동 입력
- `StartAttack` / `StartManualSkill` / `StartChanneling` — 전투·스킬 시작
- `MountUsed` / `WeaponChanged` — 탈것 사용·무기 교체
- `CrowdControl` / `Dead` — CC·사망
- `StartTeleport` / `StartLoot` / `StartInteract*` — 순간이동·루팅·상호작용 시작
- `InvalidLocation` — 제자리 뛰기(경로 막힘) 감지 시(`Unit.cpp:3540`, `AbortAllAction`)
- `StopAutoMoveQuest` / `StopAutoMoveSiegeHUD` — 명시적 자동이동 취소

> 제자리 뛰기 감지: 지상 이동 틱에서 일정 시간(`RunningInPlaceTimeMax`) 동안 이동 거리가 임계치 미만이면 `AbortAllAction(EMoveStopReason::InvalidLocation)`로 중단한다(`Unit.cpp:3530~3545`). 단, 이 로직은 자동이동(`IsAutoMoving()`)일 때는 적용되지 않는다.

---

## 14. 관련 파일 · 함수 요약

| 계층 | 파일 | 핵심 심볼 | 역할 |
|---|---|---|---|
| UI(버튼) | `Widget/QuestWidgets.cpp` | `UQuestBoardWidget::OnAutoMoveButtonClicked` (1014), `UQuestWidget::OnAutoMoveButtonClicked` (2277) | 자동이동 버튼 클릭 진입 |
| UI(미니) | `Widget/WorldWidgets.cpp` | `UMiniQuestListWidget`(4060), `UMiniQuestWidget`(4892) | 미니 퀘스트에서 자동이동 |
| Store(게이트) | `HUDStore/QuestStore.cpp` | `UQuestStore::TryQuestAutoMove` (863) | 상태 유효성 검사 후 위임 |
| Store(목표) | `HUDStore/QuestStore.cpp` | `FQuest::GetQuestWorldLocationId` (192) | 퀘스트 단계별 목표 WorldLocation 결정 |
| Store(UI갱신) | `HUDStore/QuestStore.cpp` | `IMPLEMENT_ACTION_HANDLER(UQuestStore, AutoMove)` (2687) | 자동이동/도착 상태 UI 반영 |
| Controller(진입) | `PlayerController/WorldPlayerController.cpp` | `SetAutoMoveByQuest` (5125) 외 `SetAutoMoveByMap/NpcList/SiegeGate/Comeback` | 도달 가능성 검사 + 좌표 확정 |
| Controller(공통) | `PlayerController/WorldPlayerController.cpp` | `SetAutoMoveInternal` (5084) | 선행 조건 + 상태 전환 |
| Controller(틱) | `PlayerController/WorldPlayerController.cpp` | `TickAutoMove` (1662) | 목표 좌표를 유닛에 등록 |
| Controller(이동) | `PlayerController/WorldPlayerController.cpp` | `MoveToLocation` (5294), `StopMovement` (5311), `StopMovementWithReason` (5381) | 경로 요청 래퍼 / 중단 |
| 상태 데이터 | `Q7Struct.h/.cpp` | `FAutoMoveInfo` (h:3451, cpp:146~) | 자동이동 상태 보관 |
| enum | `Q7Enum.h` | `EAutoMoveRequester` (1345), `EMoveStopReason` (1356) | 요청자/중단 사유 |
| 유닛 이동 | `Unit/Unit.cpp` | `SetMoveTo` (3190), `TickMovement...` (3495/3562), `OnArrivedAtTarget` (3645) | 목표 등록 / 경로 요청 / 도착 |
| NavMesh | `Q7BlueprintLibrary.cpp` | `MoveToLocation` (152), `GetQueryFilter` (95), `TestMoveToLocation` (111) | 실제 경로 탐색 및 이동 요청 |
| 경로 필터 | (Nav 모듈) | `UQ7NavigationQueryFilter_Road`, `UQ7NavigationQueryFilter` | 자동이동 시 도로 우선 |

---

## 15. 특이사항 / 주의점

- **자동이동 = 도로 우선.** 일반 클릭 이동과 달리 `bAutoMove=true`일 때 `NavArea_Road` 쿼리 필터를 써서 최대한 길을 따라간다(`GetQueryFilter`, `Q7BlueprintLibrary.cpp:95`).
- **하나의 이동 요청만 유지.** `MoveToLocation`은 새 요청 전에 기존 이동을 `AbortMove`로 정리한다.
- **도착 후 자동행동/자동사냥 진입 여부**는 퀘스트가 사냥 퀘스트인지(`IsHuntQuest`), 옵션 `AutoActionWhenHuntQuestMovementToggle`, 시작 시점 이미 도착 상태였는지(`bAutoActionForcedWhenArrived`)에 따라 갈린다(핸들러 `QuestStore.cpp:2687`).
- **존/던전 경계는 자동이동 불가.** 다른 던전·공성 내부·길드 하우스 등은 자동이동 대신 순간이동(텔레포트) 흐름을 사용해야 한다(`SetAutoMoveByQuest`의 방어 코드).
- **경로 시각화(`NavPathVisualizer`)**: 복귀(Comeback)를 제외한 자동이동 성공 시 경로 트래킹을 시작/종료한다.
- **선박(IsShip) 특수 처리**: 도달 거리에 `BoundRadius * 2` 보정, 별도 이동 틱(`TickMovementShip`) 사용.

---

### 부록: 다른 자동이동 진입 함수(참고)

동일한 `FAutoMoveInfo`/`TickAutoMove` 파이프라인을 공유하는 형제 기능들:

| 함수 | 요청자 | 트리거 | 파일:라인 |
|---|---|---|---|
| `SetAutoMoveByMap` | `Map` | 월드맵 위치 클릭 | `WorldPlayerController.cpp:5113` |
| `SetAutoMoveByNpcList` | `MinimapNpcList` | 미니맵 NPC 리스트 | `WorldPlayerController.cpp:5219/5230` |
| `SetAutoMoveToSiegeGate` | `SiegeHUD` | 공성 게이트 이동 | `WorldPlayerController.cpp:5252` |
| `SetAutoMoveToComebackLocation` | `Comeback` | 전투/타겟 복귀 | `WorldPlayerController.cpp:5276` |