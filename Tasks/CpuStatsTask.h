/**
 * CPU load calculation task
 * \file CpuStatsTask.h
 * \brief CPU load calculation task
*/


#ifndef CPU_STATS_TASK_H
#define CPU_STATS_TASK_H

/**
 * \brief Initialize CPU load calculation task
*/
void InitCpuStatsTask(void);

/**
 * \brief Get the current CPU load in %
 * \return CPU load in %
*/
float GetCpuLoad(void);

#endif