# Linked Anim Layer 가이드

## 개요

Linked Anim Layer는 UE5에서 AnimBP의 일부를 **런타임에 교체**할 수 있게 해주는 기능이다.
하나의 메인 AnimBP가 뼈대 역할을 하고, 무기별/상태별 AnimBP가 특정 레이어 슬롯을 채운다.

몬헌 같은 게임에서 무기별로 콤보 모션, 대기 자세, 이동 자세가 완전히 다를 때 이 패턴을 쓴다.

---

## 핵심 개념: Animation Layer Interface (ALI)

ALI는 "어떤 레이어 슬롯이 존재하는가"를 정의하는 인터페이스다.
함수처럼 생겼지만, 실제 로직은 없고 슬롯 이름과 입출력만 선언한다.

```
ALI_WeaponLayers (Animation Layer Interface 에셋)
├── UpperBodyAttackLayer  ← 공격 시 상체 포즈
├── FullBodyIdleLayer     ← 무기별 대기 자세
└── FullBodyLocomotionLayer ← 무기별 이동 자세 (필요 시)
```

**에디터에서 생성 방법:**
- Content Browser → 우클릭 → Animation → Animation Layer Interface → `ALI_WeaponLayers`

---

## 메인 AnimBP (ABP_Player)

메인 AnimBP는 전체 구조를 담는다. ALI에서 선언한 레이어를 "호출"하는 노드를 배치한다.

```
ABP_Player Anim Graph:
┌─────────────────────────────────┐
│  Output Pose                    │
│      ↑                          │
│  [UpperBodyAttackLayer] ← 교체됨 │
│      ↑                          │
│  Ground State Machine           │
│  (Idle / Locomotion)   ← 공통   │
└─────────────────────────────────┘
```

**ABP_Player에 ALI 연결 방법:**
1. ABP_Player 열기 → Class Settings → Implemented Interfaces → ALI_WeaponLayers 추가
2. AnimGraph에서 우클릭 → "UpperBodyAttackLayer" 노드 배치

---

## 무기별 AnimBP (ABP_Sword, ABP_Bow 등)

각 무기 AnimBP는 ALI를 "구현"한다. 레이어 함수 내부에 무기 전용 로직을 넣는다.

```
ABP_OneHandedSword:
└── UpperBodyAttackLayer 구현:
    └── 한손검 콤보 State Machine
        ├── Idle
        ├── Attack1 → Attack2 → Attack3 → Attack4
        └── (각 State에서 태그 체크 또는 몽타주 슬롯 연결)

ABP_Bow:
└── UpperBodyAttackLayer 구현:
    └── 활 State Machine
        ├── Idle
        ├── AimMode (AimOffset 포즈)
        └── Shoot
```

**무기 AnimBP 생성 방법:**
1. Content Browser → 우클릭 → Animation → Animation Blueprint
2. Parent Class: `AnimInstance`, Skeleton: 플레이어 스켈레톤
3. Class Settings → Implemented Interfaces → ALI_WeaponLayers 추가
4. ALI 레이어 함수들이 자동으로 나타남 → 내부 로직 작성

---

## 런타임 레이어 교체: LinkAnimClassLayers

무기를 바꿀 때 C++ 한 줄로 레이어를 교체한다.

```cpp
// AMRPlayerCharacter.cpp
void AMRPlayerCharacter::LinkWeaponAnimLayer(EMRWeaponType WeaponType)
{
    if (TSubclassOf<UAnimInstance>* LayerClass = WeaponAnimLayerClasses.Find(WeaponType))
    {
        if (*LayerClass)
        {
            GetMesh()->LinkAnimClassLayers(*LayerClass);
        }
    }
}
```

`WeaponAnimLayerClasses`는 BP_PlayerCharacter에서 설정:
```
OneHandedSword → ABP_OneHandedSword
TwoHandedSword → ABP_TwoHandedSword
Bow            → ABP_Bow
```

교체 시 메인 ABP_Player의 나머지 부분(Locomotion 등)은 그대로 유지되고,
ALI 레이어 슬롯만 새 무기 ABP의 구현으로 대체된다.

---

## 이 프로젝트에서의 구조

```
ABP_Player (메인, 공통 로직)
├── Ground State Machine
│   ├── Idle  ← IdleAnimation 변수 참조 (무기별 비동기 로드)
│   └── Locomotion ← LocomotionBlendSpace 변수 참조
├── Jump State Machine
│   ├── JumpStart / JumpLoop / JumpEnd
└── [UpperBodyAttackLayer] ← 런타임에 무기별 ABP로 채워짐
        ↑
        ABP_OneHandedSword 또는 ABP_Bow 등

무기 교체 시:
1. LoadAndApplyWeaponAnims() → 비동기로 Idle/BlendSpace 교체
2. LinkWeaponAnimLayer()     → ABP 레이어 교체
3. SwapAttackAbility()       → 공격 어빌리티 교체
```

---

## Montage와의 관계

공격 애니메이션은 Linked Anim Layer와 별개로 **Montage**로 처리한다.
Montage는 특정 슬롯(`DefaultSlot`)에서 재생되며, AnimBP 포즈 위에 오버레이된다.

