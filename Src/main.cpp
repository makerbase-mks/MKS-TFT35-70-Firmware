/**
1. FSMC ���⡢Ƭѡ����λ�ܽŶ�Ӧ��
2.SPI FLASH/ AD7843 Ƭѡ��Ӧ��
3.FALA ���ƹܽŶ�Ӧ��
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#define MKSTFT70_FIRMWARE_VER	"V0.1.0"
/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
//#include "Arduino.h"
//#include "Repetier.h"

#include "main.h"
#include "stm32f4xx_hal.h"
//#include "adc.h"
//#include "dac.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
//#include "rtc.h"
#include "sdio.h"
#include "spi.h"
//#include "tim.h"
#include "usart.h"
#include "usb_host.h"
#include "gpio.h"
#include "fsmc.h"
//#include "iwdg.h"
/* USER CODE BEGIN Includes */
#include "mks_test.h"
#include "stm32f4xx_spi.h"
#include "mks_touch_screen.h"
#include "touch_calibrate.h"
#include "ili9320.h"
//#include "printer.h"
#include "mks_cfg.h"
#include "gui.h"
#include "draw_ui.h"
#include "fontLib.h"
#include "draw_pause_ui.h"
#include "draw_ready_print.h"
#include "draw_printing.h"
#include "spi_flash.h"
#include "pic_manager.h"

#include "draw_main.h"
#include "mks_tft_function.h"
#include "mks_tft_gcode.h"
#include "mks_tft_protocol.h"
#include "mks_tft_reprint.h"
#include "MKS_USART2_IT.h"
#include "mks_tft_com.h"
#include "mks_cfg.h"
#include "mks_tft_protocol.h"
#include "draw_print_file.h"
#include "draw_more.h"
#include "misc.h"
#include "sd_usr.h"
#include "at24cxx.cpp"

#include "wifi_module.h"
#include "tim.h"

//extern void LCD_Init(void);

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */ 
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_NVIC_Init(void);
void MX_USB_HOST_Process(void);
extern void wifi_looping();

__IO uint32_t delaycnt1 = 0;

volatile uint8_t repetier_repint_flag = 0;
volatile int16_t logo_time = 0;
volatile uint8_t get_temp_flag;

uint8_t link_mutex_detect_time;
uint8_t  Get_Temperature_Flg = 0;

int32_t printer_state = PRINTER_NOT_CONNECT;
FILE_PRINT_STATE gCurFileState ;
FIL *srcfp;
PR_STATUS printerStaus = pr_idle;
static FIL curFile;

extern char curFileName[150];
extern uint32_t rePrintOffset;
extern unsigned long fileOffset;
extern CFG_ITMES gCfgItems;
extern FATFS fs;

volatile uint32_t TimeIncrease = 0;
extern uint8_t from_flash_pic;

uint32_t wifi_loop_cycle = 500;
extern char wifi_check_time;
uint8_t wifi_init_flg = 0;
uint8_t wifi_init_state = 0;
uint8_t wifi_refresh_flg = 0, cloud_refresh_flg = 0;

extern u16 DeviceCode;

static uint32_t tick1,tick2;

uint8_t preview_no_display;
/************************************************end************************/

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

uint8_t console_msg_print[128];
uint8_t lineNumber = 0;
uint8_t console_flag = 0;
uint8_t update_msg_flag = 0;

#if 1
  void console_show()
  {
		char buf[480] ={0};
		memset((void *)buf,' ',sizeof(buf));
		if(++lineNumber>=14)
			lineNumber = 0;
		GUI_DispStringAt((char const *)buf, TITLE_XPOS, lineNumber*20);
		GUI_DispStringAt((char const *)console_msg_print, TITLE_XPOS, lineNumber*20);
	 	GUI_DispStringAt("Touch To Back",200,300);
  }
  void console_back()
  {
	 clear_cur_ui();
	 disp_state_stack._disp_index += 1;
	 draw_return_ui();	  
  }
  
#endif

  int main(void)
{

	uint32_t i;
    
  /* USER CODE BEGIN 1 */
	uint8_t filePathLen;
  	
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();
  //LED1_int();
  /* Initialize all configured peripherals */
  //�ж���������
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0xC000);         //��bootloaderʱ������Ϊ0xC000, Options ->Linker ->Edit...-> 0x0800C000
                                                           //��bootloaderʱ������Ϊ0x0000, Options ->Linker ->Edit...-> 0x08000000
  MX_GPIO_Init();
  BACKLIGHT = 0;
  MX_DMA_Init();
  //MX_FSMC_Init();
  MX_FATFS_Init();
  //**MX_RTC_Init();
  MX_I2C1_Init(100000);
  MX_SDIO_SD_Init();
  //MX_SPI2_Init();
  //MX_TIM2_Init();
  //MX_TIM3_Init();
  //MX_TIM4_Init();
  //MX_TIM5_Init();
  //MX_ADC1_Init();
  //MX_USART6_UART_Init();
   //HAL::AT24CXX_Write(BAK_TEST_ADDR, (uint8_t *)&TER_R1,  4);
  MX_USB_HOST_Init();
  //MX_DAC_Init();
  MX_SPI1_Init();
  //**MX_IWDG_Init();     //ι��
  /* Initialize interrupts */
  MX_NVIC_Init();
  SPI_Cmd(SPI1, ENABLE); 
  
  /*------------test code begin--------------*/
    
    //LCD_Init();
    //mksEeprom_test();
    //mksW25Q64Test();
    //mksSdCardTest();
    
   /*for(i=0;i<255;i++)
    {
    	aDr[i]=i;
      pBuffer1++;
       HAL::AT24CXX_Write(aDr[i],(uint8_t *)&pBuffer1,1);
    	HAL::AT24CXX_Read(aDr[i], (uint8_t *)&pBuffer2,1);     
    }/*
    pBuffer1=200;
	SPI_FLASH_SectorErase(0x000000);
	SPI_FLASH_BufferWrite((uint8_t *)&pBuffer1, 0x000000, 1);
	SPI_FLASH_BufferRead((uint8_t *)&pBuffer2, 0x000000, 1); */
        //mksUSBTest(); 
   /*------------test code end----------------*/  
  #if defined(TFT70)
    RESET_1963=0;
    RESET_1963=1;
    HAL_Delay(50);
#endif
    init_win_id();
    
    printer_init();              //**   
    
	GUI_Init(); 

	MX_USART3_UART_Init(gCfgItems.baud);
	HAL_Delay(50);
	DRAW_LOGO();
	#if defined(TFT35)
	BACKLIGHT = 1;
	#endif

    //HAL_Delay(3000);
    #if SDSUPPORT
	#if SDPOWER > -1
      SET_OUTPUT(SDPOWER);
      WRITE(SDPOWER, HIGH);
	#endif
	#if defined(SDCARDDETECT) && SDCARDDETECT > -1
      //**SET_INPUT(SDCARDDETECT);
      PULLUP(SDCARDDETECT, HIGH);
	#endif
	#endif
	#if SDSUPPORT
       sd.mount();
	#endif
	
	disp_language_init();	
	GUI_UC_SetEncodeUTF8();

	
	/*add*/
	//chen 11.8
	   if(gCfgItems.overturn_180 != 0xEE)
	   {
	  	gCfgItems.touch_adj_flag = 0;
		gCfgItems.touch_adj_xMin = 95;
		gCfgItems.touch_adj_xMax = 3947;
		gCfgItems.touch_adj_yMin = 3780;
		gCfgItems.touch_adj_yMax = 150;	
	   }
	   else
	   {
		gCfgItems.touch_adj_flag = 0;
		gCfgItems.touch_adj_xMin = 3947;
		gCfgItems.touch_adj_xMax = 95;
		gCfgItems.touch_adj_yMin = 150;
		gCfgItems.touch_adj_yMax = 3780;	
	   }
	

	if(DeviceCode == 0x9488)
	{
		if(gCfgItems.overturn_180 != 0xEE)
		{
			gCfgItems.touch_adj_flag = 0;
			gCfgItems.touch_adj_xMin = 201;
			gCfgItems.touch_adj_xMax = 3919;
			gCfgItems.touch_adj_yMin = 3776;
			gCfgItems.touch_adj_yMax = 164;	
		}
		else
		{
			gCfgItems.touch_adj_flag = 0;
			gCfgItems.touch_adj_xMin = 3919;
			gCfgItems.touch_adj_xMax = 201;
			gCfgItems.touch_adj_yMin = 164;
			gCfgItems.touch_adj_yMax = 3776;			
		}
	}
	

	//Touch_Adjust();
   
      gui_view_init(); 
      //Touch_Adjust();
      setTouchBound(gCfgItems.touch_adj_xMin, gCfgItems.touch_adj_xMax, gCfgItems.touch_adj_yMax, gCfgItems.touch_adj_yMin);
      
        //**HAL_IWDG_Refresh(&hiwdg);
        //SPEAKER = ON; 
        //HAL_Delay(500);
        //SPEAKER = OFF;

	mksUsart2Init();     //**����

	
	FALACTL = FALA_ON;

    /* USER CODE END 2 */
	//MX_USART1_UART_Init(3);
	//__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);	//ʹ�ܿ����ж�	 

  /* Infinite loop */

