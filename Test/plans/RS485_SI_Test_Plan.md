# Hardware Bring‑Up Test Plan & Procedures  
**Document ID:** TDS-DTP-002 
**Revision:** 1.0  
**Author:** Jairo Huaylinos  
**Date:** 2026‑10-03 

---

# Table of Contents
1. Introduction  
2. Unit Under Test  
3. Equipment Required  
4. Test Plan Overview  
5. Test Procedures 
6. References 

---

# 1. Introduction
This document defines the Modbus over RS-485 half duplex Signal Integrity (SI) testing for the All-In-One STM32‑based Total Dissolved Solids (TDS) sensor PCB. It outlines the Unit Under Test (UUT), required test equipment, describes the test stages, and provides step‑by‑step procedures with clear success criteria.

Goal: Verify reliable Modbus RTU communication over RS‑485 half‑duplex on the prototype PCB.
Scope: Protocol‑level timing and error‑free communication. Along with basic RS‑485 electrical sanity checks.

Regulatory and formal RS‑485 electrical and commercial industrial compliance (TIA‑485, FCC Part 15, UL 61010‑1, IEC 61326‑1) are out of scope for this prototype phase. Furthermore, the TIA‑485‑A standard does not specify rise time, fall time, or jitter requirements for RS‑485 drivers.

---

# 2. Unit Under Test (UUT)

The UUT is a custom STM32‑based TDS sensor PCB, serial number 2, paired with an electrical conductivity (EC) sensor probe. The board uses an STM32F103C8T6 microcontroller with a user LED on PC13 and includes an onboard RS‑485 transceiver (PN MAX485CUA+T) for the electrical interface for Modbus protocol communication. The analog front end is an AC‑excitation EC measurement circuit designed to output a DC voltage based on the conductivity through the external EC probe. Power is supplied through a 12 V input and regulated down to 5 V, 3.3 V, 3 V, and −3 V rails.

---

# 3. Equipment Required

| Name | Part Number | Description |
|------|-------------|-------------|
| ST‑Link Programmer | ST‑LINK/V2 | SWD programmer/debugger with 3.3 V supply |
| Personal Computer | N/A | Any Desktop PC or laptop |
| 12V Power Supply | N/A | 12 V DC Wall Adapter with modified leads |
| Oscilloscope | SDS 1202X-E | Siglent 200 MHz 1Gsa/s Digital Storage Oscilloscope |
| Digital Multimeter | DM6000AR | AstroAI True-RMS DMM, 6000 Counts |
| Blink Firmware | N/A | Toggles PC13 LED for basic MCU verification |
| Modbus Dummy Slave Firmware | N/A | Sends fixed Modbus FC04 response frames for RS‑485 validation |
| ADC + Modbus Firmware | N/A | Reads ADC and reports voltage via Modbus |
| ModbusTool SW |  N/A | Modbus Master tool to poll UUT. https://github.com/ClassicDIY/ModbusTool.git |
| USB - RS485 Converter | N/A | CERRXIAN 1FT RS485 to USB Terminal Converter Serial Port Cable |

---

# 4. Test Plan Overview

The RS485 Signal Integrity Test Plan covers 3 baud rates: 1200, 9600, 19200 Bd. For each of those rates, the following are verified:
- Successful transmission and reception of Modbus RTU messages.
- Device UART message frames are transmitted with less than 1.5 character times spacing. 1 character is defined as one full UART frame. 1 character is 11 bits (1 start bit, 8 bits for data, 1 parity bit, and 1 stop bit)
- Device transmission baud rate accuracy is within 1%.
- RS-485 logic voltage levels are correct.
- Less than 200mV magnitude difference between RS485 logic HIGH (V<sub>A</sub>-V<sub>B</sub>>=0) and logic LOW (V<sub>A</sub>-V<sub>B</sub><>=0).
- Rise and fall times (t<sub>R</sub>, t<sub>F</sub>) within MAX485CUA+T manufacturer specifications (MIN: 3ns, TYP: 15ns, MAX: 40ns).

Each stage includes structured procedures with defined success criteria.

Out of scope:
- Recepetion baud rate tolerance of up to 2%. -> do not have tools to test this. this is more of a software capability.
- Response to both unicast and broadcast messages.
- Even Parity testing is not included since test plan is hardware centric, not protocol centric.

---

# 5. Test Procedures

---

## 5.1 ST‑Link Power‑Up Procedure

<p align="center">
  <img src="../assets/images/RS485_SI_Test_Setup.png"
       alt="Test Setup"
       width="600">
  <br>
  <em>Figure 1: RS485 SI Test Setup</em>
</p>

<p>&nbsp;</p>

