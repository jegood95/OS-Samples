/* jegood Joshua E Good */

/**
 * @file p3.c
 * Finds the minimum distance for a set of points using a single manager thread and
 * multiple worker threads. Expands the application of HW4 p3, in which this program
 * handles much larger sets of inputs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <math.h>

/** Number of maximum allowable threads */
#define MAX_THREADS 10
/** Number of maximum allowable points for a file */
#define MAX_POINTS 10000
/** Number of maximum allowable tasks by the user */ 
#define MAX_TASK 8

/** Enumerator specifying the type of task */
enum TASK_TYPE { LOCAL_MIN, GLOBAL_MIN };

// Struct representing a point
struct point {
	int x;
	int y;
}typedef Point;

// Struct representing task used by the manager to assign to an available worker thread
struct task {
	enum TASK_TYPE task_type;
  int index;
}typedef Task;

// Struct representing a worker thread
struct worker{
  Point p1; // changed from pointer
  Point p2; // changed from pointer
  unsigned long minDistance;
} typedef Worker;

// Struct representing the minimum distance pair of points
struct pair{
  Point p1;
  Point p2;
  unsigned long minDistance;
}typedef Pair;

// synchronization variables
/** Global array of points */
Point points[MAX_POINTS];
/** Global array of worker threads */
Worker workPairs[MAX_THREADS];
/** Global task queue of tasks */
Task task_queue[MAX_TASK];
/** Number of threads finished executing */
int total_done;
/** Current number of tasks in the queue */
int num_tasks;
/** Number of points in the global array of points */
int num_P;
/** Lock for synchronization */
pthread_mutex_t lock;
/** Condition variable to control reading task queue elements */
pthread_cond_t cv;
/** Condition variable to control adding tasks to the task queue */
pthread_cond_t cv2;
/** The minimum distance among all points in the user-given file */
unsigned long minDist;

/** The total number of worker threads */
unsigned int nworker;

/**
 * Prints a specific error message given the type of error.
 * @parma msg the error message to print
 */
static void Error_msg(const char * msg) {
	// Print the error message
  printf("%s\n", msg);
  // Exit the program with error code
	exit(1);
}

/**
 * Calculates the distance squared between two points.
 * @param p1 the first point
 * @param p2 the second point
 * @return the distance squared between these two points
 */
double calculateDist(Point p1, Point p2){
  return pow((p2.x - p1.x), 2) + pow((p2.y - p1.y), 2);
}

/**
 * Local min function. Determines the local minimum distance for a
 * given point in the point array based on the previous availabe points
 * in the global point array. Updates the calling thread's pair of local
 * minimum distance points if any are less than its current minimum distance.
 * @param t the current task to execute
 * @param id the id of the current thread executing the function
 */
void localMin(Task t, int id){
  // Loop through all points up to index and find the local min distance
  for(int i = 0; i < t.index; i++){
   // Calculate the local min distance
    unsigned long distance = calculateDist(points[i], points[t.index]);
    pthread_mutex_lock(&lock);
    // Check if the current distance is less than the one stored in the executing thread
    if(distance < workPairs[id].minDistance){
      // Update this thread to reflect the new pair of min distance points
      workPairs[id].minDistance = distance;
      workPairs[id].p1 = points[i];
      workPairs[id].p2 = points[t.index];
    }
    pthread_mutex_unlock(&lock);
  }
}

/**
 * Global min function. Calculates the overall minimum distance in the total set
 * of points and determines its pair of corresponding points. Prints this pair and
 * the global min distance to standard output.
 */
void globalMin(){
  /** The pair of min distance points */
  Pair minPair;
  // Establish the max min distance for comparison
  minPair.minDistance = ULONG_MAX;
  // Check each worker and find the minimum pair  
  for(int i = 0; i < nworker; i++){
    // Check for new global min distance
    if(workPairs[i].minDistance < minPair.minDistance){
      minPair.p1 = workPairs[i].p1;
      minPair.p2 = workPairs[i].p2;
      minPair.minDistance = workPairs[i].minDistance;
    }
  }
  
  // Check for one point in point array
  if(num_P == 1){
    printf("(%d, %d) 0\n", points[0].x, points[0].y);
  } else {
    // Print the minDistance and its pair
    printf("(%d, %d) (%d, %d) %lu\n", minPair.p1.x, minPair.p1.y, minPair.p2.x, 
           minPair.p2.y, minPair.minDistance);
  }
}

/**
 * Worker routine for each thread. Grabs a task from the task queue (if available)
 * and executes it. If the next available task is GLOBAL_MIN but the executing thread
 * is not the last available worker thread, updates the global terminated thread counter
 * and terminates the executing thread. Else, the last executing thread calls the globalMin()
 * function to determine the true minimum distance for the set of all given points.
 */
