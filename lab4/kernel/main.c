
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
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
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */


		//新增变量的初始化
		p_proc->flag = 0;
		p_proc->ticks=p_proc->priority=i<5?1:10;
		p_proc->sleep_moment = 0;
		p_proc->sleep_ticks = 0;
		p_proc->print_color = 0x07;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	// proc_table[0].ticks = proc_table[0].priority = 150;
	// proc_table[1].ticks = proc_table[1].priority =  50;
	// proc_table[2].ticks = proc_table[2].priority =  30;
	// proc_table[3].ticks = proc_table[3].priority =  25;
	// proc_table[4].ticks = proc_table[4].priority =  20;
	// proc_table[5].ticks = proc_table[5].priority =  0;

	//分配时间片
	proc_table[0].ticks = 20;
	proc_table[1].ticks = 30;
	proc_table[2].ticks = 30;
	proc_table[3].ticks = 30;
	proc_table[4].ticks = 40;
	proc_table[5].ticks = 10;

	for(int i=0;i<6;i++){
		if(i!=5){
			proc_table[i].priority=1;
		}else{
			proc_table[i].priority=10;
		}
	}

	proc_table[0].print_color= 0x01;	//蓝色
	proc_table[1].print_color= 0x02;	//绿色
	proc_table[2].print_color= 0x04;	//红色
	proc_table[3].print_color= 0x06;	//棕色
	proc_table[4].print_color= 0x0D;	//亮紫色
	proc_table[5].print_color= 0x07;	//黑色

	//新增的全局变量初始化
	reader_count=0;
	writer_count=0;

	rmutex.value=1;
	rmutex2.value=1;	//允许读一本书的读者数
	wmutex.value=1;
	S.value=1;
	rw_prio=0;

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

        /* 初始化 8253 PIT */
        out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

        put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
        enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */

	disp_pos=0;
	for(int i=0;i<80*25;i++){
		disp_str(" ");
	}
	disp_pos=0;

	restart();

	while(1){}
}

PUBLIC void reader(int milli_sec, int i){
	const char* names[3]={"Reader_A", "Reader_B", "Reader_C"};
	while(1){
		if(rw_prio==0){	//读者优先
			P(&S);
			P(&rmutex);
			if(reader_count==0){
				P(&wmutex);
			}
			reader_count++;
			V(&rmutex);
			V(&S);
			P(&rmutex2);
			r_w_now=0;
			char* msg="Read Start! Process: ";
			disp_color_str(msg, p_proc_ready->print_color);
			disp_color_str(names[i],p_proc_ready->print_color);
			disp_color_str("\n", p_proc_ready->print_color);
			milli_delay(milli_sec);
			msg="Read End! Process: ";
			disp_color_str(msg, p_proc_ready->print_color);
			disp_color_str(names[i],p_proc_ready->print_color);
			disp_color_str("\n", p_proc_ready->print_color);
			V(&rmutex2);

			P(&rmutex);
			reader_count--;
			if(reader_count==0){
				V(&wmutex);
			}
			V(&rmutex);
		}else if(rw_prio==1){

		}
	}
}

PUBLIC void writer(int milli_sec, int i){
	const char* names[2]={"Writer_D", "Writer_E"};
	while(1){
		if(rw_prio==0){	//读者优先
			P(&S);
			P(&wmutex);
			r_w_now=1;
			char *msg="Write Start! Process: ";
			disp_color_str(msg, p_proc_ready->print_color);
			disp_color_str(names[i-3],p_proc_ready->print_color);
			disp_color_str("\n", p_proc_ready->print_color);
			milli_delay(milli_sec);
			msg="Write End! Process: ";
			disp_color_str(msg, p_proc_ready->print_color);
			disp_color_str(names[i-3],p_proc_ready->print_color);
			disp_color_str("\n", p_proc_ready->print_color);
			V(&wmutex);
			V(&S);
		}else if(rw_prio==1){
			
		}
	}
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	int i = 0;
	while (1) {
		// milli_delay(200);
		reader(200, 0);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	int i = 0x1000;
	while(1){
		// milli_delay(200);
		reader(300, 1);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestC()
{
	int i = 0x2000;
	while(1){
		reader(300, 2);
	}
}

void TestD()
{
	int i = 0x3000;
	while(1){
		writer(300, 3);
	}
}

void TestE()
{
	int i = 0x4000;
	while(1){
		writer(400, 4);
	}
}

void TestF()
{
	int i = 0x5000;
	while(1){
		if(r_w_now==0){
			char* msg="Now: Reading... Reader Process Number: ";
			my_disp_str(msg);
			char* count[4]={"0", "1", "2", "3"};
			// my_disp_str(count[reader_count]);
			switch (reader_count)
			{
			case 0:
				my_disp_str("0");
				break;
			case 1:
				my_disp_str("1");
				break;
			case 2:
				my_disp_str("2");
				break;
			case 3:
				my_disp_str("3");
				break;
			default:
				break;
			}
			my_disp_str("\n");
		}else if(r_w_now==1){
			char* msg="Now Writing...\n";
			my_disp_str(msg);
		}
		process_sleep(100);
	}
}
