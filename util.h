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
void append_crc(char* array,int array_len);
int is_corrupted(char* array,int array_len);
//打印帧
void print_frame(Frame * frame);
//获取当前时间+一毫秒
Timeout *get_timeout();
//讲时间和帧封装进缓冲区
void IntoBuffer(Timeout *timeout, Frame *frame);
#endif
