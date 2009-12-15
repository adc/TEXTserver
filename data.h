#define SZ_X 60
#define SZ_Y 30
#define CHARAT(PLYR,x,y) PLYR->location->buf[y*SZ_X + x]

struct loc
{
  time_t mtime, atime;
  int x, y;
  int id;
  struct loc *up,*dn,*lf,*rt;
  int idup, iddn, idlf, idrt;
  int buf[SZ_Y * (SZ_X)]; 
};


struct plyr
{
  int fd;
  int x;
  int y;
  int worldx, worldy;
  int lastdump;
  time_t lastinput;
  struct loc *location;
} plyr;


void *get_world_buf(int x, int y);
