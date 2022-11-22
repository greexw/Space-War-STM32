/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdlib.h" // to import srand function
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// coordinates on the game board
typedef struct {
	uint8_t x;
	uint8_t y;
}  Coordinates;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN PV */
uint8_t player_width = 15;
uint8_t player_height = 30;
uint8_t player_starting_position = 90;
uint8_t player_step = 4;
uint8_t player_x_pos;
// 0: game is running, 1: game lost, 2: game won
uint8_t game_status = 0;
uint8_t second_passed = 0;
uint8_t score = 0;
uint8_t lives = 3;
uint8_t seconds = 0, minutes = 0, hours = 0;

Coordinates Shots[40];
Coordinates Airplanes[40];
// 0 - cant, 2 - can
uint8_t Player_Can_Shoot = 0;
uint8_t Seconds_To_Next_Airplane = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_RTC_Init(void);
//void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
void Make_The_Shot(void);
void Update_Shots_Position(void);
void Draw_Airplane(void);
void Update_Airplanes_Position(void);

void Draw_Player_Start_Position(void);
void GameSetup(void);
void Update_Time(void);
void Update_Score(void);
void Update_Lives(void);
void Move_Left(void);
void Move_Right(void);
int8_t ADC1_Init(void);
uint32_t Get_Seed_Value(void);
void Game_Over(void);
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
  JOYState_TypeDef JoyState = JOY_NONE;
  // uint8_t KeyState;
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
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  /* Initialize the LEDs */
  // BSP_LED_Init(LED_GREEN);
  // BSP_LED_Init(LED_ORANGE);
  BSP_LED_Init(LED_RED);
  // BSP_LED_Init(LED_BLUE);
  
  /* Configure the Key push-button in GPIO Mode */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

  /*## Initialize the LCD #################################################*/
  if (BSP_LCD_Init() != LCD_OK)
  {
	  Error_Handler();
  }

  BSP_LCD_SetFont(&Font12);
  BSP_LCD_Clear(LCD_COLOR_WHITE);
  //joystick init
  if (BSP_JOY_Init(JOY_MODE_GPIO) != IO_OK)
  {
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 95, (uint8_t *)"ERROR", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 80, (uint8_t *)"Joystick cannot be initialized", CENTER_MODE);
    Error_Handler();
  }
   //ADC-1 init
  if (ADC1_Init() != HAL_OK)
  {
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 95, (uint8_t *)"ERROR", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 80, (uint8_t *)"ADC-1 cannot be initialized", CENTER_MODE);
    Error_Handler();
  }
  // get the seed value to draw mines positions
  uint32_t seed = Get_Seed_Value();
  srand(seed);
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  GameSetup();
	  while (game_status == 0)
	  {
		    if (second_passed == 1)
		    {
		    	BSP_LCD_SetFont(&Font12);
		    	Update_Time();
		    	second_passed = 0;

		    	Draw_Airplane();
		    	Update_Shots_Position();
		    	Update_Airplanes_Position();
		    }

		    JoyState = BSP_JOY_GetState();

		    switch(JoyState)
		    {
		    case JOY_LEFT:
			  Move_Left();
		      HAL_Delay(60);
		      break;
		    case JOY_RIGHT:
		      Move_Right();
		      HAL_Delay(60);
		      break;
		      break;
		    case JOY_SEL:
		      Make_The_Shot();
		      HAL_Delay(60);
			  break;
		    default:
		      break;
		    }
	  }
	  Game_Over();
    /* USER CODE END WHILE */
    //MX_USB_HOST_Process();

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV5;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_PLL2;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL2_ON;
  RCC_OscInitStruct.PLL2.PLL2MUL = RCC_PLL2_MUL8;
  RCC_OscInitStruct.PLL2.HSEPrediv2Value = RCC_HSE_PREDIV2_DIV5;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC
                              |RCC_PERIPHCLK_USB;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV3;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
  /** Configure the Systick interrupt time
  */
  __HAL_RCC_PLLI2S_ENABLE();
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/* USER CODE BEGIN 4 */
int8_t ADC1_Init(void)
{
  uint8_t ret = HAL_OK;
  ADC_HandleTypeDef hadc1 = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /** Common configuration */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  ret = HAL_ADC_Init(&hadc1);
  if (ret != HAL_OK)
  {
    return ret;
  }

  /** Configure regular channel group */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  ret = HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  return ret;
}

