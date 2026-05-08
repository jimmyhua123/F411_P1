# STM32F411 Head-Tracking Audio Demo Checklist

## Power Policy Demo Checks

- [ ] `SIM3,` telemetry is present and includes `imu_hz`, `tel_hz`, and `audio_en`.
- [ ] `ACTIVE` prints `[POLICY] imu=20ms tel=50ms audio=on`.
- [ ] `MUTED` prints `[POLICY] imu=20ms tel=100ms audio=off`.
- [ ] `LOW_POWER` prints `[POLICY] imu=100ms tel=500ms audio=off`.
- [ ] `FAULT` prints `[POLICY] imu=100ms tel=500ms audio=off`.
- [ ] In `LOW_POWER`, moving roll or pitch by about 10 deg prints `[WAKE] motion detected` and returns to `ACTIVE`.
- [ ] In `LOW_POWER`, pressing PA0 returns to `ACTIVE`.

## INA219 Bring-Up Result

- [x] GY-219 / INA219 is readable on I2C address `0x40`.
- [x] 5V audio rail measurement log observed.

```text
[INA219] bus=4972mV shunt=1720uV current=17.20mA power=86mW
[INA219] bus=4972mV shunt=1010uV current=8.50mA power=50mW
[INA219] bus=4972mV shunt=400uV current=4.00mA power=20mW
[INA219] bus=4972mV shunt=-30uV current=0.00mA power=0mW
```

## MicroSD Logging Bring-Up Result

- [x] SPI1 microSD wiring verified.
- [x] FatFS mount succeeds.
- [x] Logger creates numbered CSV files: `LOG000.CSV`, `LOG001.CSV`, ...
- [x] CSV row sequence `seq` increments per row.
- [x] CSV records ACTIVE rows with roll/pitch/audio/power fields.
- [x] `FAULT_SD_LOG` is non-fatal; audio demo continues if SD is unavailable.

Verified first SD capture:

```csv
seq,time_ms,mode,head,roll,pitch,lvol,rvol,bus_v,current_ma,audio_rail,fault
1,1112,ACTIVE,CENTER,0.0,0.0,0.00,0.00,0.000,0.00,ON,0x00
2,1315,ACTIVE,CENTER,0.2,4.8,0.20,0.20,4.960,-0.40,ON,0x00
3,1545,ACTIVE,CENTER,0.3,4.8,0.20,0.20,4.960,-0.40,ON,0x00
```

這份清單用來確認目前韌體 demo 是否完整可展示、可量測、可寫進履歷。

## 1. 開機檢查

- [ ] 接上 STM32F411 board。
- [ ] 開啟 UART terminal。
- [ ] USART2 baudrate 設為 `115200`。
- [ ] 確認看到開機訊息：

```text
[BOOT] STM32 Head-Tracking Audio Prototype
[PWR] sensor on
[PWR] audio on
```

## 11. CubeMX CMake 復原檢查

- [ ] 如果 build/link 出現下面錯誤，先檢查 CMake source list：

```text
undefined reference to `App_Init'
undefined reference to `App_MainLoop'
```

- [ ] 原因通常是 CubeMX 重新產生 `cmake/stm32cubemx/CMakeLists.txt`，把自訂 `.c` 檔從 `MX_Application_Src` 移掉。
- [ ] 打開 `cmake/stm32cubemx/CMakeLists.txt`。
- [ ] 在 `set(MX_Application_Src ...)` 裡確認以下自訂檔案都有被列入：

```text
Core/Src/app_main.c
Core/Src/attitude.c
Core/Src/attitude_filter.c
Core/Src/audio_control.c
Core/Src/audio_tone.c
Core/Src/audio_volume_smoother.c
Core/Src/bsp_uart_log.c
Core/Src/button_event.c
Core/Src/head_detect.c
Core/Src/head_state_machine.c
Core/Src/imu_mpu6500.c
Core/Src/power_policy.c
Core/Src/sim_telemetry.c
Core/Src/system_mode.c
```

- [ ] 補回後，重新 CMake configure / build。
- [ ] 如果還是看到 `App_Init` / `App_MainLoop` undefined，第一個先確認 `Core/Src/app_main.c` 是否真的在 `MX_Application_Src`。

- [ ] 確認 I2S DMA 啟動成功：

```text
[Audio] I2S2/I2S3 DMA started
```

- [ ] 確認 telemetry 格式提示出現：

