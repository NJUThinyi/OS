
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
			if (p->ticks > greatest_ticks) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				p->ticks = p->priority;
			}
		}
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

PUBLIC void sys_process_sleep(int milli_sec, SEMAPHORE* sem){
	
}

PUBLIC void sys_my_disp_str(char* str){
	disp_str(str);
}

PUBLIC void sys_P(SEMAPHORE *sem){
	sem->value--;
	if(sem->value<0){
		sem->list[sem->list_len] = p_proc_ready;
		p_proc_ready->flag=1;
		sem->list_len++;
	}
}

PUBLIC void sys_V(SEMAPHORE *sem){
	sem->value++;
	if(sem->value<=0){
		p_proc_ready = sem->list[0];
		p_proc_ready->flag=0;
		for(int i=0;i<sem->list_len;i++){
			sem->list[i] = sem->list[i+1];
		}
		sem->list_len--;
	}
}

