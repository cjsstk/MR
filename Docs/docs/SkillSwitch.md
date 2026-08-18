# SkillSwitch (스킬 분기/전환) 시스템 분석

## 개요

`SkillSwitch`는 같은 무기군(`EClassWeapon`) 내에서 서로 다른 스킬 셋(분기 = `ESkillBranch`)을 사용할 수 있게 해주는 시스템이다. 예컨대 `TwoHandSword` 무기군 안에 *기본 양손검*(`TwoHandSword`) 분기와 *마검사*(`MagicSword`) 분기가 공존하며, 플레이어는

1. **추가 분기를 해금(Unlock)** 하고
2. **해금된 분기들 사이를 전환(Switch)**

하는 두 단계의 행동을 한다. Unlock과 Switch는 비용 체계가 분리되어 있고, 콘텐츠 오픈 마일스톤(`M50`) 이후 활성화된다.

키워드: `SkillSwitch` = 분기 정의 한 행, `SkillBranch` = 실제 적용된 분기 enum, `SkillSwitchLink` = 같은 무기군에서 서로 전환 가능한 분기들의 묶음.

---

## 1. 데이터 (CMS)

### 1.1 `Content/Cms/SkillSwitch.csv`
한 행 = 하나의 분기 정의.

| 컬럼 | 의미 |
| --- | --- |
| `OID` | `FSkillSwitchType` 식별자 |
| `Type` | 키 |
| `Milestone` | 노출 시점 |
| `Weapon` (`EClassWeapon`) | 베이스 무기군 |
| `SkillBranch` (`ESkillBranch`) | 실제 분기. 베이스 분기는 `Weapon`과 동일 |
| `Desc` | 설명 텍스트 |
| `UnlockCostCategory` (`ESkillSwitchCostCategory`) | `None / Gold / Gem / Item` |
| `UnlockCostId`, `UnlockCostValue` | 해금 비용 |

예) `(200, …, TwoHandSword, TwoHandSword, …, None)` 베이스 분기, `(201, …, TwoHandSword, MagicSword, …, Gem 4000)` 추가 분기.

### 1.2 `Content/Cms/SkillSwitchLink.csv`
같은 무기군 내에서 서로 전환할 수 있는 `SkillSwitchIds`의 묶음 + **전환 비용**.

| 컬럼 | 의미 |
| --- | --- |
| `OID` | `FSkillSwitchLinkType` |
| `Milestone` | 오픈 시점 |
| `SkillSwitchIds` (`TArray<int32>`) | 묶이는 분기들 |
| `SwitchCostCategory/Id/Value` | *해금 후* 분기 전환 시 비용 (Unlock 비용과 별개) |

예) `(1, M50, [200, 201], Gold, 0, 0)` — 양손검 ↔ 마검사 무료 전환.

### 1.3 그 외 데이터
- `Content/Cms/SkillBranchGroup.csv`: 어떤 스킬이 어느 분기에서 사용 가능한지 — `SkillInfo`가 `SkillBranchGroupRow`를 참조하고, 런타임에 `AUnit::IsSkillBranchCondition()`이 매칭 검사.
- `Content/CmsConst/SystemConst.csv`
  - `482 SkillSwitchPopupHideDay = 7` — "다시 묻지 않기" 기간
  - `483 SkillSwitchCooltime = 1000ms` — 전환 쿨다운
- `Content/Cms/ContentOpen.csv`
  - `282 SkillSwitch = M50` — 콘텐츠 오픈 시점

---

## 2. 타입 / Enum 정의

### 2.1 Enum (`Source/Q7/Cms/CMSEnum_gen.h`)
- `ESkillBranch` (504-521): `None, OneHandSword, TwoHandSword, Dagger, Bow, Staff, Tome, Shotgun, Cannon, Ram, TwoHandMace, Scimitar, ElementalStone, MagicSword, Max=14`
- `ESkillSwitchCostCategory` (6934-6941): `None=0, Gold=1, Gem=2, Item=3`
- `ETaskType::SkillSwitchUnlock=186 / SkillSwitch=187 / SkillSwitchReset=188`
- `EContentOpenCategory::SkillSwitch=63`
- `ETutorialSequence::SkillSwitch=51 / SkillSwitchUnlock=52`

