/**
 * @file p3.c
 * @author Joshua Good
 * Allows users to create aliases for common bash commands and acts similar to a
 * mini-shell in order to execute these aliases. A user may add new aliases to the
 * mini-shell, search for a previously added alias, list all available aliases, or
 * execute their defined aliases.
 */

#include <stdlib.h>
#include <stdio.h> 
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_TOKENS 7
#define MAX_TABLE_NUM 8
#define BUFFER 200

char * cpy;
// Every simple command has one of these associated with it
// Simple tokenizes the line up to 6 arguments (including alias),
// so get rid of "alias" and just parse the rest
struct simple {
	char *token[MAX_TOKENS]; // tokens of the command
	int count; // the number of tokens
};

// Shortcut for constuction
typedef struct simple Simple;

// parseSequence function prototype
Simple parseSequence();

/**
 * Checks the array of aliases to see if the a new alias can be
 * added. A new alias is valid only if the current array of aliases
 * is not full and if the alias to add is not already in the alias
 * array.
 * @param aliases the array of aliases
 * @param aCount the number of aliases in the alias array
 * @param a the alias to add
 * @return the index of the alias if the alias is a duplicate,
 *         -1 if the table is full, and -2 if the alias does not already
 *         exist in the alias table
 */
int checkAliases(Simple aliases[], int aCount, Simple a){
    // Loop through the array and identify any duplicates (if applicable)
    for(int i = 0; i < aCount; i++){    
      if(strcmp(aliases[i].token[1], a.token[1]) == 0){ // duplicate exists
        return i;
      }
    }
    // No duplicates found, but ensure that the table is not full
    if(aCount == MAX_TABLE_NUM){
      printf("Cannot add additional aliases. Table is full.\n");
      return -1;
    }
  // The alias can be added
  return -2;
}

/**
 * Searches the array of aliases for a specific alias. If the alias is
 * not found in the array, prints an error message stating the given
 * alias was not located.
 * @param aliases the array of aliases
 * @param count the number of aliases in the array
 * @param aName the name of the alias to search
 */
void search(Simple aliases[], int count, char *aName)
{
  // Special check for empty array
  if(count == 0){
    printf("ERROR: no such alias\n");
  }
  /** Index of found alias */
  int index;
  for(int i = 0; i < count; i++){
    if(strcmp(aliases[i].token[1], aName) == 0){
      index = i;
      break;
    } else {
      index = -1;
    }
  }
  if(index != -1){
    for(int j = 1; j < aliases[index].count; j++){
      printf("%s\t", aliases[index].token[j]);
    }
    printf("\n");
  } else {
    printf("ERROR: no such alias\n");
  }
}

/** 
 * Returns the index of the alias in this array if it exists. Returns 0 otherwise.
 * @param aliases the arry of aliases
 * @param count the number of aliases in the array
 * @param aName the name of the alias to search
 * @return if the alias is in the alias array
 */
int searchSimple(Simple aliases[], int count, char *aName)
{
  for(int i = 0; i < count; i++){
    if(strcmp(aliases[i].token[1], aName) == 0){
      return i;
    }
  }
  printf("ERROR: no such alias\n");
  return -1;
}

/**
 * Forks a new child process and executes an alias command from the 
 * array of aliases. Holds the parent process until the child process
 * terminates. If the child process fails, returns execution to the parent
 * process and displays the error message associated with the failed child.
 * @param simp the Simple command to execute
 */
void execCommand(Simple simp){
  // Fork and create a new child process
  /** Process id of the child process*/
  int pid = fork();
  if(pid == 0){
    // Execute the child process with arguments
    execlp(simp.token[2], simp.token[2], simp.token[3],
           simp.token[4], simp.token[5], simp.token[6]);
  } else if(pid > 0){
    /** Status of the child */
    int status;
    // Call wait 
    waitpid(pid, &status, 0);
    if(WIFEXITED(status)){
      if(WEXITSTATUS(status) != 0){
        printf("ERROR: %s return %d", simp.token[2], WEXITSTATUS(status));
      }
    }
  }
  // Seperate execution output from user input
  printf("\n");
}
  
/**
 * Adds the specified alias to the alias array. If the alias to add
 * is a duplicate, copies the new command and its options into the pre - existing
 * alias. Otherwise, adds the new alias to the alias table and increments the table's
 * count.
 * @param aliases the table of aliases
 * @param aCount pointer to the number of aliases in the alias table
 * @param c the command to be added
 * @param index the index of the duplicate alias (if applicable)
 */
void add(Simple aliases[], int *aCount, Simple c, int index)
{
  // Check for a non - duplicate entry
  if(index == -2){
    aliases[(*aCount)++] = c;
  } else {
    // Replace the current alias operation with the new operation
    for(int i = 0; i < c.count; i++){
      if(c.token[i]){
        int len = ((int) strlen(c.token[i])) + 1;
        char *cpy = (char *) malloc(len * sizeof(char *));
        strcpy(cpy, c.token[i]);
        aliases[index].token[i] = cpy;
      } else {
        aliases[index].token[i] = NULL;
      }
    }
    //Update the new alias' count
    aliases[index].count = c.count;
  }
} 
  