uint32_t Get_Seed_Value(void)
{
	uint32_t ret = 0;

	ADC1->CR2 |= 0x00000001;
	// Wait for the ADC to stabilize:
	HAL_Delay(500);
	// Start conversion:
	ADC1->CR2 |= 0x00000001;
	// Wait for end of the conversion:
	while ((ADC1->SR & 0x00000002) == 0);
	ret = (uint32_t)ADC1->DR;
	// Clear the STRT bit:
	ADC1->SR &=~0x00000010;

	return ret;
}

void GameSetup(void)
{
	BSP_LCD_Clear(LCD_COLOR_LIGHTGRAY);
	BSP_LCD_DrawVLine(200, 240, 241);
	BSP_LCD_DrawVLine(201, 240, 241);
    Update_Score();
    Update_Lives();
    Draw_Player_Start_Position();
}

void Update_Time(void)
{
	uint8_t Game_Time[6] = "00:00";
	seconds++;
	if (seconds > 59)
	{
		seconds = 0;
		minutes ++;
		if (minutes > 59)
		{
			minutes = 0;
			hours ++;
			if (hours > 23)
			{
				hours = 0;
			}
		}
	}
	Game_Time[4] = seconds%10 +48;
	Game_Time[3] = seconds/10 +48;
	Game_Time[1] = minutes%10 +48;
	Game_Time[0] = minutes/10 +48;
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DisplayStringAt(70, 20, (uint8_t *)"Time:", CENTER_MODE);
	BSP_LCD_DisplayStringAt(105, 20, (uint8_t *) Game_Time, CENTER_MODE);

	if (Player_Can_Shoot < 2)
	{
		Player_Can_Shoot ++;
	}

	Seconds_To_Next_Airplane ++;
}

void Update_Score(void)
{
	uint8_t Result[] = "Score: 00";
	uint8_t i = 0;
	while(score != 0)
	{
		Result[8-i] = score%10+48;
		score = score/10;
		i++;
	}
	BSP_LCD_DisplayStringAt(85, 35, (uint8_t *)Result, CENTER_MODE);
}

void Update_Lives(void)
{
	uint8_t LivesResult[] = "Lives: 00";
	uint8_t i = 0;
	while(lives != 0)
	{
		LivesResult[8-i] = lives%10+48;
		lives = lives/10;
		i++;
	}
	BSP_LCD_DisplayStringAt(85, 50, (uint8_t *)LivesResult, CENTER_MODE);
}

void Move_Left()
{
	if (player_x_pos - player_step > 0)
	{
		// czyścimy poprzednią pozycję
	BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
	BSP_LCD_DrawHLine(player_x_pos, 210, 15);
	BSP_LCD_DrawHLine(player_x_pos, 240, 15);
	BSP_LCD_DrawVLine(player_x_pos, 210, 30);
	BSP_LCD_DrawVLine(player_x_pos+15, 210, 30);
		// rysujemy nową
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DrawHLine(player_x_pos-player_step, 210, 15);
	BSP_LCD_DrawHLine(player_x_pos-player_step, 240, 15);
	BSP_LCD_DrawVLine(player_x_pos-player_step, 210, 30);
	BSP_LCD_DrawVLine(player_x_pos+15-player_step, 210, 30);
		// zapisujemy nowa pozycje gracza
	player_x_pos = player_x_pos-player_step;
	}
}

void Move_Right()
{
	if (player_x_pos + player_step <185)
	{
		// czyścimy poprzednią pozycję
	BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
	BSP_LCD_DrawHLine(player_x_pos, 240-player_height, player_width);
	BSP_LCD_DrawHLine(player_x_pos, 240, player_width);
	BSP_LCD_DrawVLine(player_x_pos, 240-player_height, player_height);
	BSP_LCD_DrawVLine(player_x_pos+15, 240-player_height, player_height);
		// rysujemy nową
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DrawHLine(player_x_pos+player_step, 240-player_height, player_width);
	BSP_LCD_DrawHLine(player_x_pos+player_step, 240, player_width);
	BSP_LCD_DrawVLine(player_x_pos+player_step, 240-player_height, player_height);
	BSP_LCD_DrawVLine(player_x_pos+15+player_step, 240-player_height, player_height);
		// zapisujemy nowa pozycje gracza
	player_x_pos = player_x_pos+player_step;
	}
}