### 2.2 Strong typedef (`Source/Q7/Cms/CMSType_gen.h`)
- `FSkillSwitchType`, `FSkillSwitchLinkType` — `int32` 래퍼 키
- `FSkillBranchGroupType` — 분기-스킬 매칭용 별도 키
- `SkillSwitchTypeInvalid`, `SkillSwitchLinkTypeInvalid`

### 2.3 시스템 상수 (`Source/Q7/Cms/Const_gen.h`)
- `ESystemConst::SkillSwitchPopupHideDay`, `ESystemConst::SkillSwitchCooltime`

### 2.4 행 구조체 / 테이블 (`Source/Q7/Cms/CMS_gen.h`)
- `FCMSSkillSwitchRow` (11276-11298): 위 1.1 컬럼을 그대로 매핑
- `FCMSSkillSwitchLinkRow` (11310-11331): `SkillSwitchIds` 외에 `SkillSwitchIdsRows` (FK 캐시) 보유
- `FCMSSkillSwitchTable`, `FCMSSkillSwitchLinkTable`

### 2.5 조회 API (`Source/Q7/Cms/Q7Cms.h/cpp`)
- `UCMS::GetSkillSwitchRow(FSkillSwitchType)` (CMS_gen.h:14759)
- `UCMS::GetSkillSwitchLinkRow(FSkillSwitchLinkType)` (14760)
- `UCMS::GetSkillSwitchRowBySkillBranch(ESkillBranch, bool bFindLinkedSwitch)` (Q7Cms.h:941)
- `UCMS::GetSkillSwitchLinkRowBySkillBranch(...)` — 분기로 링크 행 역조회
- 캐시 맵: `SkillSwitchRowsByClassWeaponMap`, `SwitchLinkBySkillBranch` (Q7Cms.h:1546-1548)
- 빌드: `UCMS::InitSkillSwitch()` (Q7Cms.cpp:6175-6225) — 로드 시 위 캐시를 1회 구축

---

## 3. 네트워크 프로토콜

### 3.1 클라 → 존 요청 (`Source/Q7/Network/Protocol/Clientzone_gen.h`)
- `FC2ZSkillSwitchUnlock` (6422)
  - `FSkillSwitchType SkillSwitchType` — 해금할 분기
- `FC2ZSkillSwitch` (6436)
  - `FSkillSwitchType SkillSwitchType` — 전환할 분기

### 3.2 서버 → 클라 변경 이벤트 (`g_ChangedEvent.h`)
- `FSkillSwitchUnlockTask` (4101): `_switchType`
- `FSkillSwitchTask` (4118): `TValueHolder<ESkillBranch> _skillBranch` (new/prev) + `EClassWeapon _weapon`
- `FSkillSwitchResetTask` (4136): payload 없음 (전체 reset)

### 3.3 핸들러 (`ClientWorldNetTask.cpp`)
등록(227-229), 본체(4581-4616):
- `HandleSkillSwitchUnlock` → `ACTION_DISPATCH(SkillSwitchUnlock, switchType)`
- `HandleSkillSwitch` → 별도 디스패치 없음. `UnitStore.SkillBranch`가 다른 경로로 이미 갱신되므로 그쪽에서 `SkillBranchChanged`가 떨어진다.
- `HandleSkillSwitchReset` → `ACTION_DISPATCH(SkillSwitchLoad, TArray())` — 보유 목록을 비움

### 3.4 초기 로드
- `FW2CLoadAccount.SkillSwitches : TArray<FSkillSwitchType>` (`Clientworld_gen.h:4329`, STREAM_DEFINE 4397)
- 로그인 시 `ClientWorldNet.cpp:1494`에서 `EHSActionType::SkillSwitchLoad` 디스패치

