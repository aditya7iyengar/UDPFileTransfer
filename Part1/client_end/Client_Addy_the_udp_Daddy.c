#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
FILE *fp; 

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
      if (recvData[0] == expected){			//If it was an 'A' packet, we flip the expected to a 'B', and vise versa
        if (expected == 'A'){
          expected = 'B';
        } else {
          expected = 'A';     
        }									//We then write to our file using the data received (from the 8th index in our array)
        fwrite(&recvData[8], 1, (pLen - 8), fp);
        recvData = NULL;					//A response packet is constructed and sent back to the server
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)serveraddr, sizeof(*serveraddr));
        printf("Packet ack sent\n");
        if (!feof(fp)){						
          packets++;
        } else 
          printf("Error!\n");
      } else if (recvData[0] == 'E'){		//Check to see if we received a special EOF packet, and if so then send back a special EOF
        printf("EOF command received\n");	//response packet and print out the packets sent/received, as well as close the file.
        response[0] = 'E'; response[1] = 'R';
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)serveraddr, sizeof(*serveraddr));
        pSent = atoi(&recvData[8]);
        printf("Received %dpackets. \n",packets);
        fclose(fp);
        break;
      } else if (pLen > -1){				//Other wise, we got something unexpected and need to break.
        printf("Got unexpected Char!\n");
        printf("received:%s\n", recvData);
        break;
      }
    } else {
      printf("TIMEOUT: No Packet received\n");
      sendto(sockfd, npr, strlen(npr), 0, (struct sockaddr*)serveraddr, sizeof(*serveraddr));
    }
    
  }
	return 0;
}

