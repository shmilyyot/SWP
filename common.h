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
//缓冲区最大长度是8
#define MAX_BUFFER_LENGTH 8
typedef struct timeval Timeout;

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

//帧大小最大是48个字节
#define MAX_FRAME_SIZE 48
//TODO: You should change this!
//Remember, your frame can be AT MOST 48 bytes!
#define FRAME_PAYLOAD_SIZE 39   //帧的有效负载
//帧的结构
struct Frame_t
{
    char data[FRAME_PAYLOAD_SIZE+1]; //帧的内容装在数组里面，留一个字节给字符串末尾标识\0
    uint8_t seq; //顺序号
    uint8_t ack; //确认号 0是发送帧，1是确认接受帧
    uint16_t sourceId; //源地址
    uint16_t destinationId; //目的地址uint16_t
    uint16_t crc; //crc冗余码
};
typedef struct Frame_t Frame;

struct Receive_Frame_Info{
    Frame *rframe;
    uint8_t Status; //0代表可以用，1代表接收已确认，2代表发送已确认
};
typedef struct Receive_Frame_Info recInfo;

//LAR最近接收到的确认帧,LFS最近发送的帧
//NFE期待的下一帧的序号，RWS接收窗口大小
//接受窗口
struct Window_Receiver{
    uint8_t NFE; //NFE期待的下一帧的序号
    uint8_t RWS; //RWS接收窗口大小
    LLnode *recvBuffer; //接收缓冲区
};
typedef struct Window_Receiver rWindow;

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
    rWindow *window;
};

//缓冲在窗口里的帧格式，包含了帧和时间
struct Send_Frame_Info{
    Frame *sframe;
    Timeout* timeout;
    uint8_t Status; //0代表空闲空间，1代表发送已确认，2代表发送未确认
};
typedef struct Send_Frame_Info sendInfo;

//发送窗口(缓冲区)
struct Window_Sender{
    int8_t LAR; //LAR最近接收到的确认帧
    uint8_t LFS; //LFS最近发送的帧
    sendInfo buffer[MAX_BUFFER_LENGTH]; //缓冲区数组
};
typedef struct Window_Sender sWindow;

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
    sWindow *window;
    //单条cmd拆分出来的消息队列
    LLnode* splitlist;
    //消息序号
    int messageSeq;
};

enum SendFrame_DstType 
{
    ReceiverDst,
    SenderDst
} SendFrame_DstType ;

typedef struct Sender_t Sender;
typedef struct Receiver_t Receiver;

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