---

## 4. SaveGame

`Source/Q7/Q7SaveGame.h:432-433, 607` / `.cpp:1423-1431`

- 단일 필드 `int64 SkillSwitchLastAskedUt` — 전환 확인 팝업의 *"이 기간 동안 보지 않기"* 타임스탬프
- `GetSkillSwitchLastAskedUt()` / `SetSkillSwitchLastAskedUt(bool bSet)`
  - `true` → `UClientWorld::GetUnixTimestampUtcNow()`
  - `false` → 0
- 다음 팝업 노출 여부: `LastAskedUt + SkillSwitchPopupHideDay * 86400 < now`

---

## 5. HUDStore / 상태 관리

### 5.1 Action 정의 (`Source/Q7/HUDStore/Dispatcher/ActionTypeExpressions.inl:222-225`)
```
ACTION_EXPRESSION(SkillSwitchUnlock, const FSkillSwitchType&)
ACTION_EXPRESSION(SkillSwitchLoad,   const TArray<FSkillSwitchType>&)
ACTION_EXPRESSION(SkillSwitchReq,    const FSkillSwitchType&)
ACTION_EXPRESSION(SkillSwitchReqCompleted)
```

### 5.2 `USkillStore` (`Source/Q7/HUDStore/SkillStore.h:24-46, 95-101`)

상태:
- `TArray<FSkillSwitchType> HasSkillSwitchs` — 보유한 (해금된) 분기들
- `ESkillBranch CurrentSkillBranch` — 캐릭터의 현재 분기
- `FCMSSkillSwitchRow const* CurrentSkillSwitchRow / SwitchableSkillSwitchRow` — UI 캐시
- `bool bSwitchRequested` — 응답 대기 플래그

주요 메서드 (`SkillStore.cpp`):
- `ReqSkillSwitchUnlock` (90) — `FC2ZSkillSwitchUnlock` 송신
- `ReqSkillSwitch` (99) — `FC2ZSkillSwitch` 송신
- `CanSwitchBranch(ESkillBranch)` (606-624) — 베이스 분기는 무조건 true, 그 외엔 `HasSkillSwitchs` 보유 여부
- `OnHSActionSkillSwitchReq` (692-709) — ContentOpen 게이트 → `FC2ZSkillSwitch` 전송 + `SetSkillSwitchCooldown()`
- `OnHSActionSkillBranchChanged` (717-726) — 분기 변경 시 `Current/SwitchableSkillSwitchRow` 캐시 갱신

### 5.3 쿨다운 (`Source/Q7/ActionManager/CooldownManager.cpp/.h`)
- `TSharedPtr<FCooldownInfo> SkillSwitchCooldownInfo` (.h:33-96)
- `SetSkillSwitchCooldown()` — `SystemConst::SkillSwitchCooltime * 0.001s`
- `IsInSkillSwitchCooldown()` / `GetSkillSwitchCooldownInfo()`
- Tick 만료 시 자동 Reset

### 5.4 자동 프리셋과의 통합 (`ActionManager/ActiveSkillPresetManager.*`)
- 글로벌 헬퍼 `CanSwitchSkill(From, To)` (.cpp:15-32) — ContentOpen + From/To Row + Link Row + `USkillStore::CanSwitchBranch(To)` 모두 통과 필요
- 자동 사용 중 다른 분기 스킬을 만나면:
  1. `TOptional<FSkillSwitchType> SwitchToChange`에 후보 저장 (.h:229)
  2. `Pending` 상태 진입 → 쿨다운이 풀리면
  3. `EHSActionType::SkillSwitchReq` 디스패치 → 자동 전환
  4. 전환 완료 후 스킬 사용 재개 (.cpp:459-466, 1277)

---

## 6. UI / 위젯