```text
[SIMFMT] SIM,ms,roll_cd,pitch_cd,head,target_l_pct,target_r_pct,smooth_l_pct,smooth_r_pct
[SIMFMT] SIM2,ms,mode,fault_hex,roll_cd,pitch_cd,head,target_l_pct,target_r_pct,smooth_l_pct,smooth_r_pct
[SIMFMT] SIM3,ms,mode,fault_hex,imu_hz,tel_hz,audio_en,roll_cd,pitch_cd,head,target_l_pct,target_r_pct,smooth_l_pct,smooth_r_pct
```

## 2. IMU 與姿態檢查

- [ ] 靜置板子時，roll/pitch 數值穩定。
- [ ] 左右傾斜板子，`roll_cd` 會跟著變化。
- [ ] 前後傾斜板子，`pitch_cd` 會跟著變化。
- [ ] 左傾超過門檻時，head state 進入 `LEFT`。
- [ ] 右傾超過門檻時，head state 進入 `RIGHT`。
- [ ] 低頭超過門檻時，head state 進入 `DOWN`。
- [ ] 抬頭超過門檻時，head state 進入 `UP`。
- [ ] 回到中間姿態時，head state 回到 `CENTER`。

## 3. 音訊輸出檢查

- [ ] 左右兩顆 MAX98357A 都有接好電源與喇叭。
- [ ] 開機後可聽到 440 Hz tone。
- [ ] 左傾時，右聲道音量降低。
- [ ] 右傾時，左聲道音量降低。
- [ ] `HEAD_DOWN` 時，左右聲道都靜音。
- [ ] 回到非 `HEAD_DOWN` 姿態後，聲音恢復。
- [ ] 音量變化平順，沒有明顯 click/pop。

## 4. PA0 按鍵互動檢查

- [ ] 開機預設 master volume 為 20%，避免測試時太大聲。
- [ ] 短按 PA0 一次，master volume 從 20% 切到 100%。
- [ ] 再短按 PA0 一次，master volume 從 100% 切回 20%。
- [ ] 注意：目前 100% 是 demo 上限，實際 gain 為 0.50；20% 實際 gain 仍為 0.20。
- [ ] PC13 紅燈不再顯示音量；音量狀態請看 UART log 或 telemetry。

- [ ] 長按 PA0 1 到 3 秒，系統從 `ACTIVE` 進入 `MUTED`。
- [ ] 長按切換模式不需要先切到 100%；在 20% 或 100% 都可以操作。
- [ ] 在 `MUTED` 模式下音訊為軟體靜音。
- [ ] 再長按 PA0 1 到 3 秒，系統回到 `ACTIVE`。
- [ ] 長按 PA0 3 秒以上，系統進入 `DIAGNOSTIC`。

## 5. 系統模式檢查

- [ ] `ACTIVE` 模式：

```text
IMU 50 Hz
audio on
telemetry 20 Hz
PC14 green on
PC15 yellow off
[POLICY] imu=20ms tel=50ms audio=on
```

- [ ] `MUTED` 模式：

```text
IMU 50 Hz
audio software muted
telemetry 10 Hz
PC14 green off
PC15 yellow on
[POLICY] imu=20ms tel=100ms audio=off
```

- [ ] `MUTED` 停留 5 秒後，自動進入 `LOW_POWER`。
- [ ] `LOW_POWER` 模式：

```text
IMU 10 Hz
audio software muted
telemetry 2 Hz
PC14 green off
PC15 yellow blinking
[POLICY] imu=100ms tel=500ms audio=off
```

- [ ] 在 `LOW_POWER` 長按 PA0 1 到 3 秒，回到 `ACTIVE`。
- [ ] `DIAGNOSTIC` 結束後，系統自動回到 `ACTIVE`。
- [ ] `DIAGNOSTIC` 期間 PC13 紅燈、PC14 綠燈、PC15 黃燈同時亮。

## 5.1 LED 狀態表

- [ ] PC13：紅色 status LED，active-high，用來顯示 diagnostic / fault。

```text
PC13 on  -> DIAGNOSTIC or fault latched
PC13 off -> no diagnostic/fault
```

- [ ] PC14：綠色 mode LED，外接 active-high。
- [ ] PC15：黃色 mode LED，外接 active-high。

