# 유닛 애니메이션(Unit Animation) 설계 문서

> 대상: Q7 Unreal 클라이언트 (`Source/Q7/Unit`)
> 목적: 유닛(캐릭터/몬스터/NPC) 애니메이션 시스템이 **어떻게** 구현되어 있고, **왜** 이렇게 되어 있으며, **더 나은 방식**은 무엇인지 정리한다.

---

## 0. 한눈에 보는 요약

- 애니메이션의 "두뇌"는 C++가 아니라 **AnimBlueprint의 AnimGraph 상태 머신**이다.
- C++ 코드(`AUnit`, `UUnitAnimInstance`)는 **상태 플래그(StateFlag)** 와 **컨텍스트(StateContext)** 만 채워주고, 실제 어떤 클립을 어떻게 블렌딩할지는 AnimGraph가 결정한다.
- **`AnimMontage`는 거의 쓰지 않는다.** 대신 상태 머신 + BlendSpace + 상태 플래그 조합으로 동작한다.
- 애니메이션 데이터는 **MMO 규모**에 맞춰 전부 **비동기 스트리밍**으로 로드되고, **AnimGroup + AnimName 2단계 키**로 유닛/스킬에 매핑된다.
- 멀티스레드 애니메이션 평가를 위해 모든 상태가 **AnimInstanceProxy** 구조체에 복제되어 워커 스레드에서 읽힌다.

---

## 1. 클래스 계층 구조

```
UAnimInstance (엔진)
└─ UQ7AnimInstance              프로젝트 공통 베이스: 비동기 스트리밍 로드/캐싱, HitDirection 유틸
   └─ UUnitAnimInstance         유닛 공통: Idle/Combat/Skill/Hit/CC/Dead/RandomIdle 상태 진입점
      ├─ UMonsterAnimInstance   몬스터 전용: Spawning, SkillCharging, Run
      └─ UCharacterAnimInstance 캐릭터 전용: 무기별 Idle, Sprint, Mounted, Teleport, SocialAction, Preview
```

| 클래스 | 위치 | 역할 |
|--------|------|------|
| `UQ7AnimInstance` | `Q7AnimInstance.h:36` | 공통 베이스. `LoadAnimations()`, `GatherAnimationPaths()`, `GameAssetCache` 비동기 로드, `GetHitDirectionFromLaunchNotify()` |
| `UUnitAnimInstance` | `UnitAnimInstance.h:505` | 유닛 상태 진입점. `SetSkill/SetHit/SetCombat/SetMoving/SetCrowdControl/SetDead` |
| `UMonsterAnimInstance` | `UnitAnimInstance.h:717` | `SetSpawning`, `SetSkillCharging` 추가 |
| `UCharacterAnimInstance` | `UnitAnimInstance.h:792` | 마운트/순간이동/소셜액션/프리뷰 |

### AnimInstanceProxy (멀티스레드 애니 평가)

AnimGraph가 워커 스레드에서 안전하게 읽을 수 있도록, 애님 시퀀스 포인터·BlendSpace·상태 플래그를 **Proxy 구조체**에 복제한다.

| 구조체 | 위치 |
|--------|------|
| `FQ7BaseAnimInstanceProxy` | `Q7BaseAnimInstanceProxy.h:16` — `BindStateMachineTransitions` 템플릿, `AddSafeNativeTransitionBinding` |
| `FUnitAnimInstanceProxy` | `UnitAnimInstanceProxy.h:18` |
| `FMonsterAnimInstanceProxy` | `UnitAnimInstanceProxy.h:82` |
| `FCharacterAnimInstanceProxy` | `UnitAnimInstanceProxy.h:116` |

---

## 2. 핵심 데이터 구조 (C++ ↔ AnimGraph 인터페이스)

AnimGraph가 읽는 "입력값"은 세 종류의 구조체로 정리되어 있다. C++가 이 값들을 채우면 AnimGraph가 그에 맞춰 클립을 고른다.

### 2.1 StateFlag — "지금 어떤 상태인가" (비트필드 bool)

