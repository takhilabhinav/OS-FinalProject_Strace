#include "kernel/types.h"
#include "kernel/defs.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/mmu.h"
#include "kernel/proc.h"
#include "kernel/x86.h"
#include "kernel/syscall.h"



// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int fetchint(uint addr, int *ip) {
  if (addr >= proc->sz || addr + 4 > proc->sz)
    return -1;
  *ip = *(int *)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int fetchstr(uint addr, char **pp) {
  char *s, *ep;

  if (addr >= proc->sz)
    return -1;
  *pp = (char *)addr;
  ep = (char *)proc->sz;
  for (s = *pp; s < ep; s++)
    if (*s == 0)
      return s - *pp;
  return -1;
}

// Fetch the nth 32-bit system call argument.
int argint(int n, int *ip) { return fetchint(proc->tf->esp + 4 + 4 * n, ip); }

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size n bytes.  Check that the pointer
// lies within the process address space.
int argptr(int n, char **pp, int size) {
  int i;

  if (argint(n, &i) < 0)
    return -1;
  if ((uint)i >= proc->sz || (uint)i + size > proc->sz)
    return -1;
  *pp = (char *)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int argstr(int n, char **pp) {
  int addr;
  if (argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_getTraceState(void);
extern int sys_setTraceState(void);
extern int sys_traceDump(void);
extern int sys_setTraceRun(void);
extern int sys_setTargetSuccess(void);
extern int sys_setPIDtoTrace(void);
extern int sys_setSyscallTrack(void);
extern int sys_writeTraceDump(void);


static int (*syscalls[])(void) = {
    [SYS_fork] sys_fork,   [SYS_exit] sys_exit,     [SYS_wait] sys_wait,
    [SYS_pipe] sys_pipe,   [SYS_read] sys_read,     [SYS_kill] sys_kill,
    [SYS_exec] sys_exec,   [SYS_fstat] sys_fstat,   [SYS_chdir] sys_chdir,
    [SYS_dup] sys_dup,     [SYS_getpid] sys_getpid, [SYS_sbrk] sys_sbrk,
    [SYS_sleep] sys_sleep, [SYS_uptime] sys_uptime, [SYS_open] sys_open,
    [SYS_write] sys_write, [SYS_mknod] sys_mknod,   [SYS_unlink] sys_unlink,
    [SYS_link] sys_link,   [SYS_mkdir] sys_mkdir,   [SYS_close] sys_close,
    [SYS_getTraceState] sys_getTraceState, [SYS_setTraceState] sys_setTraceState,
    [SYS_traceDump] sys_traceDump, [SYS_setTraceRun] sys_setTraceRun, 
    [SYS_setTargetSuccess] sys_setTargetSuccess, [SYS_setPIDtoTrace] sys_setPIDtoTrace, 
    [SYS_setSyscallTrack] sys_setSyscallTrack, [SYS_writeTraceDump] sys_writeTraceDump, 
};
char *syscall_names[] = {
    [SYS_fork] "fork",
    [SYS_exit] "exit",
    [SYS_wait] "wait",
    [SYS_pipe] "pipe",
    [SYS_read] "read",
    [SYS_kill] "kill",
    [SYS_exec] "exec",
    [SYS_fstat] "fstat",
    [SYS_chdir] "chdir",
    [SYS_dup] "dup",
    [SYS_getpid] "getpid",
    [SYS_sbrk] "sbrk",
    [SYS_sleep] "sleep",
    [SYS_uptime] "uptime",
    [SYS_open] "open",
    [SYS_write] "write",
    [SYS_mknod] "mknod",
    [SYS_unlink] "unlink",
    [SYS_link] "link",
    [SYS_mkdir] "mkdir",
    [SYS_close] "close",
    [SYS_getTraceState] "getTraceState",
    [SYS_setTraceState] "setTraceState",
    [SYS_traceDump] "traceDump",
    [SYS_setTraceRun] "setTraceRun",
    [SYS_setTargetSuccess] "setTargetSuccess",
    [SYS_setPIDtoTrace] "setPIDtoTrace",
    [SYS_setSyscallTrack] "setSyscallTrack",
    [SYS_writeTraceDump] "writeTraceDump",    
};




void syscall(void) {
  int num;

  num = proc->tf->eax;
  if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    proc->tf->eax = syscalls[num]();
    if (cpu->isTraceOn == 1 || proc->isStraceRun == 1 || proc->pid == cpu->nextProcPid){
        if (proc->pid != 2 && num != SYS_setTraceState && num != SYS_setTraceRun && num != SYS_traceDump && num != SYS_getTraceState){
            
            // Here, check for each option -> strace -e, -s, -f
            int need_to_trace = 0; // whether to trace and update or not.
            if (proc->target_success_fail == 1){ // -s option
                if (proc->tf->eax != -1 ){
                    cprintf("Trace: pid = %d | command_name = %s | syscall = %s | return_value = %d \n",proc->pid, proc->name, syscall_names[num], proc->tf->eax);
                    need_to_trace = 1;
                }
            } else if (proc->target_success_fail == 2) { // -f option
                if (proc->tf->eax ==-1 ){
                    cprintf("Trace: pid = %d | command_name = %s | syscall = %s | return_value = %d \n",proc->pid, proc->name, syscall_names[num], proc->tf->eax);
                    need_to_trace = 1;
                }
            } else if (proc->target_success_fail == 3){ // -e option
                if (cpu->trace_syscall == num){
                    cprintf("Trace: pid = %d | command_name = %s | syscall = %s | return_value = %d \n",proc->pid, proc->name, syscall_names[num], proc->tf->eax);
                    need_to_trace = 1;
                }
            } else { // No option specified case.
                cprintf("Trace: pid = %d | command_name = %s | syscall = %s | return_value = %d \n",proc->pid, proc->name, syscall_names[num], proc->tf->eax);
                need_to_trace = 1;
            }

            if (need_to_trace == 1){
                cpu->total_event_added = cpu->total_event_added + 1;
                // since we are adding an entry to record array, we need to set isEventArrayUsed to 0. (how we tell empty record OR not.)
                cpu->isEventArrayUsed = 1;
                int temp_write_idx = cpu->write_idx;
                char* sys_name = syscall_names[num];
                char name[20] = {'0'};
                // copy every letter from proc->name to name and then store.
                int lengthProcName = strlen(proc->name);
                int idxLoop;
                for (idxLoop = 0; idxLoop < lengthProcName; idxLoop++){
                    char c = proc->name[idxLoop];
                    name[idxLoop] = c;
                }
                name[lengthProcName] = '\0';
                char arrEmpty[1] = {'\0'};
                strncpy(cpu->command_name_arr[temp_write_idx], arrEmpty, strlen(cpu->command_name_arr[temp_write_idx]));
                strncpy(cpu->command_name_arr[temp_write_idx], name, strlen(name));
                struct event_info p1 = {proc->pid, proc->tf->eax, sys_name};
                cpu->record[temp_write_idx] = p1;
                temp_write_idx++;
                if (temp_write_idx == N) { // if write_idx is equal to N, wrap around to 0. Indices: [0 to N-1]
                    temp_write_idx = 0;
                }
                cpu->write_idx = temp_write_idx; // update the write_idx. 
            }
           
        }
    }
  } else {
    cprintf("%d %s: unknown sys call %d\n", proc->pid, proc->name, num);
    proc->tf->eax = -1;
  }
}
