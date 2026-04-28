#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os.h"
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "event_groups.h"               // ARM.FreeRTOS::RTOS:Event Groups
#include "semphr.h"                     // ARM.FreeRTOS::RTOS:Core
#include "driver_dht11.h"
#include "usart.h"
#include "stdio.h"

struct DHT11_data
{
	int temp;
	int hum;
};

void DHT11_Task (void *params)
{
	int hum,temp;
	int err;
	
	char cmd_send[400];
	char data_send[400];
	int msg_len=0;
	char temp_send[10];
	char hum_send[10];
	char len_send[10];
	
	struct DHT11_data DHTdata;
	
	DHT11_Init();/*初始化DHT11*/
	while(1)
	{
	    /*读取温湿度数据*/
	    /*暂停调度器*/
	    vTaskSuspendAll();
	    err=DHT11_Read(&hum,&temp);	
	    /*回复调度器*/
	    xTaskResumeAll();
	    DHTdata.temp=temp;
	    DHTdata.hum=hum;
	    if(err==0)
	    {
	    	/*将数据上报给OneNet云服务器*/

				/*处理待发送数据*/
				snprintf(temp_send, sizeof(temp_send), "%d", DHTdata.temp);
				snprintf(hum_send, sizeof(hum_send), "%d", DHTdata.hum);
				sprintf(data_send,"{\"id\":\"123\",\"params\":{\"temp_value\":{\"value\":%s},\"humidity_value\":{\"value\":%s}}}",temp_send,hum_send);
				msg_len=strlen(data_send);
				snprintf(len_send, sizeof(len_send), "%d", msg_len);
				sprintf(cmd_send,"AT+MQTTPUBRAW=0,\"$sys/{产品ID}/{设备名称}/thing/property/post\",%s,0,0",len_send );
				
				/*发送命令和数据包*/
				ESP8266_SendCommand(cmd_send, ">", 2000);	
				ESP8266_SendCommand(data_send, "OK", 5000);
	    }
	    else
	    {
	    	/*在串口打印错误提示*/
	    	printf("Get Data ERROR");
	    }
			
			/*获取当前任务句柄*/
			//xTaskHandle=xTaskGetCurrentTaskHandle();				
			/*获得栈空闲部分的大小*/
			//freenum=uxTaskGetStackHighWaterMark(xTaskHandle);
			//printf("FreeStack of Task %s : %d\n\r",pcTaskGetName(xTaskHandle),freenum);
			
			vTaskDelay(5000);		
	}
}

