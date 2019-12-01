
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);

//添加定时（20s）清屏函数
PUBLIC void clean_screen();
//添加查找函数
PUBLIC void do_search(TTY* p_tty);
//添加查找模式显示函数
PUBLIC void find_show(TTY* p_tty);
//添加查找模式显示之后的恢复函数
PUBLIC void recover(TTY* p_tty);

int char_start_positions[SCREEN_WIDTH*25];	//存储匹配的字符串的起始位置

/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	//初始化清屏
	CONSOLE* p_con = p_tty->p_console;
	int start = p_con->original_addr;
	int end = p_con->cursor;
	u8* p_vmem = (u8*)(V_MEM_BASE+p_con->original_addr*2);
	// char* p_vmem = (char *)(V_MEM_BASE+p_con->cursor*2);
	for(int i = start;i<end;i++){
		*p_vmem++=' ';
		*p_vmem++=DEFAULT_CHAR_COLOR;
		p_con->cursor--;		
	}
	
	//移动光标到屏幕左上角
	set_cursor(p_con->cursor);

	//增加对input_char_ptr的初始化
	input_char_ptr = 0;
	find_ptr=0;
}


/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

	switch(ch) {
	case '\n':
		if(!find_mode){
			if (p_con->cursor < p_con->original_addr +
				p_con->v_mem_limit - SCREEN_WIDTH) {
				//输入字符列表添加换行符
				input_char[input_char_ptr] = '\n';
				input_char_position[input_char_ptr] = p_con->cursor;
				input_char_ptr++;

				p_con->cursor = p_con->original_addr + SCREEN_WIDTH * 
					((p_con->cursor - p_con->original_addr) /
					SCREEN_WIDTH + 1);
				
			}
		}else{
			
		}
		break;
	case '\b':
		if(!find_mode){
			if (p_con->cursor > p_con->original_addr) {
				//原始代码
				// p_con->cursor--;
				// *(p_vmem-2) = ' ';
				// *(p_vmem-1) = DEFAULT_CHAR_COLOR;

				//修改后的退格键代码
				input_char_ptr--;
				input_char[input_char_ptr] = 0;
				while(p_con->cursor > input_char_position[input_char_ptr]){
					p_con->cursor--;
					*(p_vmem-2) = ' ';
					*(p_vmem-1) = DEFAULT_CHAR_COLOR;
				}
			}
		}else{
			find_ptr--;
			find_char[find_ptr]=0;
			while(p_con->cursor > find_char_position[find_ptr]){
				p_con->cursor--;
				*(p_vmem-2)=' ';
				*(p_vmem-1)=FIND_CHAR_COLOR;
			}
		}
		break;
	//TAB键的输出处理
	case '\t':
		if(!find_mode){
			if (p_con->cursor <
				p_con->original_addr + p_con->v_mem_limit - 4) {
					input_char[input_char_ptr] = ch;
					input_char_position[input_char_ptr] = p_con->cursor;
					input_char_ptr++;
					for(int i=0;i<4;i++){
						*p_vmem++ = ' ';
						*p_vmem++ = DEFAULT_CHAR_COLOR;
						p_con->cursor++;
					}
			}
		}else{
			if (p_con->cursor <
				p_con->original_addr + p_con->v_mem_limit - 4) {
					find_char[find_ptr] = ch;
					find_char_position[find_ptr]=p_con->cursor;
					find_ptr++;
					for(int i=0;i<4;i++){
						*p_vmem++ = ' ';
						*p_vmem++ = FIND_CHAR_COLOR;
						p_con->cursor++;
					}
				}
		}
		break;
	default:
		if(!find_mode){
			if (p_con->cursor <
				p_con->original_addr + p_con->v_mem_limit - 1) {
				//增加对屏幕字符及其对应起始位置的保存
				input_char[input_char_ptr] = ch;
				input_char_position[input_char_ptr] = p_con->cursor;
				input_char_ptr++;

				//原始代码
				*p_vmem++ = ch;
				*p_vmem++ = DEFAULT_CHAR_COLOR;
				p_con->cursor++;			
			}
		}else{
			if (p_con->cursor <
				p_con->original_addr + p_con->v_mem_limit - 1) {
				//增加对屏幕字符及其对应起始位置的保存
				find_char[find_ptr]=ch;
				find_char_position[find_ptr]=p_con->cursor;
				find_ptr++;

				//原始代码
				*p_vmem++ = ch;
				*p_vmem++ = FIND_CHAR_COLOR;
				p_con->cursor++;			
			}
		}
		break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}

