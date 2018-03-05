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
#include        <sstream>

#define ECHOMAX 1015                     /* Longest string to echo */
#define BACKLOG 128
#define DATA_LENGTH 1015
#define MAX_CLIENTS 30

using namespace std;

typedef struct _CLIENT {
    string username;
    int sockfd;
    int coins;
    int id;
    int port;
    string ip_address;
    struct _CLIENT* next;
} CLIENT;

CLIENT *client_list;

CLIENT* clients[MAX_CLIENTS];
int num_clients = 0;
int master_socket;
bool connected_already = false;


int current_id = 0;

enum headings {nothing, query, regist, deregist};

typedef struct SERVER_MESSAGE {
    int message_len;
    int clock_value;
    char header;
    char data[1015];
} server_message;

typedef struct transaction_ {
    int to_id;
    int from_id;
    int amount;
    struct transaction_ *next;
} transaction;


void handleMessage(server_message *message){
    /*if (strncmp(message, "TEST", 4) == 0){       //if its a test
        char *rest = message + 4;
        cout << rest;
    }*/

    if (message->header == query){          //oh boy we gon have to add all the new clients nao what a RUSH.
        int sockfd;
        struct sockaddr_in servaddr;
        stringstream ss;
        ss << message->data;
        string buffer;
        ss >> buffer;           //should be query
        int i, responses;
        ss >> buffer;           //should be number of clients
        responses = stoi(buffer);
        CLIENT *temp;
        CLIENT *current;
        for (i = 0; i < responses; i++){        //This resets the client list to what you get from query.
            temp = new CLIENT;
            ss >> buffer;
            temp->username = buffer;
            ss >> buffer;
            temp->ip_address = buffer;
            ss >> buffer;
            temp->port = stoi(buffer);
            ss >> buffer;
            temp->coins = stoi(buffer);
            temp->next = NULL;
            if (client_list == NULL){
                client_list = temp;
                current = client_list;
            } else {
                current->next = temp;
                current = temp;
            }
        }
        if (connected_already == false){
            temp = client_list;
            char *ip_address;
            int port;
            while (temp != NULL){
                port = temp->port;
                ip_address = (char *)temp->ip_address.c_str();

                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                bzero(&servaddr, sizeof(servaddr));
                servaddr.sin_family = AF_INET;
                servaddr.sin_port = htons(port);
                inet_pton(AF_INET, ip_address, &servaddr.sin_addr);

                if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))){
                    cout << "lmao what the fuck" << endl;
                } else {
                    cout << "we didnt' connect as expected" << endl;
                }
            } 
            connected_already = true; 
        }


    }
    cout << message->data;
}

void handleMinerMessage(server_message *message, CLIENT *client){

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

//setting max listening socket
    FD_SET(master_socket, &fds);
    if (master_socket > max_sd){
        max_sd = master_socket;
    }



//setting client sockets
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
        server_message *inc_message = new server_message;;
        inc_message->data[DATA_LENGTH-1] = 0;
        int rc;
        if ((rc = read(server, inc_message, 1024)) == 0)            //JK the server left us all alone :(
        {
            cout << "Server disconnected from us :(\n";
            close(server);
            exit(0);
        } else                      //Handle the message from the server!              
        {   
            handleMessage(inc_message);
        }
    }

    if (FD_ISSET(STDIN_FILENO, &fds))                   //Some interaction from the bro at the keyboard
    {
        server_message *message = new server_message;
        char out_message[ECHOMAX];
        out_message[ECHOMAX-1] = 0;

        fgets(out_message, ECHOMAX, stdin);
        strcpy(message->data,out_message);
        if (strncmp(out_message, "query", strlen("query")) == 0){
            message->header = query;
        }
        if (strncmp(out_message, "register", strlen("register")) == 0){
            message->header = regist;
        }
        if (strncmp(out_message, "deregister", strlen("deregister")) == 0){
            //cout << "lmao whats the problem\n";
            message->header = deregist;
        }
        if (send(server, (void *)message, 1024, 0) == 0)
        {
            cout << "idk what this error even is\n";
            close(server);
            exit(1);
        }

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
    }


    for (i = 0; i < MAX_CLIENTS; i++)           //If there is some interaction from a client.
    {   
        if (clients[i] != NULL and FD_ISSET(clients[i]->sockfd, &fds))
        {
            server_message *message = new server_message;
            int rc;
            if ((rc = read(clients[i]->sockfd, message, 1024)) == 0)        //Client has a fat cock and disconnected.
            {
                //cout << "Client disconnected: " << clients[i]->id << endl;
                close(clients[i]->sockfd);
                clients[i] = NULL;                      //Think this is a memory leak. idgaf lmao
                num_clients--;
            } else                      //Client has a fat message for us.
            {
                handleMinerMessage(message, clients[i]);
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

	if (argc != 4)
		DieWithError( "usage: tcp-client <Server-IPaddress> <Server-Port>" );
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);


    int sock, connfd;                /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    char echoBuffer[ECHOMAX];        /* Buffer for echo string */
    unsigned short echoServPort;     /* Server port */
    int recvMsgSize;                 /* Size of received message */
	connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    server = sockfd;

    echoServPort = atoi(argv[3]);  /* First arg:  local port */

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
    while (true){
        waitForServer();
    }

	//str_cli(stdin, sockfd);		/* do it all */

	exit(0);
}
