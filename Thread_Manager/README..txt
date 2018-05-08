/* jegood Joshua E Good */

Program Description:
The program "p3.c" calculates the minimum distance for a given set of file-specified (x,y)-coordinates. The program achieves synchronization through a manager thread
that assigns tasks to a number of user - specified worker threads. Each worker thread calculates its local minimum distance given a specific task, and the last available
worker thread calculates and displays the global minimum distance, as well as its pair of associated points.

Compiling and Execution:
** Pre - requisites: ensure "p3.c" and "Makefile" are situated within the same directory
1. Clear any related pre - existing files using the "make clean" statement.
2. Compile the program using "make".
3. Execute the program using "./p3 <# of threads> <filename>". A selection of sample files has been provided.
   
   If the number of threads and/or the name of the file is not specified, the program displays the following usage message and terminates:
      
      Usage: ./p3 <thread num> <list file name>
  
   If the number of threads is less than zero, the program will display the following error message and terminate:
   
      worker number should be larger than 0!
   
   If the number of threads if greater than the max number (10), the program will display the following error message and terminate:'
   
      Worker number reaches max!
   
   If a non-existent file is specified, the following error message will be printed and the program terminates:
	
	 File does not exist
	
	If the file and number of threads are valid, the manager thread reads in points and establishes tasks for the worker threads, while the worker threads execute
  these tasks, concurrently. Once the last point is compared to all other points, the last available worker thread calculates the global minimum distance for the set
  of points and prints it, alongside its associated pair of points, to standard output.