```
AnimBP 레이어 구조 (위로 갈수록 우선순위 높음):
┌─────────────────────────┐
│ Montage (DefaultSlot)   │ ← 공격 중 이 레이어가 활성화됨
├─────────────────────────┤
│ UpperBodyAttackLayer    │ ← 무기별 Idle/Transition 포즈
├─────────────────────────┤
│ Ground / Jump State M.  │ ← 이동, 점프 기반 포즈
└─────────────────────────┘
```

따라서:
- **Idle/이동 포즈 분리**: Linked Anim Layer 담당
- **공격 모션 재생**: Montage 담당 (UMRAbility_Attack에서 UAbilityTask_PlayMontageAndWait 사용)

무기별 콤보 몽타주는 BP_Attack_OneHandedSword / BP_Attack_Bow 등의 BP 서브클래스에서
`ComboMontages` 배열에 설정한다.

---

## 콤보 윈도우와 AnimNotifyState

공격 Montage에 `MRAnimNotifyState_ComboWindow`를 배치하여 콤보 가능 구간을 표시한다.

```
Montage_Sword_Attack1:
[0.0 ──────[0.3 ComboWindow 0.7]───────── 1.0]
           ↑                   ↑
     NotifyState Begin    NotifyState End
     OpenComboWindow()    CloseComboWindow()
```

**에디터에서 배치 방법:**
1. 몽타주 에셋 열기
2. Notifies 트랙에서 우클릭 → Add AnimNotifyState → MRAnimNotifyState_ComboWindow
3. 구간 드래그로 콤보 윈도우 크기 조정

**흐름 요약:**
```
공격 입력
    → TryActivateAbility (처음) 또는 BufferComboInput (재입력)
    → ActivateAbility: PlayComboMontage() 호출
    → Montage 재생 중 ComboWindow Notify Begin: OpenComboWindow()
    → 이 구간에 공격 입력 → bComboInputBuffered = true
    → ComboWindow Notify End: CloseComboWindow()
        → 버퍼된 입력 있으면 AdvanceCombo() → 다음 몽타주 재생
        → 없으면 Montage 끝까지 재생 후 EndAbility
```

---

## BP 작업 체크리스트

C++ 빌드 후 에디터에서 해야 할 작업:

### 입력 (Enhanced Input)
- [ ] IA_Attack (Input Action) 생성 — Value Type: Digital (bool)
- [ ] IMC_Player (Input Mapping Context) 에서 IA_Attack → 마우스 좌클릭 바인딩

### GE 서브클래스
- [ ] `Content/GAS/BP_Effect_AttackStaminaCost` 생성 (Parent: MREffect_AttackStaminaCost)
- [ ] `Content/GAS/BP_Effect_AttackDamage` 생성 (Parent: MREffect_AttackDamage)

### 어빌리티 서브클래스
- [ ] `Content/GAS/Abilities/BP_Attack_OneHandedSword` 생성 (Parent: MRAbility_Attack)
  - ComboMontages 배열에 몽타주 4개 설정
  - StaminaCostEffectClass = BP_Effect_AttackStaminaCost
  - DamageEffectClass = BP_Effect_AttackDamage

### 몽타주 설정
- [ ] 각 공격 몽타주에 `MRAnimNotifyState_ComboWindow` 배치
- [ ] 몽타주 Slot = `DefaultSlot` (DefaultGroup) 확인

### Animation Layer Interface
- [ ] `Content/Animation/ALI_WeaponLayers` 생성
  - 레이어 함수 추가: `UpperBodyAttackLayer`
- [ ] `ABP_Player` → Class Settings → ALI_WeaponLayers 인터페이스 추가
  - AnimGraph에 `UpperBodyAttackLayer` 링크 노드 배치
- [ ] `Content/Animation/WeaponLayers/ABP_OneHandedSword` 생성
  - ALI_WeaponLayers 구현
  - UpperBodyAttackLayer 내부에 콤보 State Machine 구성

### BP_PlayerCharacter 설정
- [ ] AttackAction = IA_Attack
- [ ] AttackAbilityClasses: OneHandedSword → BP_Attack_OneHandedSword
- [ ] WeaponAnimLayerClasses: OneHandedSword → ABP_OneHandedSword

---

## 자주 묻는 것

**Q: Linked Anim Layer와 그냥 AnimBP 변수 교체의 차이는?**

변수 교체(현재 `IdleAnimation` 방식): AnimBP 하나가 모든 무기를 처리. 무기가 늘수록 State Machine이 복잡해짐.
Linked Anim Layer: 무기별로 독립된 AnimBP. 새 무기 추가 시 새 AnimBP만 만들면 됨. 유지보수에 유리.

**Q: UnlinkAnimClassLayers는 언제 쓰나?**

특정 레이어를 기본 상태(ALI 기본 구현 없음)로 되돌릴 때. 무기 해제 시 사용 가능.
```cpp
GetMesh()->UnlinkAnimClassLayers(ABP_OneHandedSword::StaticClass());
```

**Q: 두 무기의 Locomotion이 똑같으면 굳이 무기별 ABP를 만들어야 하나?**

동일한 ABP 하나를 여러 무기에 매핑해도 된다. WeaponAnimLayerClasses에서 같은 클래스를 여러 키에 지정.

**Q: UpperBodyAttackLayer 레이어가 공격 중이 아닐 때는 어떻게 되나?**

레이어 내부 State Machine에 Idle State가 있어야 한다. 공격 중이 아닐 때는 Idle(빈 포즈 또는 대기 포즈)를 출력하면 메인 AnimBP의 Ground State Machine 포즈가 보인다.
