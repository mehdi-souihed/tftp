#ifndef COMMON_H
#define COMMON_H


static int MSG_SIZE = 512;


#define DEFAULT_SERV_PORT 8501
#define BUFMAX 1024
#define VRB if(verboseFlag)     // verboseFlag is a static variable
#define MODE "octet" // the only mode we do support

#define RRQ   1
#define WRQ   2
#define DATA  3
#define ACK   4
#define ERROR 5
#define OACK  6

#define CLIENT 10
#define SERVER 11

struct Msg
{
  
  short flag;           // datagram's flag
  short Nblock;         // number of block
  short previous_state; // mostly for server
  short action_done;    // mostly for client
  char *mode;           // in our case only 'octet'
  char *file;           // path of the file to read/write
  int size;             // size of entire datagram, useful for sendto()

};
typedef struct Msg tftp_msg;

struct Options
{
  int blksize;
  int rexmt;
};
typedef struct Options options;

/********************************************************************
 protocole tftp 
********************************************************************/
 
int tftp (int state, tftp_msg * , char *recvd,char *answer);
int tftp_error(int,char*,char*);

void make_msg (short flag, char *file, char * data, short nbBlock, char *msg);


/********************************************************************
 setters & getters for the datagram
********************************************************************/
 
int setFlag (short flag, char*);
int setFile (char *file, char*);
int setData (char *Data, char*);
int setMode (char *msg,  char*);
int setNblock (short nb,   char*);
int setError (char *nError,char*);
int setErrorCode(short, char*);

int getFlag (char*);
char* getFile (char*, char*);
char* getData (char*,char*);
int getNumberBlock (char*);
int getError (char*);

/********************************************************************
 setters for tftp_msg
********************************************************************/

int setFlagStored(short, tftp_msg*);
int setNblockStored(short, tftp_msg*);
int setFileStored(char*, tftp_msg*);



/********************************************************************
 setter for sockaddr_in structure
********************************************************************/

int setNetInfo (struct sockaddr_in *, char *host, int port);

/********************************************************************
 adapted read/write functions
********************************************************************/

int tftp_read(tftp_msg* ,char *data);
int tftp_write(tftp_msg*,char *data);

/********************************************************************
Options setter & getter 
********************************************************************/

int setOptions(options,short,char * datagram);
int getOptions(options*,char *datagram,int*);

#endif
