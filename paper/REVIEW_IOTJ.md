# IEEE Internet of Things Journal — 리뷰어 보고서 (모의 심사)

**원고:** UATimer: Lightweight Context-Aware Power Management for Heterogeneous IoT Systems
**검토 대상:** `paper/main.tex` (v3, 8페이지)
**권고:** **Major Revision** (현재 형태로는 Reject에 가까운 Major Revision)

---

## 1. 논문 요약

이 논문은 관측된 이벤트 간 유휴 간격에 대해 지수가중이동평균(EWMA,
`T_new = λT_old + (1−λ)t_idle`)을 사용해 유휴 타임아웃을 갱신하고, 거친
애플리케이션 컨텍스트(AR/VR, 야간 등)에 따라 에너지–지연 가중치를 조정하는
사용자 공간 전력 관리 계층 UATimer를 제안한다. 스마트폰, 태블릿,
Cortex-M4 센서 노드에서 5개 워크로드를 평가했으며, 기본 OS 대비 20.5–37.5%,
고정 타임아웃 대비 7.9–16.7%의 에너지 절감과 50 ms 미만의 평균 지연을
보고한다.

문제 설정은 실용적이다. 한계를 스스로 명시하려는 태도(Evidence Boundaries,
Limitations)도 좋게 본다. 그러나 **(i) 신규성, (ii) 알고리즘과 주장 사이의
불일치, (iii) 실험 증거의 신뢰성과 재현성**에서 IoT-J 게재 수준에 크게
못 미친다.

---

## 2. 주요 문제 (Major Comments)

### M1. 신규성: 고전 DPM 문헌과의 관계가 빠져 있다
유휴 간격의 지수평균으로 다음 유휴 시간을 예측해 셧다운 여부를 정하는
방법은 동적 전력 관리(DPM) 분야에서 1990년대부터 쓰여 온 기법이다.

- C.-H. Hwang and A. C.-H. Wu, "A predictive system shutdown method for
  energy saving of event-driven computation," ICCAD 1996 / ACM TODAES 2000:
  지수평균 기반 유휴 예측으로, 본 논문의 식 (1)과 사실상 같다.
- M. B. Srivastava et al., "Predictive system shutdown and other
  architectural techniques for energy efficient programmable computation,"
  IEEE TVLSI 1996.
- L. Benini, A. Bogliolo, G. De Micheli, "A survey of design techniques for
  system-level dynamic power management," IEEE TVLSI 2000.
- Douglis/Krishnan/Helmbold의 adaptive disk spin-down, Karlin 등의
  ski-rental(break-even 타임아웃은 2-competitive), Irani 등의 online DPM 연구.
- Linux cpuidle의 `menu`/`teo` 거버너, runtime PM `autosuspend_delay_ms`,
  Android Doze/App Standby. 모두 OS 수준에서 유휴 이력 기반 적응을 이미
  하고 있다.

저자는 위 연구를 인용하고 비교해야 하며, UATimer가 **무엇이 새로운지**를
분명히 해야 한다. 현재 관련 연구는 10편이고(그중 `safaei2021elite`는 본문에서
한 번도 인용되지 않는다), IoT-J 논문으로서는 크게 부족하다.

### M2. 컨텍스트 인식 메커니즘이 타임아웃 결정에 영향을 주지 않는다
- 식 (2)의 비용함수 `J(T) = w_E E(T) + w_D D(T)`는 정의만 있고 어디서도
  최소화되거나 사용되지 않는다. Algorithm 1(Fig. 2)에서 `w_D`, `w_E`는
  갱신되지만 `T`의 갱신식에는 들어가지 않는다. **따라서 의사코드대로라면
  컨텍스트는 타임아웃에 아무 영향도 주지 않는다.** 논문의 핵심 주장인
  "context-aware"가 알고리즘 차원에서 성립하지 않는다.
- `w_D += delta`가 이벤트마다 누적되므로 가중치에 상한이 없어 [0,1]을
  벗어나거나 음수가 될 수 있다. `delta` 값도 밝히지 않았다.
- `E(T)`, `D(T)`는 단위가 다르고(mJ vs ms) 정규화가 없어 `w_E + w_D = 1`의
  의미가 모호하다.
- 컨텍스트(ARVR, NIGHT 등)를 **어떻게 감지하는지**(포그라운드 앱 목록?
  시계? 센서?)에 대한 설명이 없다.

