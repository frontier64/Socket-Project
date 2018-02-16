/**
 * Title: tcp-server.cpp
 * Author: Kixo
 * About: Example socket. relays messages between clients.
 * For: CSE 434. Socket project.
 * Date: 2/5/2018
 **/

/*
TODO: implement message relaying between multiple clients.
*/

#include	<sys/socket.h> 	/* for socket() and bind() */
#include	<stdio.h>		/* printf() and fprintf() */
#include	<stdlib.h>		/* for atoi() and exit() */
#include	<arpa/inet.h>	/* for sockaddr_in and inet_ntoa() */
#include	<sys/types.h>
#include	<string.h>
#include	<unistd.h>
#include    <iostream>

using namespace std;

#define MAX_CLIENTS 30
#define	ECHOMAX	1025		/* Longest string to echo */
#define BACKLOG	128

typedef struct SERVER_MESSAGE {
    int message_len;
    char header;
    char[1018] data;

} server_message;



typedef struct {
    int id;
    int sockfd;
} CLIENT;

typedef struct {
    CLIENT *client;
    CLIENT *next;
} CLIENT_LIST;

CLIENT* clients[MAX_CLIENTS];
int num_clients = 0;
int master_socket;
int current_id = 0;

void handleMessage(char message[1025], CLIENT *client){
    int i;

    for (i = 0; i < MAX_CLIENTS; i++){
        if (clients[i] != NULL and clients[i]->sockfd != 0 and clients[i]->sockfd != client->sockfd)
        {
            if (send(clients[i]->sockfd, message, strlen(message), 0) == 0)
            {
                cout << "Error sending message to a fat client\n";
                exit(1);
            }
        }
    }
}


void waitForClient(){
    int max_sd;
    int i;
    fd_set fds;
    FD_ZERO(&fds);

    //Setting Master Socket
    FD_SET(master_socket, &fds);
    max_sd = master_socket;

    //Setting client sockets.
    for (i = 0; i < num_clients; i++)
    {
        if (clients[i] == NULL)
        {
            continue;
        }
        FD_SET(clients[i]->sockfd, &fds);
        if (clients[i]->sockfd > max_sd)
        {
            max_sd = clients[i]->sockfd;
        }
    }

    int rc;
    rc = select(max_sd + 1, &fds, NULL, NULL, NULL);            //Wait for one socket to actually have something to do
    if (rc < 0){
        cout << "Error at client interaction\n";
        exit(1);
    }

    if (FD_ISSET(master_socket, &fds)){         //If there is a new connection coming on the master socket.
        struct sockaddr_in clientAddr;
        unsigned int clientAddrLen;
        int new_socket;
        if ((new_socket = accept(master_socket, (struct sockaddr *) &clientAddr, &clientAddrLen)) < 0)
        {
            cout << "Error connecting to new client\n";
            exit(1);
        }
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] == NULL)
            {
                CLIENT *new_client = new CLIENT;
                new_client->id = current_id++;
                new_client->sockfd = new_socket;
                clients[i] = new_client;
                break;
            }
        }
        num_clients++;
        cout << "New client id: " << clients[i]->id << endl;
    }
    for (i = 0; i < MAX_CLIENTS; i++)           //If there is some interaction from a client.
    {   
        if (clients[i] != NULL and FD_ISSET(clients[i]->sockfd, &fds))
        {
            char inc_message[1025];
            inc_message[1024] = 0;
            int rc;
            if ((rc = read(clients[i]->sockfd, inc_message, 1024)) == 0)        //Client has a fat cock and disconnected.
            {
                cout << "Client disconnected: " << clients[i]->id << endl;
                close(clients[i]->sockfd);
                clients[i] = NULL;                      //Think this is a memory leak. idgaf lmao
                num_clients--;
            } else                      //Client has a fat message for us.
            {
                inc_message[rc] = 0;
                handleMessage(inc_message, clients[i]);
            }
        }
    } 

}

void 
DieWithError(const char *errorMessage) /* External error handling function */
{
	perror(errorMessage);
	exit(1);
}

void
EchoString(int sockfd)
{
    ssize_t n;
    char    line[ECHOMAX];
    int i;
    for ( ; ; ) {
	    if ( (n = read(sockfd, line, ECHOMAX)) == 0 )
   	    	return; /* connection closed by other end */

        write(sockfd, line, n );
        fputs(line, stdout);
    }
}

int
main(int argc, char **argv)
{
    int sock, connfd;                /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    char echoBuffer[ECHOMAX];        /* Buffer for echo string */
    unsigned short echoServPort;     /* Server port */
    int recvMsgSize;                 /* Size of received message */

    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage: %s <TDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        DieWithError("server: socket() failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("server: bind() failed");
  
	if (listen(sock, BACKLOG) < 0 )
		DieWithError("server: listen() failed");

	cliAddrLen = sizeof(echoClntAddr);
    
    master_socket = sock;

    cout << "Waiting for connection\n";
    int i;
    for (i = 0; i < MAX_CLIENTS; i++){
        clients[i] = NULL;
    }
    while (true)
    {
        waitForClient();
    }
	//connfd = accept( sock, (struct sockaddr *) &echoClntAddr, &cliAddrLen );

    cout << "Got connection\n";
	close(connfd);
}