//定时（20s）清屏函数
PUBLIC void clean_screen(){
	disable_int();

	TTY* p_tty = tty_table;

	//初始化清屏
	CONSOLE* p_con = p_tty->p_console;
	int start = p_con->original_addr;
	int end = p_con->cursor;
	u8* p_vmem = (u8*)(V_MEM_BASE+p_con->original_addr*2);
	// char* p_vmem = (char *)(V_MEM_BASE+p_con->cursor*2);
	for(int i = start;i<end;i++){
		*p_vmem++=' ';
		*p_vmem++=DEFAULT_CHAR_COLOR;
		p_con->cursor--;		
	}
	
	//移动光标到屏幕左上角
	set_cursor(p_con->cursor);

	//增加对input_char_ptr的初始化,对输入存储数组初始化
	for(int i = 0; i<input_char_ptr;i++){
		input_char[i]=' ';
		input_char_position[i]=0;
	}
	input_char_ptr = 0;
	enable_int();
}

/*存在的问题：
	- 换行只能查询到第一个
	- TAB键+字母会将紧跟着的4位字母变色
	- 连续相同字母无法全部变色
*/
PUBLIC void do_search(TTY* p_tty){
	int match_num=0;	//匹配的字符数，等于find_ptr时代表匹配成功
	for(int i=0;i<input_char_ptr;i++){
		for(int j=0;j<find_ptr;j++){
			if(input_char[i+j]==find_char[j]){
				match_num++;
			}else{
				match_num=0;
				break;
			}
		}
		if(match_num==find_ptr){
			char_start_positions[matched_str_num]=i;
			matched_str_num++;
			match_num=0;
		}
	}
	for(int i=0;i<find_ptr;i++){
		char_start_positions[matched_str_num]=before_find_cursor;
		matched_str_num++;
	}
}

PUBLIC void find_show(TTY* p_tty){
	CONSOLE* p_con = p_tty->p_console;
	int start = p_con->original_addr;
	int end = p_con->cursor;
	u8* p_vmem = (u8*)(V_MEM_BASE+p_con->original_addr*2);
	int count=0;
	int i=start;

	while(i<end){
		if(i==char_start_positions[count]){
			for(int j=0;j<find_ptr;j++){
				if(find_char[count]=='\t'){
					for(int k=0;k<4;k++){
						*p_vmem++;
						*p_vmem++ = FIND_CHAR_COLOR;
						i++;
					}
				}else{
					*p_vmem++;
					*p_vmem++ = FIND_CHAR_COLOR;
					i++;
				}
			}
			count++;
		}else{
			*p_vmem++;
			*p_vmem++;
			i++;
		}
	}
}

PUBLIC void recover(TTY* p_tty){
	CONSOLE* p_con = p_tty->p_console;
	int start = p_con->original_addr;
	int end = p_con->cursor;
	u8* p_vmem = (u8*)(V_MEM_BASE+p_con->original_addr*2);
	for(int i=0;i<before_find_cursor;i++){
		*p_vmem++;
		*p_vmem++=DEFAULT_CHAR_COLOR;
	}
	for(int i=before_find_cursor;i<end;i++){
		*p_vmem++=' ';
		*p_vmem++=DEFAULT_CHAR_COLOR;
		p_con->cursor--;
	}
	set_cursor(before_find_cursor);

	//初始化
	find_mode=0;
	for(int i=0;i<find_ptr;i++){
		find_char[i]=0;
	}
	find_ptr=0;
	for(int i=0;i<matched_str_num;i++){
		char_start_positions[i]=0;
	}
	matched_str_num=0;
	before_find_cursor=0;
}