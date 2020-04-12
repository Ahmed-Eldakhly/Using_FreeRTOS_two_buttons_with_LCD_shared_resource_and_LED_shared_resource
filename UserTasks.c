/*****************************************************************************
 * Module 	  : FreeRTOS User Story2 Tasks Prototypes
 *
 * File name  : UserStory2.h
 *
 * Created on : Oct 7, 2019
 *
 * Author     : Ahmed Eldakhly & Hesham Hafez
 ******************************************************************************/

/*******************************************************************************
 *                       	Included Libraries                                 *
 *******************************************************************************/
#include "UserTasks.h"
#include "semphr.h"

/*******************************************************************************
 *                           static Global Variables                           *
 *******************************************************************************/
static TaskHandle_t  AllHardwareInit_Flag = NULL;

/*Queue between PushButtonTask & LCD Task*/
static xQueueHandle Queue_PushButton_A;

/*Queue between Send Hallo & LCD Task*/
static xQueueHandle Queue_PushButton_B;

/*Mutex Handler*/
xSemaphoreHandle FirstButtonPressed;

/*******************************************************************************
 *                           Global Variables                    		       *
 *******************************************************************************/
TaskHandle_t  InitTask_Flag = NULL;

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

/*******************************************************************************
 * Function Name:	init_Task
 *
 * Description: 	Create all tasks in the system
 *
 * Inputs:			pointer to void
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void init_Task(void * a_Task_ptr)
{
	/*Create Queues*/
	Queue_PushButton_A = xQueueCreate( 3 , sizeof(uint8) );
	Queue_PushButton_B = xQueueCreate( 3 , sizeof(uint8) );

	/*Create Mutex to make one Push Button is pressed*/
	FirstButtonPressed = xSemaphoreCreateMutex();



	/*Create 5 Tasks*/
	/*Task for initialize Hardware components*/
	xTaskCreate(All_Hardware_Init_Task  , "Init_Hw_Tasks" , configMINIMAL_STACK_SIZE ,
			NULL , (6 | portPRIVILEGE_BIT) , &AllHardwareInit_Flag);

	/*Task of Push Button 1*/
	xTaskCreate(PushButtonA_Task  , "PushButtonA_Task" , configMINIMAL_STACK_SIZE ,
			NULL , (4 | portPRIVILEGE_BIT) , NULL);

	/*Task of Push Button 2*/
	xTaskCreate(PushButtonB_Task  , "PushButtonB_Task" , configMINIMAL_STACK_SIZE ,
			NULL , (3 | portPRIVILEGE_BIT) , NULL);

	/*Task for Led*/
	xTaskCreate(LED_Task  , "LED_Task" , 50 ,
			NULL , (2 | portPRIVILEGE_BIT) , NULL);

	/*Task for LCD*/
	xTaskCreate(LCD_Task  , "LCD_Task" , configMINIMAL_STACK_SIZE ,
			NULL , (1 | portPRIVILEGE_BIT) , NULL);

	/*Suspend this task from the system*/
	vTaskSuspend( InitTask_Flag);
}

/*******************************************************************************
 * Function Name:	All_Hardware_Init_Task
 *
 * Description: 	Initialize LCD & KeyPad
 *
 * Inputs:			pointer to void
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void All_Hardware_Init_Task(void * a_Task_ptr)
{

	while(1)
	{
		/*initialize Hardware components*/
		PushButton_Init();
		LCD_init();
		DIO_SetPinDirection(DIO_PIN13,OUTPUT);

		/*Suspend this task from the system*/
		vTaskSuspend( AllHardwareInit_Flag);
	}
}

/*******************************************************************************
 * Function Name:	PushButtonA_Task
 *
 * Description: 	Push Button ATask
 *
 * Inputs:			pointer to void
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void PushButtonA_Task(void * a_Task_ptr)
{
	vTaskDelay(50);
	/* Data is sent to LCD by Queue*/
	uint8 SendData = 1;
	/*de_bouncing Flag to make sure key is pressed*/
	uint8 Debouncing = 0;

	/*comparing value to check if key is pressed*/
	uint8 OldPressedValue = 0;

	while(1)
	{
		/*Check if Button is pressed and enable de_bouncing*/
		if(Debouncing == 0 && Buttons_getPressedButton()==2)
		{
			OldPressedValue =  Buttons_getPressedButton();
			Debouncing++;
			/*de_bouncing Delay*/
			vTaskDelay(10);
		}
		else if(Debouncing == 1)
		{
			/*Check if other push Button is pressed and if not take Mutex*/
			if (xSemaphoreTake(FirstButtonPressed , 10))
			{
				SendData = 1;
				/*check if button is still pressed after de_bouncing time*/
				while(OldPressedValue == Buttons_getPressedButton())
				{
					/*Send push Button was pressed to Lcd by queue*/
					xQueueSend(Queue_PushButton_A , &SendData , 2);
					vTaskDelay(20);
				}
				Debouncing = 2;
			}
		}
		/*chech if button is released*/
		else if(Debouncing == 2)
		{
			/*released Mutex*/
			xSemaphoreGive(FirstButtonPressed);
			SendData = 0;
			/*Send push Button was released to Lcd by queue*/
			xQueueSend(Queue_PushButton_A , &SendData , 2);
			Debouncing = 0;
		}
		else
		{
			/*Send push Button is in Idle state to Lcd by queue*/
			SendData = 3;
			xQueueSend(Queue_PushButton_A , &SendData , 2);
			vTaskDelay(20);
		}
	}
}