### 6.1 메인 화면 — `GachaSwitchWidgets` (`Source/Q7/Widget/GachaSwitchWidgets.cpp`)
`EClassMenu::SkillSwitch` 진입점.

- `SkillSwitchRows: TMap<EClassWeapon, TArray<const FCMSSkillSwitchRow*>>` (36-50) — 무기군별 분기 목록
- `OnHSActionSkillSwitchUnlock` 핸들러 등록 (56)
- `PreviewSkillSwitchSceneActor` (103-111) — 좌/우 무기 프리뷰 액터
- 결제 팝업 (118-145) — `UnlockCostCategory`별 `Gem / Gold / Item` 팝업 → 확인 시 `ReqSkillSwitchUnlock` (149-153)
- `BtnSwitch` 활성화 / "해금 완료" 텍스트 (159-201)
- 디테일 팝업: `OpenPopup(SkillSiwtchPopupWidgetClass)` + `SetSkillSiwtchPopup(MainBranch, SubBranch)` (204-213) — *철자 SkillSiwtch (오타) 그대로 사용 중*

### 6.2 분기 선택 팝업 — `USkillBranchSelectPopupWidget` (`ActiveSkillPresetWidgets.cpp:57-89, 1573-1599, 1958-1972`)
- `SkillSwitchL`, `SkillSwitchR` 토글 버튼 + `SetSkillBranch(L, R, SelectedBranch)`
- HUD에서 `OpenSkillBranchSelectPopupWidget` / `Close...`로 제어
- ContentOpen 미오픈 시 `ConvertWeaponToBaseSkillBranch`로 강제 베이스 분기 폴백

### 6.3 전환 확인 팝업 — `WorldWidgets.cpp:2596-2629`
- `LinkRow->SwitchCostCategory`별 결제 정보 채우기
- "다시 묻지 않기" 체크박스: `SaveGame->GetSkillSwitchLastAskedUt() + SkillSwitchPopupHideDay`로 표시 여부 판정
- 즉시 전환은 `EHSActionType::SkillSwitchReq` 디스패치

### 6.4 프리셋 표시 — `PresetWidgets.cpp:1974-1976`
- 프리셋 슬롯에서 `HasSkillSwitch + IsMatchedSkillBranch`로 스킬 표시 여부 결정

> 참고: `SkillWidgets.cpp:371`의 `SkillSwitcher`는 이름만 비슷할 뿐 `UWidgetSwitcher`라서 SkillSwitch 시스템과 무관하다.

---

## 7. Unit / 런타임 적용 (`Source/Q7/Unit/Unit.cpp`)

- 1702-1803: `UnitStorePlayer.SkillBranch` 변경 감지 → `WorldPC->OnSkillBranchChanged()` + `ACTION_DISPATCH(SkillBranchChanged, ...)` 디스패치 → `USkillStore::OnHSActionSkillBranchChanged`로 전파
- 1994 `AUnit::GetSkillBranch()` — 현재 분기 반환
- 2808 `OnWeaponOrSkillBranchChanged(Weapon, Branch)` — 애님 인스턴스에 무기/분기 동시 통지
- 2835 `GetNormalSkillRow(GetSkillBranch())` — 분기에 따른 평타 스킬 선택
- 4782 `IsSkillBranchCondition()` — 스킬의 `SkillBranchGroupRow`와 현재 분기 매칭 → 사용 가능 여부

---

## 8. 치트 (`Source/Q7/Utils/Cheater.h:461-463 / .cpp:499-501`)

```
skill_switch <ck>
skill_switch_unlock <ck>
skill_switch_unlock_reset
```

`CommandProxy`로 서버에 raw cheat 송신.

---

## 9. 전체 흐름 요약

### 9.1 로그인
```
W2C LoadAccount.SkillSwitches[]
  → ACTION_DISPATCH(SkillSwitchLoad, [...])
  → USkillStore::HasSkillSwitchs 채움
```

