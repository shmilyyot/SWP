#ifndef __COMMON_H__
#define __COMMON_H__

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

#define MAX_COMMAND_LENGTH 16
#define AUTOMATED_FILENAME 512
typedef unsigned char uchar_t;
#define WINDOW_SIZE 8;
//LAR最近接收到的确认帧,LFS最近发送的帧
//NFE期待的下一帧的序号，RWS接收窗口大小
//System configuration information
struct SysConfig_t
{
    float drop_prob;  //丢包率
    float corrupt_prob;  //误码率
    unsigned char automated;
    char automated_file[AUTOMATED_FILENAME];
};
typedef struct SysConfig_t  SysConfig;

//Command line input information
struct Cmd_t
{
    uint16_t src_id;
    uint16_t dst_id;
    char * message;
};
typedef struct Cmd_t Cmd;

//Linked list information
enum LLtype 
{
    llt_string,
    llt_frame,
    llt_integer,
    llt_head
} LLtype;

//循环双向链表
struct LLnode_t
{
    struct LLnode_t * prev;
    struct LLnode_t * next;
    enum LLtype type;
    //用空指针指向它的值
    void * value;
};
typedef struct LLnode_t LLnode;

//缓冲区最大长度是8
#define MAX_BUFFER_LENGTH 8
//Receiver and sender data structures
struct Receiver_t
{
    //DO NOT CHANGE:
    // 1) buffer_mutex
    // 2) buffer_cv
    // 3) input_framelist_head
    // 4) recv_id
    pthread_mutex_t buffer_mutex;
    pthread_cond_t buffer_cv;
    LLnode* input_framelist_head;
    uint16_t recv_id;
};

struct Sender_t
{
    //DO NOT CHANGE:
    // 1) buffer_mutex
    // 2) buffer_cv
    // 3) input_cmdlist_head
    // 4) input_framelist_head
    // 5) send_id
    //linux线程互斥量
    pthread_mutex_t buffer_mutex;
    //linux线程条件变量
    pthread_cond_t buffer_cv;
    //输入的命令链表    
    LLnode * input_cmdlist_head;
    //输入的帧链表
    LLnode * input_framelist_head;
    //发送id
    uint16_t send_id;
};

enum SendFrame_DstType 
{
    ReceiverDst,
    SenderDst
} SendFrame_DstType ;

typedef struct Sender_t Sender;
typedef struct Receiver_t Receiver;

//帧大小最大是48个字节
#define MAX_FRAME_SIZE 48
//TODO: You should change this!
//Remember, your frame can be AT MOST 48 bytes!
#define FRAME_PAYLOAD_SIZE 41   //帧的有效负载
//帧的结构
struct Frame_t
{
    char data[FRAME_PAYLOAD_SIZE]; //帧的内容装在数组里面
    uint8_t seq; //顺序号
    uint16_t crc; //crc冗余码
    uint16_t sourceId; //源地址
    uint16_t destinationId; //目的地址
    //char data[48];
};
typedef struct Frame_t Frame;


//Declare global variables here
//DO NOT CHANGE: 
//   1) glb_senders_array
//   2) glb_receivers_array
//   3) glb_senders_array_length
//   4) glb_receivers_array_length
//   5) glb_sysconfig
//   6) CORRUPTION_BITS
//用指针指向一个发送者集合数组，接收者同理
Sender * glb_senders_array;
Receiver * glb_receivers_array;
//发送者集合的长度，接收者同理
int glb_senders_array_length;
int glb_receivers_array_length;
//系统参数，包含丢包率坏包率等
SysConfig glb_sysconfig;
int CORRUPTION_BITS;
#endif 
