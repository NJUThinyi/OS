
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"


PUBLIC	PROCESS			proc_table[NR_TASKS];

PUBLIC	char			task_stack[STACK_SIZE_TOTAL];

PUBLIC	TASK	task_table[NR_TASKS] = {{TestA, STACK_SIZE_TESTA, "TestA"},
					{TestB, STACK_SIZE_TESTB, "TestB"},
					{TestC, STACK_SIZE_TESTC, "TestC"},
                    {TestD, STACK_SIZE_TESTD, "TestD"},
                    {TestE, STACK_SIZE_TESTE, "TestE"},
                    {TestF, STACK_SIZE_TESTF, "TestF"}};

PUBLIC	irq_handler		irq_table[NR_IRQ];

PUBLIC	system_call		sys_call_table[NR_SYS_CALL] = {sys_get_ticks, sys_process_sleep, sys_my_disp_str, sys_P, sys_V};

//增加的关于读者写者的全局变量
PUBLIC int reader_count;
PUBLIC int writer_count;
PUBLIC struct semaphore rmutex; 
PUBLIC struct semaphore rmutex2; 
PUBLIC struct semaphore wmutex;
PUBLIC struct semaphore S;
PUBLIC int BOOKS;
PUBLIC int rw_prio; //0：读者优先（默认），1：写者优先
PUBLIC int r_w_now; /* 0: 当前为读进程， 1：当前为写进程*/

