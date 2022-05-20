#include "kernel/types.h"
#include "kernel/x86.h"
#include "kernel/defs.h"
#include "kernel/date.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/mmu.h"
#include "kernel/proc.h"
#include "fcntl.h"

int sys_fork(void) { return fork(); }

int sys_getTraceState(void){
    int temp_traceState = 0;
    temp_traceState = cpu->isTraceOn;
    
    return temp_traceState;
}
int sys_setTraceState(void){
    int temp_nice;
    int return_value = 0;
    if (argint(0, &temp_nice) < 0) {
        cpu->isTraceOn = 0;
    } else {
        if (temp_nice == 0){
            cpu->isTraceOn = 0;
            return_value = 0;
        } else if (temp_nice == 1) {
            cpu->isTraceOn = 1;
            return_value = 1;
        } else {
            cpu->isTraceOn = 0;
            return_value = 0;
        }
    }
    return return_value;
}
int sys_setTargetSuccess(void){
    int temp_value;
    int return_value = 0;
    if (argint(0, &temp_value) < 0) {
        proc->target_success_fail = 0;
    } else {
        if (temp_value == 1){ // 1 means only trace successful syscalls
            proc->target_success_fail = 1;
            return_value = 1;
        } else if (temp_value == 2) { // 2 = only trace failed syscalls
            proc->target_success_fail = 2;
            return_value = 2;
        } else if (temp_value == 3) { // 3 = only trace specific syscall
            proc->target_success_fail = 3;
            return_value = 3;
        } else {
            proc->target_success_fail = 0;
            return_value = 0;
        }
    }
    return return_value;
}


int sys_setPIDtoTrace(void){
    int return_value = 0;
    int currpid = proc->pid;
    cpu->nextProcPid = currpid + 1;
    return_value = currpid + 1;
    return return_value;
}

int sys_setSyscallTrack(void){
    int temp_value;
    int return_value = 0;
    if (argint(0, &temp_value) < 0) {
        cpu->trace_syscall = 0;
    } else {
        cpu->trace_syscall = temp_value;
        return_value = temp_value;
    }
    return return_value;
}

int sys_traceDump(void){
    // if no event saved in the array, print "Nothing saved in the event array. "
    if (cpu->isEventArrayUsed == 0){
        cprintf("Error from TraceDump: Nothing saved in the event array. ");
        return -1;
    }
    // Read the array
    int i;
    cprintf("=================== strace dump results ===================== \n");
    for (i = cpu->write_idx; i < cpu->write_idx + N; i++){
        int currIdx = i;
        if (currIdx >= N){
            currIdx = i % N;
        }
       
        if (cpu->record[currIdx].syscall_name){
           cprintf("PID = %d | command = %s | syscall = %s | return_value = %d \n", 
            cpu->record[currIdx].pid, cpu->command_name_arr[currIdx], cpu->record[currIdx].syscall_name, cpu->record[currIdx].return_value);
        }
    }
    int return_value = 0;
    return return_value;
}

// add a system call to make proc.h variable isTraceOn to be 1.
int sys_setTraceRun(void){
    int tempVal;
    int return_value = 0;
    if (argint(0, &tempVal) < 0) {
        proc->isStraceRun = 0; // The default nice value is 0.
    } else {
        if (tempVal == 0){
            proc->isStraceRun = 0;
            return_value = 0;
        } else if (tempVal == 1) {
            proc->isStraceRun = 1;
            return_value = 1;
        } else {
            proc->isStraceRun = 0;
            return_value = 0;
        }
    }
    return return_value;
}


 // TODO: option -o <filename> 
int sys_writeTraceDump(void){
  int fd;
  struct file* f;
  if(argint(0, &fd)<0){
      return -1;
  }
  if(fd<0 || fd>=NOFILE || (f=proc->ofile[fd])==0 )return -1;
  f=filedup(f);
  return sys_traceDump();
  fileclose(f);
  //return strace_dump_file(f);

}

/*int strace_dump_file(struct file * f )
{
    int i;
 for (i = cpu->write_idx; i < cpu->write_idx + N; i++){
        int currIdx = i;
        if (currIdx >= N){
            currIdx = i % N;
        }
       
        if (cpu->record[currIdx].syscall_name){
           cprintf("PID = %d | command = %s | syscall = %s | return_value = %d \n", 
            cpu->record[currIdx].pid, cpu->command_name_arr[currIdx], cpu->record[currIdx].syscall_name, cpu->record[currIdx].return_value);
        }
        fileclose(f);
        return 0;
   
    
}
*/
int sys_exit(void) {
  exit();
  return 0; // not reached
}

int sys_wait(void) { return wait(); }

int sys_kill(void) {
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int sys_getpid(void) { return proc->pid; }

int sys_sbrk(void) {
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

int sys_sleep(void) {
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (proc->killed) {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void) {
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
