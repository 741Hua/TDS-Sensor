/*
 * Modbus_RTU_Driver.c
 *
 *  Created on: Dec 17, 2025
 *      Author: jairo
 */

// Modbus_RTU_Driver.c
#include "Modbus_RTU_Driver.h"

void ModbusRTU_Init(ModbusRTU_Driver *d, UART_HandleTypeDef *huart,
                    GPIO_TypeDef *DE_Port, uint16_t DE_Pin,
                    Modbus_FrameCallback cb) {
    d->huart = huart;
    d->DE_Port = DE_Port;
    d->DE_Pin = DE_Pin;
    d->rx_len = 0;
    d->frame_pending = 0;
    d->on_frame = cb;

    HAL_GPIO_WritePin(DE_Port, DE_Pin, GPIO_PIN_RESET); // RX mode
    __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE); // enable interrupt for received byte
    __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE); // IDLE line marks frame boundary. triggered after 1 character time silence.
    __HAL_UART_ENABLE_IT(huart, UART_IT_ERR); // enable interrupt for error detection
}

HAL_StatusTypeDef ModbusRTU_Send(ModbusRTU_Driver *d, const uint8_t *p, uint16_t len) {
    HAL_GPIO_WritePin(d->DE_Port, d->DE_Pin, GPIO_PIN_SET); // TX
    HAL_StatusTypeDef st = HAL_UART_Transmit(d->huart, p, len, 100); // in OG copilot code, p was casted to (uint8_t *) b/c assumed this function didn't define p as const
    if (st != HAL_OK) {
        HAL_GPIO_WritePin(d->DE_Port, d->DE_Pin, GPIO_PIN_RESET); // back to RX
        return st;
    }
    while (__HAL_UART_GET_FLAG(d->huart, UART_FLAG_TC) == RESET) {}
    HAL_GPIO_WritePin(d->DE_Port, d->DE_Pin, GPIO_PIN_RESET); // RX
    return HAL_OK;
}

// Call from ISR when RXNE set
void ModbusRTU_OnRxByte(ModbusRTU_Driver *d, uint8_t byte) {
    if (d->rx_len < sizeof(d->rx_buf)) {
        d->rx_buf[d->rx_len++] = byte;
    } else {
        // overflow: drop frame
        d->rx_len = 0; // reset rx_len. but we never reset the rx_buf. rx_buff will just get overwritten starting at index 0 and increasing.
    }
}

// Call from ISR when IDLE flag set
void ModbusRTU_OnIdle(ModbusRTU_Driver *d) {
    if (d->rx_len > 0) { // check that we have been receiving data, if we now get idle, that means end of frame.
        d->frame_pending = 1;
        // IDLE triggers at multiple times when bus is silent not just at 1 char time after end of frame
        // thus we have to check that a frame was in progress
    }
}

// Call from main loop; dispatch completed frames
void ModbusRTU_Poll(ModbusRTU_Driver *d) {
    if (d->frame_pending) { // if end of frame detected and ready for next frame
        d->frame_pending = 0; // reset, we already know end of frame and are taking action in the following
        if (d->on_frame) d->on_frame(d->rx_buf, d->rx_len); // if(d->on_frame) checks if there is a callback function defined. calls the the callback handler function we defined at Init (Modbus_FrameCallback cb)
        // I assume the callback function will restart the UART_ENABLE_IT() functions
        d->rx_len = 0; // reset buffer len so that next frame starts filling buffer at index 0.
    }
}