- `FUnitAnimStateFlag` (`UnitAnimInstance.h:398`):
  `bInIdle / bRandomIdle / bCombat / bMoving / bPutAwayWeapon / bSkill / bSkillRelayed / bSkillChanneling / bSkillWaiting / bHit / bHitPause / bDead / bCrowdControl / bUseCrowdControlSnapshot / bCrowdControlLoopEnd`
- `FMonsterAnimStateFlag` (`:449`):
  `bSpawning / bRagdollized / bSkillCharging / bSkillChargingRelayed / bHoldSkillState`
- `FCharacterAnimStateFlag` (`:470`):
  `bPreviewMode / bRandomPreview / bDash / bOperate / bTeleportBegin / bTeleportEnd / bSocialAction / bStopSocialAction`

> 이 플래그들은 그대로 AnimGraph 상태 머신의 **전이 조건(Transition Rule)** 으로 쓰인다.

### 2.2 StateContext — "어떻게 재생할 것인가" (연속값/방향)

- `FUnitAnimStateContext` (`:323`): `Speed`, `RunPlayRate`, `SkillPlayRate`, `SkillPlayEndTime`, `HitPauseFrameTime`, `SkillChargePlayRate`, `HitDirection`, `bHasValidSkillChannelingEnd`, `bRandomIdleForbidden`, `bHasValidSkillEndLoop`
- `FCharacterAnimStateContext` (`:359`): `SkillAimDegree`, `SkillLayeredBlendOption`, `bSkillMovable`, `bSkillUsingAimOffset`, `bIsMounted`, `SocialActionKind`, `SocialActionStartPosition` 등

### 2.3 AnimSequence 데이터셋 — "어떤 클립을 쓸 것인가" (DataAsset)

- `UCombatAnimSequenceData` (`:249`): CombatIdle/Run/Sprint/Hits/Stunned/Floating/FallDown/Dead 전투 애님 세트
- `UMountedAnimSequenceData` (`:298`): 마운트 상태 애님 세트
- `FSkillAnims` / `FSkillAnimTimes` / `FSkillAnimData` (`:73, 121, 206`): 스킬별 Default/Critical/Sub 애님 + Launch/Hit/MotionEnd 타이밍

---

## 3. 재생 흐름: 서버 패킷 → 화면 애니메이션

스킬 사용을 예로 든 전체 경로다.

```
[서버 → 클라 패킷]  FZ2CSkillBegin
   │
   ▼
AUnit::HandleSkillBegin(Packet)            Unit.cpp:5713
   │   (SkillRow/SkillInfoRow 조회, UsageTime 계산)
   ▼
AUnit::SetSkill(SkillId, SkillRow, ...)    Unit.cpp:4833
   │   (Pending 등록, HaltTime 설정)
   ▼
AUnit::PlaySkill(SkillRow, ...)            UnitSkillPlay.cpp:569
   │   (Ship/Mount 분기 → AnimInstance 캐스팅)
   ▼
UUnitAnimInstance::SetSkill(...)           UnitAnimInstance.cpp:514
   │   (CurSkillAnimation 결정, StateFlag.bSkill = true,
   │    StateContext.SkillPlayRate/EndTime 세팅)
   ▼
[AnimGraph 상태 머신이 StateFlag/Context/Proxy를 읽어
 해당 스킬 클립으로 전이 & 블렌딩하여 재생]
```

**다른 상태도 동일한 패턴**이다. `AUnit`이 서버 이벤트를 받고
`Cast<UUnitAnimInstance>(GetMesh()->GetAnimInstance())` 후
`SetCombat / SetMoving / SetHit / SetCrowdControl / SetDead`를 호출 →
AnimInstance가 플래그/컨텍스트를 갱신 → AnimGraph가 그림을 그린다.

주요 진입점:

| 흐름 | 패킷 핸들러 | → 재생 게이트 | → AnimInstance |
|------|------------|---------------|----------------|
| 스킬 | `HandleSkillBegin` `Unit.cpp:5713` | `PlaySkill` `UnitSkillPlay.cpp:569` | `SetSkill` `:514` |
| 채널링 | `HandleSkillEffectChanneling` `Unit.cpp:5999` | `PlaySkillChanneling` `:358` | `SetSkillChanneling` |
| 차징 | `HandleSkillCharge` `Unit.cpp:5786` | `PlaySkillCharging` `:369` | `SetSkillCharging` (몬스터) |
| 피격 | `PlayHit` `Unit.cpp:5032` | `SetHit`(내부) `:395` | `SetHit` `:660` |
| 사망 | — | — | `SetDead` `:976` |
| 내 유닛 스킬 요청(클라→서버) | `ReqUseSkill` `Unit.cpp:4809` → `Send(FC2ZUseSkill)` + `FSkillActionManager::OnUseSkillReq` `SkillActionManager.cpp:60` | | |

---

## 4. 상태 관리 방식: AnimMontage가 아니라 State Machine

이 프로젝트의 가장 큰 특징은 **AnimMontage를 거의 쓰지 않는다**는 것이다.

### 4.1 State Machine (AnimBP)
AnimGraph 상태 머신이 핵심 두뇌다. C++는 Proxy의 `Bind_StateMachine_Transitions_*` 함수로 **네이티브 전이 조건(C++ 람다)** 을 바인딩한다 (`UnitAnimInstanceProxy.h:78, 108~113, 210~219`).
상태 머신 예: `_IdleRun`, `_CombatIdleRun`, `_Locomotion`, `_Skill`, `_CrowdControl`, `_MountedIdleRun`, `_Preview`, `_Main`.

전이 바인딩은 베이스의 `AddSafeNativeTransitionBinding`(`Q7BaseAnimInstanceProxy.h:44`)을 통해 등록된다. `FrequencyLevel` 인자로 평가 빈도를 조절할 수 있다.

### 4.2 BlendSpace (이동·조준)
이동은 몽타주 대신 BlendSpace를 쓴다 — `RunBlendSpace`, `CombatRunBlendSpace`(`UnitAnimInstanceProxy.h:140, 149`). 스킬 조준도 AimOffset BlendSpace(`CurSkillAimOffsetBlendSpace`)로 처리한다. C++는 `StateContext.Speed`/`SkillAimDegree`만 넘기면 된다.

### 4.3 AnimNotify (게임 로직 동기화)
"애니메이션의 N번째 프레임"에 게임 이벤트를 동기화하기 위해 Notify를 적극 사용한다.

- **로직 Notify**(AnimInstance 콜백): `AnimNotify_SkillRelayed`, `AnimNotify_SkillWaitingStart/End`, `AnimNotify_HitStart`, `AnimNotify_IdleStart/End`, `AnimNotify_SpawningStart`(몬스터), `AnimNotify_TeleportBeginStart`, `AnimNotify_SocialAction`(캐릭터) 등 (`UnitAnimInstance.h:592~885`).
- **연출 Notify**(별도 모듈 `Source/Q7/AnimNotifies/`):
  `ULaunch`(히트 방향 발사/넉백), `UAnimNotify_CameraShake`, `UAnimNotifyState_TimedBeamEffect`, `UAnimNotify_Q7PlayNiagaraEffect`(풀링·아군/적 구분), `UAnimNotifyState_Q7PlaySound`(물리 표면별 사운드), `UChangeEquipSocket`(장비 소켓 변경) 등.

흥미로운 역방향 활용: `ULaunch` Notify의 `HitDirection`을 `UQ7AnimInstance::GetHitDirectionFromLaunchNotify`(`Q7AnimInstance.h:87`)가 **거꾸로 읽어서** 피격 방향 애니를 결정한다.

---

## 5. 데이터 로딩 & 매핑

### 5.1 에셋 로우 (GameResource DataTable)
`Source/Q7/Resource/GameResource.h`:
- `FModelAssetRow`(`:412`): `AnimInstanceClass`(`:400`, AnimBP 클래스), `AnimGroup`(`:424`, 애님 그룹 키), `CombatAnimSequenceData`(`:451`)
- `FSkillAnimAssetRow`(`:661`): `AnimGroup`(`:666`) 기준 스킬 애님 매핑
- 테이블: `SkillAnimAssetTable`(`:2889`), `SkillAnimTimesTable` 등