→ 컨텍스트가 `T`(또는 `T_min`/`T_max`, λ, suspend 억제)에 정확히 어떻게
반영되는지 수식과 알고리즘으로 명시하고, 컨텍스트 기능을 뺀 ablation
(EWMA만 쓰는 경우 vs 컨텍스트 포함)을 보여야 한다.

### M3. 갱신 규칙이 에너지 절감을 설명하지 못한다
- `T`가 관측 유휴 간격의 평균으로 수렴한다면(식 (6)), 정상 상태의 주기적
  워크로드(IoT sensing)에서는 타이머가 다음 이벤트 도착 시점 근처에 만료된다.
  그러면 suspend 기회가 거의 사라지거나, 도착 직전에 진입해 곧바로 깨어나
  break-even 시간보다 짧은 비효율적 전이가 생긴다. 타임아웃을 "평균 유휴
  간격"으로 두는 것이 왜 에너지적으로 합리적인지 break-even 시간
  `T_be`와 연결해 분석해야 한다.
- AR/VR(60 Hz, 이벤트 간격 ≈16 ms)에서는 매 이벤트마다 `T`가 갱신되므로
  `T`가 수십 ms 수준으로 떨어져 **가장 공격적으로 suspend하게 된다.**
  이는 AR/VR에서 보수적으로 동작한다는 본문의 주장(§VI-E)과 반대다.
  M2와 맞물려, 보고된 AR/VR 결과가 제시된 알고리즘으로 어떻게 나오는지
  설명되지 않는다.
- Algorithm 1의 `main_loop`는 `idle_ms() >= T`를 계속 검사하는 busy-polling
  구조이고, 설계 목표 1(낮은 오버헤드)과 충돌한다. suspend 이후 루프 동작,
  resume 지연이 `t_idle`에 포함되는지도 정의되지 않았다.
- `λ`, `T_0`, `T_min`, `T_max`의 실제 값이 논문 어디에도 없다.

### M4. "저전력 상태"가 무엇인지, 에너지가 어디서 절감되는지 불분명하다
- 비디오 재생과 AI 추론은 연속 워크로드인데, 유휴 타임아웃만으로 20–25%를
  절감했다는 결과는 설명이 필요하다. suspend 대상이 시스템 전체 suspend인지,
  CPU idle인지, 디스플레이인지, 라디오인지, 주변장치 runtime PM인지 밝혀야 한다.
  비디오 재생 중 시스템 suspend는 현실적으로 불가능하다.
- 기본 정책(Baseline)의 실체가 정의되지 않았다. Android의 어떤 설정(화면
  타임아웃, Doze, wakelock 정책)이고, Cortex-M4 노드에서는 어떤 OS·RTOS의
  기본 동작인가?
- 고정 타임아웃 3 s가 어떻게 평균 이벤트 지연을 30 ms에서 50 ms로 늘리는지
  메커니즘 설명이 필요하다.

### M5. 구현 설명과 IoT 플랫폼이 서로 맞지 않는다
- 구현은 Android PowerHAL 콜백과 Linux `epoll` 기반 user-space daemon으로
  설명된다. 그런데 정량 결과가 있는 IoT 노드는 **Cortex-M4**(MMU 없음, 보통
  RTOS나 bare-metal)이고, 추가 검증 대상은 "ESP32-class RISC-V board"(보통
  FreeRTOS/ESP-IDF)다. 두 플랫폼 모두 "user space", `epoll`, "kernel
  modification 불필요"라는 개념이 그대로 적용되지 않는다. MCU에서 어떤
  API로 어떤 sleep 모드(예: STOP/STANDBY, light/deep sleep)를 썼는지
  구체적으로 기술해야 한다.
- "less than 1 KB of policy state"라고 하면서 "a small number of scalar
  variables"라고 한다. 실제 상태 크기(바이트)를 제시해야 한다.

### M6. 실험 결과의 신뢰성과 통계
- **per-run 데이터가 없다**고 명시되어 있다(§V-C, §VII-D). 저널 논문에서
  원시 데이터를 잃어 분산·신뢰구간을 낼 수 없다면 실험을 다시 해야 한다.
  n = 5에 분산 정보가 없으면 ML 대비 1.7–3.8% 차이는 측정 오차 안에 있을
  가능성이 크고, "slightly outperforms"(§VI-C)라고 주장할 근거가 없다.
