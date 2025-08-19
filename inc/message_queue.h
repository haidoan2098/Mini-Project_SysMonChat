#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__

#include <mqueue.h> 

mqd_t message_queue_create();
void message_queue_send(mqd_t mq, const char *msg);
void message_queue_receive(char *buffer);

#endif