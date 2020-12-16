#include "sender.h"

void init_sender(Sender * sender, int id)
{
    //TODO: You should fill in this function as necessary
    sender->send_id = id;
    sender->input_cmdlist_head = NULL;
    sender->input_framelist_head = NULL;
    //初始化发送缓冲区
    sender->window = (sWindow*)malloc(sizeof(sWindow));
    sender->window->LAR = -1; //初始化LAR
    sender->window->LFS = -1; //初始化LFS
    for(int i=0;i<MAX_BUFFER_LENGTH;++i){
        ((sender->window->buffer)+i)->Status = 0; //刚开始缓冲区每一个窗口都是0，代表可以填充数据发送
    }
    sender->splitlist = NULL;
    sender->messageSeq = 0;
}

struct timeval * sender_get_next_expiring_timeval(Sender * sender)
{
    //TODO: You should fill in this function so that it returns the next timeout that should occur
    return NULL;
}


void handle_incoming_acks(Sender * sender,
                          LLnode ** outgoing_frames_head_ptr)
{
    int incoming_acks_length = ll_get_length(sender->input_framelist_head);
    while(incoming_acks_length>0){
        //获取接收到的ack
        LLnode * incoming_acks = ll_pop_node(&sender->input_framelist_head);
        --incoming_acks_length;
        // char * incoming_frame_Char = (char *)incoming_acks->value;
        // Frame * incoming_frame = convert_char_to_frame(incoming_frame_Char);

        // //如果确认包损坏，直接不管，等下一个确认包过来
        // if(is_corrupted(incoming_frame_Char,MAX_FRAME_SIZE)==1){
        //     fprintf(stderr, "Error in finding the frame is corrupted!");
        //     free(incoming_acks);
        //     free(incoming_frame);
        //     free(incoming_frame_Char);
        //     continue;
        // }
        // //如果这个帧不是这个发送者的,直接不管
        // if(incoming_frame->destinationId != sender->send_id){
        //     fprintf(stderr, "This Frame is not for this sender.");
        //     free(incoming_acks);
        //     free(incoming_frame);
        //     free(incoming_frame_Char);
        //     continue;
        // }

        // //接收方要求重传需要为seq的帧
        // if(incoming_frame->ack == 2){
        //     //重传

        // }

        // //这个确认帧对应的帧成功发送，释放缓冲区空间，将这个格子标志为0，窗口开始滑动
        // if(incoming_frame->ack == 1){
        //     sendInfo* bufferFrame = searchSendBuffer(incoming_frame->seq,sender);
        //     //print_frame(bufferFrame->sframe);
        //     //fprintf(stderr,"%ld  %ld",bufferFrame->timeout->tv_sec,bufferFrame->timeout->tv_usec);
        //     bufferFrame->Status = 0;
        //     //释放这个指针指向的帧的时间戳的内存，这个指针还可以下一次赋值指向
        //     free(bufferFrame->sframe);
        //     free(bufferFrame->timeout);

        //     //窗口标志位滑动


        // }


        // free(incoming_acks);
        // free(incoming_frame);
        // free(incoming_frame_Char);
    }
    //TODO: Suggested steps for handling incoming ACKs
    //    1) Dequeue the ACK from the sender->input_framelist_head
    //    2) Convert the char * buffer to a Frame data type
    //    3) Check whether the frame is corrupted
    //    4) Check whether the frame is for this sender
    //    5) Do sliding window protocol for sender/receiver pair   
}

