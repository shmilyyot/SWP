#ifndef __RECEIVER_H__
#define __RECEIVER_H__

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

void init_receiver(Receiver *, int);
void * run_receiver(void *);

struct Receive_Frame_Info{
    Frame *rframe;
    Timeout* timeout;
};
typedef struct Receive_Frame_Info recInfo;

//接受窗口
struct Window_Receiver{
    uint8_t NFE; //NFE期待的下一帧的序号
    uint8_t RWS; //RWS接收窗口大小
    recInfo buffer[MAX_BUFFER_LENGTH];
};
typedef struct Window_Receiver rWindow;
#endif
