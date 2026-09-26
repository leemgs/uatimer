# 실행 가이드 (복사/붙여넣기)

이 저장소로 **분석을 돌려 논문 표·그림·통계를 뽑고, 논문에 자동 반영**하는
가장 빠른 방법입니다. 세 갈래(A: Kaggle, B: 로컬, C: 논문 반영) 중 필요한 것만
그대로 복사해 쓰세요.

- 저장소: `https://github.com/leemgs/uatimer`
- 원시 데이터 스키마: `data/README.md` / 예시: `data/raw_runs_expected.csv`
- 분석 스크립트: `tools/analyze.py` (pandas·numpy·scipy·matplotlib만 필요, Kaggle 기본 제공)

---

## A. Kaggle에서 빠르게 (권장)

### A-1. 저장소를 클론해서 바로 실행 (인터넷 ON 필요)

Kaggle 노트북 우측 **Settings → Internet: On**으로 켠 뒤, 셀 하나에 붙여넣기:

```python
!git clone -q https://github.com/leemgs/uatimer.git
%cd uatimer
!python tools/analyze.py data/raw_runs.csv      # 저장소의 20회 데이터로 검증
```

출력: 워크로드별 절감률 ± 95% CI, Wilcoxon p값(Bonferroni 표시), 그리고
`results/`에 그림 2개 + LaTeX 표 2개(`table_results.tex`, `table_reductions.tex`).

### A-2. 내가 측정한 새 데이터로 실행

1. Kaggle 좌측 **Add Data → Upload → New Dataset**로 `raw_runs.csv` 업로드.
2. 셀 하나:

```python
!git clone -q https://github.com/leemgs/uatimer.git
%cd uatimer
# /kaggle/input 아래 업로드된 raw_runs.csv를 자동 탐색:
!python tools/analyze.py
# 또는 경로를 직접 지정:
# !python tools/analyze.py /kaggle/input/<데이터셋이름>/raw_runs.csv
```

결과 파일은 `/kaggle/working/`에 저장되어 노트북 **Output** 탭에서 내려받을 수
있습니다.

### A-3. 인터넷을 못 켜는 경우 (완전 오프라인)

저장소의 `notebooks/uatimer_kaggle.ipynb`를 **File → Import Notebook**으로
올리고 **Run All**. 정책 코드·분석이 노트북 안에 모두 들어 있어 인터넷 없이
동작합니다. (데이터셋 미첨부 시 난수 데모로 도는데, 그 숫자는 실측이 아니라고
표시됩니다. 실제 분석은 `raw_runs.csv` 데이터셋을 첨부하세요.)

---

## B. 로컬(내 PC/서버)에서

```bash
git clone https://github.com/leemgs/uatimer.git
cd uatimer

# 1) 참조 구현 빌드 + 단위 테스트 (C 컴파일러 필요)
make test

# 2) 제어 동작 재생(에너지 아님) 데모
make demo

# 3) 측정 데이터 분석 → results/ 에 표·그림 생성
python3 -m pip install -q pandas numpy scipy matplotlib   # 최초 1회
python3 tools/analyze.py data/raw_runs.csv
```

새 측정 데이터를 쓰려면 `data/raw_runs.csv`를 교체한 뒤 3)만 다시 실행하세요.

---

## C. 결과를 논문에 반영하기

`analyze.py`가 만든 표를 `paper/main.tex`에 자동 주입합니다.

```bash
python3 tools/analyze.py data/raw_runs.csv     # results/*.tex 갱신
python3 tools/update_paper_tables.py           # main.tex의 Table V/VI 교체 + 캡션 n 갱신
cd paper && pdflatex -interaction=nonstopmode main.tex && pdflatex -interaction=nonstopmode main.tex
```

> 반복 횟수(n)를 20에서 바꾼 경우, 표는 자동 갱신되지만 **본문 문장**의
> "twenty runs", "$n=20$", Bonferroni "0.05/15"는 수동 확인이 필요합니다.
> (스크립트가 해당 위치를 출력해 줍니다.)

---

## D. 마지막 7개 하드웨어 빈칸 채우기

논문에 남은 빨간 `\authorcheck{}` 7곳은 실제 장비 사양이라 저자만 채울 수
있습니다.

1. `paper/PLATFORM_SPECS.md`를 열어 `TODO`를 실제 값으로 교체 (SoC 모델,
   Android/커널 버전, MCU 모델·sleep 모드, 측정기 모델·샘플링, 측정 구간 등).
2. 자동 주입:

```bash
python3 tools/fill_specs.py
grep -c "authorcheck{" paper/main.tex     # 1 이면 정상 (남은 1개는 아래 참고)
cd paper && pdflatex -interaction=nonstopmode main.tex
```

> 마지막 남는 `\authorcheck` 1개는 "recorded event traces를 저장소에 추가"라는
> **저장소 작업 항목**입니다. 측정에 쓴 이벤트 트레이스 파일을 `data/`에 커밋한
> 뒤, `paper/main.tex`의 해당 한 줄(재현성 문단)에서 그 문장을 지우면 0이 됩니다.

---

## 요약: 제출까지 남은 저자 체크리스트

- [ ] `data/raw_runs.csv` = 실제 측정값인지 확인 (조건당 ≥ 20회 권장, 현재 충족)
- [ ] `paper/PLATFORM_SPECS.md` 채우고 `tools/fill_specs.py` 실행
- [ ] (선택) 측정 이벤트 트레이스를 `data/`에 추가하고 재현성 문장 정리
- [ ] `Algorithm 1`이 실제 측정 코드와 동작이 같은지 최종 확인
- [ ] `cd paper && pdflatex main.tex` 로 8페이지 재확인 후 제출
