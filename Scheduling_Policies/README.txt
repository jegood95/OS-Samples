/* jegood Joshua Good */

Program Description:
The program p3.c executes three processes through a user-specified scheduling policy. Once identified, the program creates three threads (each representing one process)
and executes each process based upon the aforementioned policy (either FIFO, RR, or SCHED_OTHER). The corresponding execution is printed to standard output. Once complete,
the program prints "P3 completes!" and terminates.

Compiling and Execution:
** Pre - requisites: ensure "p3.c" and "Makefile" are situated within the same directory
1. Clear any related pre - existing files using the "make clean" statement.
2. Compile the program using "make".
3. Execute the program using "./p3 <scheduling policy>", where "scheduling policy" is an integer in the range 0 - 2

    If the policy number is not specified, the program prints a usage message and terminates:
        
        Indicate the scheduling policy type.
        
    If the user specifies an invalid policy number, the program prints the following error message and sets the schedulding policy to SCHED_OTHER:
        
        Invalid selection: <scheduling policy>
        
    Otherwise, the program interprets the policy number as follows: 0 = FIFO, 1 = RR, and 2 = OTHER. Three threads are created and execute each process based upon the scheduling
    policy. The program reflects this order of execution through standard output, printing each process' id. When execution finishes, the program joins all threads, destroys each
    policy attribute, and prints "P3 completes" to standard output before terminating.