/**************change*********************/
       #if debug_flg == 1
	T_GPIO_init();
	#endif
	#ifdef SAVE_FROM_SD
	if((gCfgItems.pwroff_save_mode == 0)&&((RE_PRINT_STATUS)gCfgItems.rePrintFlag != printer_pause_reprint))
	//if(gCfgItems.pwroff_save_mode == 0)
	{	
		if(gCfgItems.fileSysType == FILE_SYS_USB)
		{
			tick1 = getTick();
			while(1)
			{
				tick2 = getTick();
				if(getTickDiff(tick2, tick1)>=3000)
				{
					break;
				}
				MX_USB_HOST_Process();
			}			
		}
		sd_data_recover();
	}
	#endif
	 if(((RE_PRINT_STATUS)gCfgItems.rePrintFlag == printer_pause_reprint) || ((RE_PRINT_STATUS)gCfgItems.rePrintFlag == printer_pwdwn_reprint))
	{
		repetier_repint_flag = 1;
		//I2C_EE_Init(100000);
		MX_I2C1_Init(100000);
		SPEAKER = 1; 
		delaycnt1 = 0; while(delaycnt1 < 10000)delaycnt1++;
		SPEAKER = 0; 
		delaycnt1 = 0; while(delaycnt1 < 10000)delaycnt1++;
		SPEAKER = 1; 
		delaycnt1 = 0; while(delaycnt1 < 10000)delaycnt1++;	
		HAL::AT24CXX_Read(BAK_FILE_PATH_LEN_ADDR, (uint8_t *)&filePathLen,  1);
		
		if(filePathLen > 0)
		{
			
			int8_t res;

			get_temp_flag=1;
			Get_Temperature_Flg=1;
			HAL::AT24CXX_Read(BAK_FILE_PATH_ADDR, (uint8_t *)curFileName, filePathLen);     //**
				
			HAL::AT24CXX_Read(BAK_PREVIEW_FROM_FLASH_ADDR, &from_flash_pic,1);

			if(from_flash_pic != 0)
			{
				flash_preview_begin = 1;
			}
			else
			{
				default_preview_flg = 1;
			}
			
			#ifdef SAVE_FROM_SD
			if((RE_PRINT_STATUS)gCfgItems.rePrintFlag == printer_pause_reprint||(gCfgItems.pwroff_save_mode != 0))
			#endif
			{
				HAL::AT24CXX_Read(BAK_REPRINT_OFFSET, (uint8_t *)&rePrintOffset,  4);
				rePrintdata_show();
			}

			memset(sd.gCurDir, 0, sizeof(sd.gCurDir));     //**
			
			//if(gCfgItems.fileSysType == FILE_SYS_SD)
			//if (SD_DET_IP == SDCARDDETECTINVERTED)
			if(gCfgItems.fileSysType == FILE_SYS_SD)
			{
				strcpy(sd.gCurDir, "1:");
				curFileName[0]=(char )'1';
                MX_SDIO_SD_Init();
				if(SD_DET_IP == SDCARDDETECTINVERTED)
				{
              		
            		FATFS_LinkDriver_sd(&SD_Driver, SD_Path);
					f_mount(&fs, (TCHAR const*)SD_Path, 0);	
				}				
				sd.ShowSDFiles();
			}
			else
			{	
				FATFS_LinkDriver_usb(&USBH_Driver, USBH_Path);
		        f_mount(&fs, (TCHAR const*)USBH_Path, 0);			
				strcpy(sd.gCurDir, "0:");	
				curFileName[0]=(char )'0';
			}
				res = f_open(&curFile, curFileName, FA_OPEN_EXISTING | FA_READ);
				if(res == FR_OK)
				{
					srcfp = &curFile;
					gCurFileState.file_open_flag = 0xaa;
					
					
					if(from_flash_pic != 1)
					{
						gCurFileState.totalSend = (unsigned char)((float)rePrintOffset  * 100 / f_size(&curFile));
					}
					else
					{
						gCurFileState.totalSend = (unsigned char)(((float)rePrintOffset -(PREVIEW_SIZE+To_pre_view)) * 100 / (f_size(&curFile)-(PREVIEW_SIZE+To_pre_view)));
					}
					

					
					//{
						printerStaus = pr_reprint;
						disp_state_stack._disp_index = 0;
						memset(disp_state_stack._disp_state, 0, sizeof(disp_state_stack._disp_state));
						disp_state_stack._disp_state[disp_state_stack._disp_index] = PRINTING_UI;
						//draw_pause();
						draw_printing();
						
					//}
					
					logo_time = 3000 / TICK_CYCLE;
				}
				else
				{
					

					
						//f_mount(&fs, (TCHAR const*)USBH_Path, 0);
						//curFileName[0]=(char )'0';
						memset(sd.gCurDir, 0, sizeof(sd.gCurDir));
						//if (SD_DET_IP == SDCARDDETECTINVERTED)
						if(gCfgItems.fileSysType == FILE_SYS_SD)
						{
							strcpy(sd.gCurDir, "1:");
							MX_SDIO_SD_Init();
							if(SD_DET_IP == SDCARDDETECTINVERTED)
							{
			              		
			            		FATFS_LinkDriver_sd(&SD_Driver, SD_Path);
								f_mount(&fs, (TCHAR const*)SD_Path, 0);	
							}							
							sd.ShowSDFiles();
						}
						else
						{
							FATFS_LinkDriver_usb(&USBH_Driver, USBH_Path);
		       				f_mount(&fs, (TCHAR const*)USBH_Path, 0);	
							strcpy(sd.gCurDir, "0:");
							//curFileName[0]=(char )'0';
						}
						i=150000;
						while(i--)
						{
							MX_USB_HOST_Process();
						}
						
					
					res = f_open(&curFile, curFileName, FA_OPEN_EXISTING | FA_READ);
					if(res == FR_OK)
					{
						srcfp = &curFile;
						gCurFileState.file_open_flag = 0xaa;
						if(from_flash_pic != 1)
						{
							gCurFileState.totalSend = (unsigned char)((float)rePrintOffset  * 100 / f_size(&curFile));
						}
						else
						{
							gCurFileState.totalSend = (unsigned char)(((float)rePrintOffset -(PREVIEW_SIZE+To_pre_view)) * 100 / (f_size(&curFile)-(PREVIEW_SIZE+To_pre_view)));
						}
						printerStaus = pr_reprint;
						disp_state_stack._disp_index = 0;
						memset(disp_state_stack._disp_state, 0, sizeof(disp_state_stack._disp_state));
						disp_state_stack._disp_state[disp_state_stack._disp_index] = PRINTING_UI;
						//draw_pause();
						draw_printing();
					}
					else
					{
						disp_state_stack._disp_index = 0;
						memset(disp_state_stack._disp_state, 0, sizeof(disp_state_stack._disp_state));
						disp_state_stack._disp_state[disp_state_stack._disp_index] = PRINT_READY_UI;
						preview_no_display = 1;
						draw_dialog(DIALOG_TYPE_REPRINT_NO_FILE);
						SPEAKER = ON; 
        				HAL_Delay(500);
					}
							
					
				}
		
			
		}
		else
		{
			//**draw_main_ui();	
			draw_ready_print();
			logo_time = 0;
		}
	}
	else
	{
		//**draw_main_ui();
		tick1 = getTick();
		while(1)
		{
			tick2 = getTick();
			if(getTickDiff(tick2, tick1)>=3000)
			{
				draw_ready_print();
				logo_time = 0;
				break;
			}
		}
		
	}	

	if(gCfgItems.wifi_type == ESP_WIFI)
	{
		wifi_init();
	}
	
    SPEAKER = 0;
					
/***************end***********************/

    
   


 
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */
    
  /* USER CODE BEGIN 3 */
    //HAL_UART_Receive_IT(&huart3, mksRxBuffer, 1)); 


/*********************change*********************/
	
		//**if(wifi_link_state != WIFI_TRANS_FILE)
	  	//**{
	  		#if tan
	  		sd.automount();
  			#endif
			
  			if(wifi_link_state != WIFI_TRANS_FILE)
  			{
  				//console 
  				switch(console_flag)
                {
                    case 1:
                      console_show(); 
                      console_flag=3;
                        break;
                    case 2:
                      console_back();  
                      console_flag=0;
					 lineNumber = 0;
                        break;
					case 3:
						if(update_msg_flag == 1)
						{
							update_msg_flag = 0;
							console_show();  
						}
                        break;
                    default:break;
                }    
	            if (console_flag != 3 )
						GUI_RefreshPage();
//				GUI_RefreshPage();
  			}
			
			mksUsart2Polling();
			clock_run();
		//**}
		
		if(wifi_init_flg == 1)
		{
		#if tan
			if(gCfgItems.wifi_type == HLK_WIFI)
			{
				if(wifi_init_state == 0)
				{
					wifi_init();
					wifi_init_state = 1;
				}
				else
				{
					wifi_looping();
				}
			}
			else
		#endif
			{
				wifi_looping();
			}
		}		
		
		//**if(wifi_init_flg == 1)
		//**{
			//wifi_looping();
		//**}

		//**if(wifi_link_state != WIFI_TRANS_FILE)
		//**{
		if(wifi_link_state != WIFI_TRANS_FILE)
		{
			if(get_temp_flag)
			{	
				get_temp_flag = 0;
				
				if(((link_mutex_detect_time >= 5) || (printer_state == PRINTER_NOT_CONNECT) || (printerStaus == pr_working))\
					&&(Get_Temperature_Flg == 1))
				{
					link_mutex_detect_time = 0;
					get_cur_temp();
				}

			}
			//get_cur_temp();

			get_cmd_ack();
			
			pushButtonCodeToFifo();
		}
		//**}  
		PowerOff_Filament_Check();

		#ifdef SAVE_FROM_SD
		if(gCfgItems.pwroff_save_mode == 0)
		{
			sd_saved_data();
			
		}
		#endif
		GUI_TOUCH_Exec();    	
        GUI_Exec();  
		MX_USB_HOST_Process();
		//SPEAKER = 0; 
		#if defined(TFT70)
		disp_pre_gcode(3,40);
		#elif defined(TFT35)
		if(preview_no_display != 1)
		{
			disp_pre_gcode(2,36);
		}
		#endif
		//sd_data_recover();

 /**********************end***********************/
  }
  /* USER CODE END 3 */
}



/** System Clock Configuration
*/

void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);                                
} 

