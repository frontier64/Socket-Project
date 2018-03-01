// This simulates vector clocks between a variable number of clients
// up to 10.  Be sure to enter just one character or integer for
// input as buffer overflow is really easy in this program.

#include<stdio.h>
#include<stdlib.h>

typedef struct{
	int client_count;
	int client[10][10];
} _table_;

_table_ registry;

// ===============================================================
// Details: These are simple functions used as an interative
//					shell for illustrating vector clock and are not
//					important.  In general, if a keyword or function
//					is ALL-CAPITOLIZED; it is not important to the
//					vector-clock algorithm.
// ===============================================================
int MAX(int a, int b){
	return((a) > (b) ? a : b);
};

void PRINT_MENU(){
	printf("\n +---------------------+\n | c = add new client  |\n | r = remove a client |\n | s = send message    |\n | o = do operation    |\n | p = print registry  |\n | q = quit            |\n +---------------------+\n\n What would you like to do? ");
};

void PRINT_CLIENT(int id){
	int index;
	printf(" Client %d: <", id);
	for(index = 0; index < 9; index++){
		printf("%d, ", registry.client[id][index]);
	}
	printf("%d>\n", registry.client[id][9]);
};

void PRINT_REGISTRY(){
	if(registry.client_count > 0){
		int index;
		for(index = 0; index < registry.client_count; index++){
			PRINT_CLIENT(index);
		}
	}
	else{
		printf("Regsitry is empty....\n");
	}
};
// ---------------------------------------------------------------


// ===============================================================
// Details: These functions are the most important for
//					illustrating vector clocks, notably operation() and
//					send_message().
// ===============================================================

// This operation adds a client to the existing registry.  It is
// over simplified as it is just a first-in-last-out rather than
// what we might implement where the user can request an id
void add_client(){
	if(registry.client_count < 10){
		int index;
		for(index = 0; index < 10; index++){
			registry.client[registry.client_count][index] = 0;
		}
		PRINT_CLIENT(registry.client_count);
		registry.client_count++;
	}
	else{
		printf(" Registry is full!\n");
	}
};

// Removes the most recent client. Like the add_client(); overly
// simple and would need to be able to remove a specific user in
// our implementation.
void remove_client(){
	if(registry.client_count > 0){
		int index;
		registry.client_count--;
		for(index = 0; index < 10; index++){
			registry.client[registry.client_count][index] = 0;
		}
		printf(" Client %d removed.\n", registry.client_count);
	}
	else{
		printf(" There are no clients to remove!\n");
	}
};

// This represents a client doing some internal operation.
// It is simple and straight forward as only the specified
// client's vector is updated at its index.
void operation(int id){
	if(id >= 0 && id < registry.client_count){
		registry.client[id][id]++;
		PRINT_CLIENT(id);
	}
	else{
		printf(" Input client ID was not valid.\n");
	}
};

// This is a message operation where the sender does an operation
// to send and the receiver does an operation when receiving.  The
// vector of the receiver is updated to themaximum value of each
// client between sender and receiver.  The receiver's clock is then
// updated to represent the operation of receiving the messsage.
void send_message(int sender, int receiver){
	if(sender != receiver && sender >= 0 && sender < registry.client_count && receiver >= 0 && receiver < registry.client_count){
		operation(sender);
		int index;
		for(index = 0; index < registry.client_count; index++){
			registry.client[receiver][index] = MAX(registry.client[sender][index], registry.client[receiver][index]);
		}
		operation(receiver);
	}
	else{
		printf(" Sender or Receiver were not valid.\n");
	}
};
// ---------------------------------------------------------------


void CHOOSE_OPTION(char command){
	switch(command){
		case 'c':{	// Add new client
			add_client();
			break;}
		case 'r':{	// Remove most recent client
			remove_client();
			break;}
		case 'q':{	// Quit from program
			printf(" Goodbye, have a nice day :)\n");
			break;}
		case 'p':{	// Print all client vector-clocks
			PRINT_REGISTRY();
			break;}
		case 'o':{	// Do an operation on a client
			int client_id;
			printf(" Which client is doing an operation? ");
			scanf("%d", &client_id);
			operation(client_id);
			break;}
		case 's':{	// Send a message from one client to another client
			int sender, receiver;
			printf(" From: ");
			scanf("%d", &sender);
			printf(" To: ");
			scanf("%d", &receiver);
			send_message(sender, receiver);
			break;}
		default:{
			break;}
	}
};


int main(){
	registry.client_count = 0;
	char choice[2];
	
	do{
		PRINT_MENU();
		scanf("%s", choice);
		CHOOSE_OPTION(choice[0]);
	} while(choice[0] != 'q');
	
	return(0);
}
