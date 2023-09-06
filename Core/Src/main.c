/* USER CODE BEGIN Header */
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "./BSP/OLED/oled.h"
#include "./BSP/SGP30/sgp30.h"
#include "./SYSTEM/delay/delay.h"
#include "math.h"
#include "stdbool.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void setO2(bool value)
{
    HAL_GPIO_WritePin(OUT8_GPIO_Port, OUT8_Pin, value);
}

void setAir(bool value)
{
    HAL_GPIO_WritePin(OUT7_GPIO_Port, OUT7_Pin, value);
}

uint32_t getCO2()
{
    SGP30_Write(0x20, 0x08);
    uint32_t sgp30_dat = SGP30_Read(); // ¶ÁÈ¡SGP30µÄÖµ
    uint32_t CO2Data = (sgp30_dat & 0xffff0000) >> 16; // È¡³öCO2Å¨¶ÈÖµ
    uint32_t TVOCData = sgp30_dat & 0x0000ffff; // È¡³öTVOCÖµ
    return CO2Data;
}

void sgp30Check()
{
    uint32_t sgp30_dat, CO2Data, TVOCData;
    do {
        delay_ms(500);
        SGP30_Write(0x20, 0x08);
        sgp30_dat = SGP30_Read(); // ¶ÁÈ¡SGP30µÄÖµ
        CO2Data = (sgp30_dat & 0xffff0000) >> 16; // È¡³öCO2Å¨¶ÈÖµ
        TVOCData = sgp30_dat & 0x0000ffff; // È¡³öTVOCÖµ
    } while (CO2Data == 400 && TVOCData == 0);
}

void oledSetBkg()
{
    oled_clear();
    oled_show_string(0, 0, "SILU-Tech", 16);
    oled_show_string(0, 16, "CO2:", 16);
    oled_show_string(104, 16, "ppm", 16);
    oled_show_string(0, 16 * 2, "CO:", 16);
    oled_show_string(104, 32, "ppm", 16);
    oled_show_string(0, 16 * 3, "O2:", 16);
    oled_show_string(104 - 8, 48, "%Vol", 16);
    oled_refresh_gram();
}

void bspCheck()
{
    while (1) {
        uint16_t ms = 500;
        HAL_GPIO_WritePin(OUT8_GPIO_Port, OUT8_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_SET);
        HAL_Delay(ms);
        HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_SET);
        HAL_Delay(ms);
        HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT3_GPIO_Port, OUT3_Pin, GPIO_PIN_SET);
        HAL_Delay(ms);
        HAL_GPIO_WritePin(OUT3_GPIO_Port, OUT3_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT4_GPIO_Port, OUT4_Pin, GPIO_PIN_SET);
        HAL_Delay(ms);
        HAL_GPIO_WritePin(OUT4_GPIO_Port, OUT4_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT5_GPIO_Port, OUT5_Pin, GPIO_PIN_SET);
        HAL_Delay(ms);
        HAL_GPIO_WritePin(OUT5_GPIO_Port, OUT5_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT6_GPIO_Port, OUT6_Pin, GPIO_PIN_SET);
        HAL_Delay(ms);
        HAL_GPIO_WritePin(OUT6_GPIO_Port, OUT6_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT7_GPIO_Port, OUT7_Pin, GPIO_PIN_SET);
        HAL_Delay(ms);
        HAL_GPIO_WritePin(OUT7_GPIO_Port, OUT7_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT8_GPIO_Port, OUT8_Pin, GPIO_PIN_SET);
        HAL_Delay(ms);
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    }
}

