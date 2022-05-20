#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int main(void) {
    setTraceState(1);
    if (fork() == 0) {
        printf(1, "child %d\n", getpid());
    } else {
        wait();
        printf(1, "parent %d\n", getpid());
    }
int shared;
fork(); 
if(getpid() == 1)
 { 
     shared = 1;
     } 
if (getpid() == 0 )
{ 
    shared = 0;
    } 
printf(1,"%d\n",shared);
    exit();
}
