/* jegood Joshua E Good */

Program Description:
The program "p3.c" prints the process ids of all user processes in the system given user - specified
options via the command line. A user may request the process name ("-n"), schedule time in user
mode ("-u"), address above and below which the program text can run ("-a"), and parent process id
for these processes. By default, and even if the user does not specify a particular option, the
program will print the process id for each process.

Compiling and Execution:
** Pre - requisites: ensure "p3.c" and "Makefile" are situated within the same directory
1. Clear any related pre - existing files using the "make clean" statement.
2. Compile the program using "make".
3. Execute the program using "./p3 <-options>", where "<-options>" are any of the options listed
   above. Options may appear in any order and may be divided based on the user's preference; however,
   each separate list of options must be preceded by a "-". For example, the following execution calls
   are legal:
   ./p3 -aunp
   ./p3 -a -u -n -p3
   ./pe -aun -p