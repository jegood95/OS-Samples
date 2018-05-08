/* jegood Joshua Good */

/**
 * @file p3_c.c
 * Client program for weather information service. Prompts the server to process one
 * of the following two requests, either: a)display the current temperatures for RDU,
 * CLT, ALT, CHS, and RIC, or b)update the current temperature for one of the aforementioned
 * cities. Allows the user to freely disconnect and connect to the weather information server
 * via the command line.
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/** Maximum length for a command */
#define MAX_CMD_LEN 128

/** 
 * Displays the prompt for the client.
 * @param cmd the command prompt to display
 */
void displayPrompt(char *cmd) {
	printf(" > ");
	scanf("%s", cmd);
	int len = (int) strlen(cmd);
	// Null terminate at length
	cmd[len] = '\0';
}

/**
 * Main method for client program. Initiates connection with server and decides
 * to either a)request weather information, or b)update weather information for
 * a particular city out of the previously-defined five. Handles illegal commands
 * to avoid server-side conflict.
 * @param argc number of command line arguments
 * @param argv arry of command line arguments
 */
int main(int argc, char * argv[]) {
	// Ensure correct program usage
	if (argc != 3) {
		printf("Usage: ./client <ip> <port>\n");
		exit(1);
	}
	/** The socket id */
	int sockID;
	/** Struct representing local address port */
	struct sockaddr_in addrport;
	// Create socket
	if((sockID = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		printf("socket() failed\n");
		exit(1);
	}
	
	// Prepare the net address
	addrport.sin_family = AF_INET;
	addrport.sin_port = htons((unsigned short) atoi(argv[2]));
	addrport.sin_addr.s_addr = inet_addr(argv[1]);
	// Connect to the server
	int status = connect(sockID, (struct sockaddr *) &addrport, sizeof(addrport));
	
	/** Buffer for a command */
	char rcvBuf[MAX_CMD_LEN];
	/** Length of command */
	int cmdLen;
	/** Option flags */
	int flags = 0;
	/** Count to determine send() and receive() errors */
	int count;
	
	//Start of while loop
	while(1){
		// Displays prompt and reads user input
		displayPrompt(rcvBuf);
    // Test to exit
		if(strcmp(rcvBuf, "e") == 0 || strcmp(rcvBuf, "E") == 0){
			// Break and exit the program
			break;
		}else if(rcvBuf[0] == 'r' || rcvBuf[0] == 'R' || strcmp(rcvBuf, "s") == 0
						 || strcmp(rcvBuf, "S") == 0){ //starts with "r" or "R"
			// Request "report" and send data to report -> errors are handled server side,
			// BUT printed by the client!
			cmdLen = (int) strlen(rcvBuf) + 1;
			// Send the request and handle failure
			if((count = send(sockID, rcvBuf, cmdLen, flags)) < 0){
				printf("send() error\n");
			}
			// Clear out space for incoming message
			memset(rcvBuf, 0, MAX_CMD_LEN);
			
			if((count = recv(sockID, (void *) rcvBuf, sizeof(rcvBuf), flags)) < 0){
				printf("recv() failed\n");
			}
			// Print the returned report status (either sucess or failure)
			printf("%s\n", rcvBuf);
			
		} else {
			printf("Invalid action\n");
		}
		// Clear out memory for command
		memset(rcvBuf, 0, MAX_CMD_LEN);
	}
	
	// Close the client socket
	if((status = close(sockID)) < 0){
		printf("Failed to close socket\n");
	}

	// Return exit success
	return EXIT_SUCCESS;
}
