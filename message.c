/*
 * message.c
 * Functions used to pack and unpack messages to send between
 * the client and the server
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "message.h"



 char* createMessage(char* user, char* message){
   char *result = malloc(strlen(user)+strlen(message)+sizeof(char)+1);
   sprintf(result, "%s*%s", user, message);
   return result;
 }

 Message* parseMessage(char* message){
   Message* mess = malloc(sizeof(Message));
   mess->user = strtok(message, "*");
   mess->msg = strtok(NULL, "");
   return mess;
 }

 /*char* messageGetMessage(char* message){
   strsep(&message, "*");
   return strsep(&message, "*");
 }*/
