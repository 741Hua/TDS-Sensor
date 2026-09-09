/*
 * modbus_slave.c
 *
 *  Created on: Dec 22, 2025
 *      Author: jairo
 */

#include "modbus_slave.h"

extern ModbusRTU_Driver rtu; // declared wherever global driver lives

//Simple register arrays for now
uint16_t g_mb_holding_regs[MODBUS_NUM_HOLDING_REGS];
uint16_t g_mb_input_regs[MODBUS_NUM_INPUT_REGS];

// Forward declarations
static uint16_t Modbus_CRC16(const uint8_t *buf, uint16_t len);
static void Modbus_SendException(uint8_t function, uint8_t exception_code);
static void Modbus_Handle_FC03(const uint8_t *req, uint16_t len);
static void Modbus_Handle_FC04(const uint8_t *req, uint16_t len);
//static void Modbus_Handle_FC06(const uint8_t *req, uint16_t len);
//static void Modbus_Handle_FC16(const uint8_t *req, uint16_t len);

// You can keep this empty for now or use it to preset registers
void Modbus_Slave_Init(void)
{
    // e.g. g_mb_holding_regs[0] = 1234;

}

static uint16_t Modbus_CRC16(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];

        for (int i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

void Modbus_Slave_OnFrame(const uint8_t *req, uint16_t len)
{
	if(len<4) return; // minimum frame is 4 bytes: (1) address + (1) func + (2) CRC
	uint8_t addr = req[0];
	uint8_t func = req[1];

	// CRC check
	uint16_t received_crc = (uint16_t)req[len - 2] | ((uint16_t)req[len - 1] << 8);
	uint16_t calc_crc = Modbus_CRC16(req, len - 2);

	if (received_crc != calc_crc) {
		return; // bad CRC, ignore
	}


	if(addr != MODBUS_SLAVE_ID && addr != 0) return; // only act if frame addressed to this slave or if it is a broadcast frame


	switch (func) {
		case 0x03:
			Modbus_Handle_FC03(req, len);
			break;
		case 0x04:
			Modbus_Handle_FC04(req, len);
			break;
		/*case 0x06:
			Modbus_Handle_FC06(req, len);
			break;*/
		default:
			Modbus_SendException(func, 0x01); // Illegal Function
			break;
	}
}

static void Modbus_Handle_FC03(const uint8_t *req, uint16_t len)
{
	//req: [addr][func][start_hi][start_lo][qty_hi][qty_lo][CRC_lo][CRC_hi]
	if(len < 8) return;
	uint16_t start = ((uint16_t)req[2]<<8) | (uint16_t)req[3];
	uint16_t qty = (uint16_t)req[5] | ((uint16_t)req[4] << 8);
	if(qty < 0x0001 || qty > 0x007D)
	{
		Modbus_SendException(0x03, 0x03);
		return;
	}
	if (qty == 0 || (start + qty) > MODBUS_NUM_HOLDING_REGS) {
		Modbus_SendException(0x03, 0x02); // ILLEGAL DATA ADDRESS
		return;
	}

    uint8_t resp[256];
    uint16_t idx = 0;

    resp[idx++] = MODBUS_SLAVE_ID;
    resp[idx++] = 0x03;
    resp[idx++] = (uint8_t)(qty * 2); // byte count

    for (uint16_t i = 0; i < qty; i++) {
    	uint16_t val = g_mb_holding_regs[start + i];
        resp[idx++] = (uint8_t)(val >> 8);
        resp[idx++] = (uint8_t)(val & 0xFF);
    }

    uint16_t crc = Modbus_CRC16(resp, idx);
    resp[idx++] = (uint8_t)(crc & 0xFF);
    resp[idx++] = (uint8_t)(crc >> 8);

    ModbusRTU_Send(&rtu, resp, idx);
}

static void Modbus_Handle_FC04(const uint8_t *req, uint16_t len)
{
	//req: [addr][func][start_hi][start_lo][qty_hi][qty_lo][CRC_lo][CRC_hi]
	if(len<8) return;
	uint16_t start = (uint16_t)req[2]<<8 | (uint16_t)req[3];
	uint16_t qty = (uint16_t) req[4]<<8 | (uint16_t)req[5];

	if(qty < 0x0001 || qty > 0x007D)
	{
		Modbus_SendException(0x04, 0x03);
		return;
	}
	// The below checks valid starting address, and that start plus qty is within range
	// This doesn't check negative qty's, but shouldn't matter bc not possible
	if (qty == 0 || (start + qty) > MODBUS_NUM_INPUT_REGS) {
		Modbus_SendException(0x04, 0x02); // ILLEGAL DATA ADDRESS
		return;
	}

	uint16_t idx = 0;
	uint8_t resp[256];
	resp[idx++] = MODBUS_SLAVE_ID;
	resp[idx++] = 0x04;
	resp[idx++] = (uint8_t) (qty*2);

	for(uint16_t i = 0; i < qty; i++)
	{
		uint16_t val = g_mb_input_regs[start + i];
		resp[idx++] = (uint8_t)(val >> 8);
		resp[idx++] = (uint8_t)(val & 0xFF);
	}
	uint16_t crc = Modbus_CRC16(resp, idx);
	resp[idx++] = (uint8_t)(crc & 0xFF);
	resp[idx++] = (uint8_t)(crc >> 8);

	ModbusRTU_Send(&rtu, resp, idx);
}

static void Modbus_SendException(uint8_t function, uint8_t exception_code)
{
    uint8_t resp[5];
    uint16_t idx = 0;

    resp[idx++] = MODBUS_SLAVE_ID;
    resp[idx++] = function | 0x80; // exception bit
    resp[idx++] = exception_code;

    uint16_t crc = Modbus_CRC16(resp, idx);
    resp[idx++] = (uint8_t)(crc & 0xFF);
    resp[idx++] = (uint8_t)(crc >> 8);

    ModbusRTU_Send(&rtu, resp, idx);
}

