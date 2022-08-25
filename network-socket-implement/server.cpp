// ./server [4097]
// recvfrom(fd, buf, BUFF_LEN, 0, (struct sockaddr *)&clent_addr, &len);
// sendto(fd, buf, BUFF_LEN, 0, (struct sockaddr *)&clent_addr, len);
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <algorithm>
#include "opencv2/opencv.hpp"
#include <iostream>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include <signal.h>
#include <unistd.h>

#ifdef __cplusplus
}
#endif

#define DSK "serverfolder/"
#define BUFF_SIZE 1024
#define MAX_LENGTH 1024
#define TIMEOUT 1
using namespace std;
using namespace cv;
#define BUFF_LEN 1024

unsigned char cvbuffer[32][1024 * 4];
char filename[BUFF_LEN] = {"./"};
int C = 100; //count frames
typedef struct
{
    int length;
    int seqnumber;
    int acknumber;
    int fin;
    int syn;
    int ack;
} header;
typedef struct
{
    header head;
    char data[1024 * 4];
} segment;
void convertUnCharToStr(char *str, unsigned char *UnChar, int ucLen)
{
    int i = 0;
    for (i = 0; i < ucLen; i++)
    {
        //格式化輸str,每unsigned char 轉換字元佔兩位置%x寫輸%X寫輸
        sprintf(str + i * 2, "%02x", UnChar[i]);
    }
}

void handlealarm(int s)
{
}
void *opencv(void *s)
{
    Mat imgServer;
    VideoCapture cap(filename);
    imgServer = Mat::zeros(540, 960, CV_8UC3);
    if (!imgServer.isContinuous())
    {
        imgServer = imgServer.clone();
    }
    int count = 0;
    while (1)
    {

        int terminal = 0;
        if (imgServer.empty())
        {
            break;
        }
        if (count >= 32)
        {
            count = count % 32;
        }
        //get a frame from the video to the container on server.
        cap >> imgServer;

        // get the size of a frame in bytes
        int imgSize = imgServer.total() * imgServer.elemSize();

        // allocate a buffer to load the frame (there would be 2 buffers in the world of the Internet)

        // copy a frame to the buffer
        int K=imgSize/1024*4;
        if(imgSize%(1024*4)!=0){
            K+=1;
        }
        // for(int i=0;i<K;i++){
            
        //     memcpy(cvbuffer[count],imgServer.data+i*(1024*4),1024*4);
        //     count++;
        //     C++;
        // }
       memcpy(cvbuffer[count],imgServer.data,1024*4);
       C++;
       count++;
        
        imshow("server", imgServer);
       
        char c = (char)waitKey(33.3333);
        if (c == 27)
            break;
    }
}
////////////////////////////////////////////////////
int maximum(int a, int b)
{
    if (a < b)
        return b;
    else
        return a;
}

