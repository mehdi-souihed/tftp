#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>

#include "triviald.h"
#include "common.h"

/*******************************************************************************
server tftp
*******************************************************************************/
int verboseFlag = 0;


int server_tftp(int port)
{
    struct sockaddr_in client_addr, my_addr, tmp_addr;
    fd_set read_fd;
    int fdmax, sock, newSock;
    char buffer[BUFMAX], expected[BUFMAX];
    int addr_len = sizeof(struct sockaddr);
    int pid, i, msg_length;


    setNetInfo(&my_addr, NULL, port);

    /* get a socket */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	perror("socket");
	exit(1);
    }
    /* bind it */
    if (bind(sock, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
	== -1) {
	perror("bind");
	exit(1);
    }

    signal(SIGCHLD, freechild);

    printf("TFTP Server Listening on port %d\n", port);
    memset(buffer, 0, sizeof buffer);
    
    //clearing the sets
    FD_ZERO(&read_fd);
    //adding the listening socket
    FD_SET(sock, &read_fd);
    fdmax = sock;
    i = 0;

    while (1) {

	if (select(fdmax + 1, &read_fd, NULL, NULL, NULL) == -1) {
	    if (errno != EINTR) {
		perror("select");
		exit(4);
	    } else
		continue;
	}

	if (FD_ISSET(sock, &read_fd)) {
	  
	    pid = fork();
	    
	    if (pid == 0) {
	      
	      initChild(&tmp_addr, &newSock);
	      //forwarding data to the child
	      msg_length = recvfrom(sock, buffer, BUFMAX, 0,
				    (struct sockaddr *) &client_addr,
				    &addr_len);
	      if (msg_length == -1)
		perror("recvfrom");
	      
	      msg_length =
		sendto(newSock, buffer, BUFMAX, 0,
		       (struct sockaddr *) &tmp_addr,
		       sizeof(struct sockaddr));
	      
	      if (msg_length == -1)
		perror("sendto");
	      
	      if(!msg_length)
		_exit(1);
	      
	      clone_server(client_addr, newSock);
	      printf("Communication finished.\n");
	      return 0;
	    }
	    
	    if (pid > 0)
		continue;
	}
    }
    close(sock);
    return 0;
}

int initChild(struct sockaddr_in *tmp_addr, int *newSock)
{
    int try_port = 8012;	// chooseport();

    if ((*newSock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	perror("socket");
	exit(1);
    }

    setNetInfo(tmp_addr, "127.0.0.1", try_port);
    
    // choosing the first port available
    
    while (1) {

      if (bind
	  (*newSock, (struct sockaddr *) tmp_addr,
	   sizeof(struct sockaddr))
	  == -1) {
	if (errno == EADDRINUSE) {
	  try_port++;
	  tmp_addr->sin_port = try_port;
	}
	
	else {
	  perror("bind");
	  exit(1);
	}
      } else
	break;
    }
    return 0;
}


int clone_server(struct sockaddr_in client_addr, int newSock)
{
  struct sockaddr_in tmp_addr;
  char buffer[BUFMAX];
  int addr_len = sizeof(struct sockaddr_in);
  int fd,test,check,msg_length,checktime,fdmax;
  int NbBlock = 0;
  tftp_msg stored;
  struct timeval t_timeout;
  fd_set listening;
  
    FD_ZERO(&listening);
    FD_SET(newSock, &listening);
    fdmax = newSock;
    stored.previous_state = 0;
    
    while (1) {
      t_timeout.tv_sec = 25;
      t_timeout.tv_usec = 0;
      
      stored.size = recvfrom(newSock, buffer, BUFMAX, 0,
			     (struct sockaddr *) &tmp_addr, &addr_len);
      
      	VRB {
	printf("S:[received]\n");
	display_data(buffer);
	printf("\n");
	}
      
      check = tftp(SERVER, &stored, buffer, buffer);
      
      if (sendto(newSock, buffer, stored.size, 0,
		 (struct sockaddr *) &client_addr,
		 sizeof(struct sockaddr)) == -1) {
	perror("sendto");
	_exit(1);
      }
      
      VRB {
	printf("S:[sent]\n");
	display_data(buffer);
	printf("\n");
      }
      
      if (check == 0 || check == -1)
	    break;

	memset(buffer, 0, BUFMAX);

	checktime = select(fdmax + 1, &listening, NULL, NULL, &t_timeout);

	if (checktime == -1) {
	    perror("select");
	    _exit(4);
	}

	if (checktime == 0) {
	  printf("timeout!\n");
	  _exit(0);
	}
    }
    
    return 0;
}

void freechild(int location){
    int status = 1;
    
    while (status)
      wait3(&status, WNOHANG, NULL);
    
}

void help(){
  
  printf("\tCommand List\n");
  printf("--help,    -h \t Displays this help.\n");
  printf("--port,    -p \t Local port.\n");
  printf("--verbose  -v \t  Verbose mode.\n");   
}


int main(int argc, char *argv[])
{
  int nb_option = 0;
  extern char *optarg;
  int parameter;
  int port = DEFAULT_SERV_PORT;
  
    const struct option long_options[] = {
	{"help", no_argument, NULL, 'h'},
	{"port", optional_argument, NULL, 'p'},
	{"v", no_argument, &verboseFlag, 1},
	{NULL, 0, NULL, 0}	/* Required at end of array.  */
    };


    while ((parameter =
	    getopt_long(argc, argv, "hvp:", long_options,
			NULL)) != -1) {
      switch ((char) parameter) {

      case 'h':
	help();
	exit(0);
	break;
      case 'p':
	port = atoi(optarg);
	break;
	
      case 'v':
	verboseFlag = 1;
	break;
	
      default:
	help();
	exit(0);
	break;
      }
    }
    
    if (port < 0 || port > 65536){
      printf("Invalid port !\n");
      exit(0);
    }
    
    server_tftp(port);
}
 