| Step # | Description | Success Criteria | P/F |
|--------|-------------|------------------|-----|
| 1 | Connect the oscilloscope probes to the RS485 A and B output wires on the TDS sensor board | Probe is connected to A and B lines with appropriate ground reference |  |
| 2 | Connect ST-Link SWCLK, SWDIO, and GND pins to the UUT on connector J2. Disconnect 3.3V pin | Board remains unpowered until 12V is applied |  |
| 3 | Connect ST-Link SWCLK, SWDIO, and GND pins to the UUT on connector J2. Do not conect the 3.3V pin | Board remains unpowered until 12V is applied |  |
| 4 | Connect the USB-RS485 Converter A, B, and GND rails to the UUT on connector J4. Connect the USB interface end to your PC. | Board remains unpowered until 12V is applied |  |
| 5 | Flash Modbus RTU slave driver code with a 9600 baud rate (use a release build, not debug build). | Release build is successfully flashed to the board successfully with no errors |  |
| 6 | Open the Modbus Master SW tool on your PC. Connect to the corresponding COM port with Baud=9600, Parity = None, Data Bits = 8, and Stop Bits = 1, Slave ID = 1. These settings should match you UART settings on the UUT's MCU. | Log shows "Connected using RTU to COMX", Stable connection. |  |
| 7 | Select "read input register" in functions and set Poll to 1000 and check the box to activate polling. Set Start Address to 0. Set Size to 1. Click Apply button| Log shows "Read succeeded: Function Code: 4." |  |
| 8 | Set oscilloscope to 500mV/Div, trigger level to 2.5V. Setup for single capture. | Oscilloscope captures ,modbus response frame from the TDS module |  |
| 9 | Measure the spacing between UART frames (11-bit Modbus Characters) | spacing between character frames are less than 1.5 characters long |  |
| 10 | Measure baud rate | Measured Baud Rate is between 9504 and 9696 |  |
| 11 | Measure the RS-485 voltage levels | voltage levels should be 0V and 5V |  |
| 12 | Measure the difference between the magnitudes of RS485 A-B and B-A | magnitude difference between A-B and B-A should be less than 200mV |  |
| 13 | Flash Modbus RTU slave driver code with a 19200 baud rate (use a release build, not debug build). | Release build is successfully flashed to the board successfully with no errors |  |
| 14 | Open the Modbus Master SW tool on your PC. Connect to the corresponding COM port with Baud=19200, Parity = None, Data Bits = 8, and Stop Bits = 1, Slave ID = 1. These settings should match you UART settings on the UUT's MCU. | Log shows "Connected using RTU to COMX", Stable connection. |  |
| 15 | Select "read input register" in functions and set Poll to 1000 and check the box to activate polling. Set Start Address to 0. Set Size to 1. Click Apply button| Log shows "Read succeeded: Function Code: 4." |  |
| 16 | Set oscilloscope to 500mV/Div, trigger level to 2.5V. Setup for single capture. | Oscilloscope captures ,modbus response frame from the TDS module |  |
| 17 | Measure the spacing between UART frames (11-bit Modbus Characters) | spacing between character frames are less than 1.5 characters long |  |
| 18 | Measure baud rate | Measured Baud Rate is between 19008 and 19392 |  |
| 19 | Measure the RS-485 voltage levels | voltage levels should be 0V and 5V |  |
| 20 | Measure the difference between the magnitudes of RS485 A-B and B-A | magnitude difference between A-B and B-A should be less than 200mV |  |
| 21 | Flash Modbus RTU slave driver code with a 1200 baud rate (use a release build, not debug build). | Release build is successfully flashed to the board successfully with no errors |  |
| 22 | Open the Modbus Master SW tool on your PC. Connect to the corresponding COM port with Baud=1200, Parity = None, Data Bits = 8, and Stop Bits = 1, Slave ID = 1. These settings should match you UART settings on the UUT's MCU. | Log shows "Connected using RTU to COMX", Stable connection. |  |
| 23 | Select "read input register" in functions and set Poll to 1000 and check the box to activate polling. Set Start Address to 0. Set Size to 1. Click Apply button| Log shows "Read succeeded: Function Code: 4." |  |
| 24 | Set oscilloscope to 500mV/Div, trigger level to 2.5V. Setup for single capture. | Oscilloscope captures ,modbus response frame from the TDS module |  |
| 25 | Measure the spacing between UART frames (11-bit Modbus Characters) | spacing between character frames are less than 1.5 characters long |  |
| 26 | Measure baud rate | Measured Baud Rate is between 1188 and 1212 |  |
| 27 | Measure the RS-485 voltage levels | voltage levels should be 0V and 5V |  |
| 28 | Measure the difference between the magnitudes of RS485 A-B and B-A | magnitude difference between A-B and B-A should be less than 200mV |  |

### Notes

---

# 6. References