### 5.2 2단계 키 매핑 (AnimGroup + AnimName)
스킬 애님은 **(AnimGroup, AnimName)** 두 단계 키로 조회된다.

- `UGameResource::GetSkillAnimData(AnimGroup, AnimName, OutData)` `GameResource.cpp:393`
- `GetSkillAnimAssetRow` `:424`, `GetSkillAnimTimes` `:440`
- 클래스 캐릭터 전투 세트: `GetClassWeaponCombatAnimSet(Race, Gender, Weapon, Height, bAirBone)` `:334`
  → 종족/성별/무기/체형 조합으로 전투 애님 세트 선택
- 기타: `GetMonsterSpawnAnimAssetRow` `:572`, `GetMountedAnimSetRow` `:712`, `GetSocialActionAnimAssetRow` `:631`

AnimGroup 키는 스폰 시 주입된다: `AnimInst->SetAnimGroup(ModelAssetRow->AnimGroup)` (`Unit.cpp:970`). 스킬 재생 시 `OutSkillPlayInfo.AnimGroupName`으로 전달되어 타이밍 계산에 쓰인다 (`UnitSkillPlay.cpp:598`, `FUnitCombatState::GetSkillTimes` `Unit.cpp:4881`).

### 5.3 비동기 스트리밍 로딩 흐름
```
AUnit::InitEquippedWeapon()          Unit.cpp:983
   │  (몬스터: AnimInstanceClass 확인 → Spawning/CombatAnimSet/스킬 애님 등록)
   ▼
AUnit::LoadAnimations()              Unit.cpp:1524
   ▼
UUnitAnimInstance::LoadAnimations(LoadingOption)   (UQ7AnimInstance 베이스)
   ▼
GatherAnimationPaths(...)            UnitAnimInstance.cpp:265(Unit)/1037(Monster)/1214(Character)
   │  (AddValidAnimation으로 SoftObjectPath 수집)
   ▼
GameAssetCache 비동기 스트리밍 → OnStreamingCompleted → OnProxyStaticDataLoad
   │  (Proxy의 IdleAnimation/RunBlendSpace 등 static 포인터 채움)
```

- AnimBP 클래스 자체도 `FModelAssetRow::AnimInstanceClass`로 지정되어 메시와 함께 스트리밍된다 (`GameResource.cpp:50, 140`).
- 테이블은 에디터 커맨드릿으로 생성: `GenerateSkillAnimInfoTableCommandlet`, `GenerateMonsterSpawnAnimTableCommandlet` (`Source/Q7Editor/Commandlet/`).

---

## 6. 왜 이렇게 구현했는가 (설계 의도)

### 6.1 왜 Montage가 아니라 State Machine + StateFlag인가
- **MMO 동기화 모델과 맞다.** 서버가 권위를 갖고 "이 유닛은 지금 스킬 중/피격/사망" 같은 **상태**를 통보한다. 몽타주처럼 "이 클립을 재생하라"는 명령형보다, "현재 상태는 X"라는 선언형이 서버-클라 상태 동기화와 자연스럽게 맞물린다. 명령이 유실되면 몽타주는 어긋난 채 남지만, 선언형 플래그는 매 프레임 최신 상태로 **수렴**한다.
- **수백 유닛 동시 처리.** 화면에 수십~수백 유닛이 있는 MMO에서, 몽타주를 일일이 명령하면 관리 비용이 폭증한다. 플래그만 갱신하고 평가를 AnimGraph(워커 스레드)에 맡기면 비용이 분산된다.
- **아티스트 친화적.** 전이 로직과 블렌딩을 AnimBP에서 편집할 수 있어, C++ 재컴파일 없이 아티스트/애니메이터가 반복 작업할 수 있다.

