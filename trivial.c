#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <getopt.h>


#include "trivial.h"
#include "common.h"
/******************************************************************************
client tftp
******************************************************************************/

int verboseFlag = 0;

int client_tftp(char *host, int port, char *file, short request, unsigned char rexmt){
  struct sockaddr_in remote_addr, my_addr;
  int socketid,msg_length,checktime,fdmax,test;
  char buf[BUFMAX];
  int addr_len = sizeof(struct sockaddr);
  int NbBlock = 0;
  int check = 0;
  tftp_msg stored;
  struct timeval t_retx;
  fd_set listening;
  options chosed;
  int nb_retx = 0;
  int position = 2;

  // setting net informations
  setNetInfo(&my_addr, NULL, 0);
  setNetInfo(&remote_addr, host, port);
  
  // structure managing the parameters
  setFileStored(file, &stored);
  stored.action_done = request;
  stored.Nblock = 0;
  stored.previous_state = request;
  stored.size = BUFMAX;
  
  chosed.blksize = 1300;
  chosed.rexmt = rexmt;
  
  if ((socketid = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {	/* On veut une socket UDP */
    perror("socket");
    exit(1);
  }
  
  if (bind
      (socketid, (struct sockaddr *) 
       &my_addr, sizeof(struct sockaddr))== -1) {
    perror("bind");
    exit(1);
  }
  
  FD_ZERO(&listening);
  FD_SET(socketid, &listening);
  fdmax = socketid;
  
    // checking if the file already exists in the client
  if(request == RRQ){
    fdmax = tftp_create(file,&test);
    if(fdmax == -1)
      exit(1);
    else remove(file);
  }
  
  setOptions(chosed,strlen(file)+strlen(MODE)+4,buf);
  make_msg(request, file, NULL, 0, buf);

  
  while (1) {
    
    VRB {
    printf("C:[sent]\n");
    display_data(buf);
      printf("\n");
    }
    
      msg_length =
	sendto(socketid, buf, stored.size, 0,
	       (struct sockaddr *) &remote_addr,
	       sizeof(struct sockaddr));
	
      if (msg_length == -1) {
	perror("sendto");
	  exit(1);
      }
      
      if (check == -1)
	exit(1);
	
	t_retx.tv_sec = rexmt;
	t_retx.tv_usec = 0;
	
	checktime = select(fdmax + 1, &listening, NULL, NULL, &t_retx);

	if (checktime == 0) {
	  nb_retx++;
	  printf("Server not answering... reattempting(%d)\n", nb_retx);
	  if (nb_retx == 5) {
	    printf("Server not answering... closing\n", nb_retx);
	    exit(0);
	  }
	  continue;
	}

	memset(buf, 0, BUFMAX);
	stored.size =
	    recvfrom(socketid, buf, BUFMAX, 0,
		     (struct sockaddr *) &remote_addr, (int *) &addr_len);

	VRB {
	printf("C:[received]\n");
	display_data(buf);
	printf("\n");
	}
	
	check = tftp(CLIENT, &stored, buf, buf);
	if (check == 0 || check == -1)
	    exit(0);
    }
    close(socketid);
    return 0;

}

void help(){
  
  printf("\tCommand List\n");
  printf("--help,    -h \t Displays this help.\n");
  printf("--read,    -r \t Requesting file to server.\n");
  printf("--write,   -w \t Sending file to server.\n");
  printf("--host,    -H \t Host you want to communicate with.\n");
  printf("--file,    -f \t File you want to receive/send.\n");
  printf("--port,    -p \t Local port.\n");
  printf("--verbose  -v \t  Verbose mode.\n");   
}



int main(int argc, char *argv[])
{
    char *file = NULL, *host = NULL;
    int nb_option = 0;
    extern char *optarg;
    int parameter;
    int request = -1;
    int port = DEFAULT_SERV_PORT;
    int rexmt = 5;


    const struct option long_options[] = {
	{"help", no_argument, NULL, 'h'},
	{"read", no_argument, NULL, 'r'},
	{"write", no_argument, NULL, 'w'},
	{"host", required_argument, NULL, 'H'},
	{"file", required_argument, NULL, 'f'},
	{"port", required_argument, NULL, 'p'},
	{"verbose", no_argument, NULL, 'v'},
	{"rexmt", required_argument, NULL, 't'},
	{NULL, 0, NULL, 0}	/* Required at end of array.  */
    };


    while ((parameter =
	    getopt_long(argc, argv, "hrwvp:H:f:t:", long_options,
			NULL)) != -1) {
	switch ((char) parameter) {

	case 'r':
	    request = RRQ;
	    break;
	case 'w':
	    request = WRQ;
	    break;

	case 'f':
	    file = malloc(strlen(optarg) + 1);
	    strcpy(file, optarg);
	    break;
	case 'H':
	    host = malloc(strlen(optarg) + 1);
	    strcpy(host, optarg);
	    break;

	case 't':
	  rexmt = atoi(optarg);
	  break;

	case 'p':
	    port = atoi(optarg);

	    break;
	    
	case 'v':
	  verboseFlag = 1;
	  break;


	case 'h':
	  help();
	  exit(0);
	  break;
	
	default:
	  help();
	  exit(0);
	  break;
	}
    }

    if (request != RRQ && request != WRQ) {
	printf("Request option must be set!\n");
	exit(0);
    }

    if (file == NULL) {
	printf("Option -f must be specified.\n");
	exit(0);
    }

    if (host == NULL) {
	printf("Option -H must be specified.\n");
	exit(0);
    }

    if(rexmt < 0 || rexmt > 255){
      printf("Incorrect rexmt value! \n");
      rexmt = 5;
    }
    
    if(port < 0 || port > 65536){
      printf("Incorrect port value !\n");
      exit(0);
    }

    
    client_tftp(host, port, file, request, rexmt);


}