//帧最大序列号255
#define MAX_MESSAGE_SEQ 255
void handle_input_cmds(Sender * sender,
                       LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling input cmd
    //    1) Dequeue the Cmd from sender->input_cmdlist_head
    //    2) Convert to Frame
    //    3) Set up the frame according to the sliding window protocol
    //    4) Compute CRC and add CRC to Frame

    int input_cmd_length = ll_get_length(sender->input_cmdlist_head);
    
    //Recheck the command queue length to see if stdin_thread dumped a command on us
    input_cmd_length = ll_get_length(sender->input_cmdlist_head);
    while (input_cmd_length > 0)
    {
        //Pop a node off and update the input_cmd_length
        LLnode * ll_input_cmd_node = ll_pop_node(&sender->input_cmdlist_head);
        if((sender->messageSeq)>=MAX_MESSAGE_SEQ) sender->messageSeq = 0;
        input_cmd_length = ll_get_length(sender->input_cmdlist_head);

        //Cast to Cmd type and free up the memory for the node
        Cmd * outgoing_cmd = (Cmd *) ll_input_cmd_node->value;
        free(ll_input_cmd_node);
            

        //DUMMY CODE: Add the raw char buf to the outgoing_frames list
        //NOTE: You should not blindly send this message out!
        //      Ask yourself: Is this message actually going to the right receiver (recall that default behavior of send is to broadcast to all receivers)?
        //                    Does the receiver have enough space in in it's input queue to handle this message?
        //                    Were the previous messages sent to this receiver ACTUALLY delivered to the receiver?
        int msg_length = strlen(outgoing_cmd->message);
        if (msg_length > MAX_FRAME_SIZE)
        {
            fprintf(stderr,"<SEND_%d>: sending messages of length greater than %d will be splited\n", sender->send_id, FRAME_PAYLOAD_SIZE);
            ll_split_head(sender,outgoing_cmd,FRAME_PAYLOAD_SIZE);
            //Do something about messages that exceed the frame size  假如信息过长，要切分
        }
        else
        {
            //加入字符串没有过长，不用切分，直接封装进拆分消息列表里
            ll_append_node(&sender->splitlist, (void *)outgoing_cmd);
        }
        int splitlist_length = ll_get_length(sender->splitlist);
        fprintf(stderr,"%d\n",splitlist_length);
        while(splitlist_length>0){
            //如果缓冲区满了，消息不能发送，在队列里死等，直到发送缓冲区有空间
            while(sendBufferFull(sender) == -1);
            //This is probably ONLY one step you want
            LLnode *splitNode = ll_pop_node(&sender->splitlist);
            --splitlist_length;
            //填充发送帧的信息,添加了冗余码
            Frame * outgoing_frame = (Frame *) malloc (sizeof(Frame));
            strcpy(outgoing_frame->data, (char *)splitNode->value);
            free(splitNode);
            char ack = 0;
            outgoing_frame->sourceId = outgoing_cmd->src_id;
            outgoing_frame->destinationId = outgoing_cmd->dst_id;
            outgoing_frame->seq = (sender->messageSeq)++;
            outgoing_frame->ack = ack;
            char * outgoing_str = convert_frame_to_char(outgoing_frame);
            uint16_t crc = crc16(outgoing_str,MAX_FRAME_SIZE-2);
            outgoing_frame->crc = crc;

            //讲时间和帧一起放入缓冲区
            Timeout *timeout = (Timeout*)malloc(sizeof(Timeout));
            calculate_timeout(timeout);
            intoSendBuffer(sender,timeout,outgoing_frame);
            //Convert the message to the outgoing_charbuf
            char * outgoing_charbuf = convert_frame_to_char(outgoing_frame);
            ll_append_node(outgoing_frames_head_ptr,
                           outgoing_charbuf);
            //free(outgoing_frame); 先别释放，在缓冲区里
        }
        //At this point, we don't need the outgoing_cmd
        free(outgoing_cmd->message);
        free(outgoing_cmd);
    }   
}

//处理超时的帧
void handle_timedout_frames(Sender * sender,
                            LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling timed out datagrams
    //    1) Iterate through the sliding window protocol information you maintain for each receiver
    //    2) Locate frames that are timed out and add them to the outgoing frames
    //    3) Update the next timeout field on the outgoing frames
}