> **⚠️ 정정 — "상태 진입 = 타이밍 자동 해결"은 과장.**
> 상태 머신이 어떤 상태로 전이해도 그 노드의 클립은 기본적으로 **position 0부터** 재생된다. 경과 시점으로 자동 seek하지 않는다. 정확히는 두 경우를 구분해야 한다.
> - **루프 상태**(Idle/Run/Combat/CC 루프): 위상(phase)이 무의미하므로 `bMoving=true`만으로 자연스럽게 진입한다. 유닛이 행동 중인 상태로 시야에 스트리밍 인 되어도 phase 0부터 돌든 중간부터 돌든 눈에 띄지 않는다 — **이 영역에서만 선언형이 거저 얻는 이점이 성립**한다.
> - **1회성 타이밍 클립**(스킬): 선언형이든 몽타주든 타이밍을 자동으로 못 맞춘다. 이 프로젝트는 중간 지점으로 seek하지 않고, **play-rate 스케일링**으로 클립 자연 길이를 서버 시전 시간(`UsageTime`)에 맞춘다:
>   ```cpp
>   // UnitAnimInstance.cpp:594-595
>   StateContext.SkillPlayEndTime = UnitUtil::GetSkillPlayEndTime(CurSkillAnimTime, UsageTime);
>   StateContext.SkillPlayRate    = StateContext.SkillPlayEndTime / UsageTime; // 예: 2초 클립 / 1초 시전 → 2배속
>   ```
>   종료 판정도 전이 규칙에서 `GetRelevantAnimTime >= SkillPlayEndTime`으로 확인한다 (`UnitAnimInstanceStateMachineTransitionRules.cpp:983`). 즉 "이미 진행 중인 스킬의 중간 합류(catch-up)"를 정밀히 seek하는 게 아니라, 스킬이 짧고 `begin` 이벤트를 트리거로 처음부터 재생하는 것을 전제로 **총 길이만 맞추는** 타협이다.

### 6.2 왜 AnimInstanceProxy로 상태를 복제하는가
언리얼의 멀티스레드 애니메이션 업데이트는 AnimGraph를 **게임 스레드가 아닌 워커 스레드**에서 평가한다. 게임 스레드가 만지는 AnimInstance 멤버를 워커가 직접 읽으면 레이스가 난다. 그래서 평가에 필요한 값만 Proxy에 스냅샷처럼 복제한다. 수백 유닛의 애니 평가를 병렬화하기 위한 필수 구조다.

### 6.3 왜 비동기 스트리밍 + 2단계 키인가
- **메모리.** 모든 클래스/몬스터/스킬 애님을 상주시키면 모바일(Android/iOS) 메모리가 버티지 못한다. 그래서 `TSoftObjectPtr` + `GameAssetCache`로 필요할 때만 스트리밍한다.
- **조합 폭발 관리.** 캐릭터 애님은 (종족 × 성별 × 무기 × 체형)의 조합이고, 스킬은 (AnimGroup × AnimName)의 조합이다. 직접 에셋을 하드코딩하는 대신 **데이터 테이블 키 조회**로 풀어, 신규 클래스/스킬 추가 시 코드 변경 없이 데이터만 늘리면 된다.

### 6.4 왜 AnimNotify에 게임 로직을 거는가
"검이 적에게 닿는 순간", "발사 타이밍" 같은 건 **클립의 특정 프레임**과 묶여야 자연스럽다. 코드 타이머로 흉내내면 클립 길이/재생속도 변화에 어긋난다. Notify는 그 프레임에 정확히 콜백을 주므로 연출-로직 동기화에 적합하다.

---

## 7. 트레이드오프 & 한계

| 항목 | 비용 |
|------|------|
| 로직이 C++와 AnimBP에 **분산** | 디버깅 시 양쪽을 오가야 함. "왜 이 클립이 안 나오지?"가 전이 규칙(AnimBP)인지 플래그(C++)인지 추적이 어렵다 |
| **StateFlag 비트 증가** | 상태가 늘수록 비트가 늘고, AnimBP 전이 규칙도 같이 늘어 복잡도가 누적 |
| AnimBP는 **머지/리뷰가 어려움** | 바이너리 에셋이라 코드처럼 diff/PR 리뷰가 안 됨 |
| **3계층 상속 + Proxy 4종** | 신규 유닛 타입 추가 시 보일러플레이트(GatherAnimationPaths, Proxy static 데이터 바인딩 등)가 많음 |
| 데이터 테이블 키 오타 | (AnimGroup, AnimName) 불일치 시 런타임에 조용히 누락 — `AddValidAnimation`의 missing 로그에 의존 |

