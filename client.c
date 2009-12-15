#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "data.h"

int set_byte(struct plyr *p, unsigned int val)
{
  CHARAT(p,p->x,p->y) = val;
  p->lastdump = p->location->mtime + 1;
  p->location->mtime = p->location->mtime + 1;
}

int read_byte(int fd) {
  int ret;
  char buf = 0;
  usleep(100);
  ret = recv(fd, &buf, 1, MSG_DONTWAIT);
  if(ret == 0) { printf("RECV FAIL :(\n"); return -1; }
  if(ret < 0) return 0;
  return buf & 0xff;
}

int write_byte(int fd, char c) {
  return send(fd, &c, 1, 0); 
}

void move_cursor(int fd, int x, int y) {
  char buf[64];
  sprintf(buf, "\x1b\x5b%d;%dH",y+1, x+1);
  send(fd, buf, strlen(buf), 0);
}

void update_loc(struct plyr *p, int newx, int newy)
{
  p->location = (struct loc *)get_world(newx, newy);
  p->worldx = newx;
  p->worldy = newy;
  p->lastdump = 0;
  printf("UPDATE LOCATION to %p @ %d,%d\n", p->location,newx,newy);
  dump_world(p);
}

void move_up(struct plyr *p) {
  p->y--;
  if(p->y < 0){
    p->y = SZ_Y-1;
    update_loc(p, p->worldx, p->worldy-1);
  }
}

void move_down(struct plyr *p) {
  p->y++;
  if(p->y >= SZ_Y){
    p->y = 0;
    update_loc(p, p->worldx, p->worldy+1);
  }
}

void move_left(struct plyr *p) {
  p->x--;
  if(p->x<0){
     p->x = SZ_X-1;
     update_loc(p, p->worldx-1, p->worldy);
  }
}

void move_right(struct plyr *p) {
  p->x++;
  if(p->x >= SZ_X) {
    p->x = 0;
    update_loc(p, p->worldx+1, p->worldy);
  } 
}

void dump_world(struct plyr *p) {
  int i,j;
  for(i = 0; i < SZ_Y; i++){
    move_cursor(p->fd, 0, i);
    for(j = 0; j < SZ_X; j++){
      //printf("displaying char @ %d,%d :: %d\n", i,j, CHARAT(p,i,j));
      write_byte(p->fd, CHARAT(p,j,i));
    }
  }

  move_cursor(p->fd, p->x, p->y);
}

void handle_escapes(struct plyr *p) {
  int z1, z2;
  
  z1 = read_byte(p->fd);
  z2 = read_byte(p->fd);
  printf("escapes: z1=%d z2=%d\n",z1,z2);      
  if(z1 == 91){//arrow keys
    if(z2 == 65){
      move_up(p);
    } else if(z2 == 66){
      move_down(p);
    } else if(z2 == 68){
      move_left(p);
    } else if(z2 == 67){
      move_right(p);
    }
  }
}


void startmsg(int fd) {
  #define MSG "\n\n\n\nHi! Welcome to the world of infinite 2d text.\n"\
              " Hit any key to start. You might want to try a client\n" \
              " which supports character mode (vs line mode). Try telnet\n"\
              " and hit ctrl+] followed by 'm c' for mode character\n"\
              " Hit any key to start\n."
              
  send(fd, MSG, strlen(MSG), 0);
  #undef MSG
}


int handle_input(struct plyr *p){
    int z;
        
    z = read_byte(p->fd);

    if(z == -1){
      return 1;
    }

      
    if(z == 0 || z == 1) return 0;
    p->lastinput = time(NULL);

    printf("user input: @ (%d,%d) = %d\n",p->x,p->y,z);
   
    if(z == 0x1b){
      handle_escapes(p);
    }
    else if(z == 253U)
    {
      read_byte(p->fd); //3
    }
    else if(z == 255U){
      read_byte(p->fd); //1
      dump_world(p);
    } else if(z == 126 || z == 127)
    {
      move_left(p);
      set_byte(p, ' ');
      CHARAT(p,p->x,p->y) = ' ';
    }
    else if(z == 13)
    {
      move_down(p);
    } else if(z == 10){
     
    } else {
      set_byte(p, z);
      move_right(p);
    }

  return 0;    
}

void check_updates(struct plyr *p) {
  if(p->lastdump != p->location->mtime)
    if(p->lastinput < time(NULL))
      dump_world(p);
  p->lastdump = p->location->mtime;
}

void handle_player(int sock) {
  int z, prevx, prevy;
  struct plyr *p = malloc(sizeof(struct plyr));
  
  p->fd = sock;
  p->x = p->y = p->lastdump = 0;
  p->worldx = p->worldy = 0;
  p->location = (struct loc *) get_world(0,0);

  if(p->location == NULL)
  {
    printf("FAILED TO GET WORLD!\n");
    close(p->fd);
    return;
  }

  startmsg(p->fd);
  
  while( read_byte(p->fd) == 0);
  dump_world(p);
  
  while(1){
    prevx = p->x;
    prevy = p->y;   
  
    //handle input
    if(handle_input(p)){
     printf("disconnecting client\n");
     break;   
    }

    //display
    if(p->x != prevx || p->y != prevy)
    {
      //printf("trying to display @ %d, %d: %d\n", prevx, prevy, CHARAT(p,prevx, prevy));
      write_byte(p->fd, CHARAT(p,prevx, prevy));
    
      move_cursor(p->fd, p->x, p->y);
      //printf("now at %d,%d: %d\n",p->x,p->y, CHARAT(p,p->x,p->y));
      
    }
    
    check_updates(p);
    
  } 
 
  send(p->fd, "Goodbye\n", 8, 0);
  close(p->fd); 
}

