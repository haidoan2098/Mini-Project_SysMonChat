#ifndef __SERVER_H__
#define __SERVER_H__

#include "server_cmd.h"


/**
 * @brief Initialize, bind and listen a TCP Server socket on the specified port.
 * 
 * @param port The TCP port number to bind the socket to.
 * @return The listening socket file descriptor on success, or -1 on failure.
 */
int server_init(int port);

/**
 * @brief Run the TCP server loop to accept and handle client connections.
 * 
 * @param server_fd The listening socket file descriptor returned by server_init().
 */ 
void server_run(int server_fd);

/**
 * @brief Handle communication with a connected client in a separate thread.
 */ 
void *client_handler(void *arg);

/**
 * @brief Handle server-side console commands in a separate thread.
 */ 
void *command_handler(void *arg);


#endif 