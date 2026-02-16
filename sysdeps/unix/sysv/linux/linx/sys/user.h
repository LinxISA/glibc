/* <sys/user.h> for LinxISA Linux user register ABI.  */

#ifndef _SYS_USER_H
#define _SYS_USER_H 1

struct user_pt_regs
{
  unsigned long regs[25];
};

struct user_regs_struct
{
  unsigned long regs[25];
};

#endif /* _SYS_USER_H */
