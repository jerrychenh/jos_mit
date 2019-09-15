// hello, world
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("hello, world\n");
	// at first thisenv is not set in libmain.c, so this pointer value is 0, 
	// deref it will cause page fault
	cprintf("i am environment %08x\n", thisenv->env_id);
}
