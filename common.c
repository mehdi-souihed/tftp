#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
//#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "common.h"


//int MSG_SIZE=512;

/*
  functions both server and client tftp have in common
*/

int setNetInfo(struct sockaddr_in *netData, char *host, int port)
{
    struct hostent *rel;

    if (host != NULL) {
	/* verifying the server */
	if ((rel = gethostbyname(host)) == NULL) {
	    printf("Can't find server '%s'!\n", host);
	    exit(EXIT_FAILURE);
	}

	(netData->sin_addr) = *((struct in_addr *) rel->h_addr);
    } else
	(netData->sin_addr.s_addr) = INADDR_ANY;

    (netData->sin_family) = AF_INET;
    (netData->sin_port) = htons(port);
    bzero(netData->sin_zero, 8);
    return 0;
}

void display_data(char *buf){
  char *res = "test";
  char flag[5];


  switch (getFlag(buf)){ 
    
  case RRQ: 
    printf("Flag: RRQ\n");

    break;
    
  case WRQ:
    printf("Flag: WRQ\n");
    
    break;
    
  case DATA:
    printf("Flag: DATA\n");
    break;
    
  case OACK:
    printf("Flag: OACK\n");
    break;

  case ERROR:
    printf("Flag: ERROR\n");
    break;
  
  default:
    printf("Unknown flag !\n");

  }
  
  
  switch (getFlag(buf)) {
  case RRQ:
    printf("File: %s\n", buf + 2);
	break;
	
  case WRQ:
	printf("File: %s\n", buf + 2);
	break;
	
  case ACK:
    printf("Block: %d \n", getNblock(buf));
    break;
    
  case DATA:
    printf("Block: %d \n", getNblock(buf));
    //    printf("Data: %s\n", getData(buf,res));
    break;
    
  case ERROR:
    printf("Error: %s\n", getData(buf, res));
    break;
  }
    return;
}

int tftp_read(tftp_msg * stored, char *data){
  int size_expected;
  int bytesRead = 0;
  int offset = (stored->Nblock - 1) * MSG_SIZE;
  char *tmp;
  struct stat sb;
  FILE *fds;
  
  fds = fopen(stored->file, "r");

  if (fds == NULL) {
    printf("The file %s could not been opened.\n", stored->file);
    perror("open");
    return 1;
  }
  //set the offset to the block we want
  fseek(fds, offset, 0);
  //get size of the file
  stat(stored->file, &sb);
  
  if (offset < 0) {
	printf("tftp_read: offset is negative !\n");
	return -2;
    }
  
  if (sb.st_size < offset) {
    // printf("File [%s] error : trying to read block out of range!\n",file);
    return -1;
    
  }
  /*   //checking if it is the last block */
  if (sb.st_size < (offset + MSG_SIZE))
    size_expected = sb.st_size - offset;
  
    else
      size_expected = MSG_SIZE;
  
  stored->size = size_expected;
  tmp = data;
  
    while (1) {
      bytesRead += fread(tmp, 1, size_expected, fds);
      
      /* check how many bytes have been read */
      
      switch (bytesRead) {
	
      case -1:
	printf("An error occured while reading!\n");
	exit(EXIT_FAILURE);
	
	default:
	  if (bytesRead == size_expected) {
	    fclose(fds);
	    return size_expected;
	  }
	  
	  size_expected -= bytesRead;
	  tmp += bytesRead;
	  break;
      }
    }
}

int tftp_create(char *file, int *fd)
{
  // user's permissions
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  
    if ((*fd = open(file, O_CREAT | O_EXCL, mode)) == -1) {
      printf("The file [%s] already exists !\n", file);
      return -1;
    }
    return (*fd);
}


int tftp_write(tftp_msg * stored, char *data)
{
    int size_expected;
    int bytesWR = 0;
    int offset = stored->Nblock * MSG_SIZE;
    char *tmp;
    FILE *fds;

    fds = fopen(stored->file, "a");
    //set the offset to the block we want
    fseek(fds, offset, 0);

    if (offset < 0) {
	printf("tftp_read: offset is negative !\n");
	exit(EXIT_FAILURE);
    }
    //checking if it is the last block
    //like this we re-set the size value to a lower one
    if (stored->size < MSG_SIZE) {
	size_expected = stored->size;
    } else
	size_expected = MSG_SIZE;

    tmp = data;
    /* forcing the expected number of bytes to be written */
    while (1) {

      bytesWR += fwrite(tmp, 1, size_expected, fds);
      
      switch (bytesWR) {
	
      case -1:
	printf("An error occured while writing!\n");
	perror("write");
	exit(EXIT_FAILURE);
	
	default:
	  if (bytesWR == size_expected) {
	    fclose(fds);
	    return size_expected;
	  }
	  
	  size_expected -= bytesWR;
	  tmp += bytesWR;
	  break;
      }
    }
}

