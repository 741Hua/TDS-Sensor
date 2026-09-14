# AFE and ADC Verification Root Cause Analysis (RCA)
## AFE Failure During Hardware Bring-Up  
**Author:** Jairo Huaylinos
**Date:** 2026-09-13 
**Board:** Custom STM32-Based TDS Sensor PCB  

---

## 1. Overview
During the AFE & ADC Verification stage of the TDS Sensor PCB HW bring-up, the AFE output voltage / ADC input voltage reading was measured to be -220mV while the EC sensor probe was submersed in room temperature tap water. Under normal operation, the AFE should never output a negative voltage. The AFE is intended to produce a voltage in the range of 0-3.3V.

This Root Cause Analysis uses a **Fishbone (Ishikawa) Methodology** to identify contributing factors across design, components, process, tools, and testing.

---

## 2. Problem Statement
The board's AFE outputs -220mV when the EC sensor probe is submersed in room temperature tap water and the modbus reported ADC voltage is 0V. Symptoms included:

- Incorrect ADC voltage reporting. 

The failure could originate from the MCU, oscillation generation stage, probe itself, amplification stage, rectification stage.

---

## 3. Fishbone (Ishikawa) Analysis

### 3.1 Categories Considered
- **Design**
- **Components**
- **Manufacturing / Assembly**
- **Tools / EDA**
- **Testing / Process**
- **Environment**

---

### 3.2 Fishbone Breakdown

#### **Design**
- Incorrect routing of AFE components.

#### **Components**
- Faulty AFE ICs (IC2: CD4060BM and IC3: LMV324AQDYYRQ1).
- Faulty AFE passive components.
- Faulty ADC on the STM32F103C8T6 MCU.
- Faulty EC Sensor Probe.

#### **Manufacturing / Assembly**
- Short circuit caused by solder bridges or tin whiskering between AFE IC pins or passives.
- Disconnected IC pins due to missing/insufficient solder, cold joints, or damaged pads. 
- Tombstoned components.
- Broken SMD package pins during assembly.

#### **Tools**
- Firmware bug forcing a 0V ADC reading.

#### **Testing / Process**


#### **Environment**
- Dust creating short circuit conditions. 

---

## 4. Diagnostic Process Summary

### Step 1 — Short-Circuit Elimination
- Continuity checks confirmed no shorts between any of the P<sub>in</sub>, 5V, 3.3V, 3V, and -3V power and ground nets.
  
### Step 2 — Initial Bring-Up
- 3.3 V rail showed severe sag when powered by the ST-Link V2 Debugger. 
- ST‑Link power produced dim LED blink   
- Attempted to power up using an external 12V power input which resulted in the 5V regulator overheating and no expected 5V output. Measured 5V output with DMM was 200mV.

### Step 3 — Isolation via Desoldering
- 3.3V regulator removed to split power tree to allow independent testing of upstream (5V, RS485) and downstream (±3V, MCU) subsystems.

### Step 4 — Upstream Regulator Testing
- With the 3.3V regulator removed, the 5V regulator was again powered on with the external 12V power supply.
- Identical failure as before: 5V regulator overheated and 5V output was measured at 200mV.
- Suspected faulty regulator, so replaced with a brand new one. Identical failure observed.
- Suspected design/footprint error, verified by comparison between regulator datasheet and schematic/PCB footprints.

<p align="center">
  <img src="../../assets/images/5V_Regulator_Datasheet_Pinout.png" width="30%" alt="5V Regulator Datasheet">
  <img src="../../assets/images/5V_Regulator_Schematic.png" width="30%" alt="5V Regulator Schematic">
  <img src="../../assets/images/5V_Regulator_PCB_footprint.png" width="30%" alt="5V Regulator PCB footprint">
</p>

<p align="center">
  <em>Figure 1: 5V Regulator datasheet pinout vs incorrect design footprint pinouts</em>
</p>

<p>&nbsp;</p>

- Conclusion: **Design error: Incorrect schematic and PCB footprints**, not component failure.

### Step 5 — Downstream Regulator Testing
- Powered up with 3.3V from the ST‑Link V2 debugger.
- Voltage sag on the 3.3V bus, measured to be 2.4V. 
- Verified 3V and -3V regulator funtionality via measurement with a DMM. +3V and -3V regulators outputed the expected stable voltages.  
- Conclusion: **downstream circuitry functional**. 

### Step 6 — Reinstallation of 3.3 V Regulator
- Suspected faulty 3.3V regulator, so replaced with a brand new one. Identical failure observed.
- Suspected design/footprint error, verified by comparison between regulator datasheet and schematic/PCB footprints.

<p align="center">
  <img src="../../assets/images/3.3V_Regulator_Datasheet_Pinout.png" width="30%" alt="3.3V Regulator Datasheet">
  <img src="../../assets/images/3.3V_Regulator_Schematic.png" width="30%" alt="3.3V Regulator Schematic">
  <img src="../../assets/images/3.3V_Regulator_PCB_footprint.png" width="30%" alt="3.3V Regulator PCB footprint">
</p>

<p align="center">
  <em>Figure 2: 3.3V Regulator datasheet pinout vs incorrect design footprint pinouts</em>
</p>

<p>&nbsp;</p>

---

## 5. Root Cause
### **Primary Root Cause**
Incorrect PCB footprints for both the 5V and 3.3V regulators due to mismatched pin mapping between schematic symbols and manufacturer datasheets.

### **Secondary Causes**
- Lack of schematic-to-footprint verification step.
- Use of generic library symbols without validating pin assignments  

---

## 6. Corrective Actions

### Implemented
- Corrected regulator footprints based on manufacturer datasheets.  
- Resoldered 5V and 3.3V regulators with jumper wires to continue prototype testing.
- Revalidated downstream rails and MCU operation via successful execution of the ST‑Link Power‑Up & Blink and External 12 V Power‑Up & Blink test stages of the HW Bring Up Test Plan.
- 
<p align="center">
  <img src="../../assets/images/5V_Regulator_Corrective_Action.jpg" width="45%" alt="5V Regulator Corrective Action">
  <img src="../../assets/images/3.3V_Regulator_Corrective_Action.jpg" width="45%" alt="3.3V Regulator Corrective Action">
</p>

<p align="center">
  <em>Figure 3: 5V and 3.3V Regulator Temporary Fix</em>
</p>

<p>&nbsp;</p>

### Recommended Preventive Actions
- Add mandatory footprint verification step to design workflow.  
- Use manufacturer-provided footprints whenever possible.

---

## 7. Conclusion
The failure was caused by incorrect footprints for both the 5V and 3.3V regulators, resulting in improper pin connections and regulator malfunction. Systematic isolation through desoldering and staged testing confirmed that downstream circuitry was functional and that the upstream regulators were the sole contributors to the voltage sag and power-up failure.

This RCA demonstrates effective use of isolation, staged testing, and datasheet-driven verification to identify and resolve complex power regulation issues.
