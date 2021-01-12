#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include "structs.h"

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
int network_layersockfd;
int wiresockfd;

void * rcvfromwiresend2network_layer ( char *argv[] ) //--
{
    int charRead_written;                
    frame frame_from_phyLayer;  
	 while (1)
	 {
             charRead_written = read(wiresockfd, &frame_from_phyLayer, sizeof(frame));
             if(charRead_written < 0)
                 error("Error reading from physical layer socket \n");
             printf("\n now will remove the frame header and transmit the included packet to network layer\n");
             charRead_written = write(network_layersockfd, &frame_from_phyLayer.my_packet, sizeof(packet));
             if(charRead_written < 0)
                 error("Error sending packet to network layer\n");
             else
                 printf("The packet is in route to network layer\n");
             printf("____________________________________________________________________________");
             printf("Recieved the frame from the physical layer\n");
             printf("Frame identifier\n");
             printf("\t Sequence Number: %d \n", frame_from_phyLayer.seq_num);
             printf("\t frame type: %d\n",frame_from_phyLayer.type);   
	 }
    return NULL;
}
int main(int argc, char *argv[])
{
    printf("\nEntering data link layer main\n");
    int mainSockfd;
    int frameSeq = 0; 
    packet packet_from_NetwLayer; 
    socklen_t clilen;
    struct sockaddr_in cli_addr;

    frame frame_to_phyLayer; 
    int physical_layer_portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
   
   //----------------------------------------------------------------------------------
   
     if (argc < 4) {
		 fprintf(stderr,"Usage: %s  wire__IP  wire_port data_port\n",argv[0] );
         exit(1);
     }

	//-------------------------------------------------------------------------------
	
    server = gethostbyname(argv[1]);
    if(server == NULL)
    {
        fprintf(stderr, "No such host can be found\n");
        exit(0);
    
    }
    physical_layer_portno = atoi(argv[2]);
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
 
    bcopy((char *) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(physical_layer_portno);
   
    wiresockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(wiresockfd < 0)
        error("Error opening socket\n");
   
    if(connect(wiresockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("Error connecting data link socket to the specified server (pysical-wire)\n");
    printf("connection to the wire successful\n");
        
	//------------------------------------------------------------------------------------	
		
    pthread_t wirepth;
    pthread_create(&wirepth,NULL,rcvfromwiresend2network_layer, NULL);
        
    bzero((char *)&serv_addr, sizeof(serv_addr));
    int data_port = atoi(argv[3]);
    serv_addr.sin_port = htons(data_port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_family = AF_INET;
    
    mainSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(mainSockfd < 0)
        error("Datalink layer error opening the socket for listen\n");

     if(bind(mainSockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
             error("error on binding");
    listen(mainSockfd, 5);
    clilen = sizeof(cli_addr);
    
    network_layersockfd = accept(mainSockfd, (struct sockaddr *) &cli_addr, &clilen); 
    if(network_layersockfd < 0)
        error("Error on accept");
    printf("create new socket: %d \n", network_layersockfd);
    close(mainSockfd);

	//-------------------------------------------------------------------------------------

	 while (1)
	 {
	
             int n = read(network_layersockfd, &packet_from_NetwLayer, sizeof(packet));
             if(n < 0)
                 error("ERROR receiving packet from network layer");
             printf("_______________________________________________________________");
             printf("Recieved a packet from network layer \n");
 
             bzero((char *) &frame_to_phyLayer, sizeof(frame));
             frame_to_phyLayer.seq_num = frameSeq;
             frameSeq++;
             frame_to_phyLayer.type = 0;
             strcpy(frame_to_phyLayer.my_packet.nickname, packet_from_NetwLayer.nickname);
             strcpy(frame_to_phyLayer.my_packet.message, packet_from_NetwLayer.message);
		
             printf("Now new wrapped framed will be sent to the wire\n");

             n = write(wiresockfd, &frame_to_phyLayer,sizeof(frame));
             if(n < 0)
                 error("Error sending a frame to the physical wire\n");
             else
                 printf("frame is written to own layer. Now physical layer will handle the read and send to other host\n");
   
             if (strcmp(packet_from_NetwLayer.message, "EXIT\n") == 0)
            {
                pthread_cancel(wirepth);
                close(wiresockfd);
                close(network_layersockfd);
                printf("\nExiting data link main function\n");
                return 0;
            }

    }

}
