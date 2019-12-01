
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* EXTERN is defined as extern except in global.c */
#ifdef	GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int		ticks;

EXTERN	int		disp_pos;
EXTERN	u8		gdt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	DESCRIPTOR	gdt[GDT_SIZE];
EXTERN	u8		idt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	GATE		idt[IDT_SIZE];

EXTERN	u32		k_reenter;

EXTERN	TSS		tss;
EXTERN	PROCESS*	p_proc_ready;

EXTERN	int		nr_current_console;

extern	PROCESS		proc_table[];
extern	char		task_stack[];
extern  TASK            task_table[];
extern	irq_handler	irq_table[];
extern	TTY		tty_table[];
extern  CONSOLE         console_table[];

//自己增加的全局变量
EXTERN char input_char[2048];   //保存非查找模式下输入的字符
EXTERN int input_char_position[2048];   //保存非查找模式下输入的字符对应的起始位置
EXTERN int input_char_ptr;  //光标指向处对应字符在input_char中的位置

EXTERN int find_mode;   //0：非查找模式，1：查找模式
EXTERN char find_char[2048];    //保存查找模式下输入的字符
EXTERN int find_ptr;    //查找模式下的对应字符在find_char中的位置

EXTERN int before_find_cursor;  //进入查找模式前光标的位置
// EXTERN u8* p_vmem_copy; //进入查找模式前对显存进行copy

EXTERN int matched_str_num; //每次查找时匹配到的字符串个数
