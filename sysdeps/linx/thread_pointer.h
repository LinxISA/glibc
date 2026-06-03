/* __thread_pointer definition.  LinxISA version.
   This file is part of the GNU C Library.  */

#ifndef _SYS_THREAD_POINTER_H
#define _SYS_THREAD_POINTER_H

#define LINX_SSR_TP 0x0000

static inline void *
__thread_pointer (void)
{
  void *tp;
  __asm__ volatile ("ssrget %1, ->%0"
                    : "=r" (tp)
                    : "i" (LINX_SSR_TP)
                    : "memory");
  return tp;
}

#endif /* _SYS_THREAD_POINTER_H */