/*******************************************************************************
 * Function Name:	PushButtonB_Task
 *
 * Description: 	Push Button B Task
 *
 * Inputs:			pointer to void
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void PushButtonB_Task(void * a_Task_ptr)
{
	vTaskDelay(60);
	/* Data is sent to LCD by Queue*/
	uint8 SendData = 1;
	/*de_bouncing Flag to make sure key is pressed*/
	uint8 Debouncing = 0;

	/*comparing value to check if key is pressed*/
	uint8 OldPressedValue = 0;

	while(1)
	{
		/*Check if Button is pressed and enable de_bouncing*/
		if(Debouncing == 0 && Buttons_getPressedButton()==1)
		{
			OldPressedValue =  Buttons_getPressedButton();
			Debouncing++;
			/*de_bouncing Delay*/
			vTaskDelay(10);
		}
		else if(Debouncing == 1)
		{
			/*Check if other push Button is pressed and if not take Mutex*/
			if (xSemaphoreTake(FirstButtonPressed , 10))
			{
				SendData = 1;
				/*check if button is still pressed after de_bouncing time*/
				while(OldPressedValue == Buttons_getPressedButton())
				{
					/*Send push Button was pressed to Lcd by queue*/
					xQueueSend(Queue_PushButton_B , &SendData , 2);
					vTaskDelay(20);
				}
				Debouncing = 2;
			}
		}
		/*chech if button is released*/
		else if(Debouncing == 2)
		{
			/*released Mutex*/
			xSemaphoreGive(FirstButtonPressed);
			SendData = 0;
			/*Send push Button was released to Lcd by queue*/
			xQueueSend(Queue_PushButton_B , &SendData , 2);
			Debouncing = 0;
		}
		else
		{
			/*Send push Button is in Idle state to Lcd by queue*/
			SendData = 3;
			xQueueSend(Queue_PushButton_B , &SendData , 2);
			vTaskDelay(20);
		}
	}
}
/*******************************************************************************
 * Function Name:	LCD_Task
 *
 * Description: 	LCD Task
 *
 * Inputs:			pointer to void
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void LCD_Task(void * a_Task_ptr)
{
	vTaskDelay(100);
	/*received data from Push Button A Task to display button state on LCD*/
	uint8 PushButton_A_Task_Data = 0;

	/*received message from Push Button B Task to display button state on LCD*/
	uint8 PushButton_B_Task_Data = 0;

	/*LED States & Buttons Strings*/
	uint8 Flag = 0;
	uint8 ButtonA[] = "By BUTTON A";
	uint8 ButtonB[] = "By BUTTON B";
	uint8 LED_ON[] = "LED ON";
	uint8 LED_OFF[] = "LED OFF";

	while(1)
	{
		/*receive messages from other tasks by queues*/
		xQueueReceive(Queue_PushButton_A , &PushButton_A_Task_Data , 1);
		xQueueReceive(Queue_PushButton_B , &PushButton_B_Task_Data , 1);

		/*Check if Button A was pressed*/
		if(PushButton_A_Task_Data == 1)
		{
			if(Flag == 0)
			{
				LCD_clearScreen();
				Flag = 1;
			}
			else if(Flag == 1)
			{
				LCD_displayStringRowColumn(0 , 0 ,LED_ON);
				LCD_displayStringRowColumn(1 , 5 , ButtonA);
				PushButton_A_Task_Data = 0;
			}
			else
			{
				/*Do nothing*/
			}
		}
		/*Check if Button B was pressed*/
		else if(PushButton_B_Task_Data == 1)
		{
			if(Flag == 0)
			{
				LCD_clearScreen();
				Flag = 2;
			}
			else if(Flag == 2)
			{
				LCD_displayStringRowColumn(0 , 0 ,LED_ON);
				LCD_displayStringRowColumn(1 , 5 ,ButtonB);
				PushButton_B_Task_Data = 0;
			}
			else
			{
				/*Do nothing*/
			}
		}
		/*check if No Button was pressed*/
		else
		{
			if((Flag == 1) || (Flag == 2))
			{
				LCD_clearScreen();
				LCD_displayStringRowColumn(0 , 0 ,LED_OFF);
				Flag = 0;
			}
		}
		vTaskDelay(20);

	}
}

/*******************************************************************************
 * Function Name:	LED_Task
 *
 * Description: 	LED on if push button is pressed
 *
 * Inputs:			pointer to void
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void LED_Task(void * a_Task_ptr)
{


	while(1)
	{
		/* check if any Buttons were pressed by take Mutex*/
		if (xSemaphoreTake(FirstButtonPressed , 1))
		{
			/*LED Off*/
			DIO_WritePin(DIO_PIN13 , LOW);

			/*release Mutex*/
			xSemaphoreGive(FirstButtonPressed);
		}
		else
		{
			/*LED On*/
			DIO_WritePin(DIO_PIN13 , HIGH);
		}
		vTaskDelay(10);
	}
}
