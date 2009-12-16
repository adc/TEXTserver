#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "../data.h"

#define SHMKEY  0xADC00ADC

void RM(int shmid)
{
  int tmp;
  struct loc *x;

  if(!shmid) return;
  printf("Removing %d\n", shmid);
  x = shmat(shmid, 0, 0);
  
  if(x != -1){    
    if(!(x->id)){ return; } 
    x->id = NULL;
    RM(x->up);
    RM(x->dn);
    RM(x->lf);
    RM(x->rt);
  } else {
    perror("shmat");
    return;      
  }

  shmctl(shmid, IPC_RMID, 0);

}

int main()
{
  int shmid;
  
  shmid = shmget(SHMKEY, 8192, 0666);
  if(shmid < 0){
    perror("shmget");
    printf("no initial\n");
    exit(-1);
  }
  
  RM(shmid);
}
