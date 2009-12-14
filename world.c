#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "data.h"

struct loc
{
  time_t mtime, atime;
  int x, y;
  struct loc *up;
  struct loc *dn;
  struct loc *lf;
  struct loc *rt;
  int buf[SZ_Y * (SZ_X)]; 
} loc;

#define SHMKEY  0xADC00ADC

int shmid;
key_t t;
struct loc *world_start;

void init_world(struct loc *world, int x, int y) {
  int i, j;
  if(world->mtime == 0)
  {
    printf("init world\n");
    world->mtime = time(NULL);  
    world->atime = time(NULL);
    world->x = x;
    world->y = y;

    printf("CLEAR CHARS\n");
    for(i = 0; i < SZ_X; i++){
      for(j = 0; j < SZ_Y; j++){
        world->buf[j*SZ_X + i] = 0x20;
      }
    }

  }  

  world->up = NULL;
  world->dn = NULL;
  world->lf = NULL;
  world->rt = NULL;
}

void start_world() {
  // loads world 0 
  shmid = shmget(SHMKEY, 8192, IPC_CREAT | 0666);
  if(shmid < 0){
    perror("shmget");
    exit(-1);
  }
  world_start = shmat(shmid, NULL, 0);
  init_world(world_start, 0, 0);
}

void *get_world(int x, int y)
{
  int i, j;
  int **b;
  struct loc *p;
  if(!world_start) start_world();
  
  //seek to world @ x,y
  
  for(p = world_start; p; p++) {
    if(p->x == x && p->y == y) return p;
    printf("uhoh\n");
  }

  return NULL;
}