- Table III의 값이 모두 지나치게 둥글다(1200, 900, 750, 1800, 1500 …).
  측정값이라면 반올림 규칙을 밝혀야 한다.
- **측정 구간과 단위가 불명확하다.** 스마트폰 웹 브라우징 에너지가 1200 mJ라면
  약 1 W 소비 기준으로 약 1초 분량이다. 각 실험의 지속시간, 반복 구간, 에너지
  적분 방식을 명시해야 한다.
- Table III의 워크로드가 **어느 기기**에서 측정되었는지 표시되어 있지 않다
  (스마트폰/태블릿 구분 없음).
- 평균 지연만 보고했다. 전력 관리에서 핵심인 tail latency(p95/p99),
  suspend/resume 전이 횟수, 상태 residency, 오탐(너무 이른 suspend) 비율이 없다.
- 데몬 자체의 CPU·메모리·wakeup 오버헤드를 측정하지 않았다(설계 목표 1 미검증).
- IoT 노드는 에너지 수치만 있다. 코인셀 기반 노드라면 **배터리 수명 추정**이
  IoT-J 독자에게 가장 중요한 지표다.

### M7. AR/VR 결과가 직관에 반한다
Adaptive의 프레임 드롭(0.5%)과 지터(1.2 ms)가 추가 suspend가 없는
Baseline(2.5%, 3.0 ms)보다 **낮다.** 전력 정책이 기본 정책보다 QoE를 개선하는
메커니즘(예: 열 스로틀링 감소?)을 설명하지 않으면 결과를 받아들이기 어렵다.
또 평균 지연 28 ms는 60 Hz 프레임 주기(16.7 ms)보다 길다. 이런 지연에서 드롭률이
가장 낮게 나오는 이유도 설명해야 한다.

### M8. 비교 대상(baseline)이 약하다
- ML baseline은 과거 유휴 간격 3개로 선형회귀하는 모델이고, 이는 사실상
  EWMA와 같은 계열(선형 필터)이다. 어떤 워크로드의 1000개 샘플로 학습했는지,
  워크로드마다 따로 학습했는지도 밝히지 않았다.
- 초록은 UATimer가 "smaller policy footprint"를 가진다고 하지만, 본문 기준 ML
  상태는 **< 64 B**, UATimer는 **< 1 KB**다. **주장이 본문 수치와 모순된다.**
- Table II는 ML이 "Online adaptation: Yes"이면서 "Training required: Yes"(오프라인
  최소제곱)라고 적고 있다. 온라인 재학습을 하는지 밝혀야 한다.
- 최소한 (a) break-even 기반 2-competitive 타임아웃, (b) Linux cpuidle `menu`/`teo`,
  (c) Android Doze, (d) 적응형 타임아웃(Douglis 등), (e) 경량 RL/TinyML 중 몇 가지와
  비교해야 한다.

### M9. IoT 저널 적합성
정량 결과 5개 중 4개가 모바일 워크로드이고, IoT 노드는 1대, 1개 워크로드뿐이다.
라디오 스택(BLE/Zigbee/LoRa/NB-IoT PSM·eDRX)과의 상호작용, 네트워크 수준 효과
(패킷 지연, 연결 유지, 다중 노드)가 없다. 제목의 "Heterogeneous IoT Systems"를
뒷받침하려면 다수 MCU 플랫폼과 무선 프로토콜 실험이 필요하다. 지금 형태로는
모바일 시스템 학회(예: HotPower, 모바일 워크숍)에 더 가깝다.

### M10. 재현성
본문은 코드와 스크립트를 `https://github.com/leemgs/uatimer`에 공개한다고 하지만,
현재 저장소에는 원고 파일(`paper/`)만 있고 **구현 코드, 평가 스크립트, 원시
데이터가 없다.** 재현성 주장을 검증할 수 없다.

---

## 3. 부수적 문제 (Minor Comments)

1. §VI-A: 기준 대비 평균 절감률을 27.3%라고 적었지만, Table IV 값
   (37.5, 25.0, 20.5, 28.0, 27.5)의 산술평균은 **27.7%**다.
2. `safaei2021elite`는 참고문헌에만 있고 본문에서 인용되지 않는다.
3. "Algorithm~\ref{alg:concept}"가 실제로는 `figure` 환경을 참조하므로 캡션이
   "Fig. 2"로 나온다. `algorithm`/`algorithmic` 패키지를 사용해야 한다.
