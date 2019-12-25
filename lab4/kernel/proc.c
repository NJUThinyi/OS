
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "string.h"
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
			if(p->flag==0){
				if (p->ticks > greatest_ticks) {
					greatest_ticks = p->ticks;
					p_proc_ready = p;
				}
			}
		}

		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				if(p->flag==0){
					p->ticks = p->priority;
				}
			}
		}
	}
}

PUBLIC void schedule_with_sleep(PROCESS* s_p){
	PROCESS*	p;
	int		greatest_ticks = 0;
	
	while ((!greatest_ticks)) {
LOOP:	for (p = &proc_table; p <= &proc_table[NR_TASKS-1]; p++) {
			if (p->flag == 0) {
				if (p->ticks > greatest_ticks) {
					greatest_ticks = p->ticks;
					p_proc_ready = p;
				}
			}   
		}
			
		if (!greatest_ticks){
			for (p = &proc_table; p <=  &proc_table[NR_TASKS-1]; p++){
				if (p->flag == 0){
					p->ticks = p->priority;
				}
			}
		}
		
		if(p_proc_ready == s_p && (get_ticks() - p_proc_ready->sleep_moment < p_proc_ready->sleep_ticks)){
				goto LOOP;//如果执行上面的循环，得到应执行的进程是睡眠进程，且已经睡眠的时间小于规定睡眠时间，重新调度
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

PUBLIC void sys_process_sleep(int milli_sec, struct s_proc* p){
	int now=get_ticks();
	p->sleep_moment=now;
	int seconds =  milli_sec * HZ /1000;
	p->sleep_ticks = seconds;
	schedule_with_sleep(p);
}

PUBLIC void sys_my_disp_str(char* str){
	disp_str(str);
}

PUBLIC void sys_P(struct semaphore *sem){
	disp_str("P...S");	
	sem->value--;
	if(sem->value<0){
		disp_str("P...R");
		sem->list[sem->list_len] = p_proc_ready;
		p_proc_ready->flag=1;
		sem->list_len++;
		schedule();
	}
	disp_str("P...E");
}

PUBLIC void sys_V(struct semaphore *sem){
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

