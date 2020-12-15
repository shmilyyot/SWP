#ifndef __SENDER_H__
#define __SENDER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <math.h>
#include <sys/time.h>
#include "common.h"
#include "util.h"
#include "communicate.h"


void init_sender(Sender *, int);
void * run_sender(void *);

struct Send_Frame_Info{
    Frame *sframe;
    Timeout* timeout;
};
typedef struct Send_Frame_Info sendInfo;

//发送窗口
struct Window_Sender{
    uint8_t LAR;
    uint8_t LFS; 
    sendInfo * buffer[MAX_BUFFER_LENGTH];
};
typedef struct Window_Sender sWindow;


#endif
