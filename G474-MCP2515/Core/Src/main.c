/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mcp2515.h"

#include "usbd_cdc_if.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

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
mcp2515_t mcp2515;
can_frame TxFrame;
can_frame RxFrame;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Em USER CODE BEGIN 4 */
void USB_Log(const char *fmt, ...) {
    static char buf[512]; // Buffer estático para economizar pilha
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len > 0) {
        uint8_t result;
        uint32_t timeout = 100; // Timeout de segurança para não travar o MCU

        do {
            result = CDC_Transmit_FS((uint8_t*)buf, len);
            timeout--;
        } while (result == USBD_BUSY && timeout > 0);
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
<<<<<<< Updated upstream
=======
  MX_HRTIM1_Init();
  MX_USB_Device_Init();
>>>>>>> Stashed changes
  /* USER CODE BEGIN 2 */
  // 1. Vincular o handle da SPI (verifique se é &hspi1 ou &hspi2 no seu CubeMX)
  mcp2515.spi_handle = &hspi1;

  // 2. Configurar os pinos (Ajuste para os nomes corretos da sua struct)
  mcp2515.cs_pin.gpio_port = MCP2515_CS_GPIO_Port;
  mcp2515.cs_pin.gpio_pin = MCP2515_CS_Pin;

  // 3. Configurar o Clock (O membro se chama 'clock', não 'osc')
  mcp2515.clock = MCP_8MHZ;

  // Agora o Setup inicial
  if (MCP_reset(&mcp2515) == ERROR_OK) {
	  MCP_setBitrate(&mcp2515, CAN_500KBPS);
	  MCP_setMode(&mcp2515, MODE_NORMAL);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // Exemplo: Enviar a cada 1 segundo
	  TxFrame.can_id = 0x123;
	  TxFrame.can_dlc = 8;
	  for(int i=0; i<8; i++) TxFrame.data[i] = i;

<<<<<<< Updated upstream
	  MCP_sendMessage(&mcp2515, &TxFrame);
=======
	          // CAPTURA O TEMPO DE INÍCIO (Ticks do HRTIM)
	          t_start = HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CNTxR;

	          if(MCP_sendMessage(&mcp2515, &TxFrame) == ERROR_OK) {
	              last_ping_tick = HAL_GetTick();
	          }
	      }

	      // 2. Recepção do Pong (G030 -> G474)
	      if (can_received_flag) {
	          can_received_flag = 0;
	          if (MCP_readMessage(&mcp2515, &RxFrame) == ERROR_OK) {
	              if (RxFrame.can_id == 0x124) {
	                  // CAPTURA O TEMPO DE CHEGADA
	                  uint32_t t_end = HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CNTxR;

	                  // Trata o overflow do timer (16-bit)
	                  uint32_t diff = (t_end >= t_start) ? (t_end - t_start) : (0xFFFF - t_start + t_end);

	                  // CALCULA LATÊNCIA E JITTER
	                  latency_us = (float)diff / TICKS_PER_US;

	                  if (last_latency > 0) {
	                      float current_jitter = latency_us - last_latency;
	                      jitter_us = (current_jitter < 0) ? -current_jitter : current_jitter;
	                  }

	                  last_latency = latency_us;
	                  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
	              }
	          }
	      }
	      uint32_t tick = HAL_GetTick();
		  USB_Log("[DEBUG] Sistema rodando. Tick: %lu\r\n", tick);
		  HAL_Delay(1000);
>>>>>>> Stashed changes

	  // Exemplo: Ler se chegou algo
	  if (MCP_readMessage(&mcp2515, &RxFrame) == ERROR_OK) {
		  // Processar RxFrame.data
	  }

	  HAL_Delay(1000);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
<<<<<<< Updated upstream
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
=======
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 18;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV6;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
>>>>>>> Stashed changes
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

<<<<<<< Updated upstream
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
=======
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
>>>>>>> Stashed changes
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