/*********************************************
 *Setters
 */

void make_msg(short flag, char *file, char *data, short Nblock,
	      char *datagram){
  setFlag(flag, datagram);
  
  switch (flag) {
    case RRQ:
      
      setFile(file, datagram);	// setFile sets the mode also ('octet')
      break;
      
    case WRQ:
      setFile(file, datagram);
      break;
      
    case ACK:
      setNblock(Nblock, datagram);
      break;
      
    case DATA:
      setData(data, datagram);
      setNblock(Nblock, datagram);
      break;

    case OACK:
      break;
      
      //'ERROR' is done in tftp_error
  }
}

int setFlag(short flag, char *datagram)
{

    if (flag < 0) {
	printf("Flag is negative !\n");
	exit(EXIT_FAILURE);
    }

    if (flag > OACK) {
	printf("Unknown flag value : [%d]\n", (int) flag);
	exit(EXIT_FAILURE);
    }

    flag = htons(flag);
    memcpy(datagram, &flag, sizeof(short));

    return 1;
}

int setFile(char *file, char *datagram)
{
    char *tmp;
    char mode[] = MODE;
    if (file == NULL)
	printf("Warning: File name is not set !\n");

    tmp = datagram;
    tmp += 2;

    memcpy(tmp, file, strlen(file));
    tmp[strlen(file)] = 0;	// manual zero to delimit string
    tmp = tmp + strlen(file) + 1;
    memcpy(tmp, mode, strlen(mode));
    tmp[strlen(mode)] = 0;	// manual zero to delimit string
    return 1;
}

int setData(char *data, char *datagram)
{
    char *tmp;
    if (data == NULL)
      printf("Warning: data is not set !\n");

    tmp = datagram;
    tmp += 4;
    
    memcpy(tmp, data, MSG_SIZE);
    tmp[strlen(data)] = 0;

    return 1;
}


int setNblock(short block, char *datagram)
{
    char *tmp;
    if (block < 0) {
	printf("Block number is negative !\n");
	exit(EXIT_FAILURE);
    }
    tmp = datagram;
    tmp += 2;

    block = htons(block);
    memcpy(tmp, &block, sizeof(short));

    return 1;
}


int setErrorCode(short Errorcode, char *datagram)
{
    char *tmp;

    // error value tested by upper function
    tmp = datagram;
    tmp += 2;

    Errorcode = htons(Errorcode);
    memcpy(tmp, &Errorcode, sizeof(short));

    return 1;
}

int setErrorMsg(char *Errormsg, char *datagram)
{
    char *tmp;

    if (Errormsg == NULL) {
	printf("Error msg is not set ! \n");
	exit(EXIT_FAILURE);
    }
    
    tmp = datagram;
    tmp += 4;
    
    memcpy(tmp, Errormsg, strlen(Errormsg));
    tmp[strlen(Errormsg)] = 0;
    return 1;
}


/********************************************
 *Getters*
 **/

/* note: all the getters allocate the res array */

int getFlag(char *datagram)
{
    short flag;

    memcpy(&flag, datagram, sizeof(short));

    return ntohs(flag);
}

char *getFile(char *datagram, char *res)
{
    char *start;
    int i = 0;

    start = datagram;
    start += 2;

    while (1) {

	if (start[i] == 0) {
	    break;
	}
	i++;
    }

    res = malloc(sizeof(char) * i);
    memcpy(res, start, i);
    return res;
}
// alternative to getFile, more generic, can give us options
char * getItem(char *datagram, char *res, int *position){

  char *start;
    int i = 0;
    
    start = datagram;
    start += *position;

    while (1) {

	if (start[i] == 0) {
	    break;
	}
	i++;
    }
    *position += i +1 ;
    res = malloc(sizeof(char) * i +1);
    memcpy(res, start, i);
    res[i]=0;
    return res;
}