/** NVIC Configuration
*/
static void MX_NVIC_Init(void)
{
  /* TIM2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM2_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
  /* TIM3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
  /* TIM4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM4_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(TIM4_IRQn);
  /* TIM5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM5_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(TIM5_IRQn);
}

/* USER CODE BEGIN 4 */
#include "mks_test.c"

static uint8_t temperature_change_frequency_cnt = 0;
static uint32_t After_finish_print_time = 0;
static volatile uint8_t fan_move_flag;
static volatile uint8_t get_ack_flag = 0;

uint8_t volatile printing_rate_update_flag = 0;
uint8_t MTRDN_LOWLEVLE_Tcnt_flg=0;
uint8_t MTRDN_LOWPOWER_FLG = 0;
uint8_t MTRDN_HIGHLEVEL_Tcnt_flg=0;
uint8_t MTRDN_HIGHLEVEL_FLG = 0;
uint8_t PWRDN_HIGHLEVEL_Tcnt_flg=0;
uint8_t PWRDN_MTRDN_flg = 0;

uint8_t PWRDN_LOWPOWER_FLG = 0;
uint8_t PWRDN_HIGHLEVEL_FLG = 0;	
uint8_t PWRDN_LOWLEVLE_Tcnt_flg=0;
uint16_t PWRDN_MTRDN_Tcnt_flg = 0;
uint8_t close_fail_flg = 1;
uint16_t close_fail_cnt = 0;
uint16_t z_high_count=0;
uint32_t T_CNT = 0;

uint8_t poweroff_det_flg;
uint32_t poweroff_det_cnt;
uint8_t poweroff_det_low_flg;
uint32_t poweroff_det_low_cnt;
uint8_t poweroff_det_high_flg;
uint32_t poweroff_det_high_cnt;

uint8_t filament_det1_flg;
uint32_t filament_det1_cnt;
uint8_t filament_det1_low_flg;
uint32_t filament_det1_low_cnt;
uint8_t filament_det1_high_flg;
uint32_t filament_det1_high_cnt;
uint8_t filament_det1_check;

uint8_t filament_det2_flg;
uint32_t filament_det2_cnt;
uint8_t filament_det2_low_flg;
uint32_t filament_det2_low_cnt;
uint8_t filament_det2_high_flg;
uint32_t filament_det2_high_cnt;
uint8_t filament_det2_check;

extern uint8_t btn_putdown_time_flg;
extern uint32_t btn_putdown_cnt;
extern uint16_t resume_time;
extern PRINT_TIME  print_time;

uint8_t filament_loading_time_flg;
uint32_t filament_loading_time_cnt;
uint8_t filament_loading_completed;
uint8_t filament_unloading_time_flg;
uint32_t filament_unloading_time_cnt;
uint8_t filament_unloading_completed;
extern uint32_t filament_rate;

uint8_t waiting_wifi_time = 0;

extern "C" void SysTick_Handler_User();
void SysTick_Handler_User()
{

	if(filament_loading_time_flg == 1)
	{
		filament_loading_time_cnt++;
		filament_rate = (((float)filament_loading_time_cnt/(gCfgItems.filament_loading_time*1000))*100);
		if(filament_loading_time_cnt >= (gCfgItems.filament_loading_time*1000))
		{
			filament_loading_time_cnt = 0;
			filament_loading_time_flg = 0;
			filament_loading_completed = 1;
		}
	}
	if(filament_unloading_time_flg == 1)
	{
		filament_unloading_time_cnt++;
		filament_rate = (((float)filament_unloading_time_cnt/(gCfgItems.filament_unloading_time*1000))*100);
		if(filament_unloading_time_cnt >= (gCfgItems.filament_unloading_time*1000))
		{
			filament_unloading_time_cnt = 0;
			filament_unloading_time_flg = 0;
			filament_unloading_completed = 1;
			filament_rate = 100;
		}
	}
	

	if(btn_putdown_time_flg == 1)
	{
		btn_putdown_cnt++;//100
	}
	resume_time++;
	
	if(poweroff_det_flg==1)
	{
		poweroff_det_cnt++;
	}
	
	if(filament_det1_flg==1)
	{
		filament_det1_cnt++;
	}

	if(filament_det1_low_flg==1)
	{
		filament_det1_low_cnt++;
	}

	if(filament_det1_high_flg==1)
	{
		filament_det1_high_cnt++;
	}
	if(filament_det2_flg==1)
	{
		filament_det2_cnt++;
	}

	if(filament_det2_low_flg==1)
	{
		filament_det2_low_cnt++;
	}

	if(filament_det2_high_flg==1)
	{
		filament_det2_high_cnt++;
	}

	if(poweroff_det_low_flg==1)
	{
		poweroff_det_low_cnt++;
	}

	if(poweroff_det_high_flg==1)
	{
		poweroff_det_high_cnt++;
	}

#if 1
	if(!(TimeIncrease * TICK_CYCLE % 5000))	//5s
	{
		get_temp_flag = 1;
		wifi_check_time = 1;
		waiting_wifi_time ++;
		if((waiting_wifi_time == 8) || (gCfgItems.wifi_type == ESP_WIFI))//40s
		{
			if(wifi_init_flg == 0)
			{
				wifi_init_flg = 1;
			}
			
		}
		
		if(wifi_refresh_flg == 0)
			wifi_refresh_flg = 1;
		
		if(cloud_refresh_flg == 0)
			cloud_refresh_flg = 1;

	}
	
	if(!(TimeIncrease * TICK_CYCLE % 3000))	// 3s
	{		
		printing_rate_update_flag = 1;
	}
	
	if(!(TimeIncrease * TICK_CYCLE % 1000))	// 1s
	{		
		get_ack_flag = 1;

		if(PWRDN_MTRDN_Tcnt_flg == 1)
		{
			PWRDN_MTRDN_flg = 1;
		}
		
		if(PWRDN_LOWLEVLE_Tcnt_flg==1)
		{
			PWRDN_LOWPOWER_FLG = 1;
		}
		
		if(PWRDN_HIGHLEVEL_Tcnt_flg==1)
		{
			PWRDN_HIGHLEVEL_FLG = 1;
		}
		
		if(MTRDN_LOWLEVLE_Tcnt_flg==1)
		{
			MTRDN_LOWPOWER_FLG = 1;
		}
		
		if(MTRDN_HIGHLEVEL_Tcnt_flg==1)
		{
			MTRDN_HIGHLEVEL_FLG = 1;
		}

		if(close_fail_flg == 1)
		{
			close_fail_cnt++;
			if(close_fail_cnt >= 5)
			{
				close_fail_cnt = 0;
				close_fail_flg = 0;
			}
		}
	}
	if(!(TimeIncrease * TICK_CYCLE % wifi_loop_cycle))	// 0.5s
	{
		wifi_loop_time = 1;	
	}

	if(!(TimeIncrease * TICK_CYCLE % 100))	// 0.1s
	{		
		fan_move_flag = 1;
	}
	

	//if(!(TimeIncrease * TICK_CYCLE % 10)) //10ms
	//	GUI_TOUCH_Exec();

	TimeIncrease++;

	mksUsart2RepeatTx();
 	
	
	////////send2Printer();
	//////////	rxPrinter();
	

	if(disp_state == MAIN_UI)
	{
		if(logo_time * TICK_CYCLE < 3000 )	// 3s
		{	
			logo_time++;
			//get_cur_temp();
		}
	}


	if(((TimeIncrease * TICK_CYCLE % 1000) == 0) )
	{		
		link_mutex_detect_time++;
		if(print_time.start == 1)
		{
			print_time.seconds++;
		}
	}
	
    #endif
	//lan
	if(key_value_calc.timer == TIMER_START)
	{
		key_value_calc.timer_count++;
	}
	if(((TimeIncrease * TICK_CYCLE % 1000) == 0) )
	{		
		if(tips_disp.timer == TIPS_TIMER_START)
		{
			tips_disp.timer_count++;
		}
	}

	if(T_CNT != 0) 
	{
		T_CNT--;
	}

	z_high_count++;
	if(z_high_count>500)z_high_count=0;

}
#if 0
#define PUTDOWN_TIME	50
extern uint8_t btn_putdown_time_flg;
extern uint32_t btn_putdown_cnt;

uint8_t close_fail_flg = 1;
uint16_t close_fail_cnt = 0;

void Close_machine_signal()
{
	//PB4:�ܽż��ܽ�
	//if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1) == Bit_SET)
        if(MKS_MT_DET1_OP == 1)   //��ʱ�滻�ܽ�
	{
		if(btn_putdown_cnt >= PUTDOWN_TIME)
		{
			//if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1) == Bit_SET)
                        if(MKS_MT_DET1_OP == 1)
			{
				btn_putdown_time_flg = 0;
				btn_putdown_cnt = 0;
				//rePrintSaveData();
				clear_cur_ui();
				disp_state = PRINT_READY_UI;
				GUI_SetBkColor(gCfgItems.background_color);
				GUI_SetColor(gCfgItems.title_color);
				GUI_Clear();
				//GUI_SetFont(&FONT_TITLE);
				if(gCfgItems.language == LANG_COMPLEX_CHINESE)
				{
					GUI_SetFont(&GUI_FontHZ16);
				}
				else
				{
					GUI_SetFont(&FONT_TITLE);
				}
				
				if(gCfgItems.language == LANG_ENGLISH)
				{
					GUI_DispStringAt("Closing Machine...", 80, 120);
				}
				else	if(gCfgItems.language == LANG_COMPLEX_CHINESE)
				{
					GUI_DispStringAt("�����P�C...", 80, 120);
				}
				else
				{
					GUI_DispStringAt("���ڹػ�...", 80, 120);
				}
				//GPIO_ResetBits(GPIOA, GPIO_Pin_3);
                                MKS_PW_OFF_OP = 0;
				//close_fail_flg = 1;
				//close_fail_cnt = 0;
				//while(close_fail_flg);
				//clear_cur_ui();
				//draw_ready_print();
			}
		}
	}
	else
	{
			btn_putdown_time_flg = 0;
			btn_putdown_cnt = 0;
	} 	

}

#endif
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 
/**************************add************************/
extern USART2DATATYPE usart2Data;
void Btn_putdown_close_machine()
{
	//**initFIFO(&gcodeCmdTxFIFO);
	//**pushFIFO(&gcodeCmdTxFIFO, (unsigned char *)"M81\n");
	memset(usart2Data.usart2Txbuf,0,sizeof(usart2Data.usart2Txbuf));
	usart2Data.usart2Txbuf[0]='M';usart2Data.usart2Txbuf[1]='8';usart2Data.usart2Txbuf[2]='1';usart2Data.usart2Txbuf[3]='\n'; 	
	usart2TxStart();		
}

static uint8_t lastSec = 0;
void clock_run()
{
	
	if(print_time.seconds >= 60)
	{
		print_time.seconds = 0;
		print_time.minutes++;
		if(print_time.minutes >= 60)
		{
			print_time.minutes = 0;
			print_time.hours++;
			/*if(print_time.hours >= 24)
			{
				print_time.hours = 0;
				print_time.days++;
			}*/
		}
		
	}
	if(disp_state == PRINTING_UI)
	{
		if(lastSec != print_time.seconds)
		{
			disp_print_time();
		}
		lastSec =  print_time.seconds;
	}
}

void get_cur_temp()
{
	pushFIFO(&gcodeCmdTxFIFO, (unsigned char *)GET_CUR_TEM_COMMAND);
}

uint8_t  DecStr2Int(int8_t * buf,  int  *result)
{
	int  index = 0;
	
	int  retVal = 0;
	int8_t  dot_flag = 0;
	int8_t  negat_flag = 0;
		
	if(buf == 0  ||  result == 0)
	{
		return  0;
	}

	do
	{
		if((buf[index] <= '9')  &&  (buf[index]  >= '0'))
		{
			
			retVal  *=  10;
			retVal  += buf[index] - '0';
			
		}
		else if(buf[index] == '-')
		{
				negat_flag = 1;
		}
		else
		{
				if(negat_flag)
				{
					*result = 0 - retVal;
				}
				else
				{
						*result = retVal;
				}
			return 1;
		}
		index++;
			
	} while(1);
	
}

uint8_t temp_update_flag = 0;
uint8_t FanSpeed_bak = 0;
uint8_t fan_change_flag = 0;
uint8_t step_update_flag = 0;
uint8_t current_update_flag = 0;
uint8_t maxFR_update_flag = 0;
uint8_t accel_update_flag = 0;

uint8_t sensivisity_update =0;
uint8_t probeOffset_update =0;
uint8_t saveSuccess_flag =0;

static uint8_t get_temper_flg = 0;
extern char cmd_code[201];

void get_cmd_ack()
{
	
	int8_t *tmpStr = 0;
	int8_t *tmpStr_Line = 0;
	int8_t *tmpStr_e = 0;
	int8_t *tmpStr_step = 0;
	
	float  tmpTemp = 0;
	float tmpStep = 0;
	uint16_t tmpCurrent = 0;
	int8_t rcv_ack_flag = 0;
	int32_t i, j, k;
	int8_t inc_flag = 0;
	int8_t num_valid = 0;
	int8_t cmdRxBuf[128] = {0};
	int8_t  tempBuf[100] = {0};
	int16_t tmpSensitive = 0;
	memset(cmdRxBuf, 0, sizeof(cmdRxBuf));

	if(popFIFO(&gcodeCmdRxFIFO,  (unsigned char *)cmdRxBuf) == fifo_ok)
	{	
		link_mutex_detect_time = 0;
		get_temper_flg = 0;
		i = 0;

		update_msg_flag=1;
		memset((char *)console_msg_print,0,sizeof(console_msg_print));
		strcpy((char *)console_msg_print,(const char *)cmdRxBuf);
		
		tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Printer stopped due to errors");
		if(tmpStr)
		{
		
		}
		//lan
		tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Settings Stored");
		if(tmpStr)
		{
				memset((char *)cmd_code,0,sizeof(cmd_code));
				strcpy((char *)cmd_code,(const char *)cmdRxBuf);
				if(console_flag!=3)
				{
					if(disp_state != DIALOG_UI)
					clear_cur_ui();
					draw_dialog(DIALOG_TYPE_M500_SUCCESS);
				}
		}
		
		#if 1
		if(gCfgItems.display_error != 0)
		{
			tmpStr_Line = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Error:Line Number");
			tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Error:");
			if((tmpStr)&&(!tmpStr_Line))
			{
				memset((char *)cmd_code,0,sizeof(cmd_code));
				strcpy((char *)cmd_code,(const char *)cmdRxBuf);
				if(disp_state != DIALOG_UI)
				clear_cur_ui();
				draw_dialog(DIALOG_TYPE_MAINBOAR_ERROR);
			}
		}
		/*
		tmpStr = (int8_t *)strstr(&cmdRxBuf[i], "error:");
		if(tmpStr)
		{
			memset(cmd_code,0,sizeof(cmd_code));
			strcpy(cmd_code,cmdRxBuf);
			if(disp_state != DIALOG_UI)
				clear_cur_ui();
			draw_dialog(DIALOG_TYPE_MAINBOAR_ERROR);
		}
		*/
		#endif
		
		tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"T:");
		if( tmpStr)
		{
			if(printer_state == PRINTER_NOT_CONNECT)
			{
				printer_state = PRINTER_CONNECT_OK;
			}
			if((*(tmpStr+2) =='i')&&(*(tmpStr+3) =='n')&&(*(tmpStr+4) =='f'))
			{
				gCfgItems.curSprayerTemp[0] = -50;
			}
			else
			{
			memset(tempBuf, 0, sizeof(tempBuf));
			k = 0;
			num_valid = 0;
			for(j = 2; tmpStr[j] != ' '; j++)
			{
				
				if(tmpStr[j] == '\0')
				{
					break;
				}
				
				tempBuf[k] = tmpStr[j];
				num_valid = 1;
				k++;
			}
			if(num_valid)
			{
				if(DecStr2Float(tempBuf, &tmpTemp)	 !=   0)//�̡�?��???��
				{
					//if((int)tmpTemp != 0)
					{
						if(gCfgItems.sprayerNum == 1)
						{
							gCfgItems.curSprayerTemp[0] = tmpTemp;
							if(gCfgItems.curSprayerTemp[0] > 999)//?T?����o?��??��???��D3??
							{
								gCfgItems.curSprayerTemp[0] = gCfgItems.curSprayerTempBak[0];
								
							}
							else
							{
								gCfgItems.curSprayerTempBak[0] = gCfgItems.curSprayerTemp[0];
							}
						}
						else
						{
								tmpStr_e = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"E:");
								if(tmpStr_e)
								{
									if(*(tmpStr_e+2) =='0')
									{
										gCfgItems.curSprayerTemp[0] = tmpTemp;
										if(gCfgItems.curSprayerTemp[0] > 999)//?T?����o?��??��???��D3??
										{
											gCfgItems.curSprayerTemp[0] = gCfgItems.curSprayerTempBak[0];
											
										}
										else
										{
											gCfgItems.curSprayerTempBak[0] = gCfgItems.curSprayerTemp[0];
										}
									}
									else if(*(tmpStr_e+2) =='1')
									{
										gCfgItems.curSprayerTemp[1] = tmpTemp;
										if(gCfgItems.curSprayerTemp[1] > 999)//?T?����o?��??��???��D3??
										{
											gCfgItems.curSprayerTemp[1] = gCfgItems.curSprayerTempBak[1];
											
										}
										else
										{
											gCfgItems.curSprayerTempBak[1] = gCfgItems.curSprayerTemp[1];
										}												
									}
								}
								else
								{
									gCfgItems.curSprayerTemp[0] = tmpTemp;
									if(gCfgItems.curSprayerTemp[0] > 999)//?T?����o?��??��???��D3??
									{
										gCfgItems.curSprayerTemp[0] = gCfgItems.curSprayerTempBak[0];
									}
									else
									{
										gCfgItems.curSprayerTempBak[0] = gCfgItems.curSprayerTemp[0];
									}
								}
						}
						
						temp_update_flag = 1;
					}
				}
				if(tmpStr[j + 1] == '/')
				{
					j += 2;
					memset(tempBuf, 0, sizeof(tempBuf));
					k = 0;
					num_valid = 0;
					for(; tmpStr[j] != ' '; j++)
					{
						
						if(tmpStr[j] == '\0')
						{
							break;
						}
						tempBuf[k] = tmpStr[j];
						num_valid = 1;
						k++;
						
					}
					if(num_valid)
					{
						if(DecStr2Float(tempBuf, &tmpTemp)	 !=   0)//??����???��
						{
							//if((int)tmpTemp != 0)
							{
								if(gCfgItems.sprayerNum == 1)
								{
									gCfgItems.desireSprayerTemp[0] = tmpTemp;
									if(gCfgItems.desireSprayerTemp[0] > 999)//?T?����o?��??��???��D3??
									{
										gCfgItems.desireSprayerTemp[0] = gCfgItems.desireSprayerTempBak[0];
									}
									else
									{
										gCfgItems.desireSprayerTempBak[0] = gCfgItems.desireSprayerTemp[0];
									}
									
									temp_update_flag = 1;		
								}
								else
								{
									tmpStr_e = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"E:");
									if(tmpStr_e)
									{
										if(*(tmpStr_e+2) =='0')
										{
											gCfgItems.desireSprayerTemp[0] = tmpTemp;
											if(gCfgItems.desireSprayerTemp[0] > 999)//?T?����o?��??��???��D3??
											{
												gCfgItems.desireSprayerTemp[0] = gCfgItems.desireSprayerTempBak[0];
											}
											else
											{
												gCfgItems.desireSprayerTempBak[0] = gCfgItems.desireSprayerTemp[0];
											}
											temp_update_flag = 1;	
										}
										else	if(*(tmpStr_e+2) =='1')
										{
											gCfgItems.desireSprayerTemp[1] = tmpTemp;
											if(gCfgItems.desireSprayerTemp[1] > 999)//?T?����o?��??��???��D3??
											{
												gCfgItems.desireSprayerTemp[1] = gCfgItems.desireSprayerTempBak[1];
											}
											else
											{
												gCfgItems.desireSprayerTempBak[1] = gCfgItems.desireSprayerTemp[1];
											}
											temp_update_flag = 1;	
										}
									}
									else
									{
											gCfgItems.desireSprayerTemp[0] = tmpTemp;
											if(gCfgItems.desireSprayerTemp[0] > 999)//?T?����o?��??��???��D3??
											{
												gCfgItems.desireSprayerTemp[0] = gCfgItems.desireSprayerTempBak[0];
											}
											else
											{
												gCfgItems.desireSprayerTempBak[0] = gCfgItems.desireSprayerTemp[0];
											}
											temp_update_flag = 1;	
									}
								}
							}
						}
					}
				}
			}
			}
		}		


		tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"B:");
		if(tmpStr)
		{
				if(printer_state == PRINTER_NOT_CONNECT)
				{
					printer_state = PRINTER_CONNECT_OK;
				}
				if((*(tmpStr+2) =='i')&&(*(tmpStr+3) =='n')&&(*(tmpStr+4) =='f'))
				{
					gCfgItems.curBedTemp = -50;
				}
				else
				{
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 2; tmpStr[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					tempBuf[k] = tmpStr[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
					if(DecStr2Float(tempBuf, &tmpTemp)	 != 	0)
					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.curBedTemp = tmpTemp;
							if(gCfgItems.curBedTemp > 999)//?T?����o?��??��???��D3??
							{
								gCfgItems.curBedTemp = gCfgItems.curBedTempBak;
							}
							else
							{
								gCfgItems.curBedTempBak = gCfgItems.curBedTemp;
							}
							temp_update_flag = 1; 						
						}
		
					}
					if(tmpStr[j + 1] == '/')
					{
						j += 2;
						memset(tempBuf, 0, sizeof(tempBuf));
						k = 0;
						num_valid = 0;
						for(; tmpStr[j] != ' '; j++)
						{
							
							if(tmpStr[j] == '\0')
							{ 							
								break;
							}
							tempBuf[k] = tmpStr[j];
							num_valid = 1;
							k++;
								
						}
						if(num_valid)
						{
							if(DecStr2Float(tempBuf, &tmpTemp)	!=	 0)
							{
								//if((int)tmpTemp != 0)
								{
									gCfgItems.desireBedTemp = tmpTemp;
									if(gCfgItems.desireBedTemp > 999)//?T?����o?��??��???��D3??
									{
										gCfgItems.desireBedTemp = gCfgItems.desireBedTempBak;
									}
									else
									{
										gCfgItems.desireBedTempBak = gCfgItems.desireBedTemp;
									} 									
									temp_update_flag = 1; 									
								}
							}
						}
					}
				}
			}
		}
		
			tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"T0:");
			if( tmpStr)
			{
				if(printer_state == PRINTER_NOT_CONNECT)
				{
					printer_state = PRINTER_CONNECT_OK;
				}
				if((*(tmpStr+2) =='i')&&(*(tmpStr+3) =='n')&&(*(tmpStr+4) =='f'))
				{
					gCfgItems.curSprayerTemp[0] = -50;
				}
				else
				{		
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 3; tmpStr[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
					if(DecStr2Float(tempBuf, &tmpTemp)	 != 	0)
					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.curSprayerTemp[0] = tmpTemp;
							if(gCfgItems.curSprayerTemp[0] > 999)//?T?����o?��??��???��D3??
							{
								gCfgItems.curSprayerTemp[0] = gCfgItems.curSprayerTempBak[0];
							}
							else
							{
								gCfgItems.curSprayerTempBak[0] = gCfgItems.curSprayerTemp[0]; 
							}
							temp_update_flag = 1;
						}
					}
					if(tmpStr[j + 1] == '/')
					{
						j += 2;
						memset(tempBuf, 0, sizeof(tempBuf));
						k = 0;
						num_valid = 0;
						for(; tmpStr[j] != ' '; j++)
						{
							
							if(tmpStr[j] == '\0')
							{
								break;
							}
							tempBuf[k] = tmpStr[j];
							num_valid = 1;
							k++;
							
						}
						if(num_valid)
						{
							if(DecStr2Float(tempBuf, &tmpTemp)	 != 	0)
							{
								//if((int)tmpTemp != 0)
								{
									gCfgItems.desireSprayerTemp[0] = tmpTemp;
									if(gCfgItems.desireSprayerTemp[0] > 999)//?T?����o?��??��???��D3??
									{
										gCfgItems.desireSprayerTemp[0] = gCfgItems.desireSprayerTempBak[0];
									}
									else
									{
										gCfgItems.desireSprayerTempBak[0] = gCfgItems.desireSprayerTemp[0]; 
									} 										
									temp_update_flag = 1;
								}
							}
						}
					}
				}
			}
		}
			
			tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"T1:");
			if( tmpStr)
			{
				if(printer_state == PRINTER_NOT_CONNECT)
				{
					printer_state = PRINTER_CONNECT_OK;
				}
				if((*(tmpStr+2) =='i')&&(*(tmpStr+3) =='n')&&(*(tmpStr+4) =='f'))
				{
					gCfgItems.desireSprayerTemp[1] = -50;
				}
				else
				{	

				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 3; tmpStr[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
					if(DecStr2Float(tempBuf, &tmpTemp)	 !=   0)
					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.curSprayerTemp[1] = tmpTemp;
							if(gCfgItems.curSprayerTemp[1] > 999)//?T?����o?��??��???��D3??
							{
								gCfgItems.curSprayerTemp[1] = gCfgItems.curSprayerTempBak[1];
							}
							else
							{
								gCfgItems.curSprayerTempBak[1] = gCfgItems.curSprayerTemp[1];			
							}
							temp_update_flag = 1;
						}
					}
					if(tmpStr[j + 1] == '/')
					{
						j += 2;
						memset(tempBuf, 0, sizeof(tempBuf));
						k = 0;
						num_valid = 0;
						for(; tmpStr[j] != ' '; j++)
						{
							
							if(tmpStr[j] == '\0')
							{
								break;
							}
							tempBuf[k] = tmpStr[j];
							num_valid = 1;
							k++;
							
						}
						if(num_valid)
						{
							if(DecStr2Float(tempBuf, &tmpTemp)	 !=   0)
							{
								//if((int)tmpTemp != 0)
								{
									gCfgItems.desireSprayerTemp[1] = tmpTemp;
									if(gCfgItems.desireSprayerTemp[1] > 999)//?T?����o?��??��???��D3??
									{
										gCfgItems.desireSprayerTemp[1] = gCfgItems.desireSprayerTempBak[1];
									}
									else
									{
										gCfgItems.desireSprayerTempBak[1] = gCfgItems.desireSprayerTemp[1];			
									}											
									temp_update_flag = 1;
								}

							}
						}
					}
				}
			}
		}
				
			tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M107");
			if( tmpStr)
			{
				//TAN
				FanSpeed_bak = gCfgItems.fanSpeed;
				gCfgItems.fanOnoff = 0;
				gCfgItems.fanSpeed = 0;
				fan_change_flag = 1;
			}

			tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M106 S");
			if( tmpStr)
			{
				j= strlen("M106 S");

				k = 0;
				memset(tempBuf, 0, sizeof(tempBuf));
				num_valid = 0;
				for(; tmpStr[j] != '\0'; j++)
				{
					
					tempBuf[k] = tmpStr[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
					int tmpInt = 0;
					if(DecStr2Int(tempBuf, &tmpInt)	 !=   0)
					{
						//TAN
						FanSpeed_bak = gCfgItems.fanSpeed;
						
						if(tmpInt == 0)
						{
							gCfgItems.fanOnoff = 0;
							gCfgItems.fanSpeed = 0;
						}
						else
						{
							gCfgItems.fanOnoff  = 1;
							gCfgItems.fanSpeed = tmpInt;
						}
						fan_change_flag = 1;
					}
				}
			}

			tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M92 X");
			if( tmpStr)
			{

				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 5; tmpStr[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
					if(DecStr2Float(tempBuf, &tmpStep)	 !=   0)
					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.xStep = tmpStep;
						}
					}
				}
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Y");
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 1; tmpStr_step[j] != ' '; j++)
				{
					
					if(tmpStr_step[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
					if(DecStr2Float(tempBuf, &tmpStep)	 !=   0)
					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.yStep = tmpStep;
						}
					}
				}	
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Z");
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 1; tmpStr_step[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;
				}
				if(num_valid)
				{
					if(DecStr2Float(tempBuf, &tmpStep)	 !=   0)
					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.zStep = tmpStep;
						}
					}
				}
				if (gCfgItems.sprayerNum ==1)
				{
					tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"E");
					memset(tempBuf, 0, sizeof(tempBuf));
					k = 0;
					num_valid = 0;
					for(j = 1; tmpStr_step[j] != ' '; j++)
					{
						
						if(tmpStr_step[j] == '\n')
						{
							break;
						}
						
						tempBuf[k] = tmpStr_step[j];
						num_valid = 1;
						k++;
						
					}
					if(num_valid)
					{
						if(DecStr2Float(tempBuf, &tmpStep)	 !=   0)
						{
							//if((int)tmpTemp != 0)
							{
								gCfgItems.e0Step = tmpStep;
							}
						}
					}	
				}
				
				step_update_flag = 1;
			}
			if(gCfgItems.sprayerNum !=1)
				{
						tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M92 T0 E");
						if(tmpStr_step)
						{
							memset(tempBuf, 0, sizeof(tempBuf));
							k = 0;
							num_valid = 0;
							for(j = 8; tmpStr_step[j] != ' '; j++)
							{
								
								if(tmpStr_step[j] == '\n')
								{
									break;
								}
								
								tempBuf[k] = tmpStr_step[j];
								num_valid = 1;
								k++;
								
							}
							if(num_valid)
							{
								if(DecStr2Float(tempBuf, &tmpStep)	 !=   0)
								{
									//if((int)tmpTemp != 0)
									{
										gCfgItems.e0Step = tmpStep;
									}
								}
							}	
						}
						tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M92 T1 E");
						if(tmpStr_step)
						{
							memset(tempBuf, 0, sizeof(tempBuf));
							k = 0;
							num_valid = 0;
							for(j = 8; tmpStr_step[j] != '\n'; j++)
							{
								
								if(tmpStr_step[j] == '\0')
								{
									break;
								}
								
								tempBuf[k] = tmpStr_step[j];
								num_valid = 1;
								k++;
								
							}
							if(num_valid)
							{
								if(DecStr2Float(tempBuf, &tmpStep)	 !=   0)
								{
									//if((int)tmpTemp != 0)
									{
										gCfgItems.e1Step = tmpStep;
									}
								}
							}	
						}
						step_update_flag = 1;
				}