---

## 8. 더 나은 방식은 없는가 (개선 제안)

> 현 구조는 "MMO + 모바일 + 대규모 유닛"이라는 제약에서 합리적인 선택이다. 아래는 **점진적 개선** 위주이며, 전면 교체를 권하지는 않는다.

### 8.1 권장 (실효성 높음)
1. **State Tree / 데이터 기반 전이로 일부 이관.** UE5의 StateTree는 상태/전이를 에셋+C++ 혼합으로 기술해 머지·리뷰·디버깅이 AnimBP보다 낫다. 신규 상태군부터 시범 적용해볼 만하다.
2. **상태 전이를 코드에서 단일 진입점으로.** 현재 `SetSkill/SetHit/...`가 산발적으로 플래그를 만진다. `RequestState(EUnitAnimState)` 같은 **명시적 상태 전이 함수** 하나로 모으면 "어떤 상태에서 어떤 상태로 갈 수 있는가"가 코드에 드러나고 잘못된 조합을 막을 수 있다.
3. **키 검증 자동화.** (AnimGroup, AnimName) 테이블을 커맨드릿/자동화 테스트(`Source/Q7/Automation/`)에서 빌드 타임에 교차 검증해, 오타·누락을 런타임 로그가 아니라 CI에서 잡는다.
4. **AnimBP 변경 로그/문서화 규약.** 바이너리라 diff가 안 되므로, 전이 규칙 변경 시 동반 문서/커밋 메시지 규약을 두면 추적성이 올라간다.

### 8.2 검토 가치는 있으나 신중해야 함
5. **Motion Matching (UE5.4+).** 이동/전투 로코모션 품질을 크게 올릴 수 있으나, 데이터·메모리·연산 비용이 커서 모바일 타겟에선 부담. PC 고사양 한정 옵션으로만 고려.
6. **Layered Animation 정리.** 이미 `SkillLayeredBlendOption`이 있으나, 상체/하체 분리를 더 체계화하면 "이동 중 스킬" 류 상태 폭발을 줄일 수 있다.

### 8.3 권장하지 않음
7. **Montage 전면 전환.** 서버 권위 상태 동기화 모델과 충돌하고, 대규모 유닛 관리 비용이 오히려 커진다. 현 구조의 근간을 흔드는 선택.

---

## 9. 빠른 참조 (file_path:line)

**상태 진입점** (`UnitAnimInstance.cpp`): `SetSkillRow` `:217`, `NativeUpdateAnimation` `~330`, `SetCombat` `:502`, `SetSkill` `:514`, `SetHit` `:660`, `SetDead` `:976`
**경로 수집**: Unit `:265`, Monster `:1037`, Character `:1214`
**패킷 핸들러** (`Unit.cpp`): `HandleSkillBegin` `:5713`, `HandleSkillCharge` `:5786`, `HandleSkillEffectChanneling` `:5999`, `SetSkill` `:4833`, `BeginSkillChanneling` `:4909`, `InitEquippedWeapon` `:983`, `LoadAnimations` `:1524`, `AddSkillAnimData` `:1538`
**재생 게이트** (`UnitSkillPlay.cpp`): `PlaySkill` `:569`, `PlaySkillChanneling` `:358`, `PlaySkillCharging` `:369`
**데이터 조회** (`GameResource.cpp`): `GetSkillAnimData` `:393`, `GetClassWeaponCombatAnimSet` `:334`

**관련 모듈 문서**: `Source/Q7/Unit/CLAUDE.md`, `Source/Q7/AnimNotifies/CLAUDE.md`
**별도 AnimInstance**: `ShipAnimInstance.h`, `Mount/MountAnimInstance.h`, `Groa/GroaAnimInstance.h`, `Doodad/DoodadAnimInstance.h`, `SpawnAnimInstance.h`
