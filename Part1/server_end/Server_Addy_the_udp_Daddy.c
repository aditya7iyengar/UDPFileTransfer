#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

FILE *fp;
int packetRecvd, packet_loss;

int main(int argc, char **argv){

	int cport = 0;							
											
	if(argc != 2)
	{
		printf("Incorrect number of arguments - 2 required. The program will terminate.\n");
		return 0;
	}
	
	cport = atoi(argv[1]);
	printf("Listening on port #%d.\n", cport);
											
	int sockfd, len;						
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
    len=sizeof(clientaddr);
    char filename[5000] = { 0 };
    int opened = 0;
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
  char response[2] = { 0 };
  bool waitingForResponse = true;
  int bytes = 0, randNum;
  while (1){								// beginning of loop sets up variables for use later
    int bytes_read = 0;
    unsigned char * buffer;
    buffer = (char*) malloc (1024);
    buffer[0] = expected;					//places the expected header in the first index of the array
    if (expected == 'A'){					//and appropriately flips the expected value to 'A' or 'B'
      expected = 'B';     
    } else if (expected == 'B'){ 
      expected = 'A';
    }
    bytes_read = fread(&buffer[8], 1, 1016, fp);
    if (ferror(fp)){							
      printf("Error reading file\n");			
      return 0;
    }
    randNum = rand() % 100 + 1;					
    
    sendto(sockfd, buffer, (bytes_read + 8), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
    printf("packet sent, waiting for response\n");
    bytes++;
    waitingForResponse = true;
    while (waitingForResponse){					//this loop waits for a response from the client that it had received the packet
      packetRecvd = recvfrom(sockfd, response, 2, 0, (struct sockaddr *)clientaddr, &len);
      if (packetRecvd > -1){					//once received, if the header contains a 'Z', then we need to resend the packet
        if (response[0] == 'Z'){				//due to packet loss
          printf("Client timeout, resending previous packet.\n");
          sendto(sockfd, buffer, bytes_read, 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
          printf("packet re-sent, waiting for response\n");
          bytes++;        
        } else if (response[0] == 'E'){			//if the client responsed with an 'E', the last packet was sent and received
           printf("Received final ack\n");
           return 0;
        } else if (response[0] != expected && response[1] == 'R'){
          printf("Client Received packet.\n");	//we can continue to send more packets
          waitingForResponse = false;
        }
      }
      if (feof(fp)) {							//if we reached the end of the file, then send a packet with an 'E' in the header,
        printf("Server Sent bytes:%d\n", bytes);//signifying that we are all done sending the data
        buffer[0] = 'E';
        char integer_string[32];
        sprintf(integer_string, "%d", bytes);
        strcpy(&buffer[8], integer_string);
        sendto(sockfd, buffer, (bytes_read + 8), 0, (struct sockaddr *)clientaddr, sizeof(*clientaddr));
        printf("final packet sent\n");
      }
    }
    
  }
  return 0;
}