/*************************M201***************************************/
			tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M201 X");
			if( tmpStr)
			{

				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 6; tmpStr[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
					tmpCurrent = atoi((const char *)tempBuf);

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.xMaxAccel = tmpCurrent;
						}
					}
				}
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Y");
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 1; tmpStr_step[j] != ' '; j++)
				{
					
					if(tmpStr_step[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
					tmpCurrent = atoi((const char *)tempBuf);

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.yMaxAccel = tmpCurrent;
						}
					}
				}	
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Z");
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 1; tmpStr_step[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;
				}
				if(num_valid)
				{
//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
					tmpCurrent = atoi((const char *)tempBuf);

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.zMaxAccel= tmpCurrent;
						}
					}
				}
				if (gCfgItems.sprayerNum ==1)
				{
					tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"E");
					memset(tempBuf, 0, sizeof(tempBuf));
					k = 0;
					num_valid = 0;
					for(j = 1; tmpStr_step[j] != ' '; j++)
					{
						
						if(tmpStr_step[j] == '\n')
						{
							break;
						}
						
						tempBuf[k] = tmpStr_step[j];
						num_valid = 1;
						k++;
						
					}
					if(num_valid)
					{
//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
					tmpCurrent = atoi((const char *)tempBuf);

						{
							//if((int)tmpTemp != 0)
							{
								gCfgItems.e0_MaxAccel = tmpCurrent;
							}
						}
					}	
				}
				accel_update_flag = 1;
			}		
			if (gCfgItems.sprayerNum !=1)
			{
				
				tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M201 T0 E");
				if( tmpStr)
				{

					memset(tempBuf, 0, sizeof(tempBuf));
					k = 0;
					num_valid = 0;
					for(j = 9; tmpStr[j] != ' '; j++)
					{
						
						if(tmpStr[j] == '\0')
						{
							break;
						}
						
						tempBuf[k] = tmpStr[j];
						num_valid = 1;
						k++;
						
					}
					if(num_valid)
					{
	//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
						tmpCurrent = atoi((const char *)tempBuf);

						{
							//if((int)tmpTemp != 0)
							{
								gCfgItems.e0_MaxAccel = tmpCurrent;
							}
						}
					}
				}
				tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M201 T1 E");
				if( tmpStr)
				{

					memset(tempBuf, 0, sizeof(tempBuf));
					k = 0;
					num_valid = 0;
					for(j = 9; tmpStr[j] != ' '; j++)
					{
						
						if(tmpStr[j] == '\0')
						{
							break;
						}
						
						tempBuf[k] = tmpStr[j];
						num_valid = 1;
						k++;
						
					}
					if(num_valid)
					{
	//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
						tmpCurrent = atoi((const char *)tempBuf);

						{
							//if((int)tmpTemp != 0)
							{
								gCfgItems.e1_MaxAccel = tmpCurrent;
							}
						}
					}	
				}
				accel_update_flag = 1;
			}
		/**************************M203***************************************/
		tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M203 X");
		if( tmpStr)
		{

			memset(tempBuf, 0, sizeof(tempBuf));
			k = 0;
			num_valid = 0;
			for(j = 6; tmpStr[j] != ' '; j++)
			{
				
				if(tmpStr[j] == '\0')
				{
					break;
				}
				
				tempBuf[k] = tmpStr[j];
				num_valid = 1;
				k++;
				
			}
			if(num_valid)
			{
				tmpCurrent = atoi((const char *)tempBuf);
				gCfgItems.xMaxFeedRate= tmpCurrent;
			}
			tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Y");
			memset(tempBuf, 0, sizeof(tempBuf));
			k = 0;
			num_valid = 0;
			for(j = 1; tmpStr_step[j] != ' '; j++)
			{
				
				if(tmpStr_step[j] == '\0')
				{
					break;
				}
				
				tempBuf[k] = tmpStr_step[j];
				num_valid = 1;
				k++;
				
			}
			if(num_valid)
			{
				tmpCurrent = atoi((const char *)tempBuf);
				gCfgItems.yMaxFeedRate = tmpCurrent;
			}	
			tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Z");
			memset(tempBuf, 0, sizeof(tempBuf));
			k = 0;
			num_valid = 0;
			for(j = 1; tmpStr_step[j] != '\n'; j++)
			{
				
				if(tmpStr[j] == '\0')
				{
					break;
				}
				tempBuf[k] = tmpStr_step[j];
				num_valid = 1;
				k++;
			}
			if(num_valid)
			{
				tmpCurrent = atoi((const char *)tempBuf);
				gCfgItems.zMaxFeedRate= tmpCurrent;
			}
			if (gCfgItems.sprayerNum ==1)
			{
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"E");
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 1; tmpStr_step[j] != ' '; j++)
				{
					if(tmpStr_step[j] == '\0')
					{
						break;
					}
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;	
				}
				if(num_valid)
				{
					tmpCurrent = atoi((const char *)tempBuf);
					gCfgItems.e0_MaxFeedRate = tmpCurrent;
				}			
			}	
			maxFR_update_flag = 1;
		}
		if (gCfgItems.sprayerNum !=1)
		{
			tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M203 T0 E");
			if(tmpStr_step)
			{
					memset(tempBuf, 0, sizeof(tempBuf));
					k = 0;
					num_valid = 0;
					for(j = 9; tmpStr_step[j] != ' '; j++)
					{
						
						if(tmpStr_step[j] == '\0')
						{
							break;
						}
						
						tempBuf[k] = tmpStr_step[j];
						num_valid = 1;
						k++;
						
					}
					if(num_valid)
					{
	//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
						tmpCurrent = atoi((const char *)tempBuf);

						{
							//if((int)tmpTemp != 0)
							{
								gCfgItems.e0_MaxFeedRate = tmpCurrent;
							}
						}
					}	
			}
			tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M203 T1 E");
			if(tmpStr_step)
			{
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 9; tmpStr_step[j] != ' '; j++)
				{
					
					if(tmpStr_step[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
					tmpCurrent = atoi((const char *)tempBuf);
					gCfgItems.e1_MaxFeedRate = tmpCurrent;
				}	
			}
			maxFR_update_flag = 1;
		}
				
				/**********************M204************/			
			tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M204 P");
			if( tmpStr)
			{

				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 6; tmpStr[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
					tmpCurrent = atoi((const char *)tempBuf);

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.printAccel = tmpCurrent;
						}
					}
				}
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"R");
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 1; tmpStr_step[j] != ' '; j++)
				{
					
					if(tmpStr_step[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
					tmpCurrent = atoi((const char *)tempBuf);

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.retractAccel = tmpCurrent;
						}
					}
				}	
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"T");
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 1; tmpStr_step[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;
				}
				if(num_valid)
				{
//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
					tmpCurrent = atoi((const char *)tempBuf);

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.travelAccel= tmpCurrent;
						}
					}
				}
				accel_update_flag = 1;
			}	
			