```text
BOOT       -> PC13 on, PC14 on, PC15 on briefly, then off
ACTIVE     -> PC14 on,  PC15 off
MUTED      -> PC14 off, PC15 on
LOW_POWER  -> PC14 off, PC15 blink
DIAGNOSTIC -> PC13 on, PC14 on, PC15 on
FAULT      -> PC13 on, PC14 off, PC15 off
```

- [ ] 開機時三顆 LED 會短暫全亮再全滅。
- [ ] 如果開機 self-test 三顆都沒亮，先檢查 LED 極性、限流電阻、GND/3.3V、PC13/PC14/PC15 腳位對應。

## 6. Diagnostic 模式檢查

- [ ] 長按 PA0 3 秒以上後，UART 印出：

```text
[DIAG] start
```

- [ ] 聽到左聲道 beep 一次。
- [ ] 聽到右聲道 beep 一次。
- [ ] 聽到左右聲道一起 beep 一次。
- [ ] 確認 IMU 診斷結果：

```text
[DIAG] imu=OK whoami=0x70
```

- [ ] 確認左 I2S 狀態：

```text
[DIAG] i2s_left=OK
```

- [ ] 確認右 I2S 狀態：

```text
[DIAG] i2s_right=OK
```

- [ ] 確認按鍵狀態有被印出：

```text
[DIAG] button=OK
```

- [ ] 確認診斷結束：

```text
[DIAG] done
```

## 7. Telemetry 檢查

- [ ] 舊版 `SIM,` 格式仍然存在。
- [ ] 新版 `SIM2,` 格式有 mode 欄位。
- [ ] `SIM2` 範例格式如下：

```text
SIM2,1234,ACTIVE,00000000,-1850,420,CENTER,20,20,20,20
SIM3,1234,ACTIVE,00000000,50,20,1,-1850,420,CENTER,20,20,20,20
```

- [ ] `ACTIVE` / `MUTED` / `DIAGNOSTIC` 模式下，telemetry 約 20 Hz。
- [ ] `LOW_POWER` 模式下，telemetry 約 2 Hz。
- [ ] 外部 3D simulator 可以讀取 `SIM2,`。
- [ ] 舊版 parser 若只讀 `SIM,`，仍可正常運作。

## 8. Demo 展示流程

- [ ] 開機，展示 boot log。
- [ ] 展示 `SIM` / `SIM2` telemetry。
- [ ] 左右傾斜，展示 stereo panning。
- [ ] 低頭，展示 `HEAD_DOWN` mute。
- [ ] 短按 PA0，展示 master volume toggle。
- [ ] 長按 PA0 1 到 3 秒，展示 `ACTIVE` / `MUTED`。
- [ ] 等待 5 秒，展示自動進入 `LOW_POWER`。
- [ ] 再長按 PA0 1 到 3 秒，展示回到 `ACTIVE`。
- [ ] 長按 PA0 3 秒以上，展示 diagnostic sequence。
- [ ] 打開外部 simulator，展示 mode-aware visualization。

## 9. 常見問題排查

- [ ] 沒聲音時，先確認 MAX98357A `VIN`、`GND`、`BCLK`、`LRC`、`DIN`。
- [ ] 沒聲音時，確認喇叭接在 `SPK+` / `SPK-`，不要接 GND。
- [ ] 只有單邊有聲音時，分別檢查 I2S2 與 I2S3 wiring。
- [ ] UART 沒輸出時，確認 USART2 baudrate 是 `115200`。
- [ ] COM port 消失時，先拔掉 MAX98357A 測試是否為供電壓降。
- [ ] IMU 失敗時，確認 MPU6500 `AD0` 接 GND，I2C 位址為 `0x68`。
- [ ] 若 CubeMX 重新產生檔案，重新確認 custom source 有被放進 CMake。

## 10. 履歷描述

- [ ] 英文版：

```text
Built an STM32F411-based head-tracking audio prototype with MPU6500 IMU sensing,
dual MAX98357A I2S DMA audio output, real-time head-pose classification,
roll-based stereo panning, button-driven user interaction, diagnostic mode,
low-power behavior, and UART telemetry for external visualization.
```

- [ ] 中文版：

```text
開發 STM32F411 智慧型頭部姿態音訊原型系統，整合 MPU6500 IMU、雙 MAX98357A
I2S DMA 音訊輸出、即時姿態分類、roll-based 左右聲道音量控制、按鍵互動、
診斷模式、低功耗行為與 UART telemetry 外部視覺化。
```
