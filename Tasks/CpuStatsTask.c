#include "CpuStatsTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//Private function prototypes
static void WorkerCpuStats(void *pvParameters);

QueueHandle_t CpuInfoQueue;

typedef struct
{
	float CPULoad;
} CpuStatsStruct;

//Function definitions
float GetCpuLoad(void)
{
	CpuStatsStruct CpuStats;
	xQueuePeek(CpuInfoQueue,&CpuStats,portMAX_DELAY);
	return CpuStats.CPULoad;
}

void InitCpuStatsTask()
{
	xTaskCreate( WorkerCpuStats, "cpustat", 256, NULL, tskIDLE_PRIORITY+3, NULL );	
	CpuInfoQueue=xQueueCreate(1,sizeof(CpuStatsStruct));
}

static void WorkerCpuStats(void *pvParameters)
{
	CpuStatsStruct CpuStats;
	static uint32_t TotalRunTime,IdleTime;
	static uint32_t OldTotalRunTime=0,OldIdleTime=0;
	TaskStatus_t IdleTaskStatus;
	
	CpuStats.CPULoad=0;
	while (1)
	{
		//Calc CPU load
		vTaskGetTaskInfo(xTaskGetIdleTaskHandle(),&IdleTaskStatus,pdFALSE,eInvalid);
		IdleTime=IdleTaskStatus.ulRunTimeCounter;
		TotalRunTime=portGET_RUN_TIME_COUNTER_VALUE();
		CpuStats.CPULoad=(1-((float) (IdleTime-OldIdleTime)/ (float)(TotalRunTime-OldTotalRunTime)))*100;

		//Publish CPU stats
		xQueueOverwrite(CpuInfoQueue,&CpuStats);

		//Store run time and idle time for next calculation
		OldIdleTime=IdleTime;
		OldTotalRunTime=TotalRunTime;		
		
		vTaskDelay(100);
			
	}

}
