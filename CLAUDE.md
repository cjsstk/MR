# Project: MR

## 프로젝트 개요
- 언리얼 엔진 5.5, C++20
- 장르: 3인칭 액션 RPG
- 몬스터헌터 라이즈 같은 [마을 - 필드] 구조, 싱글 플레이
- 핵심 기능: 착용 무기에 따라 애니메이션과 공격 콤보가 바뀜

## 코딩 컨벤션
- 언리얼 코딩 표준 준수 (UE Coding Standard)
- `if`는 항상 중괄호 사용, 중괄호는 다음 줄에서 시작
- 임시/스텁 코드 금지 — 항상 실제로 동작하는 코드로 구현
- 복잡한 구현에는 간단한 주석 추가
- 성능, 최적화, 가독성, 재사용성 고려
- 더 좋은 구조가 있으면 적극적으로 변경 가능
- 로그 코드는 지우지 않을 것

## 자주 쓰는 패턴
- 컴포넌트는 항상 `ObjectInitializer`로 생성
- GAS 어빌리티 내에서 캐릭터 접근: `GetBaseCharacter()` / `GetPlayerCharacter()`
- Store 구독은 `BindStore()`, 해제는 `NativeDestruct()`에서 자동 처리
- 에셋 로드는 `UMRGameResource`의 `AsyncLoad*` 계열 함수 사용 (StreamableManager 기반)
- DataTable 조회는 `UCMSSubsystem::GetRow<T>()` 사용

## 모듈 구조

```
Source/
├── MRStore/          MVVM 베이스 모듈 (StoreBase, MVVMWidgetBase). 타 프로젝트 재사용 가능
└── MR/               게임 로직 메인 모듈
    ├── GAS/          GameplayAbilitySystem (Ability, Attribute, Effect, Tags)
    ├── Character/    플레이어/몬스터 캐릭터
    ├── Animation/    AnimInstance 클래스
    ├── Component/    커스텀 컴포넌트 (MRCharacterMovementComponent 등)
    ├── Store/        이 프로젝트 전용 Store (CharacterStore, HUDStore)
    ├── Action/       ActionDispatcher, Action 타입 정의
    ├── Subsystem/    GameResource, CMS 서브시스템
    └── Widget/       UI 위젯 (HUD, CharacterStatus 등)
```

- 새 폴더 추가 시 `MR.Build.cs`의 `PrivateIncludePaths`에 반드시 추가

## 엔진 폴더 위치
E:/UE_5.5

## 클래스 계층

### 캐릭터
```
ACharacter
└── AMRBaseCharacter      GAS 초기화, DefaultAbilities/Effects 적용, IAbilitySystemInterface 구현
    ├── AMRPlayerCharacter 입력 처리, 카메라, 무기 타입(EMRWeaponType), 애니메이션 로딩
    └── AMRMonster         DataTable 연동, MonsterType/Level 기반 속성 스케일링
```

### GAS
- `UMRGameplayAbility` — 어빌리티 베이스. `GetBaseCharacter()` / `GetPlayerCharacter()` 제공
- `UMRAttributeSetBase` — Health, MaxHealth, Stamina, MaxStamina, AttackPower, DefensePower
- `UMRAbility_Walk` — 이동 어빌리티, `Character.State.Moving` 태그 관리
- `UMRAbility_Sprint` — 스프린트 어빌리티, MaxWalkSpeed 변경(900), StaminaDrain GE 관리

### Gameplay Tags (MRGameplayTags.h)
| 태그 | 용도 |
|------|------|
| `Character.State.Moving` | 이동 중 |
| `Character.State.Sprinting` | 스프린트 중 |
| `Character.State.Dead` | 사망 |
| `State.Stamina.RegenBlocked` | 스태미나 회복 차단 |
| `Ability.Walk` / `Ability.Sprint` | 어빌리티 ID |

### MVVM Store
- `UStoreBase` (MRStore) — Subscribe/Unsubscribe 델리게이트 관리, `NotifyStateChanged(FieldName)` 브로드캐스트
- `UMVVMWidgetBase` (MRStore) — `BindStore()`, `OnStoreStateChanged()` 오버라이드로 UI 갱신
- `UCharacterStore` (MR/Store) — FCharacterState (Health/Stamina) 보유, ActionDispatcher 연결
- `UHUDStore` (MR/Store, GameInstanceSubsystem) — Store 생성·조회 (`RegisterStore<T>()`, `GetStore<T>()`)

### 서브시스템
- `UMRGameResource` (GameInstanceSubsystem) — 에셋 비동기 로드/캐시. 데이터 구조 정의 구조체는 이 클래스 선언 위에 추가
- `UCMSSubsystem` (GameInstanceSubsystem) — DataTable 관리, `GetMonsterRow(Type)`

## 서브에이전트 활용

코드 작업 시 아래 서브에이전트를 적극 활용한다.

| 에이전트 | 언제 사용 |
|----------|-----------|
| `planner` | 새 기능 구현 전 요구사항 분석, 아키텍처 설계, 구현 계획 수립 |
| `implementer` | planner의 설계를 바탕으로 실제 코드 작성·수정 |
| `verifier` | 구현 완료 후 요구사항 충족 여부, 버그, 기존 코드와의 일관성 검토 |
| `Explore` | 코드베이스 탐색, 파일/심볼 검색, 구조 파악 |

**기본 흐름:** `Explore(파악)` → `planner(설계)` → `implementer(구현)` → `verifier(검증)`

- 단순 버그 수정이나 소규모 변경은 흐름을 단축해도 됨
- planner와 implementer는 병렬 실행 불가 (순서 의존), Explore는 planner와 병렬 가능
- verifier가 문제를 발견하면 implementer에게 재작업 위임

## 커밋 방식
- 첫 번째 줄: 전체 변경사항 요약 제목
- 두 번째 줄: 공백
- 세 번째 줄부터: `-`로 시작하는 항목별 설명 (5줄 이내)
- AI/Claude 관련 내용 제거
