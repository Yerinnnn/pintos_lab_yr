#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init(void);
struct file *process_get_file(int fd);

#endif /* userprog/syscall.h */
