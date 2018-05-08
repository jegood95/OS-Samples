/* jegood Joshua E Good */

Program Description:
The program "p3.c" Allows users to create aliases for common bash commands and acts similar to a mini-shell 
in order to execute these aliases. A user may add new aliases to the mini-shell, search for a previously 
added alias, list all available aliases, or execute their defined aliases.

Compiling and Execution:
** Pre - requisites: ensure "p3.c" and "Makefile" are situated within the same directory
1. Clear any related pre - existing files using the "make clean" statement.
2. Compile the program using "make".
3. Execute the program using "./p3". You may create any new alias using the following format:
  
  alias <alias_name> <command> <options>*, where <options>* is 0 or more options to specify for this command
  
  To view all currently available aliases, type "alias" on the command line. To view a specific, available
  alias, type "alias <alias_name>" on the command line. To execute an available alias, type "<alias_name>".