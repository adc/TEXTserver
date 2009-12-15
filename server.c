#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/* 2d text world */

#define PORT 4091

int createsocket(int portno)
{
  int yes = 1;
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(portno);
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("open socket failed");
		return -1;
	}
	
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("bind failed");
		close(sock);
		return -1;
	}
	
	if (listen(sock, 5) < 0)
	{
		perror("listen failed");
		close(sock);
		return -1;
	}
	
	return sock;
}



int main()
{
  int fd;
  int n;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	
	fd = createsocket(PORT);
  
  while(1){
    n = accept(fd, (struct sockaddr *)&addr, &addrlen);
    printf("[+] Connection from %s\n", inet_ntoa(addr.sin_addr));
    if( 1) { //fork() == 0 ) {
      handle_player(n);
      exit(0);
    }  else {
      close(n);
    }
  }
}

