#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "structs.h"

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int clientlist[2];
pthread_t threadlist[2];

void * onesocket ( int threadsockfd) 
{
    
    int charRead_written;
    frame frame_2_dataLayer;

    while (1) {

        charRead_written = read(threadsockfd, &frame_2_dataLayer, sizeof (frame));
        if (charRead_written < 0)
            error("Error reading from socket");
        
        printf("________________________________________________________\n");
        printf("Physical layer recieved a packet from: %s \n", frame_2_dataLayer.my_packet.nickname);

        if (strcmp(frame_2_dataLayer.my_packet.message, "EXIT\n") == 0) {
            close(threadsockfd);
            return NULL;
        }
        else 
        {
            
            printf("Sending the frame to the other host\n");
            printf("theadsockfd :%d, clientlist[0]: %d, clientlist[1]: %d",
                    threadsockfd, clientlist[0], clientlist[1]);
            
            if (threadsockfd == clientlist[0]) {
                charRead_written = write(clientlist[1], &frame_2_dataLayer, sizeof (frame));
                if (charRead_written < 0) 
                    error("ERROR witting to socket from clientlist[0]");
            }
            else if(threadsockfd == clientlist[1])
            {

                charRead_written = write(clientlist[0], &frame_2_dataLayer, sizeof (frame));
                if (charRead_written < 0)
                    error("ERROR witting to socket of clientlist[0]");
                printf(" writing to socket file descriptor %d", clientlist[0]);
            }
        
        }

    }
}

int main(int argc, char *argv[])
{
    printf("physical_layer main function\n");
    
    int sockfd, connectedSockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
  
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

         bzero((char *) &serv_addr, sizeof(serv_addr));
         portno = atoi(argv[1]);
         serv_addr.sin_family = AF_INET;
         serv_addr.sin_addr.s_addr = INADDR_ANY;
         serv_addr.sin_port = htons(portno);
	 sockfd = socket(AF_INET, SOCK_STREAM, 0);
         if(sockfd < 0)
             error("ERROR opening socket");
         if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
             error("ERROR on binding");

    for (int i = 0; i < 2; i = i + 1)
    {
        listen(sockfd, 10);
        clilen = sizeof (cli_addr);
        
        connectedSockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (connectedSockfd < 0)
            error("ERROR on accept");
        else
            printf("connection to wire has been accepted\n");

        clientlist[i] = connectedSockfd;
       
        pthread_t pth; 
        pthread_create(&pth, NULL, onesocket, clientlist[i]);
        threadlist[i] = pth; 
    }
    close(sockfd); 
    pthread_join(threadlist[0], NULL); 
    pthread_join(threadlist[1], NULL); 

    return 0;

}
