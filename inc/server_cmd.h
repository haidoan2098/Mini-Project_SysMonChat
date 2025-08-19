#ifndef __SERVER_CMD_H__
#define __SERVER_CMD_H__

#include "server_utils.h"
#include <mqueue.h> 

typedef struct {
    const char *command;
    const char *description;
} Command_t;

static const Command_t commands[] = {
    {"list",                           "Display list of connected clients"      },
    {"message",                        "Show all messages from clients "        },
    {"terminate <ID Client>",          "Terminate a specific connection"        },
    {"sysinfo",                        "Display system information (CPU, RAM)"  },
    {"exit",                           "Exit server command"                    },
};

/**
 * @brief Add client in the list
 */
void cmd_add_client(int sock_fd, const char *ip, int port, connection_status_t status);

/**
 * @brief Displays instructions for use and purpose of commands.
 */
void display_commands_server(void);

/**
 * @brief Execute command 1. list   
 */
void cmd_list(void);

/**
 * @brief Execute command 2. message   
 */
void cmd_message(void);

/**
 * @brief Execute command 3. terminate <ID Client>   
 */
void cmd_terminate(int id_client);  

/**
 * @brief Execute command 4. sysinfo  
 */
void cmd_sysinfo(pid_t *pid_server, mqd_t *mq);

#endif 
