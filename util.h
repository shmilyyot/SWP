#ifndef __UTIL_H__
#define __UTIL_H__

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

//Linked list functions
int ll_get_length(LLnode *);
void ll_append_node(LLnode **, void *);
LLnode * ll_pop_node(LLnode **);
void ll_destroy_node(LLnode *);

//Print functions
void print_cmd(Cmd *);

//Time functions
long timeval_usecdiff(struct timeval *, 
                      struct timeval *);

//TODO: Implement these functions
char * convert_frame_to_char(Frame *);
Frame * convert_char_to_frame(char *);

//生成crc冗余码相关函数声明声明
char get_bit(uint16_t byte,int pos);
uint16_t crc16(char* array,int array_len);
int is_corrupted(char* array,int array_len);
//打印帧
void print_frame(Frame * frame);
//获取当前时间+一毫秒
void calculate_timeout(struct timeval * timeout);
//将时间和帧封装进缓冲区
void intoSendBuffer(Sender* sender,Timeout *timeout, Frame *frame);
void intoRecBuffer(Receiver* sender,Frame *frame);
//判断缓冲区是否已满
int sendBufferFull(Sender* sender);
//找到根据序列号对应缓冲区的帧
sendInfo* searchSendBuffer(uint8_t seq,Sender *sender);
//字符串过长要切分
void ll_split_head(Sender* sender,Cmd * outgoing_cmd,int payload_size);
//查找确认已接收的报文，方便释放缓冲区并且移动LAR
uint8_t checkLastedLAR(Sender* sender,uint8_t seq);
void releaseRecBuffer(Receiver * receiver);
#endif
