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
  lock_init(&fs_lock);
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  typedef int syscall_function (int, int, int);

  struct syscall
  {
    size_t arg_cnt;
    syscall_function* func;
  };

  static const struct syscall syscall_table[] = 
  {
    {0,(syscall_function*) sys_halt},
    {1,(syscall_function*) sys_exit},
    {1,(syscall_function*) sys_exec},
    {1,(syscall_function*) sys_wait},
    {2,(syscall_function*) sys_create},
    {1,(syscall_function*) sys_remove},
    {1,(syscall_function*) sys_open}
  }

  const struct syscall* sc;
  unsigned call_nr;
  int args[3];

  copy_in (&call_nr, f->esp, sizeof call_nr);
        //NOT IMPLEMENTED

  if (call_nr >= (sizeof syscall_table)/(sizeof *syscall_table))
    thread_exit();
  sc = syscall_table + call_nr;

  ASSERT (sc->arg_cnt <= (sizeof args)/(sizeof *args));
  memset (args, 0, sizeof args);
  copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * sc->arg_cnt);
        //NOT IMPLEMENTED
  
  printf ("system call!\n");
  //thread_exit ();

  f->eax = sc->func (args[0], args[1], args[2]);
}

void halt(void)
{
  power_off();
}

void exit(int status) 
{
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_exit();
}


pid_t exec(const char *cmd_line)
{
  return process_execute(cmd_line);
}


int wait(pid_t pid)
{
  sema_down(&(thread_by_tid(pid)->wait_for);
}

bool create(const* file, unsigned initial_size)
{
  lock_acquire(&fs_lock);
  //filesys_create(......);
  lock_release(&fs_lock);
}

bool remove(const char*file)
{
  return false;
}

int open(const char *file)
{
  return -1;
}

void copy_in (uint8_t* dest, uint8_t* src, uint8_t size)
{
  
}