void Draw_Airplane(void)
{
	if (Seconds_To_Next_Airplane >= 4)
	{
		Airplanes[seconds%40].x = 10 + (rand() % 180);
		Airplanes[seconds%40].y = 20;
		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		BSP_LCD_FillCircle(Airplanes[seconds%40].x, Airplanes[seconds%40].y, 10);

		Seconds_To_Next_Airplane = 0;
	}
}

void Update_Airplanes_Position(void)
{
	for (uint16_t i = 0; i<40; i++)
		{
			if (Airplanes[i].x != 0 && Airplanes[i].y !=0 )
			{
				if (Airplanes[i].y > 225)
				{
					// USUWAMY SAMOLOCIK - ZBLIZYL SIE DO DOlNEJ KRAWEDZI
					BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
					BSP_LCD_FillCircle(Airplanes[i].x, Airplanes[i].y, 10);
					Airplanes[i].x = 0;
					Airplanes[i].y = 0;
				}
				else
				{
					// CZYSCIMY POPRZEDNIA POZYCJE
					BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
					BSP_LCD_FillCircle(Airplanes[i].x, Airplanes[i].y, 10);
					// PRZESUWAMY POCISK DO GORY
					Airplanes[i].y += 15;
					BSP_LCD_SetTextColor(LCD_COLOR_RED);
					BSP_LCD_FillCircle(Airplanes[i].x, Airplanes[i].y, 10);
				}
			}
		}
}

void Make_The_Shot(void)
{
	if (Player_Can_Shoot >= 2)
	{
		Player_Can_Shoot = 0;

		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);

		Shots[seconds%40].x = player_x_pos+(player_width/2);
		Shots[seconds%40].y = 240-player_height-15;
		BSP_LCD_FillCircle(Shots[seconds%40].x, Shots[seconds%40].y, 5);
	}
}

void Update_Shots_Position(void)
{
	for (uint16_t i = 0; i<40; i++)
	{
		if (Shots[i].x != 0 && Shots[i].y !=0 )
		{
			if (Shots[i].y < 15)
			{
				// USUWAMY POCISK - ZBLIZYL SIE DO GORNEJ KRAWEDZI
				BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
				BSP_LCD_FillCircle(Shots[i].x, Shots[i].y, 5);
				Shots[i].x = 0;
				Shots[i].y = 0;
			}
			else
			{
				// CZYSCIMY POPRZEDNIA POZYCJE
				BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
				BSP_LCD_FillCircle(Shots[i].x, Shots[i].y, 5);
				// PRZESUWAMY POCISK DO GORY
				Shots[i].y -= 10;
				BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
				BSP_LCD_FillCircle(Shots[i].x, Shots[i].y, 5);
			}
		}
	}
}

void Draw_Player_Start_Position(void)
{
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DrawHLine(player_starting_position, 240-player_height, player_width);
	BSP_LCD_DrawHLine(player_starting_position, 240, player_width);
	BSP_LCD_DrawVLine(player_starting_position, 240-player_height, player_height);
	BSP_LCD_DrawVLine(player_starting_position+player_width, 240-player_height, player_height);
	player_x_pos = player_starting_position;
}

void Game_Over(void)
{
	uint16_t score = 0;
	uint8_t Result[] = "You score 000 pts.";
	uint8_t i = 0;
	BSP_LCD_Clear(LCD_COLOR_LIGHTGRAY);
	// Game lost
	if (game_status == 1)
	{
	    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 95, (uint8_t *)"Game lost!", CENTER_MODE);
	}
	// Game won
	if (game_status == 2)
	{
		while(score != 0)
		{
			Result[12-i] = score%10+48;
			score = score/10;
			i++;
		}
	    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 80, (uint8_t *)"Game won! :)", CENTER_MODE);
	    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 95, (uint8_t *)Result, CENTER_MODE);

	}
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 80, (uint8_t *)"Press \"Reset\" to play again.", CENTER_MODE);

	while(1);
}

/**
  * @brief Toggle Leds
  * @param  None
  * @retval None
  */
void Toggle_Leds(void)
{
  static uint8_t ticks = 0;
  
  if(ticks++ > 100)
  {
    // BSP_LED_Toggle(LED_BLUE);
    ticks = 0;
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
  BSP_LED_On(LED_RED);
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

