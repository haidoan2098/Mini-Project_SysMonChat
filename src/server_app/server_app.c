
#include "server_core.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{   
    if (argc != 2) {
        printf("Usage: %s <server_port>\n", argv[0]);
        return 1;
    }

    int server_port = atoi(argv[1]);

    display_commands_server();
    int fd = server_init(server_port);
    server_run(fd);

    return 0;
}