4. §II-E, §II-F가 아직 정의되지 않은 식 (1), (2)를 참조한다(forward reference).
   구조를 재배치해야 한다.
5. 20.5–37.5% / 7.9–16.7% 수치가 초록, §VI-A, §VI-D, 결론에서 네 번 이상
   반복된다. Table III, Table IV, Fig. 3, Fig. 4도 같은 데이터를 중복해서 보여준다.
   그래프는 에너지–지연 파레토 산점도 하나로 합치는 편이 정보량이 많다.
6. §VI-F "Failure Modes"와 §VI-G "Validity"는 결과가 아니라 논의이므로
   Discussion으로 옮기는 것이 적절하다. §VI-H "Evidence Boundaries"는 반박문
   (rebuttal) 같은 어조라서 Limitations와 통합할 것을 권한다.
7. 식 (6)(수렴)은 EWMA의 자명한 성질이다. 분량을 줄이고, 그 대신 non-stationary
   전환 시 적응 지연(예: 워크로드 전환 후 T의 수렴 이벤트 수)을 실험으로 보여라.
8. Table I의 "Limitation addressed by UATimer" 열에서 UATimer 행의 값("Coarse
   rather than learned context")은 해결한 한계가 아니라 UATimer 자신의 한계다.
9. 플랫폼 기술이 모호하다. 스마트폰 SoC 모델명, Android 버전, 커널 버전,
   Cortex-M4 MCU 모델, 외부 측정기 모델과 샘플링 속도를 밝혀라.
10. 인간 대상 QoE 연구를 삭제한 것은 적절하다. 다만 AR/VR "QoE"라는 소제목은
    프레임 드롭·지터만 다루므로 "AR/VR Rendering Quality" 등으로 바꾸는 것을 권한다.
11. 단일 저자 논문에서 "We"를 쓰는 것은 IEEE 관행상 허용되지만, 일관성을 확인하라.
12. 참고문헌이 10편이다. IoT-J 일반 기준(30편 이상)과 최신 IoT 에너지 관리
    연구(TinyML 기반 DPM, 에너지 하베스팅 스케줄링, BLE/LoRa duty-cycling)를
    보강하라.

---

## 4. 저자에게 요청하는 추가 실험 (우선순위순)

1. 알고리즘 수정: 컨텍스트 가중치를 `T` 결정에 실제로 반영하고, break-even
   시간 기반 분석을 추가한다.
2. 전체 실험 재수행: 기기별 표기, 각 조건 최소 10–30회 반복, 평균 ± 표준편차와
   95% 신뢰구간, 유의성 검정(예: Wilcoxon), 원시 데이터 공개.
3. Ablation: EWMA만 / 컨텍스트만 / 둘 다 사용, λ·`T_min`·`T_max` 민감도 분석.
4. 강한 baseline: break-even 타임아웃, cpuidle `menu`/`teo`, Android Doze,
   경량 TinyML 또는 RL 정책.
5. IoT 확장: 2–3개 이상의 MCU 플랫폼, 무선 프로토콜(BLE/LoRa/NB-IoT) 포함 워크로드,
   배터리 수명 추정, 전이 횟수와 residency.
6. 오버헤드 측정: 데몬의 CPU 사용률, 추가 wakeup 수, 메모리.
7. tail latency(p95/p99)와 워크로드 전환 시 적응 곡선(T의 시계열).

---

## 5. 평가 요약

| 항목 | 평가 |
|---|---|
| 신규성 / 기여도 | 낮음 (고전 predictive shutdown과 차별성 불명확) |
| 기술적 정확성 | 보통 이하 (비용함수 미사용, AR/VR 동작 모순) |
| 실험 충실도 | 낮음 (n=5, 원시 데이터 없음, 파라미터 미기재) |
| IoT-J 적합성 | 보통 이하 (IoT 플랫폼 1대, 네트워크 측면 없음) |
| 가독성 / 구성 | 양호하나 반복이 많음 |
| 재현성 | 낮음 (코드와 데이터 미공개) |

**최종 권고: Major Revision.** 실용적 동기는 인정한다. 다만 M1–M3(신규성과
알고리즘 일관성)과 M6(원시 데이터 부재)는 원고 수정만으로 해결되지 않는다.
새 실험이 필요하므로, 현 상태로 제출하면 편집자 판단에 따라 Reject(resubmit
encouraged)가 나올 가능성이 높다.
