/* jegood Joshua Good */

/**
 * @file p3_s.c
 * Multi-client server program for weather information service. Receives a message from a client to parse
 * based upon the following commands: a){r,R} = report a city's temperature for the given hour or b){s,S} show temperatures
 * for all cities in the current hour. If the reported temperature is not the initial reported temperature for the hour, 
 * averages all reported temperatures for the selected city's hour and assigns that as the new temperature. 
 * Additionally, if a new hour passes while the server is running, the server resets the selected city's temperatures and 
 * the first temperature reported is assigned.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

/** Maximum pending connections */
#define MAXPENDING 5
/** Maximum data length for a message */
#define MAX_DATA_LEN 128
/** Number of cities handled by the server */
#define NUM_CITIES 5
/** Number of seconds in an hour */
#define SEC_IN_HOUR 3600

/** Temperature data struct */
struct Temp_Data {
	char city[4]; // city postal code
	unsigned short temperature; // current temperature
	unsigned short hourstamp; // current hour
	unsigned long count; // number of temperatures reported for the hour
	unsigned long sumTemps; // Sum of all temps for the hour
};

/** Arguments for thread creation */
struct ThreadArgs{
	int clntSock; // client socket
};

/** Array of temperature data */
struct Temp_Data temp_array[5];
/** Lock for synchronization */
pthread_mutex_t lock;

/**
 * Initializes the temperature array with each city's postal code.
 * All other values remain their default.
 * @param array_p the temperature array to be initialized
 */
void init_array(struct Temp_Data * array_p) {
	strcpy(array_p[0].city, "RDU");
	strcpy(array_p[1].city, "CLT");
	strcpy(array_p[2].city, "ALT");
	strcpy(array_p[3].city, "CHS");
	strcpy(array_p[4].city, "RIC");
}

/**
 * Appends the temperature for each city to the specified message for client
 * transfer. Performs the operation for client request 's' or 'S'.
 * @param data the message to be modified 
 */
void getTemps(char *data)
{
	char temp[4];
	for(int i = 0; i < NUM_CITIES; i++){
		strcat(data, temp_array[i].city);
		strcat(data, " ");
		sprintf(temp, "%u", temp_array[i].temperature);
		strcat(data, temp);
		strcat(data, "\t");
		// Reset temp for next temperature
		memset(temp, 0, 4);
	}
}

/**
 * Returns whether or not the given city is valid. A valid city contains any
 * of the five valid postal codes within the temperature array. Postal codes
 * must be capitalized.
 * @param data the temperature struct to evaluate
 * @return if the city is valid
 */
int isValidCity(struct Temp_Data data)
{
	// Check if city is valid
	if(strcmp(data.city, "RDU") != 0 && strcmp(data.city, "CLT") != 0 && strcmp(data.city, "ALT") != 0 
		 && strcmp(data.city, "CHS") != 0 && strcmp(data.city, "RIC") != 0){
		return 0;
	}
	return 1;
}
 
/**
 * Returns if the given timestamp is valid. A valid timestamp is not negative, 
 * does not exceed 24, and equals the current hour.
 * @param data the temperature struct to evaluate
 * @return if the given timestamp is valid 
 */
int isValidTimestamp(struct Temp_Data data)
{
	// Check if hour stamp is not valid
	if(data.hourstamp < 0 || data.hourstamp > 23){
		return 0;
	}
	// Get the current hour and convert from GMT to EST (Note: GMT is 5 hours ahead)
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int curr_sec = (int) tv.tv_sec; // in seconds
	// Convert to current hour of the day
	unsigned short curr_hour = (unsigned short) (curr_sec / SEC_IN_HOUR % 24 - 5); // in hours
	// Check if current hour and requested hour are unequal
	if(curr_hour != data.hourstamp){
		return 0;
	}
	// Hour is current, so return valid
	return 1;
}

/**
 * Records the temperature for the given city. If this is the initial reported temperature
 * for the given city, the current temperature is overwritten. Else, averages all reported
 * temperatures for this hour and assigns as the current temperature. If a new hour passes,
 * resets all cities' temperatures to their defaults and updates the current hourstamp.
 * @param data the temperature struct providing a temperature to report
 */