/***********************M851********************************/
			tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M851 X");
			if( tmpStr)
			{

				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 6; tmpStr[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
						if(DecStr2Float(tempBuf, &tmpTemp)  !=   0)

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.xProbeOffset = tmpTemp;
						}
					}
				}
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Y");
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 1; tmpStr_step[j] != ' '; j++)
				{
					
					if(tmpStr_step[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
						if(DecStr2Float(tempBuf, &tmpTemp)  !=   0)

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.yProbeOffset = tmpTemp;
						}
					}
				}	
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Z");
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 1; tmpStr_step[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;
				}
				if(num_valid)
				{
						if(DecStr2Float(tempBuf, &tmpTemp)  !=   0)
					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.zProbeOffset= tmpTemp;
						}
					}
				}
				probeOffset_update = 1;
			}			
/***********************M914********************************/
			tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M914 X");
			if( tmpStr)
			{

				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 6; tmpStr[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
					tmpSensitive = atoi((const char *)tempBuf);

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.xSensivisity = tmpSensitive;
						}
					}
				}
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Y");
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 1; tmpStr_step[j] != ' '; j++)
				{
					
					if(tmpStr_step[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
					tmpSensitive = atoi((const char *)tempBuf);

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.ySensivisity = tmpSensitive;
						}
					}
				}	
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Z");
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 1; tmpStr_step[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;
				}
				if(num_valid)
				{
//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
					tmpSensitive = atoi((const char *)tempBuf);

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.zSensivisity= tmpSensitive;
						}
					}
				}
				sensivisity_update = 1;
			}			

			