static uint32_t getSoftCO2Value()
{
    const uint8_t times = 5;
    uint32_t buf[times];

    for (size_t i = 0; i < 5; i++) {
        buf[i] = getCO2();
    }
    for (size_t i = 0; i < times - 1; i++) {
        for (size_t j = i + 1; j < times; j++) {
            if (buf[i] > buf[j]) {
                uint32_t temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }
    uint32_t sum = 0;
    for (size_t i = 1; i < times - 1; i++) {
        sum += buf[i];
    }
    return sum / (times - 2);
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */
    const bool kDebugMode = false;
    const uint32_t o2ValueRef = 1201;
    const float coVolRef = 1.17;

    const float R0 = (3.3f - coVolRef) / coVolRef * 10.0f / pow(1.0f / 98.322f, 1.0f / -1.458f);

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
    MX_DMA_Init();
    MX_ADC1_Init();
    /* USER CODE BEGIN 2 */
    uint32_t ADC_Value[100], coValue, o2Value, CO2Data;
    uint8_t cnt = 0;
    bool isAO_03Fail = false;

    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&ADC_Value, 100);

    delay_init(72);
    HAL_Delay(1000);
    oled_init();
    oled_show_string(0, 0, "Loading ...", 16);
    oled_refresh_gram();

    // bspCheck();

    SGP30_Init();
    sgp30Check();

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (true) {
        if (cnt == 0) {
            cnt = 20;
            oledSetBkg();
        }
        cnt--;

        CO2Data = getSoftCO2Value();
        oled_show_num(64, 16, CO2Data, 5, 16);
        if (CO2Data <= 400) {
            oled_show_char(64 + 8, 16, '<', 16, 1);
        }

        coValue = 0;
        o2Value = 0;
        for (size_t i = 0; i < 100; i += 2) {
            coValue += ADC_Value[i];
            o2Value += ADC_Value[i + 1];
        }
        coValue /= 50;
        o2Value /= 50;

        // CO Calc
        float coVol = (float)(coValue)*3.3 / 4096;
        float RS = (3.3f - coVol) / coVol * 10;
        float coPpm = 98.322f * pow(RS / R0, -1.458f);
        if (coPpm < 1) {
            oled_show_string(64 + 8 * 3, 32, "<1", 16);
        } else {
            oled_show_num(64, 32, (uint32_t)(coPpm), 5, 16);
        }

        // O2 Calc
        float o2Vol = (float)(o2Value) / o2ValueRef * 20.9;
        oled_show_num(64, 48, o2Vol, 2, 16);
        oled_show_char(104 - 24, 48, '.', 16, 1);
        oled_show_num(104 - 16, 48, (uint32_t)(o2Vol * 10) % 10, 1, 16);

        bool co2UseAir, coUseAir;
        if (CO2Data >= 1500) {
            HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, 0);
            HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, 1);
            co2UseAir = true;
        } else if (CO2Data >= 1000) {
            HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, 1);
            HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, 0);
            co2UseAir = false;
        } else {
            HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, 0);
            HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, 0);
            co2UseAir = false;
        }
        if (coPpm >= 20) {
            HAL_GPIO_WritePin(OUT3_GPIO_Port, OUT3_Pin, 0);
            HAL_GPIO_WritePin(OUT4_GPIO_Port, OUT4_Pin, 1);
            coUseAir = true;
        } else if (coPpm >= 10) {
            HAL_GPIO_WritePin(OUT3_GPIO_Port, OUT3_Pin, 1);
            HAL_GPIO_WritePin(OUT4_GPIO_Port, OUT4_Pin, 0);
            coUseAir = false;
        } else {
            HAL_GPIO_WritePin(OUT3_GPIO_Port, OUT3_Pin, 0);
            HAL_GPIO_WritePin(OUT4_GPIO_Port, OUT4_Pin, 0);
            coUseAir = false;
        }
        setAir(co2UseAir || coUseAir);

        if (isAO_03Fail) {
            oled_show_string(64, 48, "*20.9", 16);
            HAL_GPIO_WritePin(OUT5_GPIO_Port, OUT5_Pin, 0);
            HAL_GPIO_WritePin(OUT6_GPIO_Port, OUT6_Pin, 0);
            setO2(false);
        } else {
            if (o2Vol < 2.0) {
                isAO_03Fail = true;
            } else if (o2Vol < 17.0) {
                HAL_GPIO_WritePin(OUT5_GPIO_Port, OUT5_Pin, 0);
                HAL_GPIO_WritePin(OUT6_GPIO_Port, OUT6_Pin, 1);
                setO2(true);
            } else if (o2Vol < 19.0) {
                HAL_GPIO_WritePin(OUT5_GPIO_Port, OUT5_Pin, 1);
                HAL_GPIO_WritePin(OUT6_GPIO_Port, OUT6_Pin, 0);
                setO2(false);
            } else {
                HAL_GPIO_WritePin(OUT5_GPIO_Port, OUT5_Pin, 0);
                HAL_GPIO_WritePin(OUT6_GPIO_Port, OUT6_Pin, 0);
                setO2(false);
            }
        }

        if (kDebugMode) {
            oled_show_num(0, 32, coVol * 100, 5, 16);
            oled_show_num(0, 48, o2Value, 5, 16);
        }

        oled_refresh_gram();
        HAL_Delay(500);
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
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
        | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
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
    while (1) {
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
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line
       number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
       line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
