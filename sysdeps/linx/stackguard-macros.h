#include <stdint.h>
#include <tls.h>

#define STACK_CHK_GUARD \
  (((tcbhead_t *) ((char *) READ_THREAD_POINTER () - TLS_TCB_OFFSET))[-1].stack_guard)

#define POINTER_CHK_GUARD \
  (((tcbhead_t *) ((char *) READ_THREAD_POINTER () - TLS_TCB_OFFSET))[-1].pointer_guard)
