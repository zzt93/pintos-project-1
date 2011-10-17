#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}
/*
void
hault(void)
{

}

void
exit(int status)
{

}

pid_t 
exec(const char *cmd_line)
{

}

int wait(pid_t pid)
{

}

bool 
create(const* file, unsigned initial_size)
{

}

bool
remove(const char*file)
{

}

int 
open(const char *file)
{

}
*/
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
}
