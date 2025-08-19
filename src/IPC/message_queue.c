#include "message_queue.h"
#include <stdio.h>
#include <mqueue.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_SIZE 64

const char *queue_name = "/my_message_queue";


// Hàm tạo queue, trả về mqd_t
mqd_t message_queue_create() {
    mqd_t mq;
    struct mq_attr attr = {0};
    attr.mq_flags = 0;          // blocking mode
    attr.mq_maxmsg = 5;         // số message tối đa
    attr.mq_msgsize = MAX_SIZE; // kích thước tối đa mỗi gói

    mq = mq_open(queue_name, O_CREAT | O_WRONLY, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open (create)");
    }
    return mq; // caller phải mq_close
}

// Hàm gửi message (đã có queue mở sẵn)
void message_queue_send(mqd_t mq, const char *msg) {
    if (mq_send(mq, msg, strlen(msg) + 1, 0) < 0) {
        perror("mq_send");
    }
    // mq_close(mq);
}

void message_queue_receive(char *buffer) {
    mqd_t mq;
    unsigned int priority;

    mq =  mq_open(queue_name, O_RDONLY);    // Mở queue
    if (mq == (mqd_t)-1) {
        perror("mq_open (receive)");
        return;
    }

    if (mq_receive(mq, buffer, MAX_SIZE, &priority) < 0) {
        perror("mq_receive");
        mq_close(mq);
        return;
    }

    // mq_close(mq);
    // mq_unlink(queue_name);   // Khoát khỏi mới dùng
}