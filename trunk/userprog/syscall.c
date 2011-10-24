#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  lock_init(&fs_lock);  
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  lock_acquire(&fs_lock);
  printf("entering syscalls.....\n");

  typedef int syscall_function (int, int, int);

  struct syscall
  {
    size_t arg_cnt;
    syscall_function* func;
  };

  static const struct syscall syscall_table[] = 
  {
    {0,(syscall_function*) sys_halt},   //0
    {1,(syscall_function*) sys_exit},   //1
    {1,(syscall_function*) sys_exec},   //2
    {1,(syscall_function*) sys_wait},   //3
    {2,(syscall_function*) sys_create}, //4
    {1,(syscall_function*) sys_remove}, //5
    {1,(syscall_function*) sys_open},   //6
    {1,(syscall_function*) sys_filesize},//7
    {3,(syscall_function*) sys_read},   //8
    {3,(syscall_function*) sys_write},  //9
    {2,(syscall_function*) sys_seek},   //10
    {1,(syscall_function*) sys_tell},   //11
    {1,(syscall_function*) sys_close}   //12
  };

  struct syscall* sc;
  unsigned call_nr;
  int args[3];

  copy_in (&call_nr, f->esp, sizeof call_nr);

  if (call_nr >= (sizeof syscall_table)/(sizeof *syscall_table))
    thread_exit();
  sc = syscall_table + call_nr;

  ASSERT (sc->arg_cnt <= (sizeof args)/(sizeof *args));
  memset (args, 0, sizeof args);
  copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * sc->arg_cnt);
  
  printf ("system call: %d\n", call_nr);
  printf ("arguments are %d, %d, %d\n", (int)args[0], (int)args[1], (int)args[2]);

  f->eax = sc->func (args[0], args[1], args[2]);

  printf("eax is %d\n", f->eax);

  lock_release(&fs_lock);
}

/* Copies a byte from user address USRC to kernel address DST. */
static inline int
get_user (uint8_t *dst, const uint8_t *usrc)
{
  int eax;
  asm ("movl $1f, %%eax; movb %2, %%al; movb %%al, %0; 1:" : "=m" (*dst), "=&a" (eax) : "m" (*usrc));
  return eax != 0;
}

/* Writes BYTE to user address UDST. */
static inline int
put_user (uint8_t *udst, uint8_t byte)
{
  int eax;
  asm ("movl $1f, %%eax; movb %b2, %0; 1:" : "=m" (*udst), "=&a" (eax) : "q" (byte));
  return eax != 0;
}

void sys_halt(void)
{
  //power_off();
}

void sys_exit(int status) 
{
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_exit();
}

pid_t sys_exec(const char *cmd_line)
{
  return process_execute(cmd_line);
}

int sys_wait(pid_t pid)
{
  sema_down(&(thread_by_tid(pid)->wait_for));
}

int sys_create(const char* file, unsigned initial_size)
{
  return 0;
}

int sys_remove(const char* file)
{
  return 0;
}

int sys_open(const char *file)
{
  return 0;
}

int sys_filesize(int fd)
{
  return 0;
}

int sys_read(int fd, void *buffer, unsigned size)
{
  return 0;
}

int sys_write(int fd, void *buffer, unsigned size)
{
  printf("entering sys_write...\n");
  char* cbuffer = (char*) buffer;
  int i;
  for(i = 0; i < size; i++)
  {
    printf((char) cbuffer[i]);
  }
  printf("exiting sys_write!\n");
}

void sys_seek(int fd, unsigned position)
{

}

unsigned sys_tell(int fd)
{
  return 0;
}

void sys_close(int fd)
{

}

int copy_in (uint8_t* dest, uint8_t* src, uint8_t size)
{
  uint8_t i = 0;
  for(; i < size; i++)
  {
    if(!get_user(dest+i, src+i))
	return 0;
  }
  return 1;
}

