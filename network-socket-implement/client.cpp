// ./client [127.0.0.1:4097]
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include "opencv2/opencv.hpp"
#include <iostream>
#ifdef __cplusplus
extern "C"
{
#endif
#include <signal.h>

#ifdef __cplusplus
}
#endif

#define DSK "serverfolder/"
#define BUFF_SIZE 1024
#define MAX_LENGTH 1024
#define TIMEOUT 1
using namespace std;
using namespace cv;
unsigned char cvbuffer[32][1024 * 4];
#define BUFF_LEN 1024
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
    char data[1024*4];
} segment;
void convertStrToUnChar(char *str, unsigned char *UnChar)
{
    int i = strlen(str), j = 0, counter = 0;
    char c[2];
    unsigned int bytes[2];

    for (j = 0; j < i; j += 2)
    {
        if (0 == j % 2)
        {
            c[0] = str[j];
            c[1] = str[j + 1];
            sscanf(c, "%02x", &bytes[0]);
            UnChar[counter] = bytes[0];
            counter++;
        }
    }
    return;
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
    // strcpy(A.data,);

    return A;
}
void *opencv(void *s)
{
    sleep(3);
    Mat imgClient;
    imgClient = Mat::zeros(540, 960, CV_8UC3);

    int count = 0;
    char zer[1024 * 4];
    while (1)
    {
        if (count >= 32)
        {
            count = count % 32;
        }
        memcpy(imgClient.data, cvbuffer[count], strlen((char *)cvbuffer[count]));
        count++;
        

        imshow("client", imgClient);
        //Press ESC on keyboard to exit
        // notice: this part is necessary due to openCV's design.
        // waitKey means a delay to get the next frame.
        char c = (char)waitKey(33.3333);
        if (c == 27)
            break;
    }
}

void del(char sum[], int del, int len)
{
    int i = 0;
    for (; i < strlen(sum) - len - del; i++)
    {
        sum[del + i] = sum[len + del + i];
    }

    sum[(del + strlen(sum) - len - del)] = '\0';
}

void udpevent(int fd, struct sockaddr_in dst)
{

    struct sockaddr_in recv;
    socklen_t len = sizeof(dst);
    int seqn = 1;

    int count = 0;
    char buffer[1024 * 4] = {};
    pthread_t t1;
    pthread_create(&t1, NULL, &opencv, NULL);
   
    while (1)
    {
        
        segment receivepacket;
        segment transferpacket;

        memset(&receivepacket, 0, sizeof(receivepacket));
        memset(&transferpacket, 0, sizeof(transferpacket));
        if (recvfrom(fd, &receivepacket, sizeof(receivepacket), 0, (struct sockaddr *)&recv, &len) != sizeof(receivepacket))
        {
            printf("recv fail");
            exit(0);
        }
        if(receivepacket.head.length!=strlen((char*)receivepacket.data)){
            printf("recv fail");
            exit(0);
        
        }
        if (receivepacket.head.fin == 1 && receivepacket.head.seqnumber == seqn)
        { //over
            printf("recv	fin\n");
            transferpacket = makepacket(0, seqn, seqn, 1, 1, buffer);
            if (sendto(fd, &transferpacket, sizeof(transferpacket), 0, (struct sockaddr *)&dst, sizeof(dst)) != sizeof(transferpacket))

            {
                printf("send fail");
                exit(0);
            }

            printf("send	finack\n");
            exit(0);
        }

        else
        {

            if (receivepacket.head.seqnumber == seqn)
            {
               
                if (receivepacket.head.seqnumber % 32 == 0)
                {
                    for (int i = 0; i < 32; i++)
                    {
                        memset(cvbuffer[i], 0, 1024 * 4);
                    }
                    printf("flush\n");
                }
                printf("recv	data 	#%d\n", seqn);
                printf("send	ack 	#%d\n", seqn);

                transferpacket = makepacket(0, seqn, seqn, 0, 1, buffer);
                if (seqn == 1)
                {
                    count = 0;
                }
                else
                {
                    count = (seqn - 1) % 32;
                }
                
                convertStrToUnChar(receivepacket.data, cvbuffer[count]);
                

                if (sendto(fd, &transferpacket, sizeof(transferpacket), 0, (struct sockaddr *)&dst, sizeof(dst)) != sizeof(transferpacket))

                {
                    printf("send fail");
                    exit(0);
                }
                seqn += 1;
            }
            else
            {
                printf("drop	data	#%d\n", receivepacket.head.seqnumber);
                transferpacket = makepacket(0, seqn - 1, seqn - 1, 0, 1, buffer);
                if (sendto(fd, &transferpacket, sizeof(transferpacket), 0, (struct sockaddr *)&dst, len) != sizeof(transferpacket))

                {
                    printf("send fail");
                    exit(0);
                }
                printf("send	ack 	#%d\n", seqn - 1);
            }
        }
    }
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
    if (argc != 5)
    {
        printf("./server serveraddr serverport clientaddr clientport\n");
        exit(0);
    }
    strcpy(serveraddr, argv[1]);
    strcpy(serverport, argv[2]);
    strcpy(clientaddr, argv[3]);
    strcpy(clientport, argv[4]);
    serverportt = atoi(serverport);
    serveraddrr = atoi(serveraddr);
    clientaddrr = atoi(clientaddr);
    clientportt = atoi(clientport);

    int client_fd, ret;
    struct sockaddr_in ser_addr, client_addr;

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd < 0)
    {
        printf("create socket fail!\n");
        return -1;
    }

    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(serveraddr);
    ser_addr.sin_port = htons(serverportt);
    memset(ser_addr.sin_zero, '\0', sizeof(ser_addr.sin_zero));

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(clientaddr);
    client_addr.sin_port = htons(clientportt);
    memset(client_addr.sin_zero, '\0', sizeof(client_addr.sin_zero));
    ret = bind(client_fd, (struct sockaddr *)&client_addr, sizeof(client_addr));
    if (ret < 0)
    {
        printf("socket bind fail!\n");
        return -1;
    }

    udpevent(client_fd, ser_addr);
    close(client_fd);

    return 0;
}
