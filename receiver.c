#include "receiver.h"

//初始化每一个接收者并且给予一个id
void init_receiver(Receiver * receiver,
                   int id)
{
    receiver->recv_id = id;
    receiver->input_framelist_head = NULL;
}

//处理到达的信息
void handle_incoming_msgs(Receiver * receiver,
                          LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling incoming frames
    //    1) Dequeue the Frame from the sender->input_framelist_head 
    //    2) Convert the char * buffer to a Frame data type
    //    3) Check whether the frame is corrupted
    //    4) Check whether the frame is for this receiver
    //    5) Do sliding window protocol for sender/receiver pair

    int incoming_msgs_length = ll_get_length(receiver->input_framelist_head);
    uint16_t send_id; //发送方的id
    while (incoming_msgs_length > 0)
    {
        //Pop a node off the front of the link list and update the count
        //从链表里取结点，这个结点不是帧，只是包含了消息
        LLnode * ll_inmsg_node = ll_pop_node(&receiver->input_framelist_head);

        //每次从消息队列取一个节点出来，把发送者id取出来，发送确认报文的时候要用到
        send_id = ((char *)ll_inmsg_node->value)[2];
        //--incoming_msgs_length; //好像可以优化成--，再次获取长度效率太低
        incoming_msgs_length = ll_get_length(receiver->input_framelist_head);

        //DUMMY CODE: Print the raw_char_buf
        //NOTE: You should not blindly print messages!
        //      Ask yourself: Is this message really for me?
        //                    Is this message corrupted?
        //                    Is this an old, retransmitted message?  

        //把收到的消息从帧的value里取出来。并且强制转型为char*，本来是void*        
        char * raw_char_buf = (char *) ll_inmsg_node->value;
        //把结点里的消息转为帧
        Frame * inframe = convert_char_to_frame(raw_char_buf);

        //Free raw_char_buf
        free(raw_char_buf);
        
        printf("<RECV_%d>:[%s]\n", receiver->recv_id, inframe->data);

        free(inframe);
        free(ll_inmsg_node);

        //把确认报文ack添加到outgoing_frames_head_ptr，它会把帧取出来发送给发送者
        char data[MAX_FRAME_SIZE];
        data[2]=receiver->recv_id;
        data[4]=send_id;
        data[6] = 1;
        uint16_t crc = crc16(data+2,46);
        char crc1, crc2;
        crc1 = crc; //低地址后八位
        crc2 = crc>>8; //高地址前八位
        data[0] = crc2;
        data[1] = crc1;
        Frame *outframe = convert_char_to_frame(data);
        ll_append_node(&outgoing_frames_head_ptr,(void *)(outframe->data));
    }
}

//线程启动函数，参数是每一个线程的receiver，包含了id等参数值
void * run_receiver(void * input_receiver)
{   
    //更高精度的时间计算。
    //struct timespec有两个成员，一个是秒，一个是纳秒, 所以最高精确度是纳秒。 
    //struct timeval有两个成员，一个是秒，一个是微秒, 所以最高精确度是微秒。
    struct timespec   time_spec;
    struct timeval    curr_timeval;
    const int WAIT_SEC_TIME = 0;
    const long WAIT_USEC_TIME = 100000;
    //线程开始时传进来的Receiver
    Receiver * receiver = (Receiver *) input_receiver;
    //给线程创建一个新的帧链表的
    LLnode * outgoing_frames_head;


    //This incomplete receiver thread, at a high level, loops as follows:
    //1. Determine the next time the thread should wake up if there is nothing in the incoming queue(s)
    //2. Grab the mutex protecting the input_msg queue
    //3. Dequeues messages from the input_msg queue and prints them
    //4. Releases the lock
    //5. Sends out any outgoing messages

    //初始化条件变量和互斥锁
    pthread_cond_init(&receiver->buffer_cv, NULL);
    pthread_mutex_init(&receiver->buffer_mutex, NULL);

    //进入接收消息的死循环
    while(1)
    {    
        //NOTE: Add outgoing messages to the outgoing_frames_head pointer
        outgoing_frames_head = NULL;
        gettimeofday(&curr_timeval, 
                     NULL);

        //Either timeout or get woken up because you've received a datagram
        //NOTE: You don't really need to do anything here, but it might be useful for debugging purposes to have the receivers periodically wakeup and print info
        time_spec.tv_sec  = curr_timeval.tv_sec;
        time_spec.tv_nsec = curr_timeval.tv_usec * 1000;
        time_spec.tv_sec += WAIT_SEC_TIME;
        time_spec.tv_nsec += WAIT_USEC_TIME * 1000;
        if (time_spec.tv_nsec >= 1000000000)
        {
            time_spec.tv_sec++;
            time_spec.tv_nsec -= 1000000000;
        }

        //*****************************************************************************************
        //NOTE: Anything that involves dequeing from the input frames should go 
        //      between the mutex lock and unlock, because other threads CAN/WILL access these structures
        //*****************************************************************************************
        pthread_mutex_lock(&receiver->buffer_mutex);

        //Check whether anything arrived
        int incoming_msgs_length = ll_get_length(receiver->input_framelist_head);
        if (incoming_msgs_length == 0)
        {
            //Nothing has arrived, do a timed wait on the condition variable (which releases the mutex). Again, you don't really need to do the timed wait.
            //A signal on the condition variable will wake up the thread and reacquire the lock
            pthread_cond_timedwait(&receiver->buffer_cv, 
                                   &receiver->buffer_mutex,
                                   &time_spec);
        }

        handle_incoming_msgs(receiver,
                             &outgoing_frames_head);

        pthread_mutex_unlock(&receiver->buffer_mutex);
        
        //CHANGE THIS AT YOUR OWN RISK!
        //Send out all the frames user has appended to the outgoing_frames list
        int ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        while(ll_outgoing_frame_length > 0)
        {
            LLnode * ll_outframe_node = ll_pop_node(&outgoing_frames_head);
            char * char_buf = (char *) ll_outframe_node->value;
            
            //The following function frees the memory for the char_buf object
            send_msg_to_senders(char_buf);

            //Free up the ll_outframe_node
            free(ll_outframe_node);
            //--ll_outgoing_frame_length; //这里同样可以优化
            ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        }
    }
    pthread_exit(NULL);

}
