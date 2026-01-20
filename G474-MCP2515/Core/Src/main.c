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
#include "hrtim.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mcp2515.h"
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
uint8_t babbling_idiot_mode = 0; // 0 = Normal, 1 = Babbling Idiot
uint32_t last_ping_tick = 0;
uint8_t can_received_flag=0;


// Variáveis para latência em microssegundos
float latency_us = 0;
float last_latency = 0;
float jitter_us = 0;

// Constante de conversão baseada no seu clock (96MHz conforme imagem)
// Periodo = 1 / 96.000.000 segundos
#define TICKS_PER_US 96.0f

uint32_t t_start = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_HRTIM1_Init();
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
      // Habilita interrupção de Transmissão (TX0IE) e Recebimento (RX0IE)
      // O valor 0x05 habilita MERRE (erro), TX0IE (transmissão) e RX0IE (recepção)
	  setRegister(&mcp2515, MCP_CANINTE, 0x05);

      MCP_setMode(&mcp2515, MODE_NORMAL);

  }

  HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_A);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // 1. Envio do Ping (G474 -> G030)
	      if (!babbling_idiot_mode && (HAL_GetTick() - last_ping_tick > 1000)) {
	          TxFrame.can_id = 0x123;
	          TxFrame.can_dlc = 1;
	          TxFrame.data[0] = 0xAA;

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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 13;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == MCP2515_INT_Pin)
    {
        uint8_t irq_status = readRegister(&mcp2515, MCP_CANINTF);

        // 1. Tratamento de Recebimento (Prioridade)
        if (irq_status & 0x03) // RX0IF ou RX1IF
        {
            can_received_flag = 1;
            // IMPORTANTE: Se o loop principal demorar, o pino INT fica preso aqui.
            // Para garantir que o pino libere, desabilite RXIE temporariamente
            // ou limpe aqui se a biblioteca permitir.
        }

        // 2. Tratamento de Transmissão
        if (irq_status & 0x04) // TX0IF
        {
            modifyRegister(&mcp2515, MCP_CANINTF, 0x04, 0x00);
            if (babbling_idiot_mode) {
                TxFrame.can_id = 0x000;
                TxFrame.can_dlc = 8;
                MCP_sendMessageTo(&mcp2515, 0, &TxFrame);
            }
        }

        // 3. Limpeza de ERROS (Importante!)
        // Se houver um erro de barramento e você não limpar o flag de erro,
        // o pino INT nunca subirá novamente.
        if (irq_status & 0xA0) // MERRF ou ERRIF
        {
            modifyRegister(&mcp2515, MCP_CANINTF, 0xA0, 0x00);
        }
    }
}

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