segment makepacket(int length, int seqn, int ackn, int fin, int ack, char data[])
{

    segment A;
    A.head.length = length;
    A.head.seqnumber = seqn;
    A.head.acknumber = ackn;
    A.head.fin = fin;
    A.head.syn = 0;
    A.head.ack = ack;
    bzero(A.data, 1024 * 4);
    strcpy(A.data, data);
   
    return A;
}
void convertStrToUnChar(char* str, unsigned char* UnChar)  
{  
    int i = strlen(str), j = 0, counter = 0;  
    char c[2];  
    unsigned int bytes[2];  
  
    for (j = 0; j < i; j += 2)   
    {  
        if(0 == j % 2)  
        {  
            c[0] = str[j];  
            c[1] = str[j + 1];  
            sscanf(c, "%02x" , &bytes[0]);  
            UnChar[counter] = bytes[0];  
            counter++;  
        }  
    }  
    return;  
}  
void udpevent(int fd, struct sockaddr_in dst)
{
    // pthread_t t1;
    // pthread_create(&t1, NULL, &opencv, NULL);
    // sleep(2);

    char transferdata[1024 * 4] = {};
    struct sockaddr_in recv;
    socklen_t len = sizeof(sockaddr_in);

    int windowsize = 1;
    int threshold = 16;
    int seqn = 1;
    int base = 0;

    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_handler = handlealarm;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGALRM, &action, 0);

    int segmentsize = 1024 * 4;
    int datalen;
    int count = 0;
    char zer[1024 * 4];
    int terminal = 0;
    int END = 0;
    while (1)
    {
        while ((seqn <= C) && (seqn - base) <= windowsize)
        {

            segment transferpacket;
            if (seqn == C)
            {
                bzero(transferdata, 1024 * 4);
                transferpacket = makepacket(0, seqn, base, 1, 0, transferdata);

                sendto(fd, &transferpacket, sizeof(transferpacket), 0, (struct sockaddr *)&dst, sizeof(dst)) != sizeof(transferpacket);

                printf("send	fin\n");
            }
            else
            {

                if (seqn == 1)
                {
                    count = 0;
                }
                else
                {
                    count = (seqn - 1) % 32;
                }

                bzero(transferdata, 1024 * 4);
                convertUnCharToStr(transferdata, cvbuffer[count], strlen((char *)cvbuffer[count]));
                transferpacket = makepacket(strlen((char *)cvbuffer[count]), seqn, base, 0, 0, transferdata);

                printf("send	data	#%d,	windowsize = %d\n", transferpacket.head.seqnumber, windowsize);
            }
            sendto(fd, &transferpacket, sizeof(transferpacket), 0, (struct sockaddr *)&dst, sizeof(dst)) != sizeof(transferpacket);

            seqn++;
        }
        alarm(TIMEOUT);
        segment receivepacket;

        while (recvfrom(fd, &receivepacket, sizeof(receivepacket), 0, (struct sockaddr *)&recv, &len) < 0)
        {
            if (errno == EINTR)
            { //timeout
               
                seqn = base + 1;
               
                threshold = maximum(1, windowsize / 2);
                printf("time	out,		threshold = %d\n", threshold);
                windowsize = 1;

                while ((seqn <= C) && (seqn - base) <= windowsize)
                {

                    segment transferpacket;
                    if (seqn == C)
                    {
                        bzero(transferdata, 1024 * 4);
                        transferpacket = makepacket(0, seqn, base, 1, 0, transferdata);
                        printf("send	fin\n");
                    }
                    else
                    {

                        if (seqn == 1)
                        {
                            count = 0;
                        }
                        else
                        {
                            count = (seqn - 1) % 32;
                        }

                        bzero(transferdata, 1024 * 4);
                        convertUnCharToStr(transferdata, cvbuffer[count], strlen((char *)cvbuffer[count]));
                        transferpacket = makepacket(strlen((char *)cvbuffer[count]), seqn, base, 0, 0, transferdata);

                        printf("send	data	#%d,	windowsize = %d\n", transferpacket.head.seqnumber, windowsize);
                    }
                    sendto(fd, &transferpacket, sizeof(transferpacket), 0, (struct sockaddr *)&dst, sizeof(dst)) != sizeof(transferpacket);

                    seqn++;
                }
                alarm(TIMEOUT);
            }
            else
            {
            }
        }

        if (receivepacket.head.fin == 1)
        {
            printf("recv	finack\n");
            exit(0);
        }
        else
        {
            printf("recv	ack 	#%d\n", receivepacket.head.seqnumber);
            if (receivepacket.head.seqnumber > base)
            {
                base = receivepacket.head.seqnumber;
                if (windowsize < threshold)
                {
                    windowsize *= 2;
                }
                else
                {
                    windowsize += 1;
                }
                if (base == 1)
                {
                    count = 0;
                }
                else
                {
                    count = (base - 1) % 32;
                }
               memset(cvbuffer[count],0,1024*4);
               
                
                
                alarm(0);
            }
            else
            {
            }
        }
    }
    close(fd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    char serverport[BUFF_LEN] = {};
    int serverportt;
    char serveraddr[BUFF_LEN] = {};
    int serveraddrr;
    char clientport[BUFF_LEN] = {};
    int clientportt;
    char clientaddr[BUFF_LEN] = {};
    int clientaddrr;
    if (argc != 6)
    {
        printf("./server serveraddr serverport clientaddr clientport filename\n");
        exit(0);
    }
    strcpy(serveraddr, argv[1]);
    strcpy(serverport, argv[2]);
    strcpy(clientaddr, argv[3]);
    strcpy(clientport, argv[4]);
    strcat(filename, argv[5]);
    serverportt = atoi(serverport);
    serveraddrr = atoi(serveraddr);
    clientaddrr = atoi(clientaddr);
    clientportt = atoi(clientport);

    int server_fd, ret;
    struct sockaddr_in ser_addr, client_addr;

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0)
    {
        printf("create socket fail!\n");
        return -1;
    }

    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(serveraddr);
    ser_addr.sin_port = htons(serverportt);
    memset(ser_addr.sin_zero, '\0', sizeof(ser_addr.sin_zero));
    ret = bind(server_fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
    if (ret < 0)
    {
        printf("socket bind fail!\n");
        return -1;
    }
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(clientaddr);
    client_addr.sin_port = htons(clientportt);
    memset(client_addr.sin_zero, '\0', sizeof(client_addr.sin_zero));

    udpevent(server_fd, client_addr);

    close(server_fd);
exit:
    return 0;
}
