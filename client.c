/*
 * Implementation of the client side of the chatroom
 *
 * Command Line Arguments
 * ./client {username} {IP address} {port number}
 */

#define _POSIX_C_SOURCE 200809L
#include "message.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <memory.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdarg.h>


#define BUFSIZE 2048
char* exitCmd = "/exit";

typedef struct ThreadInfo {
  int socket;
  struct sockaddr_in servaddr;
} ThreadInfo;


void* responseThread(void* info){
  int socket = ((ThreadInfo*) info)->socket;
  struct sockaddr_in servaddr = ((ThreadInfo*) info)->servaddr;
  int recvlen;
  unsigned char buf[BUFSIZE];
  socklen_t addrlen = sizeof(servaddr);



  for(;;){
    recvlen = recvfrom(socket, buf, BUFSIZE, 0, (struct sockaddr *)&servaddr, &addrlen);
    if (recvlen > 0) {
      buf[recvlen] = 0;
      printf("%s\n", buf);
    }
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  char* name;
  char* ip;
  int port;
  int socketNum;
  struct sockaddr_in myaddr;
  struct sockaddr_in servaddr;
  pthread_t recievedThreadID;
  ThreadInfo* info;
  char buf[100];
  char* combinedMessage;

  if(argc != 4){
    printf("Incorrect number of command line arguments\n");
    return 1;
  }


  /* Get Command Line Arguments */
  name = argv[1];
  ip = argv[2];

  if(sscanf (argv[3], "%i", &port)!=1){
    printf("Unable to parse port number. Exiting...\n");
    return 1;
  }

  if ((socketNum = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("cannot create socket\n");
    return 1;
  }

  /* Set up local address data */
  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(0);

  /* Bind the socket */
  if (bind(socketNum, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
    perror("bind failed\n");
    return 1;
  }

  /* fill in the server's address and data */
  memset((char*)&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = inet_addr(ip);

  /* Set up Receive Thread */
  info = malloc(sizeof(ThreadInfo));
  info->socket = socketNum;
  info->servaddr = servaddr;
  pthread_create(&recievedThreadID, NULL, &responseThread, info);


  /* Listen to User Input and send to server */
  while(fgets(buf, sizeof(buf),stdin)){
    strtok(buf, "\n");

    combinedMessage = createMessage(name, buf);
    if (sendto(socketNum, combinedMessage, strlen(combinedMessage), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
      perror("send to has failed");
      return 0;
    }
  }

  /* Close receive thread */
  pthread_kill(recievedThreadID, 1);

  return 0;
}