/**********************M906*****************************/
			tmpStr = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M906 X");
			if( tmpStr)
			{

				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 6; tmpStr[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
					tmpCurrent = atoi((const char *)tempBuf);

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.xCurrent = tmpCurrent;
						}
					}
				}
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Y");
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 1; tmpStr_step[j] != ' '; j++)
				{
					
					if(tmpStr_step[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;
					
				}
				if(num_valid)
				{
//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
					tmpCurrent = atoi((const char *)tempBuf);

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.yCurrent = tmpCurrent;
						}
					}
				}	
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"Z");
				memset(tempBuf, 0, sizeof(tempBuf));
				k = 0;
				num_valid = 0;
				for(j = 1; tmpStr_step[j] != ' '; j++)
				{
					
					if(tmpStr[j] == '\0')
					{
						break;
					}
					
					tempBuf[k] = tmpStr_step[j];
					num_valid = 1;
					k++;
				}
				if(num_valid)
				{
//						if(DecStr2Int(tempBuf, &tmpCurrent)  !=   0)
					tmpCurrent = atoi((const char *)tempBuf);

					{
						//if((int)tmpTemp != 0)
						{
							gCfgItems.zCurrent = tmpCurrent;
						}
					}
				}
//				if (gCfgItems.sprayerNum ==1)
//				{
//					tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"E");
//					memset(tempBuf, 0, sizeof(tempBuf));
//					k = 0;
//					num_valid = 0;
//					for(j = 1; tmpStr_step[j] != ' '; j++)
//					{
//						
//						if(tmpStr_step[j] == '\0')
//						{
//							break;
//						}
//						
//						tempBuf[k] = tmpStr_step[j];
//						num_valid = 1;
//						k++;
//						
//					}
//					if(num_valid)
//					{
//						tmpCurrent = atoi((const char *)tempBuf);
//						gCfgItems.e0Current = tmpCurrent;
//					}	
//				}
				current_update_flag = 1;
			}
