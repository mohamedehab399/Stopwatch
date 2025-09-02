# Stopwatch with Dual Mode ⏱️⏳

A digital stopwatch system built using **ATmega32 Microcontroller** and programmed in **Embedded C**.  
The stopwatch operates in two modes: **Count-Up** and **Count-Down**.

---

## ✨ Features
- **Dual Mode Operation**
  - Count-Up mode (⏱️)  
  - Count-Down mode (⏳) with buzzer alert when time reaches zero  
- **External Interrupts**
  - **INT0** → Reset stopwatch  
  - **INT1** → Pause stopwatch (also silences buzzer if active)  
  - **INT2** → Resume stopwatch  
- **Time Adjustment**
  - Buttons to increment/decrement **hours, minutes, seconds**  
- **Visual Indicators**
  - **Red LED** → Count-Up mode  
  - **Yellow LED** → Count-Down mode  
- **6-digit Multiplexed 7-Segment Display**
  - Driven by **7447 BCD-to-7seg decoder**  
  - Displays time in `HH:MM:SS` format  
- **Buzzer**
  - Activated when countdown finishes  

---

## 🛠 Technologies & Tools
- **C Programming (AVR-GCC)**
- **ATmega32 Microcontroller**
- **Timer1 in CTC Mode**
- **External Interrupts (INT0, INT1, INT2)**
- **Proteus Simulation**

---

## 🎯 Learning Outcomes
- Practiced using **Timers & Prescalers** for time measurement  
- Gained experience with **external interrupts** for real-time control  
- Implemented **multiplexing** for multi-digit 7-segment displays  
- Built a complete embedded system from scratch  

---

## 📂 Project Structure
