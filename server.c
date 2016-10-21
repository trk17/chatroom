/*
 * Author: Thomas Kiernan
 *
 * Main Server code used for the chatroom application
 * Implements the UDP backend used to connect people
 *
 * Command Line Arguments
 * ./server {port number}
 */

#include "message.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <memory.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdarg.h>

#define BUFSIZE 2048

char *pingCmd = "/ping";
char *joinCmd = "/join";
char *leaveCmd = "/leave";
char *whoCmd = "/who";

int socketNum;
char message[900];
char buffer[1000];
char writer[100];
socklen_t addrlen = sizeof(struct sockaddr_in);
struct sockaddr_in whoDest;

/*
 * Structure placed into the queue for connected clients
 */
typedef struct RoomMember {
  char* name;
  struct sockaddr_in servaddr;
} RoomMember;

/*
 * Applied to the queue upon a who command
 * Has each member of the queue send its name to the address
 * at whoDest
 */
void sendMemberName(void* ptr){
  RoomMember* member = (RoomMember*) ptr;
  sendto(socketNum, member->name, strlen(member->name), 0, (struct sockaddr *)&whoDest, addrlen);
}

/*
 * Applied to the queue when a message needs to be sent out
 * Has each memeber of the queue send the message
 * stored in message and from user
 */
void sendMessage(void* ptr){
  RoomMember* member = (RoomMember*) ptr;

  memset(buffer, 0, sizeof(buffer));
  strcpy(buffer, writer);
  strcat(buffer,": ");
  strcat(buffer, message);

  sendto(socketNum, buffer, strlen(buffer), 0, (struct sockaddr *)&(member->servaddr), addrlen);
}

/*
 * Used in qsearch and qremove to compare if two roommebers are the same
 * Is used during /join /leave and sending out messages
 */
int compareRoomMembers(void* member1, const void* member2){
  if(strcmp(((RoomMember*)member1)->name, ((RoomMember*)member2)->name) == 0){
    return 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  int portNum;
  struct sockaddr_in myaddr;
  struct sockaddr_in remaddr;
  int recvlen;
  queue_t* chatroom = qopen();

  if(argc != 2){
    printf("Incorrect number of command line arguments.\nPlease only provide a Port Number\n");
    return 1;
  }

  /* Get Port Number */
  if(sscanf (argv[1], "%i", &portNum)!=1){
    printf("Unable to parse port number. Exiting...\n");
    return 1;
  }

  /* create a UDP socket */
  if ((socketNum = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("cannot create socket\n");
    return 0;
  }

  /* bind the socket to any valid IP address and a specific port */
  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(portNum);

  if (bind(socketNum, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
    perror("bind failed\n");
    return 0;
  }

  /* now loop, receiving data and printing what we received */
  for (;;) {
    char buf[BUFSIZE];
    Message* mess;
    printf("waiting on port %d\n", portNum);

    recvlen = recvfrom(socketNum, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
    if (recvlen > 0) {
      buf[recvlen] = 0;
      mess = parseMessage(buf);

      /* PING COMMAND */
      if(strncmp(mess->msg, pingCmd, strlen(pingCmd)) == 0){
        char *responseMessage = "The Server Is Up And Running";

        sendto(socketNum, responseMessage, strlen(responseMessage), 0, (struct sockaddr *)&remaddr, addrlen);

      /* JOIN COMMAND */
      } else if(strncmp(mess->msg, joinCmd, strlen(joinCmd)) == 0){
        RoomMember* newMember = (RoomMember*) malloc(sizeof(RoomMember));
        newMember->name = (char*) malloc(strlen(mess->user) + 1);
        memcpy(newMember->name, mess->user, strlen(mess->user) + 1);
        memcpy(&newMember->servaddr, &remaddr, sizeof(struct sockaddr_in));

        if(qsearch(chatroom, compareRoomMembers, newMember) != NULL){
          char* response = "A user with this name is already in the chatroom";
          sendto(socketNum, response, strlen(response), 0, (struct sockaddr *)&remaddr, addrlen);
          free(newMember);

        } else {
          char* notification = " has joined the chatroom!";
          char* response = "Welcome to the chatroom!";

          memset(message, 0, sizeof(message));
          strcpy(message, mess->user);
          strcat(message, notification);
          memset(writer, 0, sizeof(writer));
          strcpy(writer, "ServerBot");
          qapply(chatroom, sendMessage);

          qput(chatroom, newMember);
          sendto(socketNum, response, strlen(response), 0, (struct sockaddr *)&remaddr, addrlen);
        }

      /* LEAVE COMMAND */
      } else if(strncmp(mess->msg, leaveCmd, strlen(leaveCmd)) == 0){
        RoomMember* newMember = (RoomMember*) malloc(sizeof(RoomMember));
        newMember->name = (char*) malloc(strlen(mess->user) + 1);
        memcpy(newMember->name, mess->user, strlen(mess->user) + 1);

        if(qremove(chatroom, compareRoomMembers, newMember) == NULL){
          char* response = "No one with the provided username is in the chatroom";
          sendto(socketNum, response, strlen(response), 0, (struct sockaddr *)&remaddr, addrlen);
        } else {
          char* response = "You have left the chatroom.";
          char* notification = " has left the chatroom!";

          sendto(socketNum, response, strlen(response), 0, (struct sockaddr *)&remaddr, addrlen);

          memset(message, 0, sizeof(message));
          strcpy(message, mess->user);
          strcat(message, notification);
          memset(writer, 0, sizeof(writer));
          strcpy(writer, "ServerBot");
          qapply(chatroom, sendMessage);
        }

      /* WHO COMMAND */
      } else if(strncmp(mess->msg, whoCmd, strlen(whoCmd)) == 0){
        char* response = "The current members of the chatroom are:";
        whoDest = remaddr;
        sendto(socketNum, response, strlen(response), 0, (struct sockaddr *)&remaddr, addrlen);
        qapply(chatroom, sendMemberName);

      } else {
        /* Check if user is in the chat room */
        RoomMember* newMember = (RoomMember*) malloc(sizeof(RoomMember));
        newMember->name = (char*) malloc(strlen(mess->user) + 1);
        memcpy(newMember->name, mess->user, strlen(mess->user) + 1);
        memcpy(&newMember->servaddr, &remaddr, sizeof(struct sockaddr_in));
        if(qsearch(chatroom, compareRoomMembers, newMember) != NULL){

          /* Remove them so they dont get message */
          qremove(chatroom, compareRoomMembers, newMember);
          memset(message, 0, sizeof(message));
          strncpy(message, mess->msg, sizeof(message) - 1);
          memset(writer, 0, sizeof(writer));
          strncpy(writer, mess->user, sizeof(writer) - 1);
          /* Send message to everyone */
          qapply(chatroom, sendMessage);
          /* Put them back */
          qput(chatroom, newMember);
        } else {
          char* response = "You must join the chatroom before you can send messages";
          sendto(socketNum, response, strlen(response), 0, (struct sockaddr *)&remaddr, addrlen);
          free(newMember);
        }
      }
      printf("Handled message\n");
      free(mess);
    }
  }
}