void recordTemp(struct Temp_Data data)
{
	printf("\nRecording temp\n");
	// Locate the city to record
	for(int i = 0; i < NUM_CITIES; i++){
		printf("I'm in the loop\n");
		pthread_mutex_lock(&lock); // lock and parse
		if(strcmp(data.city, temp_array[i].city) == 0){
			// Check if a new hour has passed
			if(temp_array[i].hourstamp != data.hourstamp){
				// At a new hour, so overwrite with current hour and reset sumTemps
				temp_array[i].hourstamp = data.hourstamp;
				temp_array[i].sumTemps = 0;
				temp_array[i].count = 0;
			}
			// Check if this is the first overwrite for the hour
			if(temp_array[i].count == 0){ // No overwrites yet
				// Overwrite temp and update count
				temp_array[i].temperature = data.temperature;
				temp_array[i].hourstamp = data.hourstamp;
				temp_array[i].sumTemps += (unsigned long) data.temperature;
				(temp_array[i].count)++;
				pthread_mutex_unlock(&lock); // unlock before breaking
				break;
			} else {
				printf("Here we are, after count 1\n");
				// Sum recorded temps and average, then floor
				(temp_array[i].count)++;
				temp_array[i].sumTemps += (unsigned long) data.temperature;
				temp_array[i].temperature = (unsigned short) floor(((double) temp_array[i].sumTemps / temp_array[i].count));
				temp_array[i].hourstamp = data.hourstamp;
				pthread_mutex_unlock(&lock); // unlock before breaking
				break;
			}
		}
		pthread_mutex_unlock(&lock);
	}
}

/**
 * Formats the data in the client message as a temperature struct.
 * A message is formatted request, city, hour, and then temperature.
 * @param msg the message from which to create a temperature struct
 * @return the temperature struct for this message
 */
struct Temp_Data getData(char *msg)
{
	struct Temp_Data data;
	char *token;
	strtok(msg, ":"); // request
	// Check for invalid city
	if((token = strtok(NULL, ":")) == NULL){
		strcpy(data.city, "inv");
	} else {
		strcpy(data.city, token); // city
	}
	// Check for invalid hourstamp
	if((token = strtok(NULL, ":")) == NULL){
		data.hourstamp = 25;
	} else {
		data.hourstamp = (unsigned short) atoi(token);
	}
	if((token = strtok(NULL, ":")) == NULL){
		data.temperature = 0;
	} else {
		data.temperature = (unsigned short) atoi(token);
	}
	
	// Return a temperature struct
	return data;
} 
 
/**
 * Parses the client's request. If the the request is {s,S}, displays the current
 * temperature for each city. Else, interprets the client's message as a report and
 * generates a temperature struct for that message. If the client reports an invalid
 * city, sets the return message to "Error city code!". If the client reports an invalid
 * hourstamp, sets the return message to "Error hourstamp!". Else, sets the return message
 * to "Successfully report temperature!" to reply to the client.
 * @param msg the client's request to parse
 */
void parseMessage(char *msg)
{
	// Determine if message is to show or receive temperatures
	if(msg[0] == 's' || msg[0] == 'S'){
		// Clear out first character
		msg[0] = '\0';
		// Generate the string of temps
		getTemps(msg);
	} else { // Report a temperature
		// Make a copy of this string
		int len = (int) sizeof(msg);
		char cpy[len];
		strcpy(cpy, msg);
		// Get the reported temperature data
		struct Temp_Data data = getData(cpy);
		// Check if the city is invalid
		if(!isValidCity(data)){
			pthread_mutex_lock(&lock);
			strcpy(msg, "Error city code!");
			pthread_mutex_unlock(&lock); // unlock before breaking
			return;
		}
		// Reset message and check if the timestamp is valid
		if(!isValidTimestamp(data)){
			pthread_mutex_lock(&lock);
			strcpy(msg, "Error hourstamp!");
			pthread_mutex_unlock(&lock);
			return;
		}
		// Record the temperature for the given city
		recordTemp(data);
		pthread_mutex_lock(&lock);
		strcpy(msg, "Successfully report temperature!");
		pthread_mutex_unlock(&lock);
	}
}

/**
 * Handles a client's request. Initially receives a message from a client
 * and parses it. Once parsed, acquires the resulting updated message and replies
 * back to the client. Then, waits to receive further messages from that client.
 * If the client terminates, closes the client socket.
 * @param clntSocket the socket of the client in which to receive and send messages
 */
