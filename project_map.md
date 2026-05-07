# STM32F411 Head-Tracking Audio Project Map

```text
                         +--------------------------------+
                         | STM32F411 Head-Tracking Audio  |
                         | wearable / smart headset demo  |
                         +---------------+----------------+
                                         |
                                         v
+--------------------------------------------------------------------------------+
| 1. Core Sensor + Audio Prototype                                                |
+--------------------------------------------------------------------------------+
| [DONE] MPU6500 I2C bring-up                                                     |
| [DONE] accel-only roll / pitch                                                  |
| [DONE] low-pass attitude filter                                                 |
| [DONE] head state machine: CENTER / LEFT / RIGHT / DOWN / UP                   |
| [DONE] dual MAX98357A I2S DMA output                                            |
| [DONE] 440 Hz sine tone                                                         |
| [DONE] roll-based stereo panning                                                |
| [DONE] HEAD_DOWN software mute                                                  |
+--------------------------------------------------------------------------------+
                                         |
                                         v
+--------------------------------------------------------------------------------+
| 2. User Interaction + Product State                                             |
+--------------------------------------------------------------------------------+
| [DONE] PA0 short press: 20% / 100% master volume toggle                         |
| [DONE] PA0 long press 1-3 sec: ACTIVE <-> MUTED                                 |
| [DONE] PA0 long press 3+ sec: DIAGNOSTIC                                        |
| [DONE] ACTIVE / MUTED / LOW_POWER / DIAGNOSTIC / FAULT modes                    |
| [DONE] status LED layer                                                         |
|        ACTIVE      -> PC14 green on                                             |
|        MUTED       -> PC15 yellow on                                            |
|        LOW_POWER   -> PC15 yellow blink                                         |
|        DIAGNOSTIC  -> PC13 + PC14 + PC15 all on                                 |
|        FAULT       -> PC13 red on                                               |
+--------------------------------------------------------------------------------+
                                         |
                                         v
+--------------------------------------------------------------------------------+
| 3. Firmware Reliability Layer                                                   |
+--------------------------------------------------------------------------------+
| [DONE] fault_manager.c / fault_manager.h                                        |
| [DONE] fault flags: IMU_LOST / I2S_LEFT / I2S_RIGHT / AUDIO_CLIP / BUTTON_STUCK |
| [DONE] FAULT forces audio target to 0                                           |
| [DONE] I2S DMA keeps running with silence                                       |
| [DONE] UART fault log                                                           |
| [DONE] long press clear request for fault recovery                              |
+--------------------------------------------------------------------------------+
                                         |
                                         v
+--------------------------------------------------------------------------------+
| 4. Power Policy Demo                                                            |
+--------------------------------------------------------------------------------+
| [DONE] power_policy.c / power_policy.h                                          |
| [DONE] mode-based IMU / telemetry / audio policy                                |
|                                                                                |
|        ACTIVE      imu 20ms   telemetry 50ms    audio on                        |
|        MUTED       imu 20ms   telemetry 100ms   audio off                       |
|        LOW_POWER   imu 100ms  telemetry 500ms   audio off                       |
|        DIAGNOSTIC  imu 20ms   telemetry 50ms    audio on                        |
|        FAULT       imu 100ms  telemetry 500ms   audio off                       |
|                                                                                |
| [DONE] LOW_POWER motion wake: roll/pitch delta about 10 deg                     |
| [DONE] LOW_POWER button wake                                                    |
+--------------------------------------------------------------------------------+
                                         |
                                         v
+--------------------------------------------------------------------------------+
| 5. Telemetry + External Visualization                                           |
+--------------------------------------------------------------------------------+
| [DONE] SIM legacy telemetry                                                     |
| [DONE] SIM2 mode + fault telemetry                                              |
| [DONE] SIM3 power-policy telemetry                                              |
| [TEMP] SIM telemetry disabled while INA219 bring-up is being tested             |
|                                                                                |
| Next:                                                                          |
| [TODO] update PC/web 3D simulator to prefer SIM3                                |
| [TODO] show mode, fault flags, audio level, and power policy on dashboard       |
+--------------------------------------------------------------------------------+
                                         |
                                         v
+--------------------------------------------------------------------------------+
| 6. Power Measurement Hardware                                                   |
+--------------------------------------------------------------------------------+
| [DONE] GY-219 / INA219 wired on shared I2C1 bus                                 |
| [DONE] INA219 address 0x40 detected                                             |
| [DONE] 5V audio rail measurement through VIN+ -> VIN-                           |
| [DONE] first measured samples recorded                                          |
| [DONE] mode-tagged PWR telemetry line                                           |
| [DONE] 100 ms INA219 sample period with 16-sample moving average                |
| [DONE] per-mode current min/max reset                                           |
| [DONE] AO3401 added after INA219 for switched MAX98357A 5V_AUDIO rail           |
| [DONE] LOW_POWER turns audio rail off; other modes keep audio rail on           |
| [DONE] HW-221 translates PB8 to 0 V / 5 V AO3401 gate drive                     |
| [DONE] LOW_POWER v2 audio rail cutoff accepted                                  |
|        ACTIVE 20% ~= 18.0 mA                                                    |
|        HEAD_DOWN  ~= 5.6 mA                                                     |
|        LOW_POWER  ~= -0.3 mA, treated as near-zero INA219 offset                |
| [DONE] power_report.c / power_report.h per-state avg/min/max summary           |
| [DONE] DEMO_SUMMARY health and audio rail cutoff UART output                    |
|                                                                                |
|        bus ~= 4.972 V                                                           |
|        current ~= 17.20 mA -> 0.00 mA                                           |
|        power ~= 86 mW -> 0 mW                                                   |
|                                                                                |
| Next:                                                                          |
| [TODO] check LOW_POWER wake audio recovery and pop noise                        |
| [TODO] decide whether power report fields should later be shown in simulator    |
+--------------------------------------------------------------------------------+
                                         |
                                         v
+--------------------------------------------------------------------------------+
| 7. Near-Term Next Milestones                                                    |
+--------------------------------------------------------------------------------+
| [NEXT] Re-enable SIM3 after INA219 verification                                 |
| [NEXT] Record LOW_POWER wake recovery and pop-noise result                      |
| [NEXT] Record live power_report summary during demo                             |
| [NEXT] Update external simulator for SIM3 + power display                       |
+--------------------------------------------------------------------------------+
                                         |
                                         v
+--------------------------------------------------------------------------------+
| 8. Later Product Polish                                                         |
+--------------------------------------------------------------------------------+
| [LATER] pitch-based audio behavior                                              |
| [LATER] gyro/yaw fusion                                                         |
| [LATER] real low-power MCU sleep mode                                           |
| [LATER] audio pop-noise testing                                                 |
| [LATER] enclosure/wiring diagram                                                |
| [LATER] demo video script                                                       |
+--------------------------------------------------------------------------------+
```

## Current Story

```text
This project is no longer only an audio prototype.

It now demonstrates:

sensor input
  -> head pose interpretation
  -> stereo audio behavior
  -> product-like modes
  -> fault handling
  -> power policy
  -> measured power telemetry
```

## Best Next Step

```text
Keep SIM telemetry disabled while checking INA219 stability.
Then collect mode-by-mode current logs:

ACTIVE
MUTED
LOW_POWER
FAULT

After that, re-enable SIM3 and show power data in the external simulator.
```
