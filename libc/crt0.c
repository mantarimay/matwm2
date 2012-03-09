/* crt0.c: C runtime entry point 
 * Written by Kirn Gill <segin2005@gmail.com>
 */
 
#include <stdlib.h>
 
char* __progname;
char** environ;

#ifndef NULL
# define NULL (void *) 0x0
#endif

extern int main(int argc, char **argv, char **envp);

void _start(int argc, char **argv, char **envp)
{
	environ = envp;
	argv[0] ? __progname = argv[0] : __progname = "";
	exit(main(argc, argv, envp));
}
