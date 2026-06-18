# DesertDragon 전투 패턴

## 설계 컨셉
- 전투 중 비행 없음 — 지상 전투만 사용
- HP 30% 이하 시 둥지로 후퇴 — 날아가거나 걸어가는 것 중 랜덤 선택
- 전투 패턴은 거리와 무관하게 혼합 선택 — 근접이어도 화염을 쓰고, 원거리여도 돌진

---

## 공격 목록

### 지상 공격

| 어빌리티 태그 | 애니메이션 | 클래스 | 사거리 |
|--------------|-----------|--------|--------|
| `Ability.Monster.Bite` | BiteAttack_Montage | BP (UMRAbility_MonsterAttack) | 근접 ≤ 300 |
| `Ability.Monster.ClawAttack` | ClawsAttackLeft/Right_Montage | BP (UMRAbility_MonsterAttack) | 근접 ≤ 300 |
| `Ability.Monster.TwoHitCombo` | 2HitComboClawsAttackForward_Montage | BP (UMRAbility_MonsterAttack) | 근접 ≤ 300 |
| `Ability.Monster.SpitFireBall` | SpitFireBall_Montage | BP (UMRAbility_MonsterAttack) | 제한 없음 |
| `Ability.Monster.SpreadFire` | SpreadFire_Montage | `UMRAbility_MonsterSpreadFire` | ≤ 800 |

### 전환 어빌리티 (둥지 이동 시 사용)

| 어빌리티 태그 | 애니메이션 | 클래스 |
|--------------|-----------|--------|
| `Ability.Monster.TakeOff` | TakeOff_Montage (변환 필요) | `UMRAbility_MonsterTakeOff` |
| `Ability.Monster.Land` | FlyStationaryToLanding_Montage | `UMRAbility_MonsterLand` |

---

## Blackboard 키 목록

| 키 이름 | 타입 | 초기화 위치 | 용도 |
|--------|------|-----------|------|
| `TargetActor` | Object | DetectPlayer 서비스 | 현재 타겟 플레이어 |
| `HomeLocation` | Vector | AIController.OnPossess | 스폰 위치 (배회 복귀) |
| `IsFlying` | Bool | AIController.OnPossess | 비행 상태 동기화 |
| `NestLocation` | Vector | MRMonsterSpawner | 둥지 위치 (후퇴 목표) |

### 둥지 위치 설정
`MRMonsterSpawner`의 `FMRSpawnEntry.NestLocationOffset` 으로 설정.
- 스포너 액터 위치 기준 로컬 오프셋 (cm)
- `FVector::ZeroVector`면 둥지 시스템 미사용

---

## C++ 파일 목록

| 파일 | 경로 | 용도 |
|------|------|------|
| `MRAbility_MonsterTakeOff.h/.cpp` | `GAS/Ability/` | 이륙 → Flying 태그 + MovementMode + BB 설정 |
| `MRAbility_MonsterLand.h/.cpp` | `GAS/Ability/` | 착지 → Flying 태그/MovementMode/BB 해제 |
| `MRAbility_MonsterSpreadFire.h/.cpp` | `GAS/Ability/` | 전방 콘 AOE 브레스 (타이머 기반 틱 데미지) |
| `MRAnimNotify_SpawnMonsterProjectile.h/.cpp` | `Animation/` | BB TargetActor 방향 프로젝타일 스폰 |
| `MRBTDecorator_CheckHealth.h/.cpp` | `AI/` | HP 비율 BT 조건 (HP ≤ X%) |
| `MRBTTask_FlyTo.h/.cpp` | `AI/` | NavMesh 없이 직선 비행 이동 |
| `MRBTComposite_WeightedRandom.h/.cpp` | `AI/` | 가중치 기반 랜덤 자식 선택 Composite |

GameplayTags 추가: `Ability.Monster.FlySpitFireBall`, `Ability.Monster.FlySpreadFire`

---

## BehaviorTree 구조

