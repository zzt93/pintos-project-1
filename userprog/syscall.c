#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{  
  printf("syscall_init beginning...\n");
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&fs_lock);  
  printf("syscall_init complete\n");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //lock_acquire(&fs_lock);
  printf("entering syscalls.....\n");

  //int q;
  //for(q = 19; q >= 0; q--)
  //  printf("q is %d: \t%d\n", (((int*)f->esp) + q), (int)*(((int*)f->esp) + q));

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
  printf ("arguments are %u, %u, %u\n", (unsigned)args[0], (unsigned)args[1], (unsigned)args[2]);

  f->eax = sc->func (args[0], args[1], args[2]);

  printf("eax is %d\n", f->eax);

  //lock_release(&fs_lock);
}

/* Copies a byte from user address USRC to kernel address DST. */
static inline int
get_user (uint8_t *dst, const uint8_t *usrc)
{
  if(is_user_vaddr(usrc) &&  is_kernel_vaddr(dst))
  {
    int eax;
    asm ("movl $1f, %%eax; movb %2, %%al; movb %%al, %0; 1:" : "=m" (*dst), "=&a" (eax) : "m" (*usrc));
    return eax != 0;
  }
  else
  {
    if(is_user_vaddr(usrc))
      printf("Kernel address is outside of its space: is %u\n", (unsigned)dst);
    if(is_kernel_vaddr(dst))
      printf("User address is outside of its space: is %u\n", (unsigned)dst);
    return 0;
  }
}

/* Writes BYTE to user address UDST. */
static inline int
put_user (uint8_t *udst, uint8_t byte)
{
  if(is_user_vaddr(udst))
  {
    int eax;
    asm ("movl $1f, %%eax; movb %b2, %0; 1:" : "=m" (*udst), "=&a" (eax) : "q" (byte));
    return eax != 0;
  }
  else
  {
    return 0;
  }
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

void sys_halt(void)
{
  shutdown_power_off();
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
  //sema_down(&(thread_by_tid(pid)->wait_for));
  return 0;
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
  // step 1: copyin buffer
  char* a = malloc(size * sizeof(char));
  copy_in(a, buffer, size);  

  // setp 2: write
  if(fd == 1) {
    putbuf(a, size);
  } else {
  }

  free(a);

  return 0; //strlen(buffer)+1;
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

