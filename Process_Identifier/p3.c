/**
 * @file p3.c
 * @author Joshua Good (jegood)
 * Lists the process identifiers of different CPU tasks given an assortment 
 * of command line arguments. A user may optionally specify if he or she wishes to
 * print the process' schedule time (in user mode), the process' name, the address
 * above and below which the program text can run, and the process' parent id.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

// Struct representing a process id
struct ProcessID{
  // The process id
  int pid;
  // Program name for the process
  char pname[300];
  // address above which program text can run
  unsigned long above;
  // address below which program text can run
  unsigned long below;
  // Process schedule time
  unsigned long time;
  // Process' parent id
  int parid;
};

// Struct representing a node in a linked list
struct NodeTag{
  // PID to hold
  struct ProcessID value;
  // Pointer to the next PID in the list
  struct NodeTag *next;
};

// Shortcut for constructing PIDs
typedef struct ProcessID PID; 
// Shortcut for processing nodes
typedef struct NodeTag Node;
 
/**
 * Checks the command line arguments for correct format.
 * A correctly formatted argument begins with a leading "-".
 * @param argc number of command line arguments
 * @param argv command line arguments
 */
 void checkFormat(int argc, char *argv[])
{
  // Loop through all command line arguments
  for(int i = 1; i < argc; i++){
    // Check for missing hyphen
    if(argv[i][0] != '-'){
      // Print usage message and exit
      fprintf(stderr, "Usage: ./p3 <-options>\n");
      exit(EXIT_FAILURE);
    } else if(!isalpha(argv[i][1])){ // No letter is provided
      // Print usage message and exit
      fprintf(stderr, "Options require alphabetic characters.\n");
      exit(EXIT_FAILURE);
    }
  } 
} 
 
/**
 * Print the header information based on user - selected options.
 * @param bitmap the bitmap field in which to check selected options
 */ 
void printHeader(int bitmap)
{
  printf("pid\t");
  if(bitmap & 1) {
    printf("pname\t");
  }
  if(bitmap & 4){
    printf("address\t");
  }
  if(bitmap & 2){
    printf("%8s\t", "time");
  }
  if(bitmap & 8){
    printf("parent\t");
  }
  printf("\n");
}

/**
 * Prints the PIDs based on user - specified options.
 * By default, the process id for each pid will always be printed
 * @param head the head of the list
 * @param bitmap the bitmap field in which to check selected options
 */
void printPIDs(Node *head, int bitmap)
{
  // Print the user - specified information for each process id
  for(Node *n = head; n; n = n->next){
    // Print the process id
    printf("%d\t", n->value.pid);
    // Check the bitflags for printing appropriate arguments
    if(bitmap & 1) { // process name
      // Print the process name
      printf("%s\t", n->value.pname);
    }
    if(bitmap & 4){ // process addresses above and below
      // Print the process address
      printf("%lu, %lu\t", n->value.above, n->value.below);
    }
    if(bitmap & 2){ // process schedule time
      // Print the schedule time in user mode
      printf("%lu\t", n->value.time);
    }
    if(bitmap & 8){ // process' parent id 
      // Print the process' parent id
      printf("%d\t", n->value.parid);
    }
    // End bit flag search
    printf("\n");
  }
}

/**
 * Returns the process id for this process
 * @param stat the stat file for the current process identifier
 * @return the process id
 */
int getProcessID(FILE *stat){
  /** The process id of this process */
  int pid;
  fscanf(stat, "%d", &pid);
  return pid;
}

/**
 * Returns the program name for this process
 * @param stat the stat file for the current process identifier
 * @return the program name for this process
 */
void getProcessName(FILE *stat, PID *process)
{
  /** The program name for this process */
  char name[300];
  // Scan stat file for program name
  fscanf(stat, "%s ", name);
  strcpy(process->pname, name);
}

/**
 * Returns the parent id for this process.
 * @param stat the stat file for the current process identifier
 * @return the parent of id for this process
 */
int getParentID(FILE *stat)
{
  /** The parent id for this process */
  int parid;
  fscanf(stat, "%*c %d", &parid);
  return parid;
}

/**
 * Returns the schedule time in user mode for this process
 * @param stat the stat file for the current process identifier
 * @return the schedule time in user mode for this process
 */
int getUserTime(FILE *stat)
{
  /** The scheduled time in user mode for this process */
  unsigned long time;
  fscanf(stat, "%*d %*d %*d %*d %*u %*u %*u %*u %*u %lu", &time);
  return time;
}

/**
 * Returns the address above and below which the program
 * text can run.
 * @param stat the stat file for the current process identifier
 * @param process struct representing the process' addresses to modify
 */