```
Root
└── Selector [Service: DetectPlayer]
    │
    ├── [1] Sequence ── 둥지 후퇴 (HP ≤ 30%, NestLocation 있는 경우)  ★ 최고 우선순위
    │   ├── Decorator: CheckHealth (HP ≤ 0.30)
    │   ├── Decorator: BB NestLocation IsSet
    │   ├── Decorator: BB TargetActor IsSet
    │   └── WeightedRandom ── [이동 방법 랜덤]
    │       ├── Sequence [날아서 이동] [Weight=1.0]
    │       │   ├── Task: ActivateAbility (Ability.Monster.TakeOff)
    │       │   ├── Task: FlyTo (NestLocation, FlightHeight=300, AcceptableRadius=400)
    │       │   └── Task: ActivateAbility (Ability.Monster.Land)
    │       └── Task: MoveTo (NestLocation, AcceptableRadius=200) [Weight=1.0]
    │
    ├── [2] Sequence ── 전투 (타겟 있을 때)
    │   ├── Decorator: BB TargetActor IsSet
    │   ├── Decorator: IsInRange (1500)           ← 전투 반경 밖은 [3]추적으로 넘김
    │   ├── [Service: FaceTarget]
    │   ├── WeightedRandom ── 공격/이동 패턴 혼합
    │   │
    │   │   ── 근접 공격 (IsInRange 300 이내에서만 성공) ──
    │   │   ├── Task: Bite         [W=2.0] + Decorator: IsInRange(300)
    │   │   ├── Task: ClawAttack   [W=2.0] + Decorator: IsInRange(300)
    │   │   ├── Task: TwoHitCombo  [W=1.5] + Decorator: IsInRange(300)
    │   │
    │   │   ── 화염 공격 (거리 제한 없거나 넓음) ──
    │   │   ├── Task: SpitFireBall [W=2.5]                              ← 항상 선택 가능
    │   │   ├── Task: SpreadFire   [W=1.5] + Decorator: IsInRange(800)
    │   │
    │   │   ── 돌진+근접 콤보 (거리 무관하게 강제 접근) ──
    │   │   └── Sequence [돌진 후 물기] [W=2.0]
    │   │       ├── Task: MoveTo (TargetActor, AcceptableRadius=250)    ← 근접까지 돌진
    │   │       └── WeightedRandom
    │   │           ├── Task: Bite       [W=1.0]
    │   │           └── Task: ClawAttack [W=1.0]
    │   │
    │   └── Task: Wait (0.8~2.0초)   ← 공격 후 짧은 호흡
    │
    ├── [3] Sequence ── 추적
    │   ├── Decorator: BB TargetActor IsSet
    │   └── Task: MoveTo (TargetActor)
    │
    └── [4] Task: Wait (1~3초) ── [Idle]
```

### 거리별 실제 동작

| 거리 | 선택 가능한 패턴 | 효과 |
|------|----------------|------|
| ≤ 300 (근접) | Bite/Claw/TwoHit + SpitFireBall + SpreadFire + 돌진콤보 | 근접 공격 위주지만 화염도 섞임 |
| 300~800 (중거리) | SpitFireBall + SpreadFire + **돌진콤보** | 화염 공격 + 30%확률로 돌진 |
| 800~1500 (원거리) | SpitFireBall + **돌진콤보** | 화염구 + 40%확률로 돌진 |

> **WeightedRandom 동작 원리**: 가중치로 순서를 무작위화 후 순차 시도.
> 자식에 붙은 Decorator(IsInRange)가 실패하면 해당 자식을 건너뛰고 다음 시도.
> 모두 실패하면 Sequence 전체가 Failed → [3] 추적으로 폴백.

> **돌진 콤보**: MoveTo가 근접 범위(250)까지 이동 후 Bite/Claw를 실행.
> 이미 근접이면 MoveTo가 즉시 성공하므로 바로 공격. 멀리 있으면 달려와서 공격.

---

## Blueprint 설정 체크리스트

