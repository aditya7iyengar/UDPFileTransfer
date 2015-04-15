#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

FILE *fp;
int packetRecvd, packet_loss;

struct numbererd_packet{
    unsigned char * data;
    int number;
};

int checksum(char * buf){
    
    int remainder;
    char * iter = buf;
    uint sum =0;
    while (!(*iter == '/' && *(iter + 1) =='@' && *(iter + 2) == '/')){
        //printf("iter:%c", *iter);
        sum += *iter;
        iter++;
    }
    printf ("sum: %d\n",sum);
    
    remainder = sum%16;
    
    printf("remainder: %d\n", remainder);
    
    return remainder;
}

int main(int argc, char **argv){
    
    int cport = 0;
    
    if(argc != 2)
    {
        printf("Incorrect number of arguments - 2 required. The program will terminate.\n");
        return 0;
    }
    
    cport = atoi(argv[1]);
    printf("Listening on port #%d.\n", cport);
    
    unsigned int sockfd, len;
    struct sockaddr_in *serveraddr = malloc(sizeof(struct sockaddr_in));
    struct sockaddr_in *clientaddr = malloc(sizeof(struct sockaddr_in));
    struct timeval to;
    
    serveraddr->sin_family = AF_INET;
    serveraddr->sin_port = htons(cport);
    serveraddr->sin_addr.s_addr = htonl(INADDR_ANY);
    
    to.tv_sec = 10;
    to.tv_usec = 0;
    
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bind(sockfd, (struct sockaddr *) serveraddr, sizeof(*serveraddr));
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
    
    
    while(1){
        len=sizeof(*clientaddr);
        char filename[5000] = { 0 };
        //int opened = 0;
        printf("Waiting for connection.\n");
        packetRecvd = recvfrom(sockfd, filename, 5000, 0, (struct sockaddr *)clientaddr, &len);
        if (packetRecvd > -1){
            printf("Trying to open filename:%s:\n", filename);
            fp=fopen(filename, "rb");
            if (fp != NULL && packetRecvd > -1){
                sendto(sockfd, filename, sizeof(filename), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
                printf("Connection response sent.\n");
                break;
            } else {
                char response[] = "Bad File Name";
                sendto(sockfd, response, sizeof(response), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
                printf("Bad filename, response sent.\n");
            }
        }
    }
    
    char expected = 'A';
    //int exp_packet = 0;
    char response[2] = { 0 };
    bool waitingForResponse = true;
    bool initialized = false;
    char * buffer;
    char * buffer2;
    char * buffer3;
    char * buffer4;
    char * buffer5;
    //char * bufferAdi; 
    buffer = (char*) malloc (1024);
    buffer2 = (char*) malloc (1024);
    buffer3 = (char*) malloc (1024);
    buffer4 = (char*) malloc (1024);
    buffer5 = (char*) malloc (1024);
    //bufferAdi = (char*) malloc (1024);
    int bytes = 0;
    int bytes_read = 0;
    int bytes_read2 = 0;
    int bytes_read3 = 0;
    int bytes_read4 = 0;
    int bytes_read5 = 0;
    while (1){
        if (initialized){
        	buffer5 = (char*) malloc (1024);
        	buffer5[0] = 'q';
            bytes_read5 = fread(&buffer5[40], 1, 981, fp);
            buffer5[bytes_read5+40] = '/';
            buffer5[bytes_read5+41] = '@';
            buffer5[bytes_read5+42] = '/';
        }
        else {
            bytes_read = fread(&buffer[40], 1, 981, fp);            
            buffer[1021] = '/';
            buffer[1022] = '@';
            buffer[1023] = '/';
            
            bytes_read2 = fread(&buffer2[40], 1, 981, fp);            
            buffer2[1021] = '/';
            buffer2[1022] = '@';
            buffer2[1023] = '/';
            
            bytes_read3 = fread(&buffer3[40], 1, 981, fp);          
            buffer3[1021] = '/';
            buffer3[1022] = '@';
            buffer3[1023] = '/';
            
            bytes_read4 = fread(&buffer4[40], 1, 981, fp);           
            buffer4[1021] = '/';
            buffer4[1022] = '@';
            buffer4[1023] = '/';
            
            bytes_read5 = fread(&buffer5[40], 1, 981, fp);         
            buffer5[1021] = '/';
            buffer5[1022] = '@';
            buffer5[1023] = '/';
        }
        //buffer[8] = checksum(buffer[40]);
        
    	int n = checksum(&buffer[40]);
    	char num[sizeof(int)*3+2];
    	snprintf(num, sizeof(num), "%d", n);
    	printf("num:%s\n", num);
    	printf("num[0]:%c\n", num[0]);
    	printf("num[1]:%c\n", num[1]);
    	buffer[0] = 'A';
    	buffer[9] = num[0];
    	buffer[10] = num[1];
    	
    	buffer[11] = '1';
    	buffer2[11] = '2';
    	buffer3[11] = '3';
    	buffer4[11] = '4';
    	buffer5[11] = '5';
    	
    	
        if (expected == 'A'){
            expected = 'B';
        } else if (expected == 'B'){
            expected = 'A';
        }
        if (ferror(fp)){
            printf("Error reading file\n");
            return 0;
        }
        
        
        sendto(sockfd, buffer, (bytes_read + 43), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
            printf("Packet %d sent.\n", (buffer[11]-'0'));
        sendto(sockfd, buffer2, (bytes_read2 + 43), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
			printf("Packet %d sent.\n", (buffer2[11]-'0'));
        sendto(sockfd, buffer3, (bytes_read3 + 43), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
			printf("Packet %d sent.\n", (buffer3[11]-'0'));
        sendto(sockfd, buffer4, (bytes_read4 + 43), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
			printf("Packet %d sent.\n", (buffer4[11]-'0'));
        sendto(sockfd, buffer5, (bytes_read5 + 43), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
			printf("Packet %d sent.\n", (buffer5[11]-'0'));
        
        printf("5 packets sent, waiting for response\n");
        bytes+=(bytes_read + bytes_read2 + bytes_read3 + bytes_read4 + bytes_read5);
        waitingForResponse = true;
        while (waitingForResponse){
            packetRecvd = recvfrom(sockfd, response, 2, 0, (struct sockaddr *)clientaddr, &len);
            if (packetRecvd > -1){
                if (response[0] == 'Z'){
                    printf("Client timeout, resending previous packet.\n");
                    sendto(sockfd, buffer, bytes_read + 43, 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
                    sendto(sockfd, buffer2, (bytes_read2 + 43), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
                    sendto(sockfd, buffer3, (bytes_read3 + 43), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
                    sendto(sockfd, buffer4, (bytes_read4 + 43), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
                    sendto(sockfd, buffer5, (bytes_read5 + 43), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
                    printf("packets re-sent, waiting for response\n");
                    bytes+=(bytes_read + bytes_read2 + bytes_read3 + bytes_read4 + bytes_read5);;
                } else if (response[0] == 'E'){
                    printf("Received final ack\n");
                    return 0;
                } else if (response[0] != expected && response[1] == 'R'){
                    printf("Client Received packets.\n");
                    free (buffer);
                    buffer = buffer2;
                    buffer2 = buffer3;
                    buffer3 = buffer4;
                    buffer4 = buffer5;
                    buffer2[0] = 'q';
                    buffer3[0] = 'q';
                    buffer4[0] = 'q';
                    buffer5[0] = 'q';
                    initialized = true;
                    waitingForResponse = false;
                }
            }
            if (feof(fp)) {							
                printf("Server Sent bytes:%d\n", bytes);
                buffer[1] = 'E';
                char integer_string[32];
                sprintf(integer_string, "%d", bytes);
                strcpy(&buffer[8], integer_string);
                sendto(sockfd, buffer, (bytes_read + 43), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
                printf("final packet sent\n");
            }
        }
        
    }
    return 0;
}
