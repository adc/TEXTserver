#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "data.h"

#define SHMKEY  0xADC00ADC


void init_world(struct loc *world, int x, int y) {
  int i, j;
  if(world->mtime == 0) {
    printf("init world @ %d,%d\n",x,y);
    world->mtime = time(NULL);  
    world->atime = time(NULL);
    world->x = x;
    world->y = y;

    for(i = 0; i < SZ_X; i++)
      for(j = 0; j < SZ_Y; j++)
        world->buf[j*SZ_X + i] = 0x20;
  }  
}

struct loc* world_start;

struct loc* start_world() {
  // loads world 0 
  int shmid;
  
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
  return world_start;
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

struct loc *link(int *id, int x, int y)
{
  struct loc* ret;
  
  if(*id) { 
    ret = shmat(*id, NULL, 0);
    //clear out links
    if(!ret){
      perror("shmat");
      exit(-1);
    }
  }
  else {
    ret = new_loc(x,y);
    *id = ret->id;
  }
  return ret;
}


void *get_world(struct plyr *player, int x, int y)
{
  int i, j;
  int **b;
  struct loc *p,*q;
  
  //seek to world @ x,y
  if(!world_start) world_start = start_world();

  if(player->location != world_start)
    shmdt(player->location);
    
  for(p = world_start ; p; ) {
    if(p->x == x && p->y == y){
      
     return p;
    }
    printf("seeking to %d, %d from %d, %d\n", x,y, p->x, p->y);
    if(x > p->x){    
      printf("LINK RIGHT\n");
      q = link(&p->rt, p->x+1, p->y);
      q->lf = p->id;
    }
    if(x < p->x){
      printf("LINK LEFT\n");
      q = link(&p->lf, p->x-1, p->y);
      q->rt = p->id;
    }
    if(y > p->y){
      printf("LINK DOWN\n");
      q = link(&p->dn, p->x, p->y+1);
      q->up = p->id;
    }
    if(y < p->y){
      printf("LINK UP\n");
      q = link(&p->up, p->x, p->y-1);
      q->dn = p->id;
    }
    
    if(p != world_start)
      printf("Detaching %p = %d\n",p,shmdt(p));
    p = q;
  }

  return NULL;
}
