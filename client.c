#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
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
  int packetNum;
  
  // Use to check if packets are received in the correct order
  int correctOrder[5] = {1, 2, 3, 4, 5};
  //Can't figure out how to append each packet number into the array and then check
  int receivedOrder[5];
  
  //int pSent;
  char npr[2] = { "ZR" };
  int packets = 0;
  while (1){								
    char * recvData;				
    recvData = (char*) malloc (1024);
    char response[2] = {expected, 'R'};	  
    pLen = recvfrom(sockfd, recvData, 1024, 0, NULL, NULL);
    

       
    //printf("Packet received from server.\n");
    
    if (pLen > -1){		
		//int packetNum = recvData[1020] -'0';
		//printf("Packet %d\n", packetNum);		
		    // Check and print off packet number as they are received
    packetNum = recvData[11] - '0';
    printf("Packet #%d received from server\n",packetNum); 
    
    //Keep track of what number order packet you're at in order to test if correct packet received
    if(packets <= 5)
    {
		packets++; 
	} else
	{
		packets = 0;
	}
	printf("Expected : %d\n", packets);			
      if (recvData[0] == 'A'){			
        if (expected == 'A'){
          expected = 'B';
        } else {
          expected = 'A';     
        }
        //printf ("recvData[9]: %c\n", recvData[9]);
        
    	int ch1 = recvData[9] - '0';
    	printf ("ch1: %d\n",ch1);
    	int ch2 = recvData[10] - '0';
    	printf ("ch2: %d\n",ch2);
    	int ch;
    	if ( ch2 > 0)
    		ch = ch1*10 + ch2;
    	else
    		ch = ch1;
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
          		//packets++;
        	}else 
          		printf("Error!\n");
        }
      } else if (recvData[1] == 'E'){		
        printf("EOF command received\n");	
        response[0] = 'E'; response[1] = 'R';
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)serveraddr, sizeof(*serveraddr));
        //pSent = atoi(&recvData[8]);
        //printf("Received %d packets. \n",packets);
        fclose(fp);
        break;
      } else if (pLen > -1){				
       // printf("Got unexpected Char!\n");
       // printf("received:%s\n", recvData);
      }
    } else {
		
      printf("TIMEOUT: No Packet received\n");
      sendto(sockfd, npr, strlen(npr), 0, (struct sockaddr*)serveraddr, sizeof(*serveraddr));
    }
    
  }
	return 0;
}

