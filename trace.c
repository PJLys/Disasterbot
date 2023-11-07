#include "trace.h"

uint8_t TraceIdx=0;

inline void IncTraceIdx()
{
	TraceIdx++;
	if (TraceIdx==TRACE_SIZE) TraceIdx=0;
}

TraceElement TraceBuffer[TRACE_SIZE];

void TraceManual(uint32_t Par)
{
	TraceBuffer[TraceIdx].Cnt=portGET_RUN_TIME_COUNTER_VALUE();
	TraceBuffer[TraceIdx].Id=TRACE_MANUAL;
	TraceBuffer[TraceIdx].Par=Par;
	IncTraceIdx();
}
void TraceTaskSwitchedIn(uint32_t TaskNr)
{
	TraceBuffer[TraceIdx].Cnt=portGET_RUN_TIME_COUNTER_VALUE();
	TraceBuffer[TraceIdx].Id=TRACE_TASK_SWITCHED_IN;
	TraceBuffer[TraceIdx].Par=TaskNr;
	IncTraceIdx();
}