void getAddresses(FILE *stat, PID *process)
{
  /** Address above which the program can run */
  unsigned long abv;
  /** Address below which the program can run */
  unsigned long blw;
  
  // Scan addresses from stat and assign these values for this process
  fscanf(stat, "%*u %*d %*d %*d %*d %*d %*d %*u %*u %*d %*u %lu %lu", &abv, &blw);
  process->above = abv;
  process->below = blw;
}

/**
 * Insert method for the list of PIDs. PIDs are ordered based
 * on their respective parent ids in ascending order.
 * @param head the head of the list
 * @param process the PID to insert into the list
 */
Node *insert(Node *head, PID process)
{
  /** The next node in the list */
  Node *n = (Node *) malloc(sizeof(Node));
  
  // Special case head is NULL
  if(head == NULL){
    n->value = process;
    n->next = NULL;
    head = n;
    return head;
  }
  
  /** The current node */
  Node *current = head;
  
  // Assign the PID to the correct position in the list
  while(current != NULL){
    // Check for equal process id and parent id
    if(current->value.parid == process.parid){
      while(current->next != NULL && current->next->value.parid == process.parid){
        current = current->next;
      }
      // This works
      n->value = process;
      n->next = current->next;
      current->next = n;
      return head;
    }
    
    // Special condition if the end is reached
    if (current->next == NULL){
      break;
    }
    
    // Update current to the next node
    current = current->next;
  }
  
  // PID to add does not match any previous PIDs, so add to end of list
  n->value = process;
  n->next = NULL;
  current->next = n;
  
  //Return head node
  return head;
}

/**
 * Opens the "/proc" directory in the system and reads through each directory,
 * retrieving the process' id, program name, associated addresses, parent id,
 * and schedule time in user mode. The current process is then inserted into
 * the list of processes based on matching parent ids. The list of processes
 * is then printed once complete.
 * @param bitmap the bitmap field in which to check selected options
 */
void handleArguments(int bitmap)
{
  /** Head of the linked list */
  Node *head = NULL;
  //open proc directory
  DIR *proc = opendir("/proc");
  /** The current directory accessed */
  struct dirent *current = readdir(proc);
  
  // Access available directories
  while(current){
    
    // Check the current directory name for appropriate format
    if(isdigit(current->d_name[0])){
      
      /** Name of the process directory */
      char pdir[256];
      /** Struct representing this process id */
      PID process;
      
      // Create the process directory for reading
      strcpy(pdir, "/proc/");
      strcat(pdir, current->d_name);
      strcat(pdir, "/stat");
      /** The process directory */
      FILE *stat;
      //Open the process directory
      stat = fopen(pdir, "r");
    
      // Get all desirable information for this process
      // Get the process id for this directory
      // To be updated
      process.pid = getProcessID(stat);
      // Get the process name
      getProcessName(stat, &process);
      // Get the parent id
      process.parid = getParentID(stat);
      // Get the schedule time (user mode)
      process.time = getUserTime(stat);
      // Get the process' addresses
      getAddresses(stat, &process);
      
      // Assert the current Process identifier in the list of PIDs
      head = insert(head, process);
      // Close the current stat file
      fclose(stat);
    }
    // open the next available directory
    current = readdir(proc);
  }
  
  // Print out the list of processes
  printPIDs(head, bitmap);
  
  // Free all the nodes
  while(head){
    Node *next = head->next;
    free(head);
    head = next;
  }
  // free memory
  free(current);
}
 
/**
 * Main operation for the program. Handles system calls and checks user - specified
 * options for appropriate listing actions. Prints the header for the program based
 * on the discovered options and processes them accordingly.
 * @param argc number of command line arguments
 * @param argv command line arguments
 * @return exit status
 */
int main(int argc, char *argv[])
{
  /** Command line argument to process */
  char *input = (char *) malloc(6 * sizeof(char));
  /** index of current option */
  int i;
  /** bitmap to determine specified command line options */
  int bitmap = 0;
  
  // Check for incorrect argument format
  checkFormat(argc, argv);
  
  // Read in each option and process accordingly
  for(int j = 1; j < argc; j++ ){
    
    // Determine if the current argument has any available option(s)
    if((sscanf(argv[j], "%s", input) == 1)) {
      // Point to the fist option in the argument
      i = 1;
      // Determine the type of option for each available in the argument
      // Code re-purposed from provided homework example code
      while(input[i] != '\0'){
        switch(input[i]){
          case 'n': // process name
            bitmap = bitmap | 1;
            break;
          case 'u': // schedule time
            bitmap = bitmap | 2;
            break;
          case 'a': // process address
            bitmap = bitmap | 4;
            break;
          case 'p': // parent of process
            bitmap = bitmap | 8;
            break;
          default:
            bitmap = bitmap | 0;
        }
        // Point to the next option
        i++;
      }
    }
  }
  // print appropriate headers
  printHeader(bitmap);
  // handle bit flag arguments
  handleArguments(bitmap);
  
  // free memory
  free(input);
  
  //Return program exit status
  return EXIT_SUCCESS;
}