### BB Data Asset 키 추가

`BT_DesertDragon`의 BlackboardAsset에 다음 키를 추가해야 한다:

| 키 이름 | 타입 | 비고 |
|--------|------|------|
| `NestLocation` | Vector | 기본값 (0,0,0) — 스포너에서 런타임 설정 |

### 어빌리티 BP

| BP 이름 | 부모 클래스 | 주요 설정 |
|---------|------------|---------|
| `BP_Ability_Monster_Bite` | UMRAbility_MonsterAttack | AttackMontage=BiteAttack_Montage, DamageMultiplier=1.0 |
| `BP_Ability_Monster_ClawAttack` | UMRAbility_MonsterAttack | AttackMontage=ClawsAttackLeft_Montage, DamageMultiplier=0.8 |
| `BP_Ability_Monster_TwoHitCombo` | UMRAbility_MonsterAttack | AttackMontage=2HitComboClawsAttackForward_Montage, DamageMultiplier=1.2 |
| `BP_Ability_Monster_SpitFireBall` | UMRAbility_MonsterAttack | AttackMontage=SpitFireBall_Montage, DamageMultiplier=1.5 |
| `BP_Ability_Monster_SpreadFire` | UMRAbility_MonsterSpreadFire | AttackMontage=SpreadFire_Montage |
| `BP_Ability_Monster_TakeOff` | UMRAbility_MonsterTakeOff | TakeOffMontage=TakeOff_Montage |
| `BP_Ability_Monster_Land` | UMRAbility_MonsterLand | LandMontage=FlyStationaryToLanding_Montage |

모든 BP Ability에 AssetTags 설정 필수 (BT의 `ActivateAbility` Task가 태그로 매칭).

### 프로젝타일 BP

| BP 이름 | 부모 클래스 | 설정 |
|---------|------------|------|
| `BP_FireballProjectile` | AMRProjectile | MeshComponent 파이어볼 메시, CollisionComponent 반경=40, LifeSpan=5 |

### AnimMontage 설정

| 몬타주 | 추가할 AnimNotify |
|--------|-----------------|
| SpitFireBall_Montage | `UMRAnimNotify_SpawnMonsterProjectile` — ProjectileClass=BP_FireballProjectile, SpawnSocketName=jaw_01 |
| BiteAttack_Montage | `UMRAnimNotifyState_MeleeHitbox` |
| ClawsAttackLeft/Right_Montage | `UMRAnimNotifyState_MeleeHitbox` |
| 2HitComboClawsAttackForward_Montage | `UMRAnimNotifyState_MeleeHitbox` × 2 |
| **TakeOff (AnimSequence)** | **→ AnimMontage로 변환 필요** (우클릭 → Create AnimMontage) |

### DesertDragon BP

- `DefaultAbilities`에 위 모든 BP Ability 등록
- AI Controller의 `BehaviorTree` 프로퍼티에 `BT_DesertDragon` 할당

---

## UMRBTTask_FlyTo 파라미터 가이드

| 프로퍼티 | 기본값 | 설명 |
|---------|--------|------|
| AcceptableRadius | 300 | 도착 판정 거리 |
| FlightHeight | 600 | 타겟 위치 기준 Z 오프셋 |

- `bUsePathfinding=false` 로 NavMesh 무시, 직선 비행
- **MOVE_Flying 전환이 먼저 되어 있어야 정상 동작** (TakeOff 어빌리티가 처리)

---

## SpreadFire 파라미터 가이드

| 프로퍼티 | 기본값 | 설명 |
|---------|--------|------|
| ConeHalfAngleDeg | 45° | 브레스 폭 |
| ConeRange | 800 | 최대 사거리 |
| DamageTickInterval | 0.3s | 데미지 판정 간격 |
| DamageStartDelay | 0.5s | 몬타주 시작 후 첫 피해까지 딜레이 |
| DamageMultiplier | 0.3 | 틱당 데미지 배율 |