/**
 * Processes user created aliases for system commands. Reads in a line of input
 * from standard input and creates a struct for this alias. If the alias
 * only contains "alias", the list of aliases is printed. If the alias contains
 * "alias <alias name>", the alias name and its options is printed if it exists in 
 * the alias list; an error message, otherwise. If an available alias is specified,
 * executes the command represented by that alias with the alias' arguments. If a new
 * alias is specified, creates the alias and adds it to the list of available aliases. 
 * @param argc the number of command line arguments
 * @param argv the array of command line arguments
 * @return program exit status
 */
int main(int argc, char *argv[]) {
	/** The command to process */
  Simple command;
  /** Table of aliases */
  Simple aliases[MAX_TABLE_NUM];
  /** Count of aliases in table */
  int aCount = 0;
  /** Buffer to read in arguments */
  char buffer[BUFFER];
  /** The next character to read in */
  int c;
  /** The next character to read */
  int i = 0;
  /** End flag */
  int end = 0;
  
  // Begin scanning the command line for arguments
  while(1){
    // Notify user of 
    printf(">");
    while((c = getchar()) != '\n'){
      //Check user input for program termination
      if(i == 0 && c == -1){
        // Set the end flag and break
        end = 1;
        break;
      }
      
      buffer[i] = c;
      // Update to the next position
      i++;
    }
    
    // Test the end flag for progam termination
    if(end){
      // Free all memory
      for(int i = 0; i < aCount; i++){
        for(int j = 0; j < aliases[i].count; j++){
          free(aliases[i].token[j]);
        }
      }
      free(cpy);
      // Break and exit the program
      break;
    }
    
    // Null terminate the buffer
    buffer[i] = '\0';
    // Reset the buffer
    i = 0;
    
    // Parse the line for a new command
    command = parseSequence(buffer);
    // Allocate memory for new tokens
    for(int i = 0; i < command.count - 1; i++){
      int len = strlen(command.token[i]);
      cpy = (char *) malloc((len + 1) * sizeof(char *));
      strcpy(cpy, command.token[i]);
      command.token[i] = cpy;
    }
    
    // Handle arguments
    // Search alias array for any matching commands
    if(strcmp(command.token[0], "alias") != 0){
      /** The index of the alias in the array, -1 otherwise */
      int i = searchSimple(aliases, aCount, command.token[0]);
      if(i != -1){
        if(command.count >= 3){
          // Make a copy of this alias and tack on any additional arguments
          Simple simp;
          simp.count = aliases[i].count;
          // Allocate memory for new tokens
          for(int j = 0; j < aliases[i].count - 1; j++){
            int len = strlen(aliases[i].token[j]);
            cpy = (char *) malloc((len + 1) * sizeof(char *));
            strcpy(cpy, aliases[i].token[j]);
            simp.token[j] = cpy;
          }
          // Tack on any additional arguments from command to the end of simp
          for(int j = 1; j < command.count; j++){
            simp.token[simp.count++ - 1] = command.token[j];
          }
          // execute the copy of the command (with additional arguments)
          execCommand(simp);
        } else {
          // execute the copy of the command (without additional arguments)
          execCommand(aliases[i]);
        }
      }
    } else if(command.count == 2){ // the "alias" argument is specified
      // Print out each alias and its tokens
      for(int i = 0; i < aCount; i++){
        for(int j = 1; j < aliases[i].count; j++){
          printf("%s\t", aliases[i].token[j]);
        }
        printf("\n");
      }
    } else if(command.count == 3){ // search for a specified alias
      search(aliases, aCount, command.token[1]);
    } else { // Add the new alias to the array of aliases
      /** 
       * Index reference to the status of this alias based
       * calls to checkAliases()
       */
      int index = checkAliases(aliases, aCount, command);
      // Ensure the alias can be added
      if(index != -1){
        // Add the alias to the alias table and update count
        add(aliases, &aCount, command, index);
      }
    }
    
    // Reset the buffer
    int len = (int) strlen(buffer);
    for(int i = 0; i < len; i++){
      buffer[i] = '\0';
    }
  }
  
	/** Return exit success */
  return EXIT_SUCCESS;
}

/**
 * "parseSequence" function is used to parse the char line got from the standard input 
 * into the simple structure to pass arguments into system calls later. Code referenced
 * from hw2 slides.
 * @param line the line of input to parse
 */
Simple parseSequence(char * line) {
	int i, t;
	Simple c;

	t = 0;

	i = 0;
	while (isspace(line[i]))
		i++;
	c.token[t] = &line[i];

	while (line[i] != '\0' && t < MAX_TOKENS - 1) {
		t++;

		while (!isspace(line[i]) && line[i] != '\0')
			i++;

		while (isspace(line[i])) {
			line[i] = '\0';
			i++;
		}

		c.token[t] = &line[i];
	}
	c.count = t + 1;
	c.token[t] = NULL;

	return c;
}