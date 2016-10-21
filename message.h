#pragma once
/*
 * message.h -- public interface to the message module
 *
 * Used as a standard way to combine the username and message
 * into a single string for passing between the server and client
 */

 typedef struct Message {
   char* user;
   char* msg;
 } Message;

 char* createMessage(char* user, char* message);
 Message* parseMessage(char* message);
