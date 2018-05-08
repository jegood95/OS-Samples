/* jegood Joshua Good */

/**
 * @file p3.cu
 * Calculates the minimum distance for a set of file-specified points using GPU 
 * multi-threading. This program requires access to a CUDA-enabled GPU (i.e. NVIDIA 
 * graphics card).
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <time.h>

/** Maximum number of threads per block */
#define MAX_THRDS 1024

// Point struct
struct point{
int x;
int y;
int index;
double minDistance;
}typedef Point;

/**
 * Calculates the minimum distance for this point from each point
 * in the points array.
 * @param points the point array
 * @param numPoints number of points in the point array
 */
__global__ void calcMinDist(Point *points, int numPoints)
{
	// Compute the minimum distance for each point in the point array
	for(int i = 0; i < numPoints; i++){
		// Ensure we don't calculate the distance to a point from itself
		if(i != points[blockIdx.x].index){
			double distance = sqrt(pow((double)(points[i].x - points[blockIdx.x].x), 2) + pow((double)(points[i].y - points[blockIdx.x].y), 2));
			// Check if distance is a new minimum distance for this point
			if(distance < points[blockIdx.x].minDistance){
				points[blockIdx.x].minDistance = distance;
			}
		}
	}
}

/**
 * Calculates the minimum distance for a set of file-specified points using a CUDA
 * kernel function. Reports this information and its associated minimum distance points
 * alongside the time taken to complete this process.
 * @param argc number of command line arguments
 * @param argv list of command of line arguments
 */ 
int main(int argc, char *argv[])
{
	FILE *fp;
	// Ensure a valid file is given
	if(!(fp = fopen(argv[1], "r"))){
		printf("Usage: ./p3 <input file>\n");
		exit(EXIT_FAILURE);
	}
	
	/** Start time for a process */
	clock_t start;
	/** End time for a process */
	clock_t finish;
	// Start process clock
	start = clock();
	
	// Initially loop through and calculate the number of points in the file
	Point p;
	/** Number of points in the file */
	int numPoints = 0;
	while(fscanf(fp, "%d%d", &p.x, &p.y) == 2){ // read, but don't store(*)
		numPoints++;
	}
	
	// Rewind the file and assign points in the array of points
	rewind(fp);
	/** Index of point in points array */
	int index = 0;
	Point points[numPoints];
	for(int i = 0; i < numPoints; i++){
		// Scan in next point
		fscanf(fp, "%d %d", &p.x, &p.y);
		p.index = index;
		p.minDistance = INFINITY;
		points[i] = p;
		index++;
	}
	
	// Allocate memory for kernel threads
	double minDist = INFINITY;
	Point *arr_p;
	int size = numPoints * sizeof(Point);
	cudaMalloc((void**)&arr_p, size);
	cudaMemcpy(arr_p, points, size, cudaMemcpyHostToDevice);
	
	// Launch the kernel to do work
	// Runs numPoints blocks with one thread each
	calcMinDist<<<numPoints, 1>>>(arr_p, numPoints);
	// Use result on host
	cudaMemcpy(points, arr_p, size, cudaMemcpyDeviceToHost);
	
	// Determine minDist for these points
	for(int i = 0; i < numPoints; i++){
		if(points[i].minDistance < minDist){
			minDist = points[i].minDistance;
		}
	}
	
	// Determine which points have minimum distance
	for(int i = 0; i < numPoints; i++){
		if(points[i].minDistance == minDist){
			printf("(%d,%d)", points[i].x, points[i].y);
		}
	}
	// Print the minimum distance for the set of points
	printf("%lf\n", minDist);
	
	// End process time
	finish = clock();
	// Print the process time
	printf("Time : %lf seconds\n", (double) (finish - start) / CLOCKS_PER_SEC);
	
	// Free memory
	cudaFree(arr_p);
	
	// Return EXIT_SUCCESS
	return 0;
}
