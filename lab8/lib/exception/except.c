#include "../../include/except.h"
#include "../../include/uart.h"
#include "../../include/task.h"

void dumpState(){
	unsigned long esr,elr,spsr;
	asm volatile("mrs %0, esr_el1	\n":"=r"(esr):);
	asm volatile("mrs %0, elr_el1	\n":"=r"(elr):);
	asm volatile("mrs %0, spsr_el1	\n":"=r"(spsr):);

	uart_printf("--------------------\n");
	uart_printf("SPSR: 0x%x\n",spsr);
	uart_printf("ELR: 0x%x\n",elr);
	uart_printf("ESR: 0x%x\n",esr);
	uart_printf("--------------------\n");
	//uart_printf("Exception Return Address: 0x%x\n",elr);
	//uart_printf("Exception Class: 0x%x\n",(esr>>26)&0x3f);//0x15 for svc inst
	//uart_printf("Instruction Specific Syndrome: 0x%x\n",esr&0x1ffffff);//issued imm value for svc inst
}
void x0Set(unsigned long v){
	unsigned long* task;
	asm volatile("mrs %0, tpidr_el1	\n":"=r"(task):);
	task[16]=v;
}

void _except_handler(trap_frame *tf){
    //uart_printf("Sync Exception\n");
    unsigned long esr, svc;
    asm volatile("mrs %0, esr_el1  \n":"=r"(esr):);

    unsigned long x0,x1,x2;


    if(((esr>>26)&0x3f) == 0x15){
        svc = esr & 0x1ffffff;
        switch(svc){
            case 0:
                {
                uart_printf("svc = %d \n",svc);
                dumpState();
                break;
                }
            case 1:
                {
               // uart_printf("svc = %d \n",svc);
                cur_exit();
                break;
                }
            case 2:
                {
                //uart_printf("svc = %d \n",svc);
                exec(tf->regs[0], tf->regs[1]);
                break;
                }
            case 3:
                {
                //uart_printf("svc = %d \n",svc);
                uart_puts(tf->regs[0]);
                tf->regs[0] = tf->regs[1];
                return;
                }
            case 4:
                {
                unsigned long ret;
                ret = uart_gets(tf->regs[0],tf->regs[1],1);
                tf->regs[0] = ret;
                return ;
                }
            case 5:
                {
                //task_struct *cur = get_current();
                int pid = gettid();
                tf->regs[0] = pid;
                return ;
                }
            case 6:
                {
                //uart_printf("parent elr:%x\n",tf->elr_el1);
                sys_fork(tf);
                return;
                }
            case 7://open
                {
                int file_index = sys_open(tf->regs[0],tf->regs[1]);
                tf->regs[0] = file_index;
                return;
                }
            case 8://write
                {
                int length = sys_write(tf->regs[0],tf->regs[1],tf->regs[2]);
                tf->regs[0] = length;
                return;
                }
            case 9://read
                {
                int length = sys_read(tf->regs[0],tf->regs[1],tf->regs[2]);
                tf->regs[0] = length;
                return;
                }
            case 10://close
                {
                int ret = sys_close(tf->regs[0]);
                tf->regs[0] = ret;
                return ;
                }
        }


    }else if(((esr>>26)&0x3f)==0x24){
		unsigned long far;
		asm volatile("mrs %0, far_el1\n":"=r"(far):);
		uart_printf("[Data Abort] pid: %d, far_el1: %x\n",gettid(),far);
		cur_exit();
	}
}


void _TODO_(){
    dumpState();
    while(1){}
}
