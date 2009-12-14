#define SZ_X 60
#define SZ_Y 30
#define CHARAT(PLYR,x,y) PLYR->location->buf[y*SZ_X + x]

void *get_world_buf(int x, int y);
