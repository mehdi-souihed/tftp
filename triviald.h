#ifndef TRIVIALD_H
#define TRIVIALD_H

int server_tftp(int port);
int clone_server(struct sockaddr_in, int);
int initChild(struct sockaddr_in*, int*);
void freechild(int);


#endif
