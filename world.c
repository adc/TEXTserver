#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "data.h"

#define SHMKEY  0xADC00ADC

int shmid;
key_t t;
struct loc *world_start;

void init_world(struct loc *world, int x, int y) {
  int i, j;
  if(world->mtime == 0)
  {
    printf("init world @ %d,%d\n",x,y);
    world->mtime = time(NULL);  
    world->atime = time(NULL);
    world->x = x;
    world->y = y;

    for(i = 0; i < SZ_X; i++){
      for(j = 0; j < SZ_Y; j++){
        world->buf[j*SZ_X + i] = 0x20;
      }
    }

    world->up = world->dn = world->lf = world->rt = NULL;
  }  

}

void start_world() {
  // loads world 0 
  shmid = shmget(SHMKEY, 8192, IPC_CREAT | 0666);
  if(shmid < 0){
    perror("shmget");
    exit(-1);
  }
  world_start = shmat(shmid, NULL, 0);
  if(world_start->id != shmid){
    printf("NEW !!\n");
    memset(world_start, 0, sizeof(struct loc));
    world_start->id = shmid;
  }
  init_world(world_start, 0, 0);
}

struct loc *new_loc(int x, int y)
{
  struct loc *l;
  int shmid;

  shmid = shmget(IPC_PRIVATE, 8192, IPC_CREAT | 0666);
  if(shmid < 0){
    perror("shmget");
    exit(-1);
  }

  l = shmat(shmid, NULL, 0);
  memset(l, 0, sizeof(struct loc));
  l->id = shmid;
  init_world(l, x, y);
  
  printf("CREATED NEW LOCATION %p @ %d,%d\n", l,x,y);
  return l;
}

struct loc *link(struct loc *cur, struct loc** dir, int x, int y)
{
  if(*dir) return *dir;
  *dir = new_loc(x,y);
  return *dir;
}


void *get_world(int x, int y)
{
  int i, j;
  int **b;
  struct loc *p,*q;
  if(!world_start) start_world();
  
  //seek to world @ x,y
  printf("seeking to %d, %d from %d, %d\n", x,y, p->x, p->y);
  for(p = world_start; p; ) {
    if(p->x == x && p->y == y) return p;

    if(x > p->x){    
       printf("LINK RIGHT\n");
       q = link(p, &p->rt, p->x+1, p->y);
       q->lf = p;
       q->idlf = p->id;
       p->idrt = q->id;
    }
    if(x < p->x){
       printf("LINK LEFT\n");
       q = link(p, &p->lf, p->x-1, p->y);
       q->rt= p;
       q->idrt = p->id;
       p->idlf = q->id;
    }
    if(y > p->y){
       printf("LINK DOWN\n");
       q = link(p, &p->dn, p->x, p->y+1);
       q->up = p;
       q->idup = p->id;
       p->iddn = q->id;
    }
    if(y < p->y){
       printf("LINK UP\n");
       q = link(p, &p->up, p->x, p->y-1);
       q->dn = p;
       q->iddn = p->id;
       p->idup = q->id;
    }
    
    p = q;
  }

  return NULL;
}
