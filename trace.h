/**
 * FreeRTOS trace buffer
 * \file trace.h
 * \brief FreeRTOS trace buffer
*/

#ifndef TRACE_H
#define TRACE_H

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"

#ifndef TRACE_SIZE
#define TRACE_SIZE 256
#endif

#define TRACE_MANUAL 1
#define TRACE_TASK_SWITCHED_IN 2

typedef struct 
{
	uint32_t Cnt;		//CPU tick counter
	uint8_t Id;			//Trace entry type
	uint32_t Par;		//Trace entry parameter
} TraceElement;

extern TraceElement TraceBuffer[];	//Trace buffer containing 'TRACE_SIZE' entries


/**
 * \brief Insert a manual entry in the trace buffer
 * \param Par: User defined parameter stored in the trace buffer
*/
void TraceManual(uint32_t Par);

/**
 * \brief Function called when a task has been switched in by the kernel. Do not call manually.
 * \param TaskNr: Task nr of the task that has been switched in
*/
void TraceTaskSwitchedIn(uint32_t TaskNr);
#endif