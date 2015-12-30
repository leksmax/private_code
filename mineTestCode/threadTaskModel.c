int threadTask(void *arg)
{    
    //ͳһ�����߳�ѭ������
    fd_set rset;
    int ret, nfds, pipefd;
    struct timeval timeout = {0};
    //STUN_DWORD lastTime, currentTime;
    
    STUN_BYTE *sendMsg = NULL;
    STUN_DWORD sendMsgSize;
    STUN_BYTE *recvMsg = NULL;
    STUN_DWORD recvMsgSize;
    STUN_DWORD recvMsgType;
    
    TR069_STUN_MESSAGE validInfo;
    memset(&validInfo, 0, sizeof(validInfo));

    //���socket�������ⲿͨ��
    validInfo.sockfd = stun_UDPOpenSocket(AF_INET, SOCK_DGRAM, 0);
    if(validInfo.sockfd < 0){
        TR069Printf("open the udp socket failed\n");
        return RESULT_ERROR;
    }

    /* Initialize */
    //��ȡ���عܵ������ڲ�ͨ��
    g_stunPipe = tr069_msgq_create(sizeof(struct Message), 20);
    pipefd = tr069_msgq_fd(g_stunPipe);

    
    validInfo.type = BINDING_REQ_BASIC;
    validInfo.state = STUN_TASK_SUSPEND;
    currentTime = tr069_sec( );
    lastTime = currentTime - STUN_REBIND_INTERVAL;
    //ѭ������
    while(1){
        //�ж�ѭ�����õ���ָ��
        if(sendMsg != NULL){
            free(sendMsg);
            sendMsg = NULL;
        }
        
        if(recvMsg != NULL){
            free(recvMsg);
            recvMsg = NULL;
        }
        //ѭ���У���select���г�ʼ��
        FD_ZERO(&rset);
        FD_SET(pipefd, &rset);
        nfds = pipefd;
        //�����switch�ܿ����̵߳Ĺ���ͼ���
        switch(validInfo.state){
            case STUN_TASK_ACTIVE:
                timeout.tv_sec = STUN_REBIND_INTERVAL;
                
                FD_SET(validInfo.sockfd, &rset);
                if(nfds < validInfo.sockfd){
                    nfds = validInfo.sockfd;
                }

                switch(validInfo.type){
                    case BINDING_REQ_BASIC:
                    case BINDING_CHANGE_BASIC:
                        currentTime = tr069_sec();
                        if(currentTime - lastTime < STUN_REBIND_INTERVAL){
                            break;
                        }
                        lastTime = currentTime;
                        
                        /* New session. Generate the unique transaction  ID. */
                        stun_GenerateTransactionID(&(validInfo.bindingReqID));
                        stun_PackBindingRequestMsg(&(validInfo.bindingReqID), validInfo.type, &sendMsg, &sendMsgSize);
                        stun_UDPSendMsg(validInfo.sockfd, sendMsg, sendMsgSize);
                        break;
                    case BINDING_REQ_AUTHORIZED:
                    case BINDING_CHANGE_AUTHORIZED:
                        stun_PackBindingRequestMsg(&(validInfo.bindingReqID), validInfo.type, &sendMsg, &sendMsgSize);
                        stun_UDPSendMsg(validInfo.sockfd, sendMsg, sendMsgSize);
                        break;
                    default:
                        break;
                }
                
                break;
            case STUN_TASK_SUSPEND:
                timeout.tv_sec = 0x7fffffff;
                break;
            default:
                break;
        }

        TR069Printf(" timeout = %ld, sockfd = %u, pipefd = %d, nfds = %d\n",
                timeout.tv_sec, validInfo.sockfd, pipefd, nfds);
        //�Ա��ص�pipe������socket����ͳһ��select
        ret = select( nfds + 1, &rset, NULL, NULL, &timeout );
        if(ret <= 0){
            validInfo.type = BINDING_REQ_BASIC;
            TR069Printf("timeout\n");
            continue;
        }
        //�ȶԱ���pipe�����б���Ϊ�������̹߳���
        if(FD_ISSET(pipefd, &rset)){
            /* ACS disable/enable the STUN function */
            struct Message msg;
            if (tr069_msgq_getmsg(g_stunPipe, (char *)(&msg))){
                TR069Printf("STUN pipe read failed\n");
                continue;
            }
            
            TR069Printf("stun recv message id = %d, arg = %d\n", msg.id, msg.arg0);
            
            switch(msg.id){
                case STUN_TASK_ACTIVE:
                case STUN_TASK_SUSPEND:
                    validInfo.state = msg.id;
                    break;
                default:
                    break;
            }
            //������socket�����б𣬲��ҽ�������
        }else if(FD_ISSET(validInfo.sockfd, &rset)){
            if(stun_UDPRecvMsg(validInfo.sockfd, &recvMsg, &recvMsgSize) < 0){
                TR069Printf("receive message error\n");
                continue;
            }
            //����Ϣ���������ͽ��й��̷���
            recvMsgType = stun_GetMessageType(recvMsg, recvMsgSize);
            switch(recvMsgType){
            case TR111_CONNECTION_REQUEST_TYPE:
                stun_ProcessUPDConnectRequestMsg(recvMsg, recvMsgSize, &validInfo);
                
                break;
            case BINDING_RESPONSE:
            case BINDING_ERROR_RESPONSE:
                stun_ProcessBindingResponseMsg(recvMsg, recvMsgSize, &validInfo );
                
                break;
            default:
                TR069Printf("Unrecognized message\n");
                break;
            }
        }    
    }
    //�ر�����socket
    stun_UDPCloseSocket(validInfo.sockfd);
    
    return 0;
}
