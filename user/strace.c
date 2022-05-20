#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/syscall.h"
#include "kernel/fcntl.h"


int getNumberSys(char* word){
    if (strcmp(word, "fork") == 0) return 1;
    else if (strcmp(word, "exit") == 0) return 2;
    else if (strcmp(word, "wait") == 0) return 3;
    else if (strcmp(word, "pipe") == 0) return 4;
    else if (strcmp(word, "read") == 0) return 5;
    else if (strcmp(word, "kill") == 0) return 6;
    else if (strcmp(word, "exec") == 0) return 7;
    else if (strcmp(word, "fstat") == 0) return 8;
    else if (strcmp(word, "chdir") == 0) return 9;
    else if (strcmp(word, "dup") == 0) return 10;
    else if (strcmp(word, "getpid") == 0) return 11;
    else if (strcmp(word, "sbrk") == 0) return 12;
    else if (strcmp(word, "sleep") == 0) return 13;
    else if (strcmp(word, "uptime") == 0) return 14;
    else if (strcmp(word, "open") == 0) return 15;
    else if (strcmp(word, "write") == 0) return 16;
    else if (strcmp(word, "mknod") == 0) return 17;
    else if (strcmp(word, "unlink") == 0) return 18;
    else if (strcmp(word, "link") == 0) return 19;
    else if (strcmp(word, "mkdir") == 0) return 20;
    else if (strcmp(word, "close") == 0) return 21;
    else {
        return 0;
    }
}


int main(int argc, char *argv[]) {
  int i;
  if (argc <= 1) {
    printf(1, "strace: read error\n");
    exit();
  }

  for (i = 1; i < argc; i++) {
    //printf(1, "what was the argument passed in: %s \n", argv[i]);
    
    if (strcmp(argv[i], "on") == 0){
        setTraceState(1);
    }
    if (strcmp(argv[i], "off") == 0){
        setTraceState(0);
        
    }

    if (strcmp(argv[i], "run") == 0){
        setTraceRun(1);
        if (!argv[3]){
                printf(1, "arg[3] does not exist. \n");
                argv[3] = "";
        }
        
        char *runargv[] = {argv[2], argv[3], 0};
        
        exec(argv[2], runargv);
    }
    if (strcmp(argv[i], "dump") == 0){
        setTraceState(0);
        setTraceRun(0);
        traceDump(0);

        if (getTraceState(1) == 1){ // if strace was on, put it back to cpu->isTraceOn = 1.
            setTraceState(1);
        }
    }

    if (strcmp(argv[i], "-e") == 0){
        if (!argv[2]){
            printf(1, "strace: missing argument. Format is: $strace -e [sys_call_name] %s\n", argv[i]);
            exit();
        }
        char *temp = argv[2];
        int numSys = getNumberSys(temp);
        
        if (numSys == 0){
            printf(1, "strace: No such system call. \n");
            exit();
        }
        setTargetSuccess(3); // 3 means only track the syscall that was passed in.
        setPIDtoTrace(1);
        setSyscallTrack(numSys);
        

    }
    if (strcmp(argv[i], "-s") == 0){ // ONLY successful syscall
        setTargetSuccess(1);
        setPIDtoTrace(1);

    }
    if (strcmp(argv[i], "-f") == 0){ // ONLY failed syscall
        setTargetSuccess(2);
        setPIDtoTrace(1);
    }
     /* TODO */
    if (strcmp(argv[i], "-o") == 0){ // -o filename
        if (!argv[2]){
            printf(1, "strace: missing argument. Format is: $strace -o [filename] %s\n", argv[i]);
            exit();
        }
        char *temp = argv[2];
        //printf(1, "filename entered is: ", temp);
       int fd = open(temp,O_CREATE| O_WRONLY);
       if(fd < 0) printf(1, "error\n");
        writeTraceDump(fd);
        close(fd);

    }
  }
  exit();
}
