#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
FILE *fp; 

bool checksum(char * buf, int check){
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
    printf("check: %d\n", check);
    

    return check==remainder;

} 

int main(int argc, char **argv){
	
	if(argc != 3)							
	{										
		printf("Incorrect number of arguments - 3 required. The program will terminate.\n");
		return 0;
	}
	char* serverIP = argv[1];				
	int serverPort = atoi(argv[2]);
											
	int sockfd;
	struct sockaddr_in *serveraddr = malloc(sizeof(struct sockaddr_in));
	struct timeval to;
	
	serveraddr->sin_family = AF_INET;
	serveraddr->sin_port = htons(serverPort);
	serveraddr->sin_addr.s_addr = inet_addr(serverIP);

	to.tv_sec = 2;
	to.tv_usec = 0;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
	
  bool waitingForHS = true;					
  while (waitingForHS){
    printf("Enter a filename to get: ");
    char inputFilename[5000] = { 0 }; 
    fgets(inputFilename, 5000, stdin);
    int length = strlen(inputFilename) - 1;
    inputFilename[length] = '\0';

    while(1){ 								
	    sendto(sockfd, inputFilename, strlen(inputFilename), 0, (struct sockaddr*)serveraddr, sizeof(*serveraddr));
      printf("Connection request sent, waiting for 2s\n");
      char recvData[5000] = { 0 };    
      recvfrom(sockfd, recvData, 5000, 0, NULL, NULL);
											
      if (strcmp(recvData, inputFilename)  == 0){
        printf("Connection confirmed.\n");
        waitingForHS = false;
        fp = fopen(inputFilename, "w+b");	
        break;
      } else {
        printf("Bad filename, received: \"%s\"\n",recvData);
        break;    
      }
    }
  }
											
  char expected = 'A';
  int pLen;
  int pSent;
  char npr[2] = { "ZR" };
  int packets = 0;
  while (1){								
    unsigned char * recvData;				
    recvData = (char*) malloc (1024);
    char response[2] = {expected, 'R'};	  
    pLen = recvfrom(sockfd, recvData, 1024, 0, NULL, NULL);
    printf("Packet received from server.\n");
    if (pLen > -1){							
      if (recvData[0] == 'A'){			
        printf ("recvData[9]: %c\n", recvData[9]);
        
    	int ch = recvData[9] - '0';
        printf ("ch: %d\n",ch);

    	if (!checksum(&recvData[40],ch)){

            printf("nope");

    	}
    	else {									
        	fwrite(&recvData[40], 1, (pLen - 43), fp);
        	recvData = NULL;					
        	sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)serveraddr, sizeof(*serveraddr));
        	printf (" pLen = %d ", pLen);
        	printf("Packet ack sent\n");
        	if (!feof(fp)){						
          		packets++;
        	}else 
          		printf("Error!\n");
        }
      } else if (recvData[1] == 'E'){		
        printf("EOF command received\n");	
        response[0] = 'E'; response[1] = 'R';
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)serveraddr, sizeof(*serveraddr));
        pSent = atoi(&recvData[8]);
        printf("Received %d packets. \n",packets);
        fclose(fp);
        break;
      } else if (pLen > -1){				
        printf("Got unexpected Char!\n");
        printf("received:%s\n", recvData);
      }
    } else {
      printf("TIMEOUT: No Packet received\n");
      sendto(sockfd, npr, strlen(npr), 0, (struct sockaddr*)serveraddr, sizeof(*serveraddr));
    }
    
  }
	return 0;
}