### 9.2 해금 (Unlock)
```
GachaSwitchWidgets (UI)
  → 결제 팝업 (Gold/Gem/Item)
  → SkillStore::ReqSkillSwitchUnlock(type)
  → FC2ZSkillSwitchUnlock 송신
  → 서버 처리
  → FSkillSwitchUnlockTask
  → ACTION_DISPATCH(SkillSwitchUnlock, type)
  → HasSkillSwitchs.Add(type)
```

### 9.3 전환 (Switch)
```
WorldHUD / PresetManager
  → ACTION_DISPATCH(SkillSwitchReq, type)
  → SkillStore::OnHSActionSkillSwitchReq
       ├─ ContentOpen 게이트 체크
       ├─ FC2ZSkillSwitch 송신
       └─ CooldownManager::SetSkillSwitchCooldown()
  → 서버에서 UnitStore.SkillBranch 갱신
  → AUnit이 변경 감지
  → ACTION_DISPATCH(SkillBranchChanged, ...)
  → SkillStore::OnHSActionSkillBranchChanged
       └─ Current/SwitchableSkillSwitchRow 캐시 재계산
```

### 9.4 자동 프리셋 중 분기 자동 전환
```
프리셋 자동 사용 중 다른 분기 스킬 발견
  → SwitchToChange = TOptional<FSkillSwitchType> { ... }
  → Pending
  → 쿨다운 만료
  → ACTION_DISPATCH(SkillSwitchReq, ...)
  → 전환 완료 → 스킬 사용 재개
```

### 9.5 콘텐츠 게이팅
- `EContentOpenCategory::SkillSwitch` (M50)이 닫혀 있으면
  - `ConvertWeaponToBaseSkillBranch`로 베이스 분기 강제
  - `OnHSActionSkillSwitchReq`에서 송신 차단

---

## 10. 핵심 데이터 모델 다이어그램

```
EClassWeapon (무기군)
  └─ FCMSSkillSwitchRow ── ESkillBranch (분기)
       │   UnlockCost (Gold/Gem/Item)
       │
       └─ FCMSSkillSwitchLinkRow (같은 무기군 내 묶음)
              SwitchCost (Gold/Gem/Item)
              SkillSwitchIds[]

USkillStore
  ├─ HasSkillSwitchs[]    ← Load / Unlock 으로 누적
  ├─ CurrentSkillBranch   ← UnitStore에서 동기화
  └─ Current/SwitchableSkillSwitchRow (UI 캐시)

CooldownManager
  └─ SkillSwitchCooldownInfo (1초)

SaveGame
  └─ SkillSwitchLastAskedUt (팝업 억제용)
```

---

## 11. 주의 / 메모

- **Unlock 비용과 Switch 비용이 분리** — UI/검증을 만질 때 헷갈리기 쉬움
- **`HandleSkillSwitch`는 별도 Action 디스패치를 하지 않는다** — `SkillBranchChanged`는 `UnitStore.SkillBranch` 변경 경로에서 흘러나오므로, 패킷이 도착해도 곧바로 `SkillSwitch` Action이 떨어지진 않는다
- **`SkillSiwtch` 오타** — `GachaSwitchWidgets.cpp`의 팝업 클래스명/메서드(`SkillSiwtchPopupWidgetClass`, `SetSkillSiwtchPopup`)는 오타 상태로 유지되고 있음. 검색 시 주의
- **SkillBranch ↔ SkillBranchGroup 혼동 금지** — 전자는 분기 enum, 후자는 *어떤 스킬이 어느 분기에서 쓰일 수 있는지*를 정의하는 별도 테이블
- **쿨다운 1초**는 `SystemConst.SkillSwitchCooltime`로 관리 — 데이터에서 조정 가능
- **콘텐츠 미오픈 시 강제 베이스 분기**는 클라 측에서도 `ConvertWeaponToBaseSkillBranch`로 폴백 처리하므로, 시스템 점검 등으로 닫혔을 때 안전한 동작이 보장됨
