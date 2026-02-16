/* Assembly/C macros for LinxISA Linux syscalls (bring-up).
   This file is part of the GNU C Library.

   NOTE: LinxISA is a block-structured ISA. Linux-style syscalls use
   `acrc 1` with the following convention:
     a7 = syscall number
     a0..a5 = args
     a0 = return value (negative errno in range [-4095, -1] on error)
 */

#ifndef _LINUX_LINX_SYSDEP_H
#define _LINUX_LINX_SYSDEP_H 1

#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/unix/sysdep.h>
#include <tls.h>

#undef SYS_ify
#define SYS_ify(syscall_name) __NR_##syscall_name

#ifdef __ASSEMBLER__

/* We intentionally keep the assembler side minimal for bring-up. */

#undef ENTRY
#define ENTRY(name)            \
  .globl name;                \
  .type  name, @function;     \
name:

#undef END
#define END(name)              \
  .size name, .-name

#define L(label) .L##label

/* System call wrapper helper.
   TODO: once Linx has a dedicated syscall instruction, use it. */
#undef PSEUDO
#define PSEUDO(name, syscall_name, args)                        \
  .text;                                                        \
  .align 2;                                                     \
  ENTRY (name);                                                 \
  /* a7 = syscall number */                                     \
  addiw zero, SYS_ify (syscall_name), ->a7;                     \
  acrc 1;                                                       \
  /* errors are returned as -errno in a0 */                     \
  addiw zero, -4096, ->a7;                                      \
  /* if a0 > -4096 then error */                                \
  C.BSTART COND, .Lsyscall_error##name;                         \
  setc.ltu a7, a0;

#undef PSEUDO_END
#define PSEUDO_END(name)                                        \
.Lsyscall_error##name:                                          \
  /* Tail-call to generic handler in libc. */                   \
  j __syscall_error;                                            \
  END (name)

#undef PSEUDO_NOERRNO
#define PSEUDO_NOERRNO(name, syscall_name, args)                \
  .text;                                                        \
  .align 2;                                                     \
  ENTRY (name);                                                 \
  addiw zero, SYS_ify (syscall_name), ->a7;                     \
  acrc 1;

#undef PSEUDO_END_NOERRNO
#define PSEUDO_END_NOERRNO(name) END (name)

#undef PSEUDO_ERRVAL
#define PSEUDO_ERRVAL(name, syscall_name, args)                 \
  PSEUDO_NOERRNO (name, syscall_name, args)                     \
  neg a0, a0;

#undef PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(name) END (name)

#undef ret_NOERRNO
#define ret_NOERRNO C.BSTART.STD RET
#undef ret_ERRVAL
#define ret_ERRVAL C.BSTART.STD RET
#undef ret
#define ret C.BSTART.STD RET

#else /* !__ASSEMBLER__ */

#undef HAVE_INTERNAL_BRK_ADDR_SYMBOL
#define HAVE_INTERNAL_BRK_ADDR_SYMBOL 1

#define INTERNAL_SYSCALL(name, nr, args...) \
  internal_syscall##nr (SYS_ify (name), args)

#define INTERNAL_SYSCALL_NCS(number, nr, args...) \
  internal_syscall##nr (number, args)

/* LinxISA Linux userspace syscall trap. */
#define __SYSCALL_INSN "acrc 1\n\t"
#define __SYSCALL_CLOBBERS "memory"

#define internal_syscall0(number, dummy...)                             \
({                                                                      \
  long int _sys_result;                                                 \
  {                                                                     \
    register long int __a7 asm ("a7") = (number);                       \
    register long int __a0 asm ("a0");                                  \
    __asm__ volatile (__SYSCALL_INSN                                    \
                      : "=r" (__a0)                                     \
                      : "r" (__a7)                                      \
                      : __SYSCALL_CLOBBERS);                            \
    _sys_result = __a0;                                                 \
  }                                                                     \
  _sys_result;                                                          \
})

#define internal_syscall1(number, arg0)                                 \
({                                                                      \
  long int _sys_result;                                                 \
  long int _arg0 = (long int) (arg0);                                   \
  {                                                                     \
    register long int __a7 asm ("a7") = (number);                       \
    register long int __a0 asm ("a0") = _arg0;                          \
    __asm__ volatile (__SYSCALL_INSN                                    \
                      : "+r" (__a0)                                     \
                      : "r" (__a7)                                      \
                      : __SYSCALL_CLOBBERS);                            \
    _sys_result = __a0;                                                 \
  }                                                                     \
  _sys_result;                                                          \
})

#define internal_syscall2(number, arg0, arg1)                           \
({                                                                      \
  long int _sys_result;                                                 \
  long int _arg0 = (long int) (arg0);                                   \
  long int _arg1 = (long int) (arg1);                                   \
  {                                                                     \
    register long int __a7 asm ("a7") = (number);                       \
    register long int __a0 asm ("a0") = _arg0;                          \
    register long int __a1 asm ("a1") = _arg1;                          \
    __asm__ volatile (__SYSCALL_INSN                                    \
                      : "+r" (__a0)                                     \
                      : "r" (__a7), "r" (__a1)                          \
                      : __SYSCALL_CLOBBERS);                            \
    _sys_result = __a0;                                                 \
  }                                                                     \
  _sys_result;                                                          \
})

