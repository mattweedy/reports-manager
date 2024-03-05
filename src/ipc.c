#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define QUEUE_NAME "/my-manufacturing-app-queue"
#define MAX_SIZE 1024
#define MSG_STOP "exit"

// Function to send a message to the queue
void send_message(mqd_t mq, char *message) {
    if (mq_send(mq, message, MAX_SIZE, 0) == -1) {
        perror("Failed to send message");
        exit(1);
    }
}

// Function to receive a message from the queue
void receive_message(mqd_t mq) {
    char buffer[MAX_SIZE + 1];
    ssize_t bytes_read;

    bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
    if (bytes_read >= 0) {
        buffer[bytes_read] = '\0';
        if (!strncmp(buffer, MSG_STOP, strlen(MSG_STOP))) {
            printf("Received stop message, exiting.\n");
            exit(0);
        } else {
            printf("Received: %s\n", buffer);
        }
    } else {
        perror("mq_receive");
    }
}

int main() {
    mqd_t mq;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (mq == (mqd_t) -1) {
        perror("mq_open");
        exit(1);
    }

    // Here you can call send_message or receive_message functions as needed

    mq_close(mq);
    mq_unlink(QUEUE_NAME);

    return 0;
}