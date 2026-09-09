/*
 * modbus_slave.h
 *
 *  Created on: Dec 22, 2025
 *      Author: jairo
 */

#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include <stdio.h>
#include "Modbus_RTU_Driver.h"

// define slave address
#define MODBUS_SLAVE_ID 1

// register map sizes
#define MODBUS_NUM_HOLDING_REGS 32
#define MODBUS_NUM_INPUT_REGS 32

extern uint16_t g_mb_holding_regs[MODBUS_NUM_HOLDING_REGS];
extern uint16_t g_mb_input_regs[MODBUS_NUM_INPUT_REGS];

// Called by RTU driver when a frame is ready. this is the callback function
void Modbus_Slave_OnFrame(const uint8_t *req, uint16_t len);

// Initialize Modbus slave (if needed later)
void Modbus_Slave_Init(void);

#endif





