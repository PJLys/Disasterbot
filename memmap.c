#include "memmap.h"

#include "FreeRTOS.h"

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include "task.h"

#define MAX_TASKS 20 //Max number of tasks to process


const uint32_t MEM_IO_REGS_START=	0x0000;
const uint32_t MEM_IO_REGS_STOP=	0x0FFF;
const uint32_t MEM_EEPROM_START=	0x1000;
const uint32_t MEM_EEPROM_STOP=		0x1FFF;
const uint32_t MEM_SRAM_START=		0x2000;
const uint32_t MEM_SRAM_STOP=		0x9FFF;

extern uint8_t __data_start[];
extern uint8_t __data_end[];
extern uint8_t __bss_start[];
extern uint8_t __bss_end[];
extern uint8_t __heap_start[];
extern uint8_t *ucHeap;


int SelectNextTask(int PrevAddr, TaskStatus_t *TaskArray);


int cmpfunc (const void * a, const void * b) {
	TaskStatus_t *TaskA, *TaskB;
	TaskA=(TaskStatus_t *) a;
	TaskB=(TaskStatus_t *) b;
	
	if (TaskA->pxStackBase<TaskB->pxStackBase) return -1;
	else if (TaskA->pxStackBase>TaskB->pxStackBase) return 1;
	else return 0;
	
}

void MemMap()
{
	int NumTasks;
	uint8_t a;
	TaskStatus_t TaskStatusArray[MAX_TASKS];
	int TCBSize;
	
	
	NumTasks=uxTaskGetSystemState(TaskStatusArray,MAX_TASKS,NULL);
	qsort(TaskStatusArray, NumTasks, sizeof(TaskStatus_t), cmpfunc);
	
	printf("MEMORY MAP:\r\n");
	printf("-----------\r\n");
	printf("IO registers start:\t\t\t0x%04x\r\n",MEM_IO_REGS_START);
	printf("IO registers end:\t\t\t0x%04x\r\n",MEM_IO_REGS_STOP);
	printf("EEPROM start:\t\t\t\t0x%04x\r\n",MEM_EEPROM_START);
	printf("EEPROM end:\t\t\t\t0x%04x\r\n",MEM_EEPROM_STOP);
	printf("SRAM start:\t\t\t\t0x%04x\r\n",MEM_SRAM_START);
	printf ("\t.DATA start:\t\t\t0x%04x\r\n",__data_start);
	printf ("\t.DATA end:\t\t\t0x%04x\r\n",__data_end-1);
	printf ("\t.BSS start:\t\t\t0x%04x\r\n",__bss_start);
	printf ("\t.BSS end:\t\t\t0x%04x\r\n",__bss_end-1);
	printf ("\t.HEAP start:\t\t\t0x%04x\r\n",ucHeap);
	
	for (a=0;a<NumTasks;a++)
	{
		printf ("\tTask name: %s\r\n",TaskStatusArray[a].pcTaskName);
		printf ("\t\tSTACK end:\t\t0x%04x\r\n",TaskStatusArray[a].pxStackBase);
		printf ("\t\tTCB start:\t\t0x%04x\r\n",TaskStatusArray[a].xHandle);
		TCBSize=(*(uint16_t *) ((uint16_t) (TaskStatusArray[a].xHandle)-2))-4;
		printf ("\t\tTCB end:\t\t0x%04x\r\n",(uint16_t)(TaskStatusArray[a].xHandle)+TCBSize-1);
	}
	
	
	printf ("\t.HEAP end:\t\t\t0x%04x\r\n",ucHeap+configTOTAL_HEAP_SIZE-1);
	printf ("\t.Bare metal STACK end:\t\t0x%04x\r\n",ucHeap+configTOTAL_HEAP_SIZE);
	printf ("\t.Bare metal STACK start:\t0x%04x\r\n",MEM_SRAM_STOP);
	printf("SRAM stop:\t\t\t\t0x%04x\r\n",MEM_SRAM_STOP);
	
	printf("-----------\r\n");
}


