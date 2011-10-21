#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

void halt(void);
void exit(int status);
id_t exec(const char *cmd_line);
int wait(pid_t pid);
bool create(const* file, unsigned initial_size);
bool remove(const char*file);
int open(const char *file);

struct lock fs_lock;

#endif /* userprog/syscall.h */
