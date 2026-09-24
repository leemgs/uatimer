# v4 수정 내역 — `REVIEW_IOTJ.md` 대응표

원칙: **실험 데이터(Table V 수치)는 한 글자도 바꾸거나 새로 만들지 않았다.**
원고만으로 해결할 수 없는 정보(파라미터 값, 플랫폼 세부 사항 등)는 본문에
빨간색 `\authorcheck{...}` 표시로 남겼다. 모두 20곳이다.

## 리뷰 항목별 조치

| 리뷰 | 조치 | 상태 |
|---|---|---|
| M1 신규성 / 고전 DPM | §I, §II-A/B 재작성. Hwang & Wu, Srivastava, Benini 서베이, Karlin(2-competitive), Irani, Douglis, Helmbold, Simunic, Linux cpuidle 등 9편 추가. "새로운 예측기가 아니다"를 명시하고, 기여를 (bounded + context + portable deployment)로 재정의. Table I에 predictive shutdown과 OS idle governor 행 추가 | 원고 수준에서 해결 |
| M2 컨텍스트가 T에 영향 없음 | 매 이벤트마다 `delta`를 누적하던 방식을 **모드별 고정 가중치 조회**로 교체. 식 (6)-(7) `T_eff = clip(ρ_m·T̂, T_min^(m), T_max)` 도입. J(T)는 **Proposition 1(Topkis 단조성)** 로 모드 순서를 정당화하는 데 사용. E·D 정규화 명시. 컨텍스트 감지(규칙 테이블) 기술 | 해결. 단, 실제 구현과 일치하는지 저자 확인 필요 |
| M3 갱신 규칙과 에너지 절감 | 식 (1) break-even 시간과 하한 `T_min ≥ T_be` 추가. §III-F에서 주기 워크로드 분석(balanced 모드는 절감이 거의 없고, energy 모드는 `(1−ρ)P` 동안 sleep). AR/VR에서 T̂가 16.7 ms로 붕괴하는 문제를 floor와 inhibit으로 해결. busy-polling을 **event-driven one-shot timer**로 교체(Algorithm 1). resume 지연으로 인한 편향 명시 | 해결 |
| M4 저전력 상태 / Baseline 정의 | §V-B에서 비디오와 AI 워크로드가 시스템 suspend를 뜻하지 않는다고 명시. 관리 대상 상태와 Baseline 정의는 `\authorcheck` | **저자 입력 필요** |
| M5 MCU와 user-space 모순 | §IV-A: MCU에서는 펌웨어 모듈로 동작하며, "no kernel modification"은 Linux/Android에 한정한다고 수정. RISC-V 검증 내용은 `\authorcheck` | 부분 해결 |
| M6 통계와 원시 데이터 | 1.7–3.8% 차이를 "outperform"이라고 쓰던 주장을 철회. 측정 구간, 기기, 측정기는 `\authorcheck`. **재실험은 원고로 대체할 수 없음** | **재실험 필요** |
| M7 AR/VR 역설 | Baseline보다 드롭률이 낮은 이유를 메커니즘으로 설명할 수 없다고 인정하고, 주장을 "악화시키지 않았다"로 축소. 28 ms 지연과 프레임 주기의 관계 명확화 | 해결 |
| M8 약한 baseline, footprint 모순 | 초록의 "smaller policy footprint"를 삭제하고 "without an offline training phase"로 교체. Table IV의 ML 행 수정(Inputs only / Offline fit / 4 coeff < 64 B). 고정 타임아웃이 `T_be`로 튜닝되지 않았다는 점 명시. 더 강한 baseline은 후속 실험 목록에 포함 | 원고 수준 해결, 실험은 필요 |
| M9 IoT 적합성 | 한계로 명시하고 후속 측정 목록(iv)에 포함 | **실험 필요** |
| M10 재현성 | 저장소에 코드가 없다는 점을 `\authorcheck`로 표시 | **저자 조치 필요** |
| m1 평균 27.3% | 27.7%로 수정. Table VI에 평균 행 추가(27.7 / 11.6 / 2.7) | 해결 |
| m2 safaei 미인용 | §II-B에서 인용 | 해결 |
| m3 Algorithm이 figure로 표시됨 | `algorithm` + `algpseudocode`로 교체 | 해결 |
| m4 식 전방 참조 | §II의 식 참조를 제거하고 Parameterization을 §III으로 이동 | 해결 |
| m5 수치 반복, 중복 그림 | 에너지·지연 막대그래프 2개를 **에너지–지연 산점도 1개(Fig. 2)** 로 통합. 결과 절의 반복 문단 삭제 | 해결 |
| m6 Failure/Validity 위치 | Discussion으로 이동. Evidence Boundaries를 Limitations와 통합 | 해결 |
| m7 자명한 수렴식 | 짧게 줄이고 적응 속도(`−1/ln λ` 이벤트) 추가 | 해결 |
| m8 Table I UATimer 행 | 열 제목을 "Main limitation"으로 변경 | 해결 |
| m9 플랫폼 세부 | `\authorcheck` | **저자 입력 필요** |
| m10 QoE 소제목 | "AR/VR Rendering Quality"로 변경 | 해결 |
| m12 참고문헌 수 | 10편에서 19편으로 늘림(30편 권장 수준에는 아직 못 미침). 인용 순서대로 재정렬 | 부분 해결 |
| — AI 사용 고지 | 이번 수정에 Claude를 사용했으므로 Acknowledgment에 추가 | 해결 |

## 저자가 반드시 확인할 것

1. **Algorithm 1과 식 (6)-(7)이 Table V를 만든 코드와 같은 동작인지 확인한다.**
   다르면 코드에 맞게 본문을 고친다. 결과에 맞춰 알고리즘을 바꾸면 안 된다.
2. `\authorcheck` 20곳을 채운다(`grep -n authorcheck main.tex`).
3. 추가한 고전 참고문헌 9편의 서지 정보를 DOI로 검증한다.
4. 현재 8페이지다. `\authorcheck` 내용을 채우면 9페이지로 넘어갈 수 있다(추가 요금 USD 175/page).
