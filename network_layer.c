#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include "structs.h"

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void * rcvmsg (int threadsockfd)
{

    packet packet_from_dataLink;         
    
    int charRead_written;                 
    
	while (1)
	{

            charRead_written = read(threadsockfd, &packet_from_dataLink, sizeof(packet));
            if(charRead_written < 0)
                error("network Layer: error reading the socket\n");
            /printf("\n____________________________________________________\n");
            printf("\t Here is the message from host %s : %s \n",
                    packet_from_dataLink.nickname,packet_from_dataLink.message);

	}
	return NULL;
}


int main(int argc, char *argv[])
{
    printf("\nEntering network layer main function.\n");
   
    int netw_sockfd;
    int portno;
    
    struct sockaddr_in serv_addr;
    struct hostent * server;
    
    char buffer[256];
    packet packet_to_dataLink;
 
    if (argc < 4) {
       fprintf(stderr,"usage %s data_addr data_port nickname\n", argv[0]);
       exit(0);
    }
   
   
   //----------------------------------------------
   
    server = gethostbyname(argv[1]);
    if(server == NULL)
    {
        fprintf(stderr, "Error, no such host \n");
        exit(0);
    }
	
	
	
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr,
            server->h_length);
			
    portno = atoi(argv[2]);
    serv_addr.sin_port = htons(portno);
    
    printf("Creating endpoint of network layer to connect to server data-link \n");
    
    netw_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(netw_sockfd <0)
        error("ERROR opening the network layer socket\n");
    
	//------
    
    printf("attempting to connect to the socket that is bound to the address server specified\n");
    int connectStatus;
    connectStatus = connect(netw_sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if(connectStatus < 0)
        error("Error connecting to data link layer \n");
    else
        printf(" bi-directional Connection established\n");
    
	
	//------------------------------------------------
	
	
	pthread_t pth;
	pthread_create(&pth,NULL,rcvmsg,netw_sockfd);
        
        printf("Please Enter a Message: ");

	while(1)
	{
            
            bzero(buffer, 256);
            fgets(buffer, 255, stdin);
            bzero((char*) &packet_to_dataLink, sizeof(packet));
            strcpy(packet_to_dataLink.message, buffer);
            strcpy(packet_to_dataLink.nickname, argv[3]);
            
            int n = write(netw_sockfd, &packet_to_dataLink, sizeof(packet));
            if(n< 0)
                error("Error writing to socket newt_sockfd");

		if (strcmp (buffer, "EXIT\n")==0)
		{
			pthread_cancel(pth);
			close(netw_sockfd);
             printf("\nExiting main function of Network Layer\n");
			return 0;
		}
	}

}