//			if(gCfgItems.sprayerNum != 1)
			{
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M906 T0 E");
				if (tmpStr_step)
				{
					memset(tempBuf, 0, sizeof(tempBuf));
					k = 0;
					num_valid = 0;
					for(j = 9; tmpStr_step[j] != ' '; j++)
					{
						
						if(tmpStr_step[j] == '\0')
						{
							break;
						}
						
						tempBuf[k] = tmpStr_step[j];
						num_valid = 1;
						k++;
						
					}
					if(num_valid)
					{
						tmpCurrent = atoi((const char *)tempBuf);
						gCfgItems.e0Current = tmpCurrent;
					}
					current_update_flag = 1;			
				}
				tmpStr_step = (int8_t *)strstr((const char *)&cmdRxBuf[i], (const char *)"M906 T1 E");
				if (tmpStr_step)
				{
					memset(tempBuf, 0, sizeof(tempBuf));
					k = 0;
					num_valid = 0;
					for(j = 9; tmpStr_step[j] != ' '; j++)
					{
						
						if(tmpStr_step[j] == '\0')
						{
							break;
						}
						
						tempBuf[k] = tmpStr_step[j];
						num_valid = 1;
						k++;
						
					}
					if(num_valid)
					{
						tmpCurrent = atoi((const char *)tempBuf);
						gCfgItems.e1Current = tmpCurrent;
					}
					current_update_flag = 1;			
				}				
			}
			
		}
}


#if 0
void PB0_PB1_pwrdn_mtrdn()
{
	
	//if((gCfgItems.pwrdn_mtrdn_level_flg == 1)&&(gCfgItems.print_finish_close_machine_flg != 1))
	if(gCfgItems.pwrdn_mtrdn_level_flg == 1)
	{
		if(printerStaus == pr_working && (MKS_PWRDN == 0|| MKS_MTRDN == 0))	
		{
			//MKS_PWRDN 
			//MKS_MTRDN 
			//printerStaus = pr_pause

			PWRDN_MTRDN_Tcnt_flg = 1;
			
			if(PWRDN_MTRDN_flg == 1)
			{
				PWRDN_MTRDN_Tcnt_flg = 0;
				PWRDN_MTRDN_flg = 0;
				PWRDN_CNT++;
				if(PWRDN_CNT >=3)
				{
					PWRDN_CNT = 0;
					//**I2C_EE_Init(100000);
					 MX_I2C1_Init(100000);
					clear_cur_ui();
					printerStaus = pr_pause;
					draw_pause();
					mksBpAlrmEn = 1;
				}
			}
		}
		else
		{
			PWRDN_MTRDN_Tcnt_flg = 0;
		}
	}
	//?��???�꨺?
	else
	{
		
		if(printerStaus == pr_working)	
		{
			//??��?PB0
			if(MKS_PWRDN == 0)	
			{
				//MKS_PWRDN ??��?

				PWRDN_MTRDN_Tcnt_flg = 1;
				
				if(PWRDN_MTRDN_flg == 1)
				{
					PWRDN_MTRDN_Tcnt_flg = 0;
					PWRDN_MTRDN_flg = 0;
					PWRDN_CNT++;
					if(PWRDN_CNT >=3)
					{
						PWRDN_CNT = 0;
						//**I2C_EE_Init(100000);
						 MX_I2C1_Init(100000);
						clear_cur_ui();
						printerStaus = pr_pause;
						draw_pause();
						mksBpAlrmEn = 1;
					}
				}
			}
			else
			{
				PWRDN_MTRDN_Tcnt_flg = 0;
			}

			
			if(MKS_MTRDN == 0)
			{
				MTRDN_LOWLEVLE_Tcnt_flg = 1;					
			}

			if(MTRDN_LOWPOWER_FLG == 1)
			{
				if(MKS_MTRDN == 1)
				{
					MTRDN_HIGHLEVEL_Tcnt_flg = 1;
				} 				
			}
			else
			{
				if(MKS_MTRDN == 1)
				{
						MTRDN_LOWLEVLE_Tcnt_flg = 0;
						MTRDN_LOWPOWER_FLG = 0;
						MTRDN_HIGHLEVEL_Tcnt_flg = 0;
						MTRDN_HIGHLEVEL_FLG = 0;
				}
			}
			if(MTRDN_HIGHLEVEL_FLG == 1)//??��???1s
			{
				if(MKS_MTRDN == 1)//
				{
					MTRDN_LOWLEVLE_Tcnt_flg = 0;
					MTRDN_LOWPOWER_FLG = 0;
					MTRDN_HIGHLEVEL_Tcnt_flg = 0;
					MTRDN_HIGHLEVEL_FLG = 0;
					
					//**I2C_EE_Init(100000);
					 MX_I2C1_Init(100000);
					clear_cur_ui();
					printerStaus = pr_pause;
					draw_pause();
					mksBpAlrmEn = 1;
				}
			}
			else
			{
				if((MKS_MTRDN == 0)&&(PWRDN_HIGHLEVEL_Tcnt_flg == 1))
				{
						MTRDN_LOWLEVLE_Tcnt_flg = 0;
						MTRDN_LOWPOWER_FLG = 0;
						MTRDN_HIGHLEVEL_Tcnt_flg = 0;
						MTRDN_HIGHLEVEL_FLG = 0;
				} 				
			}
		} 		
	}
}



void PB4_Close_machine()
{
	if(gCfgItems.print_finish_close_machine_flg == 1)
	{
		//PB4:1��???��2a1��??
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4) == 1)
		{
			if(btn_putdown_cnt >= PUTDOWN_TIME)
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4) == 1)
				{
					btn_putdown_time_flg = 0;
					btn_putdown_cnt = 0;
					rePrintSaveData();
					clear_cur_ui();
					//GUI_SetFont(&FONT_TITLE);
					if(gCfgItems.language == LANG_COMPLEX_CHINESE)
					{
						GUI_SetFont(&GUI_FontHZ16);
					}
					else
					{
						GUI_SetFont(&FONT_TITLE);
					}
					
					if(gCfgItems.language == LANG_ENGLISH)
					{
						GUI_DispStringAt("Closing Machine...", 80, 120);
					}
					else	if(gCfgItems.language == LANG_COMPLEX_CHINESE)
					{
						GUI_DispStringAt("?y?����P?C...", 80, 120);
					}
					else
					{
						GUI_DispStringAt("?y?��1??��...", 80, 120);
					}
					Btn_putdown_close_machine();
					close_fail_flg = 1;
					close_fail_cnt = 0;
					while(close_fail_flg);
					clear_cur_ui();
					//draw_ready_print();
					draw_dialog(DIALOG_TYPE_M80_FAIL);
				}
			}
		}
		else
		{
				btn_putdown_time_flg = 0;
				btn_putdown_cnt = 0;
		} 	
	}

}


#endif


#if 1
void Close_machine_display()
{
	clear_cur_ui();
	disp_state = PRINT_READY_UI;
	GUI_SetBkColor(gCfgItems.background_color);
	GUI_SetColor(gCfgItems.title_color);
	GUI_Clear();
	//GUI_SetFont(&FONT_TITLE);
	#if 0
	if(gCfgItems.language == LANG_COMPLEX_CHINESE)
	{
		GUI_SetFont(&GUI_FontHZ16);
	}
	else
	{
		GUI_SetFont(&FONT_TITLE);
	}
	
	if(gCfgItems.language == LANG_ENGLISH)
	{
		GUI_SetFont(&GUI_FontHZ_fontHz18);
		GUI_DispStringAt("Closing Machine...", 320, 210);
	}
	else	if(gCfgItems.language == LANG_COMPLEX_CHINESE)
	{
		GUI_DispStringAt("�����P�C...", 350, 210);
	}
	else
	{
		GUI_DispStringAt("���ڹػ�...", 350, 210);
	}
	#endif
	#if defined(TFT70)
	GUI_DispStringAt(common_menu.close_machine_tips, 320, 210);	
	#else defined(TFT35)
	GUI_DispStringAt(common_menu.close_machine_tips, 190, 140);	
	#endif
	Btn_putdown_close_machine();
	close_fail_flg = 1;
	close_fail_cnt = 0;
	while(close_fail_flg);
	clear_cur_ui();
	//draw_ready_print();
	draw_dialog(DIALOG_TYPE_M80_FAIL);

}


