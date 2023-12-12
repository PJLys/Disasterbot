/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include <dash7.h>
#include "main.h"
#include <math.h>

#include "SHT40.h" // humidity & temperature sensor
#include "LTR329.h" // light sensor

uint8_t Rx_data[10];

I2C_HandleTypeDef hi2c1; // Handles i2c communication with SHT40
I2C_HandleTypeDef hi2c3; // Handles i2c communication with LTR329

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

float radiation;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);


/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void SHT40_Init()
{
	HAL_I2C_Init(&hi2c1); // Initialize and configure the I2C peripheral
}
void SHT40_Read(float *t, float *rh, uint8_t mode)
{
	uint8_t data[2] = {mode, 0x00};
	uint8_t buffer[6];

    HAL_I2C_Master_Transmit(&hi2c1, SHT40_I2C_ADDRESS, data, 2, HAL_MAX_DELAY);
    HAL_Delay(10);
    HAL_I2C_Master_Receive(&hi2c1, SHT40_I2C_ADDRESS, buffer, 6, HAL_MAX_DELAY);

    uint16_t t_ticks = buffer[0]*256 + buffer[1];
    uint16_t checksum_t = buffer[2];
    uint16_t rh_ticks = buffer[3]*256 + buffer[4];
    uint16_t checksum_rh = buffer[5];

    float t_degC = -45 + 175.0 * t_ticks/65535;
    float rh_pRH = -6 + 125.0 * rh_ticks/65535;

    if (rh_pRH > 100){
    	rh_pRH = 100; }
    if (rh_pRH < 0){
    	rh_pRH = 0; }

    *t = t_degC;
    *rh = rh_pRH;
}

void LTR329_Init()
{
    uint8_t data;

    // Activate the sensor
    data = LTR329_ACTIVE_MODE;
    HAL_I2C_Mem_Write(&hi2c3, LTR329_I2C_ADDRESS, LTR329_ALS_CONTR, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);

    // Set measurement rate
    data = LTR329_MEAS_RATE;
    HAL_I2C_Mem_Write(&hi2c3, LTR329_I2C_ADDRESS, LTR329_ALS_MEAS_RATE, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}
void LTR329_Read(uint8_t *data)
{
    //uint8_t data[4];
    // Read 4 bytes of data starting from LTR329_ALS_DATA_CH1_0
    HAL_I2C_Mem_Read(&hi2c3, LTR329_I2C_ADDRESS, LTR329_ALS_DATA_CH1_0, I2C_MEMADD_SIZE_8BIT, data, 4, 100);

    // Combine bytes to get the light data for each channel
    //*ch1 = (uint16_t)(data[1] << 8) | data[0];
    //*ch0 = (uint16_t)(data[3] << 8) | data[2];

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	  HAL_UART_Receive_IT(&huart2, Rx_data,3);
	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, 0);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
    /* USER CODE BEGIN 1 */
    float t, rh;
    uint8_t data[4];
    /* USER CODE END 1 */

    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_I2C3_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    /* USER CODE BEGIN 2 */
    SHT40_Init();
    LTR329_Init();
    HAL_UART_Receive_IT(&huart2, Rx_data, 3);

    uart_rx_buffer_clear();

    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);

    static uint8_t txdata[] = {0x14, 0x87, 0x33} ;

    /* USER CODE END 2 */
    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
    	/* USER CODE END WHILE */
	  enum msg_type_e msg_type = uart_rx_buffer[1];

	  if (msg_type == REQUEST_DATA) {
		  //HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
		  SHT40_Read(&t, &rh, SHT40_MEAS_HIGH_PRECISION);
		  uint8_t temperature[4];
		  message Tempm = {.msg_start_char = MSG_START_CHARACTER, .msg_end_char = MSG_END_CHARACTER,
				  .msg_data.temperature = 30, .msg_type = RESPONSE_TEMPERATURE};
		  int TempBytes = create_payload(Tempm, temperature);
		  HAL_UART_Transmit(&huart2, temperature, TempBytes, 10);


		  SHT40_Read(&t, &rh, SHT40_MEAS_HIGH_PRECISION);
		  uint8_t humidity[4];
		  message Humm = {.msg_start_char = MSG_START_CHARACTER, .msg_end_char = MSG_END_CHARACTER,
				  .msg_data.humidity = rh, .msg_type = RESPONSE_HUMIDITY};
		  int HumBytes = create_payload(Humm, humidity);
		  HAL_UART_Transmit(&huart2, humidity, HumBytes, 10);


		  LTR329_Read(data);
		  uint8_t light[4];
		  message Lightm = {.msg_start_char = MSG_START_CHARACTER, .msg_end_char = MSG_END_CHARACTER,
				  .msg_data.light = data, .msg_type = RESPONSE_LIGHT};
		  int LightBytes = create_payload(Lightm, light);
		  HAL_UART_Transmit(&huart2, light, LightBytes, 10);


		  SHT40_Read(&t, &rh, SHT40_MEAS_HIGH_PRECISION);
		  float radiate = t;
		  uint8_t radiation[4];
		  message Radm = {.msg_start_char = MSG_START_CHARACTER, .msg_end_char = MSG_END_CHARACTER,
				  .msg_data.radiation = radiate, .msg_type = RESPONSE_RADIATION};
		  int RadBytes = create_payload(Radm, radiation);
		  HAL_UART_Transmit(&huart2, radiation, RadBytes, 10);


		  SHT40_Read(&t, &rh, SHT40_MEAS_HIGH_PRECISION);
		  float dustsens = rh;
		  uint8_t dust[4];
		  message Dustm = {.msg_start_char = MSG_START_CHARACTER, .msg_end_char = MSG_END_CHARACTER,
				  .msg_data.radiation = dustsens, .msg_type = RESPONSE_DUST};
		  int DustBytes = create_payload(Dustm, dust);
		  HAL_UART_Transmit(&huart2, dust, DustBytes, 10);

		  uart_rx_buffer_clear();
	  	  } else {
	  		HAL_UART_Transmit_IT(&huart2, txdata, 3);
	  	  }

	  HAL_Delay(10);
	  /* START SLEEP until huart2 receives data*/
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00000E14;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x00000E14;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
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
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure the GPIO pin for the LED */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = LD3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LD3_GPIO_Port, &GPIO_InitStruct);

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

#ifdef  USE_FULL_ASSERT
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