#include "TemplateTask.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//Private function prototypes
static void WorkerTemplate(void *pvParameters);

//Function definitions
void InitTemplateTask()
{
	xTaskCreate( WorkerTemplate, "template", 256, NULL, tskIDLE_PRIORITY+3, NULL );	
}

static void WorkerTemplate(void *pvParameters)
{
	while (1)
	{
			
	}

}