void HandleTCPClient(int clntSocket)
{
	char echoBuffer[MAX_DATA_LEN];
	int rcvMsgSize;
	
	// Receive message from client
	if((rcvMsgSize = recv(clntSocket, echoBuffer, MAX_DATA_LEN, 0)) < 0){
		pthread_mutex_lock(&lock);
		printf("recv() failed\n");
		pthread_mutex_unlock(&lock); // unlock before breaking
	}
	
	while(rcvMsgSize > 0){
		parseMessage(echoBuffer);
		// Calculate the new message length to send
		rcvMsgSize = (int) sizeof(echoBuffer);
		// Send the result back to client
		if(send(clntSocket, echoBuffer, rcvMsgSize, 0) != rcvMsgSize){
			pthread_mutex_lock(&lock);
			printf("send() failed");
			pthread_mutex_unlock(&lock); // unlock before breaking
		}
		
		// Receive the next message
		if((rcvMsgSize = recv(clntSocket, (void *) echoBuffer, MAX_DATA_LEN, 0)) < 0){
			pthread_mutex_lock(&lock);
			printf("recv() failed\n");
			pthread_mutex_unlock(&lock); // unlock before breaking
		}
	}
	
	// Close the client socket
  close(clntSocket);
}

/**
 * Creation function for process threads. Detaches this thread to deallocate
 * resources upon return and extracts the socket file descriptor from the thread
 * arguments. Then deallocates memory. Finally, processes the client's request.
 * @param args thread arguments used to parse client's request
 */
void *ThreadMain(void *args)
{
	/** Socket descriptor for client connection */
	int clntSock;
	// Gaurantee that thread resources are deallocatecd upon return
	pthread_detach(pthread_self());
	
	// Extract socket file descriptor from argument
	clntSock = ((struct ThreadArgs *) args)->clntSock;
	// Deallocate memory for argument
	free(args);
	
	// Handle the client
	HandleTCPClient(clntSock);
	
	return (NULL);
}

/**
 * Creates the server socket for the stream. Binds the server to the local
 * address listens for some socket to connect to. Returns the socket ID for
 * the server.
 * @param port the local port from which to connect
 * @return the socket ID for the server
 */
int CreateTCPServerSocket(unsigned short port)
{
	/** Socket to create */
  int sock;
	/** Local address */
  struct sockaddr_in echoServAddr;

  // Create socket for incoming connections
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		printf("socket() failed\n");
	}
      
  // Construct local address structure
  memset(&echoServAddr, 0, sizeof(echoServAddr)); // zero out structure
  echoServAddr.sin_family = AF_INET; //Internet address family
  echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
  echoServAddr.sin_port = htons(port); // Local port

  // Bind to the local address
  if(bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0){
		printf("bind() failed\n");
	}
    
  // Mark the socket so it will listen for incoming connections
  if (listen(sock, MAXPENDING) < 0){
		printf("listen() failed");
	}
	
	// Return the new socket
  return sock;
}

/**
 * Establishes server connection to the client. Returns this client's socket ID.
 * @param servSock the server socket in which to connect
 * @return this client's socket ID
 */
int AcceptTCPConnection(int servSock)
{
	/** Length of the client address port */
	unsigned int clntLen;
	/** Client address */
	struct sockaddr_in echoClntAddr;
	/** Client socket */
	int clntSock;
	
	// Accept the client's connection
	clntLen = sizeof(echoClntAddr);
	if((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0){
		printf("accept() failed\n");
	}
	
	// Return the updated client socket
	return clntSock;
}

/**
 * Main method for server program. Initiates connection with a client and handles multiple
 * active clients using multi-threading. Each thread then processes its own message and
 * terminates once its associated client disconnects from the server.
 * @param argc number of command line arguments
 * @param argv array of command line arguments
 */
int main(int argc, char * argv[]) {
	int servSock;
	int clntSock;
	unsigned short echoServPort;
	pthread_t threadID;
	struct ThreadArgs *threadArgs;

	signal(SIGPIPE,SIG_IGN);
	init_array(temp_array);

	if (argc != 2) {
		printf("Usage: ./server <port>\n");
		exit(1);
	}

	// Set the server port
	echoServPort = atoi(argv[1]);
	servSock = CreateTCPServerSocket(echoServPort);
	
	while(1){
		clntSock = AcceptTCPConnection(servSock);
		
		// Create separate memory for client argument
		threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs));
		//if((threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs))) == NULL){
		threadArgs->clntSock = clntSock;
		//}
		
		// Create client thread
		if(pthread_create (&threadID, NULL, ThreadMain, (void *) threadArgs) != 0){
			printf("Error creating thread\n");
		}
	}
	
	// Code begins to differ here
	return EXIT_SUCCESS;
}