void * worker_routine(void * arg) {
  while(1){
    // Type convert void* to int*
    int *a = (int *) arg;
    pthread_mutex_lock(&lock);
    
    // Check if no tasks are available
    while(num_tasks == 0){
      pthread_cond_wait(&cv, &lock);
    }
    
    // Get the next available task if type is LOCAL_MIN
    if(task_queue[0].task_type == LOCAL_MIN){
      /** Next available task */
      Task t = task_queue[0];
      // Left-shift to adjust queue
      for(int i = 1; i < num_tasks; i++){
        task_queue[i - 1] = task_queue[i];
      }
      //Decrement number of available tasks
      num_tasks--;
      // Signal manager thread to add more tasks
      pthread_cond_signal(&cv2);
      pthread_mutex_unlock(&lock);
      // Compute the localMin for this thread
      localMin(t, *a);
    } else if(task_queue[0].task_type == GLOBAL_MIN && total_done != nworker - 1){ // Check for completed thread
      // Current thread is done
      total_done++;
      // Signal manager thread
      pthread_cond_signal(&cv2);
      pthread_mutex_unlock(&lock);
      return NULL;
    } else {
      // Last worker reached, so compute global min
      globalMin();
      return NULL;
    }
  }
  return NULL;
}

/**
 * Reads in a set of points from a file and computes their minimum distance using
 * a task queue controlled and worked by a manager and number of user-specified worker
 * threads, respectively. Creates tasks for the worker threads until the last thread
 * determines the global minimum distance, and then desynchronizes these threads.
 * @param argc number of command line arguments
 * @param argv array of command line arguments
 */
int main(int argc, char * argv[]) {
  /** Loop counter*/
	unsigned int i;
  
  // Check for incorrect usage
	if (argv[1]==NULL||argv[2]==NULL){
		Error_msg("Usage: ./p3 <thread num> <list file name>");
	}

  // Read in the number of workers
	nworker = atoi(argv[1]);
  // Check invalid number of workers
	if (nworker <= 0) {
		Error_msg("worker number should be larger than 0!");
	}
	if (nworker > MAX_THREADS) {
		Error_msg("Worker number reaches max!");
	}
  
  /** File containing points */
  FILE *fp;
  
  // Open the file for processing and check if it exists
	if(!(fp = fopen(argv[2], "r"))){
		printf("File does not exist\n");
		exit(EXIT_FAILURE);
	}

	/** Start time for a process */
	clock_t start;
	/** End time for a process */
	clock_t finish;
	// Start process clock
	start = clock();
	
	//synchronization initialization
	/** Worker thread */
  pthread_t workers[nworker];
  // Initialize all global counters to 0
  num_tasks = 0;
  num_P = 0;
  total_done = 0;
  /** Worker information for a thread */
  Worker w;
  // Set each worker's min distance to max
  w.minDistance = ULONG_MAX;
  
  /** Array of worker indexes to distinguish threads */
	unsigned int worker_index[nworker];
  // Create worker threads
	for (i = 0; i < nworker; i++) {
		// Set the current worker index
    worker_index[i] = i;
    // Assign a spot for the thread in the array of work pairs 
    workPairs[i] = w;
    // Initialize current thread
		if (pthread_create(&workers[i], NULL, worker_routine, (void *)&worker_index[i]) != 0) {
			// Error on thread creation
      Error_msg("Error creating thread");
		} 
	}
  
  /** Next point */
  Point p;
  /** Track EOF */
  int c;
  /** The next task to execute */
  Task t;
  while(1){
    // Read in a new point
    c = fscanf(fp, "%d %d", &p.x, &p.y);
    pthread_mutex_lock(&lock);
    // Check if the manager cannot add a new task
    while(num_tasks == MAX_TASK){
      // Wait for an empty spot
      pthread_cond_wait(&cv2, &lock);
    }
    
    // Check for EOF
    if(c == EOF){
      // Set information for the global min distance calculation
      t.task_type = GLOBAL_MIN;
      task_queue[num_tasks++] = t;
      // Signal last worker thread to execute task
      pthread_cond_signal(&cv);
      pthread_mutex_unlock(&lock);
      break;
    }
    // Add the point to the global array of points
    points[num_P++] = p;
    // Establish a local task and add it to the task queque
    t.task_type = LOCAL_MIN;
    t.index = num_P - 1;
    task_queue[num_tasks++] = t;
    // Signal worker threads to execute tasks
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&lock);
  }

	// manager routine, join all threads with main once local and global calculations
  // are complete
	for (i = 0;i<nworker; i++) {
		pthread_join(workers[i], NULL);
	}
	
	// End process time
	finish = clock();
	// Print the process time
	printf("Time : %lf seconds\n", (double) (finish - start) / CLOCKS_PER_SEC);

	// Free memory
  free(fp);
  
  // Return exit success
	return 0;
}