#define PW_PORT_READ    GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)
#define PW_PORT_READ    GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)
#define MT_PORT_READ    GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)
#define DT_PORT_READ    GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)
extern volatile unsigned char mksBpAlrmEn;



//�ϵ���ϼ��
//�ϵ�:PB0�ϵ��⣬������Ϊ�ڶ���ͷ�Ķ��ϼ��ӿڡ�
//����:PB1���ϼ�⣬
//�ػ�ģ����ܽ�:
//PB4������ͨ�������ļ���ѡ���220DETģ����PWCģ��;
//Ĭ�Ͻ�PWCģ�顣
void PowerOff_Filament_Check()
{
	volatile uint8_t i;

	//�ϵ���
	if(gCfgItems.insert_det_module == 1)//��220detģ�飬�ϵ���
	{
		if((printerStaus == pr_working)&&(gCfgItems.mask_PB0_PB1_Function!=1))//��ӡ���������ͣ����
		{
			if(DT_PORT_READ== 0)
			{
				poweroff_det_flg = 1;
				if(poweroff_det_cnt >= 1000)
				{
					if((DT_PORT_READ==0)&&(gCfgItems.have_ups==1))
					{
						poweroff_det_flg = 0;
						poweroff_det_cnt= 0;
						
						MX_I2C1_Init(100000);
						clear_cur_ui();
						stop_print_time();
						printerStaus = pr_pause;
						//draw_pause();
						if(from_flash_pic==1)
							flash_preview_begin = 1;
						else
							default_preview_flg = 1;						
						draw_printing();
						mksBpAlrmEn = 1;

						return;				
					}

					poweroff_det_flg = 0;
					poweroff_det_cnt= 0;

					Close_machine_display();

					return;
				}
			}
			else
			{
				poweroff_det_flg = 0;
				poweroff_det_cnt= 0;
			}
		}		
	}
	else//��PWC�ػ�ģ��
	{
		if(DT_PORT_READ == 0)//
		{
			poweroff_det_low_flg = 1;//������ʱ
		}
		
		if(poweroff_det_low_cnt >= 80)// 1s����Ϊ�ߵ�ƽ
		{
			if(DT_PORT_READ == 1)
			{
				poweroff_det_low_flg = 0;
				poweroff_det_low_cnt = 0;
				poweroff_det_flg=1;
			}
			else
			{
				poweroff_det_low_flg = 0;
				poweroff_det_low_cnt = 0;
			}
		}
		
		if(poweroff_det_flg==1)
		{
			poweroff_det_high_flg = 1;
			if(poweroff_det_high_cnt >= 80)
			{
				if(DT_PORT_READ == 1)
				{
					if((printerStaus== pr_working)&&(gCfgItems.have_ups==1))//��ӡ������UPS�󱸵�Դ���������ͣ����
					{
						poweroff_det_high_flg = 0;
						poweroff_det_high_cnt = 0;
						poweroff_det_flg=0;

						MX_I2C1_Init(100000);
						clear_cur_ui();
						stop_print_time();
						printerStaus = pr_pause;
						//draw_pause();
						if(from_flash_pic==1)
							flash_preview_begin = 1;
						else
							default_preview_flg = 1;						
						draw_printing();
						mksBpAlrmEn = 1;

						return;
					}
					//ûUPS��Դ��û�ڴ�ӡ��ֱ�ӹػ���
					poweroff_det_high_flg = 0;
					poweroff_det_high_cnt = 0;
					poweroff_det_flg=0;
					Close_machine_display();
					return;					
				}
				else
				{
					poweroff_det_low_flg = 0;
					poweroff_det_low_cnt = 0;
					poweroff_det_high_flg = 0;
					poweroff_det_high_cnt = 0;
					poweroff_det_flg = 0;
				}
			}
		}
	}
	//�ϵ���
	// PB0 as second  filament checking interface 
	if((printerStaus == pr_working)&&(gCfgItems.mask_PB0_PB1_Function!=1))//��ӡ���������ͣ����
	{
		if(gCfgItems.filament_det2_level_flg == 1)//���Ͻӿڽ����ƽΪ�ߵ�ƽ����ʱ�Ĵ���
		{
			//�ߵ�ƽ����ʱ���Ǽ��2s�͵�ƽ��2s�ߵ�ƽ
      if(PW_PORT_READ == 0)
			{
				filament_det2_low_flg = 1;//������ʱ
			}
			
			if(filament_det2_low_cnt >= 3000)// 2s
			{
	        if(PW_PORT_READ == 1)
				{
					filament_det2_high_flg = 1;
					filament_det2_check=1;

					filament_det2_low_flg = 0;
					filament_det2_low_cnt = 0;
				}
				else
				{
					filament_det2_low_flg = 0;
					filament_det2_low_cnt = 0;
				}
			}
				
			if(filament_det2_check ==1)
			{
				if(filament_det2_high_cnt >= 3000)
				{
					if(PW_PORT_READ == 1)
					{
						filament_det2_high_flg= 0;
						filament_det2_high_cnt= 0;
						filament_det2_check=0;
						
						MX_I2C1_Init(100000);
						clear_cur_ui();
						stop_print_time();
						printerStaus = pr_pause;
						//draw_pause();
//						if(from_flash_pic==1)
//							flash_preview_begin = 1;
//						else
//							default_preview_flg = 1;	
						//lan
//						draw_printing();
						draw_dialog(DIALOG_TYPE_FILAMENT_NO_PRESS);
						
						mksBpAlrmEn = 1;

						return;		
					}
					else
					{
						filament_det2_low_flg = 0;
						filament_det2_low_cnt = 0;
						filament_det2_high_flg= 0;
						filament_det2_high_cnt= 0;
						filament_det2_check=0;
					}
				}
			}
		}
		else//���Ͻӿڽ����ƽΪ�͵�ƽ����ʱ����
		{
			if(PW_PORT_READ == 0)
			{
				filament_det2_flg = 1;
				if(filament_det2_cnt >= 3000)
				{
					if(PW_PORT_READ == 0)
					{
						filament_det2_flg = 0;
						filament_det2_cnt= 0;

						MX_I2C1_Init(100000);
						clear_cur_ui();
						stop_print_time();
						printerStaus = pr_pause;
						//draw_pause();
//						if(from_flash_pic==1)
//							flash_preview_begin = 1;
//						else
//							default_preview_flg = 1;	
						//lan
//						draw_printing();
						draw_dialog(DIALOG_TYPE_FILAMENT_NO_PRESS);

						mksBpAlrmEn = 1;

						return;				
					}
				}
			}
			else
			{
				filament_det2_flg = 0;
				filament_det2_cnt= 0;
			}
		}
	}	
	//���ϼ��
	if((printerStaus == pr_working)&&(gCfgItems.mask_PB0_PB1_Function!=1))//��ӡ���������ͣ����
	{
		if(gCfgItems.filament_det1_level_flg == 1)//���Ͻӿڽ����ƽΪ�ߵ�ƽ����ʱ�Ĵ���
		{
			//�ߵ�ƽ����ʱ���Ǽ��2s�͵�ƽ��2s�ߵ�ƽ
      if(MT_PORT_READ == 0)
			{
				filament_det1_low_flg = 1;//������ʱ
			}
			
			if(filament_det1_low_cnt >= 3000)// 2s
			{
        if(MT_PORT_READ == 1)
				{
					filament_det1_high_flg = 1;
					filament_det1_check=1;

					filament_det1_low_flg = 0;
					filament_det1_low_cnt = 0;
				}
				else
				{
					filament_det1_low_flg = 0;
					filament_det1_low_cnt = 0;
				}
			}
				
			if(filament_det1_check ==1)
			{
				if(filament_det1_high_cnt >= 3000)
				{
					if(MT_PORT_READ == 1)
					{
						filament_det1_high_flg= 0;
						filament_det1_high_cnt= 0;
						filament_det1_check=0;
						
						MX_I2C1_Init(100000);
						clear_cur_ui();
						stop_print_time();
						printerStaus = pr_pause;
						//draw_pause();
//						if(from_flash_pic==1)
//							flash_preview_begin = 1;
//						else
//							default_preview_flg = 1;	
						//lan
//						draw_printing();
						draw_dialog(DIALOG_TYPE_FILAMENT_NO_PRESS);
						mksBpAlrmEn = 1;

						return;		
					}
					else
					{
						filament_det1_low_flg = 0;
						filament_det1_low_cnt = 0;
						filament_det1_high_flg= 0;
						filament_det1_high_cnt= 0;
						filament_det1_check=0;
					}
				}
			}
		}
		else//���Ͻӿڽ����ƽΪ�͵�ƽ����ʱ����
		{
			if(MT_PORT_READ == 0)
			{
				filament_det1_flg = 1;
				if(filament_det1_cnt >= 3000)
				{
					if(MT_PORT_READ == 0)
					{
						filament_det1_flg = 0;
						filament_det1_cnt= 0;

						MX_I2C1_Init(100000);
						clear_cur_ui();
						stop_print_time();
						printerStaus = pr_pause;
						//draw_pause();
//						if(from_flash_pic==1)
//							flash_preview_begin = 1;
//						else
//							default_preview_flg = 1;						
						//lan
//						draw_printing();
						draw_dialog(DIALOG_TYPE_FILAMENT_NO_PRESS);

						mksBpAlrmEn = 1;
						return;				
					}
				}
			}
			else
			{
				filament_det1_flg = 0;
				filament_det1_cnt = 0;
			}
		}
	}
}
#endif
/**************************add_end************************/

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
