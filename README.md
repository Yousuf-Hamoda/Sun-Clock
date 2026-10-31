# Sun Clock 🌅

An alarm clock I designed and built from scratch that wakes you up with **light instead of sound**. At a set time it switches on a bedside lamp, mimicking a sunrise for a gentler, more natural wake-up.

The lamp is never wired to the circuit. Instead, an Arduino reads a real-time clock and, at the alarm time, fires an **infrared transmitter that replays the lamp remote's own signal** to turn the lamp on wirelessly from across the room.

---

## Why I built it

I'm a heavy sleeper and normal alarms never worked for me. Studies suggest light exposure suppresses melatonin, so instead of a loud noise, this brings a lamp on at wake-up time the way the sun would. The whole thing is a small standalone unit that sits on my nightstand and runs on its own.

---

## How it works

At the set time, an **Arduino** reads a **DS1307 real-time clock** over I²C, then drives an **infrared LED** that copies the exact code my lamp's remote sends, switching the lamp on with no physical connection to the lamp itself.

```
[ DS1307 RTC ] --I²C--> [ Arduino ] --D3--> [ transistor ] --> [ IR LED ] ~~~IR~~~> [ lamp ]
                                                                                (existing remote receiver)
```

- **Timekeeping:** a DS1307 real-time clock chip keeps accurate time even when unplugged, using a coin-cell backup battery and a 32.768 kHz crystal.
- **Triggering the lamp:** an IR LED, driven through a **2N2222 / BC547 transistor** for range, transmits the captured "on" code on the standard 38 kHz carrier.
- **Power:** a **USB-C port** soldered directly to the board keeps it always powered.
- **Design:** the circuit is drawn in **KiCad** across two schematic sheets (a transmitter and an IR-capture receiver). The Electrical Rules Check still flags the Nano's unused pins (no no-connect markers yet) and missing power flags, and the PCB layout is not routed yet.

---

## Hardware (Bill of Materials)

| Part | Notes |
|------|-------|
| Arduino Nano (ATmega328P) | Microcontroller; Uno works too |
| DS1307 RTC module | Timekeeping; coin-cell backup. DS3231 is a more accurate drop-in |
| CR2032 coin cell | Keeps the clock running when unpowered |
| IR LED (940 nm) | Transmits the lamp remote's code |
| 2N2222 / BC547 NPN transistor | Drives the IR LED for range |
| 100 Ω resistor | IR LED current limit |
| 1 kΩ resistor | Transistor base resistor |
| 2 × 5.1 kΩ resistors | USB-C CC pulldowns (needed to draw 5 V) |
| USB-C breakout | 5 V power in |

---

## Firmware

The IR version has two sketches, using the **IRremote (v4.x)** and **RTClib** libraries. The original LED version is kept alongside them.

- **`reciever.ino`:** the one-time capture rig. Wire an IR receiver module (e.g. TSOP38238) to **D2**, open the Serial Monitor at 9600 baud, and press your lamp remote's buttons. It prints the protocol, address, and command, plus a ready-to-paste `IrSender` line.
- **`transmitter.ino`:** the sun clock itself. Reads the DS1307 and, at the alarm time, transmits the captured "on" code on **D3**. Includes an optional auto-off time. The `sendNEC` codes committed here are placeholders; replace them with the codes captured from your own remote.

- **`light_alarm/light_alarm.ino`:** the first version, before the IR redesign. At the alarm time it fades an LED in with PWM over 15 minutes, and a sound sensor on **D2** toggles the light on a double clap (two claps within 850 ms). Uses **RTClib** only.

### Setup

1. Install the **IRremote** and **RTClib** libraries via the Arduino Library Manager.
2. Upload `reciever.ino`, capture your lamp remote's **ON** (and optional **OFF**) codes, and copy the printed send lines.
3. Paste those into `lampOn()` / `lampOff()` in `transmitter.ino`.
4. On the first upload of `transmitter.ino`, uncomment the `rtc.adjust(...)` line to set the clock, then re-comment it and upload again.

> If capture prints `Protocol=UNKNOWN`, your remote uses a non-standard protocol. Capture the raw timing with IRremote's `ReceiveDump` example and send it with `IrSender.sendRaw()` instead.

---

## Status

The original build (the board, the lamp, and the capture rig) has since been retired, so the lamp's real codes are no longer on hand. The `sendNEC` codes in `transmitter.ino` are examples; capture your own with `reciever.ino` before using it.

---

## Design evolution

This started rougher than the final version. Earlier attempts included switching a mains lamp through a relay (scrapped as unsafe) and driving an onboard high-power LED through a MOSFET. The IR-transmitter approach won out because it's the safest and simplest way to control an existing lamp without touching mains voltage or rewiring anything.

---

## Skills used

`C / C++` · `Arduino` · `DS1307 RTC` · `I²C` · `IR transmit / capture` · `KiCad (schematic capture + ERC)` · `Soldering` · `USB-C power`

---

## Repo contents

```
├── transmitter.ino          # sun clock firmware (RTC + IR transmit)
├── reciever.ino             # IR capture rig (grabs the remote's codes)
├── light_alarm/
│   └── light_alarm.ino      # first version: PWM sunrise fade + double-clap toggle
├── kicad/                   # project, root sheet, transmitter + receiver sheets, PCB outline, ERC report
└── README.md
```