// returns the size of DATA
char *getData(char *datagram, char *res)
{
    char *tmp = datagram;
    int i = 0;

    tmp += 4;
    // it is not end message
    // as the data part is 512 bytes
    if (tmp[MSG_SIZE] == 0) {
	res = malloc(sizeof(char) * MSG_SIZE);
	memcpy(res, tmp, MSG_SIZE);
	return (char *) res;
    }

    else {

	while (1) {
	    if (tmp[i] == 0)
		break;
	    i++;
	}

	res = malloc(sizeof(char) * i);
	memcpy(res, tmp, i);

	return (char *) res;
    }
}


int getNblock(char *datagram)
{
    short Nblock;
    char *tmp = datagram;

    tmp += 2;

    memcpy(&Nblock, tmp, sizeof(short));

    return ntohs(Nblock);
}

/* processes the errors and builds message */
/* 'msg' is non-null only for zero */
/* returns size of the message */
int tftp_error(int code, char *msg, char *answer)
{
    char errormsg[64];
    int size;

    switch (code) {

    case 1:
	strcpy(errormsg, "File not found");
	break;
    case 2:
	strcpy(errormsg, "Acces violation");
	break;
    case 4:
	strcpy(errormsg, "Illegal TFTP operation");
	break;
    case 6:
	strcpy(errormsg, "File already exists");
	break;
    case 0:
	strcpy(errormsg, msg);
    }

    setFlag(ERROR, answer);
    setErrorCode(code, answer);
    setErrorMsg(errormsg, answer);

    size = strlen(errormsg) + 4;
    return size;
}

/* SETTERS for tftp_msg structure  */

int setFlagStored(short flag, tftp_msg * datagram)
{

    if (flag < 0) {
	printf("Flag is negative !\n");
	exit(EXIT_FAILURE);
    }

    if (flag > ERROR) {
	printf("Unknown flag value : [%d]\n", (int) flag);
    }

    datagram->flag = flag;
    return 1;
}

int setFileStored(char *file, tftp_msg * datagram)
{

    if (file == NULL) {
	printf("The file name is not set ! \n");
	exit(EXIT_FAILURE);
    }

    datagram->file = malloc(strlen(file));
    memcpy(datagram->file, file, strlen(file));

    return 1;
}

int setNblockStored(short block, tftp_msg * datagram)
{

    if (block < 0) {
	printf("Block number is negative !\n");
	exit(EXIT_FAILURE);
    }

    datagram->Nblock = block;

    return 1;
}

/********************/
/*Options setter & getter*/

int setOptions(options chosed,short position,char * datagram){
  char *tmp = datagram;
  char b[] = "blksize";
  char r[] = "rexmt";
  char value[8];
  
  if(chosed.rexmt >= 0 && chosed.rexmt <= 255){
    sprintf(tmp,"%s%c%d%c",r,0,chosed.rexmt,0);
  }
  
  else{
    printf("Invalid rexmt value: out of range, must be within [0;255]!\n");
    printf("Ignoring rexmt value\n");
  }
}
 
int getOptions(options *chosed, char * buf,int * position){
  char *mode;
  char *option1;
  char *value1;

  // mode
  getItem(buf,mode,position);

  //option 1
  option1=getItem(buf,option1,position);
  
  if(strcmp(option1,"rexmt") == 0){
    value1=getItem(buf,value1,position);
    chosed->rexmt = atoi(value1);
    return 1;
  }

  else return 0;
  
}


