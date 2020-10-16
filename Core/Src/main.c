/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "lawicel.h"

#include "ucanopen.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char deviceSoftwareVersion[32] = "v0.05_DEV";

static Button_t UserButton;
Button_t *pUserButton = &UserButton;

static Signal_t UserLed;
Signal_t *pUserLed = &UserLed;

// Manufacturer OD
const uint16_t deviceTest16bitRO = 123;
uint8_t deviceTestArray12bytes[12];
uint8_t deviceTestArray256bytes[256];

uint8_t TPDO1_data[8];
uint8_t TPDO2_data[8];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void
SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int
main(void)
{
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* Startup delay for CAN initialization */
	HAL_Delay(2000);

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_CAN_Init();
	MX_RTC_Init();
	/* USER CODE BEGIN 2 */

	/* Clock */
	set_time(946730096);   // Sat, 01 Aug 2020 13:37:00 GMT

	/* interface */
	*pUserLed = signal_init(LED_GPIO_Port, LED_Pin, SignalActiveLevel_HIGH, 1);
	*pUserButton = button_init(BUTTON_GPIO_Port, BUTTON_Pin, ButtonActiveLevel_LOW, 1);

	/* Enable UART receive */
	UART2CAN_UART_Receive(&huart2, pUartRxBuf);

	/* uCANopen init */
	uco_init(&uCO, uCO_OD);

	//FIXME!
	uCO.NodeId = 100;
	uCO.NodeState = NODE_STATE_OPERATIONAL;
	uCO.NMT.HeartbeatTime = 1000;

	for (int i = 0; i < sizeof(deviceTestArray12bytes); i++)
	{
		deviceTestArray12bytes[i] = i * 2;
	}

	for (int i = 0; i < sizeof(deviceTestArray256bytes); i++)
	{
		deviceTestArray256bytes[i] = i;
	}

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{

		lawicel_proceed(pUartRxBuf, pUartTxBuf);

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		uco_run(&uCO);

	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void
SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
	RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
		| RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2 | RCC_PERIPHCLK_RTC;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

void
button_released_callback(Button_t *p)
{
	if (p->PrevState == ButtonState_Pressed)
	{
		signal_blink(pUserLed, 100, 2,
						signal_get_output(pUserLed) ?
							SIGNAL_INVERTED : SIGNAL_NORMAL);

		uco_tpdo_transmit(&uCO, 1);
		uco_tpdo_transmit(&uCO, 2);
	}
}

void
button_pressed_long_callback(Button_t *p)
{
	signal_blink(pUserLed, 150, 3,
					signal_get_output(pUserLed) ?
						SIGNAL_INVERTED : SIGNAL_NORMAL);
}

uCO_ErrorStatus_t
uco_tpdo_prepare_data(uCO_t *p, int num)
{
	uCO_ErrorStatus_t result = UCANOPEN_ERROR;
	uint32_t unixtime = now();

	if (num == 1) {
		TPDO1_data[0] = (unixtime) & 0xFF;
		TPDO1_data[1] = (unixtime >> 8) & 0xFF;
		TPDO1_data[2] = (unixtime >> 16) & 0xFF;
		TPDO1_data[3] = (unixtime >> 24) & 0xFF;

		memcpy(&TPDO1_data[4], deviceTestArray12bytes, 4);

		p->TPDO[0].data.address = TPDO1_data;
		p->TPDO[0].data.size = sizeof(TPDO1_data);

		result = UCANOPEN_SUCCESS;
	}
	if (num == 2) {
		memcpy(TPDO2_data, &deviceTestArray12bytes[4], 8);

		p->TPDO[1].data.address = TPDO2_data;
		p->TPDO[1].data.size = sizeof(TPDO2_data);

		result = UCANOPEN_SUCCESS;
	}
	return result;
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void
Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	while (1)
	{
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		HAL_Delay(150);
	}

	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void
assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
