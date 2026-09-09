/*
 * Modbus_RTU_Driver.h
 *
 *  Created on: Dec 16, 2025
 *      Author: jairo
 */

#ifndef INC_MODBUS_RTU_DRIVER_H_
#define INC_MODBUS_RTU_DRIVER_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

typedef void (*Modbus_FrameCallback)(const uint8_t *buf, uint16_t len);

typedef struct {
    UART_HandleTypeDef *huart; // uart struct, see uart header
    GPIO_TypeDef *DE_Port; // TX enable port.
    uint16_t DE_Pin; // TX enable pin.
    uint8_t rx_buf[256]; // receive buffer, Modbus spec defines as maximum of 257 characters.
    volatile uint16_t rx_len; // receive buffer length, volatile because this tell compiler to expect this variable to change due to interrupts.
    volatile uint8_t frame_pending; // set when IDLE ready, end of frame detected
    Modbus_FrameCallback on_frame; // function to call when a frame is ready, callback function is user defined and established in Init()
} ModbusRTU_Driver;

void ModbusRTU_Init(ModbusRTU_Driver *d, UART_HandleTypeDef *huart,
                    GPIO_TypeDef *DE_Port, uint16_t DE_Pin,
                    Modbus_FrameCallback cb);

HAL_StatusTypeDef ModbusRTU_Send(ModbusRTU_Driver *d, const uint8_t *p, uint16_t len); // send function
void ModbusRTU_OnRxByte(ModbusRTU_Driver *d, uint8_t byte); // RX interrupt handler
void ModbusRTU_OnIdle(ModbusRTU_Driver *d); // IDLE interrupt handler
void ModbusRTU_Poll(ModbusRTU_Driver *d); // this is main loop polling.

#endif /* INC_MODBUS_RTU_DRIVER_H_ */
