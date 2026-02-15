/* Definition for thread-local data handling.  NPTL/LinxISA bring-up version.
   This file is part of the GNU C Library.
 */

#ifndef _LINX_TLS_H
#define _LINX_TLS_H 1

#include <dl-sysdep.h>

#ifndef __ASSEMBLER__
# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include <dl-dtv.h>

/* Bring-up: TP is a System Status Register (SSR 0x0000). */
# define LINX_SSR_TP 0x0000

static inline void *__linx_read_tp(void)
{
  void *tp;
  __asm__ volatile ("ssrget %1, ->%0"
                    : "=r" (tp)
                    : "i" (LINX_SSR_TP)
                    : "memory");
  return tp;
}

static inline void __linx_write_tp(void *tp)
{
  __asm__ volatile ("ssrset %0, %1"
                    :
                    : "r" (tp), "i" (LINX_SSR_TP)
                    : "memory");
}

# define READ_THREAD_POINTER() __linx_read_tp()

/* Get system call information.  */
# include <sysdep.h>

/* The TP points to the start of the thread blocks.  */
# define TLS_DTV_AT_TP 1
# define TLS_TCB_AT_TP 0

/* Get the thread descriptor definition.  */
# include <nptl/descr.h>

typedef struct
{
  dtv_t *dtv;
  void *private;
} tcbhead_t;

/* This is the size of the initial TCB.  */
# define TLS_INIT_TCB_SIZE 0
# define TLS_TCB_SIZE 0

/* This is the size we need before TCB - actually, it includes the TCB.  */
# define TLS_PRE_TCB_SIZE \
  (sizeof (struct pthread)                                                  \
   + ((sizeof (tcbhead_t) + __alignof (struct pthread) - 1)                 \
      & ~(__alignof (struct pthread) - 1)))

/* The thread pointer points to the end of the TCB.  */
# define TLS_TCB_OFFSET 0

# define INSTALL_DTV(tcbp, dtvp) (((tcbhead_t *) (tcbp))[-1].dtv = (dtvp) + 1)
# define INSTALL_NEW_DTV(dtv) (THREAD_DTV() = (dtv))
# define GET_DTV(tcbp) (((tcbhead_t *) (tcbp))[-1].dtv)

# define TLS_INIT_TP(tcbp) \
  ({ __linx_write_tp ((char *) (tcbp) + TLS_TCB_OFFSET); true; })

# define THREAD_DTV() \
  (((tcbhead_t *) (READ_THREAD_POINTER () - TLS_TCB_OFFSET))[-1].dtv)

# define THREAD_SELF \
  ((struct pthread *) (READ_THREAD_POINTER ()                               \
                       - TLS_TCB_OFFSET - TLS_PRE_TCB_SIZE))

# define TLS_DEFINE_INIT_TP(tp, pd) \
  void *tp = (void *) (pd) + TLS_TCB_OFFSET + TLS_PRE_TCB_SIZE

/* libthread_db support is not yet implemented for LinxISA bring-up.  */
# define DB_THREAD_SELF 0

# include <tcb-access.h>

# define NO_TLS_OFFSET -1

#endif /* !__ASSEMBLER__ */

#endif /* _LINX_TLS_H */
