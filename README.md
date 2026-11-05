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

- **`light_alarm/light_alarm.ino`:** the first version, before the IR redesign. At the alarm time it fades an LED in with PWM over 15 minutes along an exponential, sunrise-matched curve (see below), and a sound sensor on **D2** toggles the light on a double clap (two claps within 850 ms). Uses **RTClib** only.

### The sunrise brightness curve (`light_alarm.ino`)

The first version faded the LED in with a **linear** PWM ramp (`map(t, 0, T, 0, 255)`). Looking at how real daylight behaves showed that a linear ramp isn't the right shape, so the fade now follows an **exponential** curve.

**What the sun actually does.** Under a clear sky, horizontal illuminance during morning civil twilight (sun 6° below the horizon up to sunrise) climbs from about **3.4 lux to about 410 lux**. That's roughly a 120× increase over about 30 minutes at mid-latitudes ([AMS Glossary](https://glossary.ametsoc.org/wiki/civil-twilight/)). Before that, nautical twilight starts near 0.008 lux, so light grows by about two orders of magnitude for every 6° the sun climbs. In other words, light rises **exponentially in time**, not linearly. The end point also fits the use: clinical dawn simulators peak at about 100–300 lux ([Dawn simulation](https://en.wikipedia.org/wiki/Dawn_simulation)), which is about what the sun gives at sunrise.

**What the LED does.** `analogWrite` doesn't output a lower voltage. It switches the pin between 0 V and 5 V, and the duty cycle sets the *average* current. An LED's light output is roughly proportional to its current, so **brightness ∝ duty cycle**. If you varied a DC voltage instead, the current (and the light) would rise exponentially once you passed the forward voltage (Shockley diode equation). That's very hard to control, and it's why PWM is used.

**The equation.** To make the LED follow civil twilight, compressed into the fade time *T*:

```
duty(t) = 255 · (1/120)^(1 − t/T)        0 ≤ t ≤ T
```

This starts at about 2/255 (the "first light" level) and finishes at 255, growing about 120× just like the sky does. A 120:1 range also fits inside 8-bit PWM (1–255). The full night-to-sunrise range of about 50,000:1 would not.

**Why it also looks right.** Eyes judge brightness on a roughly logarithmic / cube-root scale (Weber–Fechner, CIE lightness). Here is the perceived lightness (CIE L*, 0–100) for both ramps over a 15-minute fade:

| minute | 0 | 1 | 3 | 5 | 7.5 | 10 | 12 | 15 |
|---|---|---|---|---|---|---|---|---|
| linear ramp | 0 | 31 | 52 | 64 | 76 | 85 | 92 | 100 |
| exponential (sun) | 7 | 10 | 16 | 24 | 36 | 52 | 68 | 100 |

The linear ramp reaches half of its perceived brightness in the first 3 minutes and then hardly changes. It looks like the light "pops on" and then stalls. The exponential ramp brightens gradually through the whole fade, the way a real dawn does.

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