#define internal_syscall3(number, arg0, arg1, arg2)                     \
({                                                                      \
  long int _sys_result;                                                 \
  long int _arg0 = (long int) (arg0);                                   \
  long int _arg1 = (long int) (arg1);                                   \
  long int _arg2 = (long int) (arg2);                                   \
  {                                                                     \
    register long int __a7 asm ("a7") = (number);                       \
    register long int __a0 asm ("a0") = _arg0;                          \
    register long int __a1 asm ("a1") = _arg1;                          \
    register long int __a2 asm ("a2") = _arg2;                          \
    __asm__ volatile (__SYSCALL_INSN                                    \
                      : "+r" (__a0)                                     \
                      : "r" (__a7), "r" (__a1), "r" (__a2)              \
                      : __SYSCALL_CLOBBERS);                            \
    _sys_result = __a0;                                                 \
  }                                                                     \
  _sys_result;                                                          \
})

#define internal_syscall4(number, arg0, arg1, arg2, arg3)               \
({                                                                      \
  long int _sys_result;                                                 \
  long int _arg0 = (long int) (arg0);                                   \
  long int _arg1 = (long int) (arg1);                                   \
  long int _arg2 = (long int) (arg2);                                   \
  long int _arg3 = (long int) (arg3);                                   \
  {                                                                     \
    register long int __a7 asm ("a7") = (number);                       \
    register long int __a0 asm ("a0") = _arg0;                          \
    register long int __a1 asm ("a1") = _arg1;                          \
    register long int __a2 asm ("a2") = _arg2;                          \
    register long int __a3 asm ("a3") = _arg3;                          \
    __asm__ volatile (__SYSCALL_INSN                                    \
                      : "+r" (__a0)                                     \
                      : "r" (__a7), "r" (__a1), "r" (__a2), "r" (__a3)  \
                      : __SYSCALL_CLOBBERS);                            \
    _sys_result = __a0;                                                 \
  }                                                                     \
  _sys_result;                                                          \
})

#define internal_syscall5(number, arg0, arg1, arg2, arg3, arg4)         \
({                                                                      \
  long int _sys_result;                                                 \
  long int _arg0 = (long int) (arg0);                                   \
  long int _arg1 = (long int) (arg1);                                   \
  long int _arg2 = (long int) (arg2);                                   \
  long int _arg3 = (long int) (arg3);                                   \
  long int _arg4 = (long int) (arg4);                                   \
  {                                                                     \
    register long int __a7 asm ("a7") = (number);                       \
    register long int __a0 asm ("a0") = _arg0;                          \
    register long int __a1 asm ("a1") = _arg1;                          \
    register long int __a2 asm ("a2") = _arg2;                          \
    register long int __a3 asm ("a3") = _arg3;                          \
    register long int __a4 asm ("a4") = _arg4;                          \
    __asm__ volatile (__SYSCALL_INSN                                    \
                      : "+r" (__a0)                                     \
                      : "r" (__a7), "r" (__a1), "r" (__a2), "r" (__a3), \
                        "r" (__a4)                                      \
                      : __SYSCALL_CLOBBERS);                            \
    _sys_result = __a0;                                                 \
  }                                                                     \
  _sys_result;                                                          \
})

#define internal_syscall6(number, arg0, arg1, arg2, arg3, arg4, arg5)   \
({                                                                      \
  long int _sys_result;                                                 \
  long int _arg0 = (long int) (arg0);                                   \
  long int _arg1 = (long int) (arg1);                                   \
  long int _arg2 = (long int) (arg2);                                   \
  long int _arg3 = (long int) (arg3);                                   \
  long int _arg4 = (long int) (arg4);                                   \
  long int _arg5 = (long int) (arg5);                                   \
  {                                                                     \
    register long int __a7 asm ("a7") = (number);                       \
    register long int __a0 asm ("a0") = _arg0;                          \
    register long int __a1 asm ("a1") = _arg1;                          \
    register long int __a2 asm ("a2") = _arg2;                          \
    register long int __a3 asm ("a3") = _arg3;                          \
    register long int __a4 asm ("a4") = _arg4;                          \
    register long int __a5 asm ("a5") = _arg5;                          \
    __asm__ volatile (__SYSCALL_INSN                                    \
                      : "+r" (__a0)                                     \
                      : "r" (__a7), "r" (__a1), "r" (__a2), "r" (__a3), \
                        "r" (__a4), "r" (__a5)                          \
                      : __SYSCALL_CLOBBERS);                            \
    _sys_result = __a0;                                                 \
  }                                                                     \
  _sys_result;                                                          \
})

extern long int __syscall_error (long int neg_errno);

#endif /* !__ASSEMBLER__ */

#endif /* _LINUX_LINX_SYSDEP_H */