void * run_sender(void * input_sender)
{   
    //更高精度的时间计算。
    //struct timespec有两个成员，一个是秒，一个是纳秒, 所以最高精确度是纳秒。 
    struct timespec   time_spec;
    //struct timeval有两个成员，一个是秒，一个是微秒, 所以最高精确度是微秒。
    struct timeval    curr_timeval;
    const int WAIT_SEC_TIME = 0;
    const long WAIT_USEC_TIME = 100000; //一毫秒
    Sender * sender = (Sender *) input_sender;    
    LLnode * outgoing_frames_head;
    struct timeval * expiring_timeval;
    long sleep_usec_time, sleep_sec_time;
    
    //This incomplete sender thread, at a high level, loops as follows:
    //1. Determine the next time the thread should wake up
    //2. Grab the mutex protecting the input_cmd/inframe queues
    //3. Dequeues messages from the input queue and adds them to the outgoing_frames list
    //4. Releases the lock
    //5. Sends out the messages

    pthread_cond_init(&sender->buffer_cv, NULL);
    pthread_mutex_init(&sender->buffer_mutex, NULL);

    while(1)
    {    
        outgoing_frames_head = NULL;

        //Get the current time 获取当前时间,毫秒精度
        gettimeofday(&curr_timeval, 
                     NULL);

        //从毫秒精度手动将毫秒*1000，升级到纳秒精度 time_spec用来考虑什么时候唤醒线程
        //time_spec is a data structure used to specify when the thread should wake up
        //The time is specified as an ABSOLUTE (meaning, conceptually, you specify 9/23/2010 @ 1pm, wakeup)
        time_spec.tv_sec  = curr_timeval.tv_sec;
        time_spec.tv_nsec = curr_timeval.tv_usec * 1000;

        //Check for the next event we should handle 
        expiring_timeval = sender_get_next_expiring_timeval(sender);

        //Perform full on timeout
        if (expiring_timeval == NULL)
        {
            time_spec.tv_sec += WAIT_SEC_TIME;
            time_spec.tv_nsec += WAIT_USEC_TIME * 1000;
        }
        else
        {
            //Take the difference between the next event and the current time
            sleep_usec_time = timeval_usecdiff(&curr_timeval,
                                               expiring_timeval);

            //Sleep if the difference is positive
            if (sleep_usec_time > 0)
            {
                sleep_sec_time = sleep_usec_time/1000000;
                sleep_usec_time = sleep_usec_time % 1000000;   
                time_spec.tv_sec += sleep_sec_time;
                time_spec.tv_nsec += sleep_usec_time*1000;
            }   
        }

        //Check to make sure we didn't "overflow" the nanosecond field
        if (time_spec.tv_nsec >= 1000000000)
        {
            time_spec.tv_sec++;
            time_spec.tv_nsec -= 1000000000;
        }

        
        //*****************************************************************************************
        //NOTE: Anything that involves dequeing from the input frames or input commands should go 
        //      between the mutex lock and unlock, because other threads CAN/WILL access these structures
        //*****************************************************************************************
        pthread_mutex_lock(&sender->buffer_mutex);

        //Check whether anything has arrived
        int input_cmd_length = ll_get_length(sender->input_cmdlist_head);
        int inframe_queue_length = ll_get_length(sender->input_framelist_head);
        //没有确认报文到达或者没有消息要发送，线程等待一下
        //Nothing (cmd nor incoming frame) has arrived, so do a timed wait on the sender's condition variable (releases lock)
        //A signal on the condition variable will wakeup the thread and reaquire the lock
        if (input_cmd_length == 0 &&
            inframe_queue_length == 0)
        {
            
            pthread_cond_timedwait(&sender->buffer_cv, 
                                   &sender->buffer_mutex,
                                   &time_spec);
        }
        //Implement this
        handle_incoming_acks(sender,
                             &outgoing_frames_head);

        //Implement this
        handle_input_cmds(sender,
                          &outgoing_frames_head);

        pthread_mutex_unlock(&sender->buffer_mutex);


        //Implement this
        handle_timedout_frames(sender,
                               &outgoing_frames_head);

        //CHANGE THIS AT YOUR OWN RISK!
        //Send out all the frames
        int ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        
        while(ll_outgoing_frame_length > 0)
        {
            LLnode * ll_outframe_node = ll_pop_node(&outgoing_frames_head);
            char * char_buf = (char *)  ll_outframe_node->value;

            //Don't worry about freeing the char_buf, the following function does that
            send_msg_to_receivers(char_buf);

            //Free up the ll_outframe_node
            free(ll_outframe_node);
            --ll_outgoing_frame_length;
            // ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        }
    }
    pthread_exit(NULL);
    return 0;
}