// PLATFORM faire deux define CLIENT and SERVER
int tftp(int platform, tftp_msg * stored, char *data_rcvd, char *answer)
{
    int check, flag, newblock;
    int fd,position=2;
    char *file, data[512], *pdata;
    options chosed;
    
    flag = getFlag(data_rcvd);


    switch (flag) {

	//Only server can receive RRQ
    case RRQ:
	//platform == 1 server, platform == 0 client
	if (platform != SERVER) {
	    tftp_error(0, "Client receiving RRQ flag!", answer);
	    return -1;
	}
	// We have to be idle when receiving this flag
	if (stored->previous_state != 0) {	// have to be set before the main loop
	    tftp_error(4, NULL, answer);
	    return -1;
	}
	//update the state
	stored->previous_state = RRQ;

	file = getItem(data_rcvd, file,&position);
	//storing file name
	setFileStored(file, stored);
	
	//storing actual value of Nblock
	setNblockStored(1, stored);
	
	// checking if file exists
	check = tftp_read(stored, data);
	if (check == 1) {
	    stored->size = tftp_error(1, NULL, answer);
	    return -1;
	}
	// Get the options
	/*	if(getOptions(&chosed,data_rcvd,&position)){
	  
	  stored->size = 20;
	  printf("We have options!\n");
	  //verification  
	  setOptions(chosed,2,answer);
	    make_msg(OACK,NULL,NULL,0,answer);
	    return 1;
	  }
	*/
	stored->size = check + 4;

	make_msg(DATA, stored->file, data, stored->Nblock, answer);
	return 1;

	//Only server can receive WRQ
    case WRQ:
	if (platform != SERVER) {
	    tftp_error(0, "Client receiving WRQ flag!", answer);
	    return -1;
	}
	// We have to be idle when receiving this flag
	if (stored->previous_state != 0) {	// have to be set before the main loop

	    tftp_error(4, NULL, answer);
	    return -1;
	}
	//update the state
	stored->previous_state = WRQ;
	file = getFile(data_rcvd, file);
	//storing file name
	setFileStored(file, stored);

	//storing actual value of Nblock
	setNblockStored(0, stored);

	check = tftp_create(stored->file, &fd);

	if (check == -1) {
	    stored->size = tftp_error(6, NULL, answer);
	    return -1;
	}

	stored->size = strlen(stored->file) + 2;
	make_msg(ACK, NULL, NULL, stored->Nblock, answer);

	return 1;

    case ACK:

      if (stored->previous_state != RRQ && stored->previous_state != WRQ) {
	tftp_error(4, NULL, answer);
	return -1;
      }
      
	newblock = getNblock(data_rcvd);
	if (newblock != stored->Nblock ) {
	  stored->size =
		tftp_error(0, "Unexpected number block", answer);
	  printf("block:%d\n",newblock);
	  
	  return -1;
	}
	
	// informations are correct, we send the next block
	stored->Nblock++;
	//
	stored->size = stored->size - 4;
	check = tftp_read(stored, data);
	stored->size = check + 4;

	if (check == -1) {
	  printf("Last block sent.\n");
	  return 0;
	}
	
	make_msg(DATA, stored->file, data, stored->Nblock, answer);
	

	return 1;
	/*
    case OACK:
	  
	  if(stored->previous_state == RRQ)
	  make_msg(ACK,NULL,NULL,0,answer);
	  
	  if(stored->previous_state == WRQ){
	    check = tftp_read(stored, data);
	    //if so the communication is well finished
	    if (check == -1) {
	      return 0;
	    }
	    // informations are correct, we send the next block
	    stored->Nblock++;
	    stored->size = stored->size - 2;
	    check = tftp_read(stored, data);
	    stored->size = check + 4;
	    make_msg(DATA, stored->file, data, stored->Nblock, answer);
	  }
	    return 1;
	*/
	    
    case DATA:
	// if so, stored structure has to be initialized before main loop
	if (stored->action_done == RRQ || stored->previous_state == WRQ) {
	    newblock = getNblock(data_rcvd);

	    //if unexpected block
	    if (newblock != stored->Nblock + 1) {	// init at 0
		tftp_error(4, NULL, answer);
		return -1;
	    }

	    stored->Nblock++;
	    //if it is the first block, create the file
	    if (stored->Nblock == 0) {
		check = tftp_create(stored->file, &fd);
		if (check == -1) {
		    tftp_error(6, NULL, answer);
		    return -1;
		}
	    }

	    pdata = getData(data_rcvd, pdata);
	    stored->size -= 4;
	    check = tftp_write(stored, pdata);


	    stored->size = check + 4;
	    // building ACK of this received DATA

	    make_msg(ACK, NULL, NULL, stored->Nblock, answer);
	    //if it was the last block
	    if (stored->size < MSG_SIZE) {
	      //	printf("Last block received.\n");
		return 0;
	    }
	    break;
	    
	case ERROR:
	  return -1;
	    
	default:
	  printf("Unknown flag!\n");
	  return -1;
	}
    }
}
