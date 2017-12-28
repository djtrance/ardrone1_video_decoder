#include <stdlib.h> // for exit() //
#include <netdb.h>
#include <stdio.h>
#include <string.h>   // memset() //

#define DRONE_COMMAND_PORT 5556
#define MAX_MSG 1024

int main(int argc, char *argv[]) {
  
  // declare variables
  int    sd, rc, flags;
  char   command[MAX_MSG];  
  struct sockaddr_in clientAddr;
  struct sockaddr_in droneAddr;
  struct hostent     *h;
  struct timespec    wait_command;
  FILE *fr;

  // initialize variables
  flags = 0;
  wait_command.tv_sec  = 0;
  wait_command.tv_nsec = 5000000;
  
  // check command line args //
  if(argc<3) {
    printf("usage : %s <server name/IP> <filename of command list> \n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // get server IP address (no check if input is IP address or DNS name)
  h = gethostbyname(argv[1]);
  if(h==NULL) {
    printf("%s: unknown host '%s' \n", argv[0], argv[1]);
    exit(EXIT_FAILURE);
  }

  // create structure for ardrone address & port
  droneAddr.sin_family = h->h_addrtype;
  droneAddr.sin_port   = htons(DRONE_COMMAND_PORT);
  memcpy((char *) &droneAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  
  // create structure for this client
  clientAddr.sin_family = AF_INET;
  clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  clientAddr.sin_port = htons(0);

  // socket creation
  sd = socket(AF_INET,SOCK_DGRAM,0);
  if(sd<0) {
    printf("%s: cannot open socket \n",argv[0]);
    exit(EXIT_FAILURE);
  }

  // bind client's the port and address
  rc = bind(sd, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
  if(rc<0) {
    printf("%s: cannot bind port\n", argv[0]);
    exit(EXIT_FAILURE);
  }
    
  // read command from text
  printf("%s: read command from %s \n",argv[0],argv[2]);
  if ((fr = fopen (argv[2], "r")) == NULL) {
		printf("file open error.\n");
		exit(EXIT_FAILURE);
  }
  
  // send commands
  printf("%s: sending commands to  %d\n", argv[0], inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));
  while(fgets(command, sizeof(command), fr) != NULL)
  { 
	 command[strlen(command)-1] = 0x0d;  // change last character from new line to carriage return
	 printf("%s: %s\n",argv[0],command);
     
     rc = sendto(sd, command, strlen(command)+1, flags,  (struct sockaddr *) &droneAddr, sizeof(droneAddr));
     if(rc<0) {
       printf("%s: can not send data\n",argv[0]);
       close(sd);
       exit(EXIT_FAILURE);
     }
     nanosleep(&wait_command , NULL);
  }
  
  fclose(fr);
  
  return 1;

}

