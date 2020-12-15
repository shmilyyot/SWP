#include "util.h"
//这是一个循环双向链表
//Linked list functions
int ll_get_length(LLnode * head)
{
    LLnode * tmp;
    int count = 1;
    if (head == NULL)
        return 0;
    else
    {
        tmp = head->next;
        while (tmp != head)
        {
            count++;
            tmp = tmp->next;
        }
        return count;
    }
}

void ll_append_node(LLnode ** head_ptr, 
                    void * value)
{
    LLnode * prev_last_node;
    LLnode * new_node;
    LLnode * head;

    if (head_ptr == NULL)
    {
        return;
    }
    
    //Init the value pntr
    head = (*head_ptr);
    new_node = (LLnode *) malloc(sizeof(LLnode));
    new_node->value = value;

    //The list is empty, no node is currently present
    if (head == NULL)
    {
        (*head_ptr) = new_node;
        new_node->prev = new_node;
        new_node->next = new_node;
    }
    else
    {
        //Node exists by itself
        prev_last_node = head->prev;
        head->prev = new_node;
        prev_last_node->next = new_node;
        new_node->next = head;
        new_node->prev = prev_last_node;
    }
}


LLnode * ll_pop_node(LLnode ** head_ptr)
{
    LLnode * last_node;
    LLnode * new_head;
    LLnode * prev_head;

    prev_head = (*head_ptr);
    if (prev_head == NULL)
    {
        return NULL;
    }
    last_node = prev_head->prev;
    new_head = prev_head->next;

    //We are about to set the head ptr to nothing because there is only one thing in list
    if (last_node == prev_head)
    {
        (*head_ptr) = NULL;
        prev_head->next = NULL;
        prev_head->prev = NULL;
        return prev_head;
    }
    else
    {
        (*head_ptr) = new_head;
        last_node->next = new_head;
        new_head->prev = last_node;

        prev_head->next = NULL;
        prev_head->prev = NULL;
        return prev_head;
    }
}

void ll_destroy_node(LLnode * node)
{
    if (node->type == llt_string)
    {
        free((char *) node->value);
    }
    free(node);
}

//Compute the difference in usec for two timeval objects
long timeval_usecdiff(struct timeval *start_time, 
                      struct timeval *finish_time)
{
  long usec;
  usec=(finish_time->tv_sec - start_time->tv_sec)*1000000;
  usec+=(finish_time->tv_usec- start_time->tv_usec);
  return usec;
}

Timeout *get_timeout(){
    Timeout *timeout = (Timeout *)malloc(sizeof(Timeout));
    gettimeofday(timeout,NULL);
    timeout->tv_usec += 100000;
    if (timeout->tv_usec >= 1000000){
        timeout->tv_sec++;
        timeout->tv_usec -= 1000000;
    }
    return timeout;
}

//Print out messages entered by the user
void print_cmd(Cmd * cmd)
{
    fprintf(stderr, "src=%d, dst=%d, message=%s\n", 
           cmd->src_id,
           cmd->dst_id,
           cmd->message);
}


char * convert_frame_to_char(Frame * frame)
{
    //TODO: You should implement this as necessary
    char * char_buffer = (char *) malloc(MAX_FRAME_SIZE);
    memset(char_buffer,
           0,
           MAX_FRAME_SIZE);
    memcpy(char_buffer, 
           frame,
           MAX_FRAME_SIZE);
    return char_buffer;
}


Frame * convert_char_to_frame(char * char_buf)
{
    //TODO: You should implement this as necessary
    //为帧创建空间
    Frame * frame = (Frame *) malloc(sizeof(Frame));
    //为帧设置初始化值
    memset(frame,
           0,
           sizeof(char)*sizeof(Frame));
    //为帧赋值消息内容
    memcpy(frame, 
           char_buf,
           sizeof(char)*sizeof(Frame));
    return frame;
}

//计算crc16冗余码，返回一个short
//低位在后高位在前
uint16_t crc16(char* array,int array_len){
    uint16_t wCRCin = 0x0000;  
    uint16_t wCPoly = 0x1021;  
    uint8_t wChar = 0;
    while (array_len--)
    {
        wChar = *(array++);  
        wCRCin ^= (wChar << 8);
        for(int i = 0; i < 8; i++){  
            if(wCRCin & 0x8000)  
                wCRCin = (wCRCin << 1) ^ wCPoly;
            else
                wCRCin = wCRCin << 1;
        }
    }
    return wCRCin; 
}

char get_bit(uint16_t byte,int pos){
    byte = (byte>>(15-pos))&1;
    if(byte==1) return 1;
    else return 0;
}

int is_corrupted(char* array,int array_len){
    Frame * frame = convert_char_to_frame(array);
    uint16_t crc = crc16(array,array_len-2);
    return crc != frame->crc;
}

void print_frame(Frame* frame)
{
    fprintf(stderr, "\nframe:\n");
    fprintf(stderr, "frame->crc=%d\n", frame->crc);
    fprintf(stderr, "frame->src=%d\n", frame->sourceId);
    fprintf(stderr, "frame->dst=%d\n", frame->destinationId);
    fprintf(stderr, "frame->seq=%d\n", frame->seq);
    fprintf(stderr, "frame->ack=%d\n", frame->ack);
    fprintf(stderr, "frame->data=%s\n", frame->data);
    fprintf(stderr, "#frame--------\n");
}