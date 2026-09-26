# 플랫폼 사양 기입 양식 (논문의 마지막 7개 빈칸)

논문 `main.tex`에는 저자만 알 수 있는 하드웨어/측정 사실 7곳이 빨간색
`\authorcheck{...}`로 남아 있습니다. 아래 `TODO`를 **실제 값으로 바꾼 뒤**
`python tools/fill_specs.py`를 실행하면 논문에 자동으로 채워지고 빨간 표시가
사라집니다.

> 형식: `KEY: 값` (콜론 뒤 한 칸). KEY는 바꾸지 마세요. 값만 바꾸세요.
> 값에 LaTeX 특수문자(%, &, _, #)가 있으면 백슬래시로 이스케이프하세요 (\%, \&).

```specs
# 1) 스마트폰
PHONE_SOC: TODO            # 예: Qualcomm Snapdragon 8 Gen 2
PHONE_ANDROID: TODO        # 예: 14
PHONE_KERNEL: TODO         # 예: 5.15

# 2) 태블릿
TABLET_SOC: TODO           # 예: MediaTek Kompanio 1200 (Cortex-A73)
TABLET_OS: TODO            # 예: Android 13

# 3) IoT 센서 노드
MCU_MODEL: TODO            # 예: STMicroelectronics STM32L4 (Cortex-M4)
NODE_RADIO: TODO           # 예: BLE 5.0
MCU_ENV: TODO              # 예: FreeRTOS  (RTOS 또는 bare-metal)
SLEEP_MODE: TODO           # 예: Stop 2  (MCU 저전력 모드 이름)
WAKE_SOURCES: TODO         # 예: GPIO and RTC events

# 4) RISC-V 이식 확인
RISCV_CHECK: TODO          # 예: a firmware build and trace replay (에너지 측정은 아님)

# 5) 측정기
METER_MODEL: TODO          # 예: Monsoon HVPM  (IoT 노드에 쓴 외부 계측기)
METER_RATE: TODO           # 예: 5 kHz sampling

# 6) 측정 구간
WINDOW_LENGTH: TODO        # 예: 60 s per run
```

기입 후:

```bash
python tools/fill_specs.py        # main.tex에 주입
grep -c "authorcheck{" paper/main.tex   # 0 이 나오면 완료
cd paper && pdflatex -interaction=nonstopmode main.tex   # 다시 컴파일
```
