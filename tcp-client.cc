/**
 * Title: tcp-client.cpp
 * Author: Kixo
 * About: Interact with socket server. Send messages and receive messages from other clients.
 * For: CSE 434. Socket Project
 * Date: 2/5/2018
 **/

#include        <sys/socket.h>          /* for socket() and bind() */
#include        <stdio.h>               /* printf() and fprintf() */
#include        <stdlib.h>              /* for atoi() and exit() */
#include        <arpa/inet.h>           /* for sockaddr_in and inet_ntoa() */
#include        <sys/types.h>
#include        <string.h>
#include        <unistd.h>
#include        <iostream>

#define ECHOMAX 1025                     /* Longest string to echo */
#define BACKLOG 128

using namespace std;

typedef struct SERVER_MESSAGE {
    int message_len;
    char header;
    char[1018] data;

} server_message;

typedef struct MINER_MESSAGE {
    
}

void handleMessage(char* message){
    if (strncmp(message, "TEST", 4) == 0){       //if its a test
        char *rest = message + 4;
        cout << rest;
    } else {
        cout << message;
    }
}

int server;
void waitForServer(){
    int max_sd;
    int i;
    fd_set fds;
    FD_ZERO(&fds);

    //Setting server socket.
    FD_SET(server, &fds);
    max_sd = server;

    //Setting stdin socket
    FD_SET(STDIN_FILENO, &fds);
    if (STDIN_FILENO > max_sd) max_sd = STDIN_FILENO;



    int rc;
    rc = select(max_sd + 1, &fds, NULL, NULL, NULL);            //Wait for one socket to actually have something to do
    if (rc < 0){
        cout << "Error at client interaction\n";
        exit(1);
    }

    if (FD_ISSET(server, &fds))         //The server is sending us a message oh boy!
    {
        char inc_message[ECHOMAX];
        inc_message[ECHOMAX-1] = 0;
        int rc;
        if ((rc = read(server, inc_message, 1024)) == 0)            //JK the server left us all alone :(
        {
            cout << "Server disconnected from us :(\n";
            close(server);
            exit(0);
        } else                      //Handle the message from the server!              
        {   
            inc_message[rc] = 0;
            cout << inc_message;
        }
    }

    if (FD_ISSET(STDIN_FILENO, &fds))
    {
        char out_message[ECHOMAX];
        out_message[ECHOMAX-1] = 0;
        fgets(out_message, ECHOMAX, stdin);

        if (send(server, out_message, strlen(out_message), 0) == 0)
        {
            cout << "idk what this error even is\n";
            close(server);
            exit(1);
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
str_cli(FILE *fp, int sockfd)
{
	ssize_t n;
    char    sendline[ECHOMAX], recvline[ECHOMAX];

    while (fgets(sendline, ECHOMAX, fp) != NULL) {

        write(sockfd, sendline, strlen(sendline));

        if ( (n = read(sockfd, recvline, ECHOMAX)) == 0)
            DieWithError("str_cli: server terminated prematurely");

        recvline[n] = '\0';
        fputs(recvline, stdout);
    }
}

int
main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in servaddr;

	if (argc != 3)
		DieWithError( "usage: tcp-client <Server-IPaddress> <Server-Port>" );
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    server = sockfd;

    while (true){
        waitForServer();
    }

	//str_cli(stdin, sockfd);		/* do it all */

	exit(0);
}
