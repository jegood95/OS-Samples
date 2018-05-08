/* jegood Joshua E Good */

Program Description:
The program "p3.cu" calculates the minimum distance for a user file-specified set of points using CUDA kernel multithreading and parallelism. The program additionally prints the process
time in seconds, calculated from the time the first point is read in until the minimum distance is printed.

Compiling and Execution:
** Pre - requisites: 
	- Ensure "p3.cu" and "Makefile" are situated within the same directory.
	- Since this program uses NVIDIA's CUDA architecture, ensure that you have a CUDA enabled device (i.e. NVIDIA graphics card) before proceeding.
1. Clear any related pre - existing files using the "make clean" statement.
2. Compile the program using "make".
3. Execute the program using "./p3 <input file>". A selection of sample input files have been provided.
	 If you do not specify the input file, the program will print the following usage message:
  
		Usage: ./p3 <input file>
  
  The program launches a block for each point in the file on the kernel via a call from the host (CPU). For each point, the kernel calculates its minmimum distance from every other point
	and transfers this data back to the host once the calculation for the last point completes. The host then calculates the overall minimum distance from each point and prints the points
	with this minimum distance. Finally, the host prints the time taken to complete the overall program process, frees any CUDA-allocated memory, and exits.
	
Time Comparison:
Compared to similar program execution on a multi-threaded worker queue, the CUDA based implementation runs significantly faster. Using a 10,000-point file, the time to complete calculation
on the multi-threaded worker queue with 10 worker threads implementation was 128.900000 seconds versus 1.010000 seconds reported by the CUDA-based implementation. This drastic difference
in process time stems from both the design of the multi-threaded system and the application of parallelism invoked in the CUDA-based implementation. Regarding the multi-threaded system,
since each thread must wait on an available task from the main (host) thread, execution time will drastically slow, even with a maximum of ten concurrent worker threads. Conversely, since
the CUDA-based implementation employs parallelism to simultaneously perform calculations on all points, the time to complete this process will be significantly faster.