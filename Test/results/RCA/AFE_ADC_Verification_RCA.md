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
### Step 1 — Verify AFE and sensor probe responsiveness
- Measured AFE output and voltage difference between the two EC probe wires when the EC probe was submerged in high‑concentration salt water and when the EC probe was in air.  
- AFE output changed from about -220 mV (submerged) to about -80 mV (in air), confirming the AFE responds to probe conditions.
- Observed a large voltage difference in air and near 0 V difference in high‑concentration salt water, confirming the EC probe is functional and not the root cause.  
- Conclusion: **AFE and EC probe are responsive; probe failure is not the root cause.**

### Step 2 — Visual inspection and contamination removal
- Board had been sitting uncovered on the bench and accumulated dust and particulate debris.
- Executed a visual inspection and found string‑like dust particles and tin whiskers on the board.
- Removed dust with compressed air and tweezers. Removed all tin whiskers on the AFE circuitry via mechanical removal and by reflow with a soldering iron.
- Re‑measured AFE output after cleaning: **-220 mV persisted. Short circuits due to tin whiskering and dust are not the root cause.**

### Step 3 — Oscillator generator diagnosis and RC timing network correction
- Probed the oscillator output at pin 7 of IC2 and observed a square wave with correct amplitude limits (+3V to -3V), and a measured frequency of 290.1 Hz. One magnitude off from the design target of 3.95 kHz.

<p align="center">
  <img src="../../assets/images/5V_Regulator_Corrective_Action.jpg" alt="Oscillator output">
</p>

<p align="center">
  <em>Figure 1: Oscillator Output at IC2 Pin 7</em>
</p>

- Suspected fault in the oscillators external RC timing network. So, verified that the SMD resistor package values match the schematic. Found that the correct resistors were used per design.
- Suspect a faulty RC smd component. So, measured resistances and capacitance with a DMM: R4 ≈ 100 kΩ, R5 ≈ 100 kΩ. Measured C25 ≈ 1 nF.
- Measured continuity of the RC network and found it matched the schematic design.
- Suspect design error in the external RC timing network. Calculated expected frequency from the datasheet and discovered R5 should be 10 kΩ, not 100 kΩ, to achieve 3.95 kHz.  
- Replaced R5 with a 10 kΩ resistor and verified the oscillator produced the expected output frequency and voltages.  
- Observation: **Fixing the oscillator frequency did not resolve the -220 mV AFE output issue.**

### Step 4 — Probe amplifier stages and identify clipping
- Probed outputs of Op Amp A and Op Amp C in the AFE chain.  
- Observed square waves that only swung from **0 V** to approximately **-0.8 V to -1 V**; the positive portion of the waveform was missing.  
- Interpreted the waveform as positive‑side clipping at the amplifier stages, suggesting the op‑amp positive supply was not present or not connected.

### Step 5 — Power‑rail continuity checks and solder joint inspection
- Performed continuity checks between the 3 V rail and the op‑amp IC power pin (Pin 4) and between ground nets and the IC ground pins (Pins 5 and 10).  
- Found **no continuity** between the 3 V rail and the op‑amp power pin and unexpected continuity patterns on some pins.  
- Visual inspection revealed **cold solder joints** at the op‑amp power and ground pins and at other pins on the IC.

### Step 6 — Reflow soldering and verification
- Reflowed the suspect op‑amp pins by dragging a soldering iron across the pins to ensure proper solder wetting and joint formation.  
- Re‑ran continuity checks between the 3 V rail and the op‑amp power pin and between ground nets and the op‑amp ground pins; continuity matched the schematic design.  
- Re‑probed amplifier outputs after reflow and observed the negative‑voltage clipping (**-220 mV**) was resolved.

### Step 7 — Component replacement and final verification
- Despite restored continuity, AFE output did not reach the expected near‑3 V level when the EC probe was dipped in the high‑concentration salt solution (expected ≈ 3 V).  
- Suspected the op‑amp IC had been damaged; replaced the op‑amp with a new, known‑good device.  
- After replacement, observed ADC/AFE output near the expected maximum (measured **~2.4 V**) when the EC probe was dipped in the high‑concentration salt solution.  
- Verified proper waveforms and expected signals at each op‑amp stage.

### Step 3
- Ok then quickly summarize: I probed the the Op Amp A output and op Amp C output. Both demonstrated square waves from around 0V to a negative voltage (aobut -800mV or -1V). Suspect unconnected 3V power input to Op Amp IC. Continutity check betweent the power rails and the IC pins revealed unconnected 3V power input pin (Pin 4), unconnected gnd pins (pin 10 and 5). Visual inspection revealed cold solder joints at these pins and other pins. Corrective action to reflow solder to the pins using soldering iron. Post resolder, ran continuity check between IC pins and expected signals or power buses. Also ran continuity check to verify board matches AFE shematic design. Continuity per design verified. This did solve the negative voltage issue, no longer seeing -220mV. However, we were not seeing and expecte AFE output voltage near the 3V max when the EC sensor was dipped in a high concentration salt water solution (way beyond 1000ppm since it was a teaspoon of salt in a 1/4 cup of water).

Suspected damaged Op Amp IC. Replaced with brand new one. observed ADC output near 3V max (2.4V) when EC probe was dipped in the high concentration salt water solution. Verified proper signals at each op amp stage.


---

## 5. Root Cause
### **Primary Root Cause**
Incorrect R5 resistor value (100kohm instead of 10kohm) and a damaged op amp IC CD4060BM (IC3) from rework.  

### **Secondary Causes**
- Lack of schematic-to-footprint verification step.
- Use of generic library symbols without validating pin assignments  

---

## 6. Corrective Actions

### Implemented
- 
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
