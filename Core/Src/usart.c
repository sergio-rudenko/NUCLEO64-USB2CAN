/**
 ******************************************************************************
 * File Name          : USART.c
 * Description        : This file provides code for the configuration
 *                      of the USART instances.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

uint8_t uartRxData[UART_RX_BUFFER_SIZE];
uint8_t uartTxData[UART_TX_BUFFER_SIZE];

rBuffer_t uartRxBuf;
rBuffer_t *pUartRxBuf = &uartRxBuf;

rBuffer_t uartTxBuf;
rBuffer_t *pUartTxBuf = &uartTxBuf;

/* USER CODE END 0 */

UART_HandleTypeDef huart2;

/* USART2 init function */

void MX_USART2_UART_Init(void) {

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}

}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle) {

	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (uartHandle->Instance == USART2) {
		/* USER CODE BEGIN USART2_MspInit 0 */

		/* USER CODE END USART2_MspInit 0 */
		/* USART2 clock enable */
		__HAL_RCC_USART2_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**USART2 GPIO Configuration
		 PA2     ------> USART2_TX
		 PA3     ------> USART2_RX
		 */
		GPIO_InitStruct.Pin = USART_TX_Pin | USART_RX_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USART2 interrupt Init */
		HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
		/* USER CODE BEGIN USART2_MspInit 1 */

		/* Initialize ring buffers */
		ring_buffer_init(pUartRxBuf, uartRxData, sizeof(uartRxData));
		ring_buffer_init(pUartTxBuf, uartTxData, sizeof(uartTxData));

		/* USER CODE END USART2_MspInit 1 */
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle) {

	if (uartHandle->Instance == USART2) {
		/* USER CODE BEGIN USART2_MspDeInit 0 */

		/* USER CODE END USART2_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USART2_CLK_DISABLE();

		/**USART2 GPIO Configuration
		 PA2     ------> USART2_TX
		 PA3     ------> USART2_RX
		 */
		HAL_GPIO_DeInit(GPIOA, USART_TX_Pin | USART_RX_Pin);

		/* USART2 interrupt Deinit */
		HAL_NVIC_DisableIRQ(USART2_IRQn);
		/* USER CODE BEGIN USART2_MspDeInit 1 */

		/* USER CODE END USART2_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */

void UART2CAN_UART_Receive(UART_HandleTypeDef *handle, rBuffer_t *rb) {
	uint8_t *p = ring_buffer_get_write_ptr(rb);

	HAL_UART_Receive_IT(handle, p, 1);
}

void UART2CAN_UART_Transmit(UART_HandleTypeDef *handle, rBuffer_t *rb) {
	uint8_t *p = ring_buffer_get_read_ptr(rb);
	size_t len = ring_buffer_get_read_linear_size(rb);

	HAL_UART_Transmit_IT(handle, p, len);
	ring_buffer_move_read_index(rb, len);
}

bool UART2CAN_UART_Is_Tx_Busy(UART_HandleTypeDef *handle) {
	return (handle->gState == HAL_UART_STATE_BUSY_TX_RX
			|| handle->gState == HAL_UART_STATE_BUSY_TX);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *handle) {
	if (*ring_buffer_get_write_ptr(pUartRxBuf) != '\n') {
		ring_buffer_move_write_index(pUartRxBuf, 1);
	}

	UART2CAN_UART_Receive(handle, pUartRxBuf);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *handle) {
	if (ring_buffer_available(pUartTxBuf)) {
		UART2CAN_UART_Transmit(handle, pUartTxBuf);
	}
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
