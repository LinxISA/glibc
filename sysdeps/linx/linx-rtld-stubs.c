/* Linx rtld bring-up support stubs.
   Copyright (C) 2026 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static inline void
linx_rtld_barrier (void)
{
  __asm__ __volatile__ ("" ::: "memory");
}

/* Bring-up fallback for rtld exception wrappers.
   NOTE: this does not yet provide full architectural state save/restore.  */
int __sigsetjmp (jmp_buf env, int savemask);
libc_hidden_proto (__sigsetjmp)
int
__sigsetjmp (jmp_buf env, int savemask)
{
  (void) env;
  (void) savemask;
  return 0;
}
libc_hidden_def (__sigsetjmp)

long __syscall_error (long err);
hidden_proto (__syscall_error)
long
__syscall_error (long err)
{
  __set_errno (-err);
  return -1;
}
hidden_def (__syscall_error)

uint32_t
__atomic_load_4 (const volatile void *ptr, int memorder)
{
  (void) memorder;
  return *(const volatile uint32_t *) ptr;
}

void
__atomic_store_4 (volatile void *ptr, uint32_t val, int memorder)
{
  (void) memorder;
  *(volatile uint32_t *) ptr = val;
}

uint64_t
__atomic_load_8 (const volatile void *ptr, int memorder)
{
  (void) memorder;
  return *(const volatile uint64_t *) ptr;
}

void
__atomic_store_8 (volatile void *ptr, uint64_t val, int memorder)
{
  (void) memorder;
  *(volatile uint64_t *) ptr = val;
}

uint32_t
__atomic_exchange_4 (volatile void *ptr, uint32_t desired, int memorder)
{
  uint32_t old;
  (void) memorder;
  old = *(volatile uint32_t *) ptr;
  *(volatile uint32_t *) ptr = desired;
  return old;
}

bool
__atomic_compare_exchange_4 (volatile void *ptr, void *expected,
                             uint32_t desired, bool weak,
                             int success_memorder, int failure_memorder)
{
  uint32_t old;
  uint32_t *exp = (uint32_t *) expected;

  (void) weak;
  (void) success_memorder;
  (void) failure_memorder;

  old = *(volatile uint32_t *) ptr;
  if (old == *exp)
    {
      *(volatile uint32_t *) ptr = desired;
      return true;
    }

  *exp = old;
  return false;
}

uint32_t
__atomic_fetch_add_4 (volatile void *ptr, uint32_t val, int memorder)
{
  uint32_t old;
  (void) memorder;
  old = *(volatile uint32_t *) ptr;
  *(volatile uint32_t *) ptr = old + val;
  return old;
}
extern char **__environ attribute_hidden;

char *
getenv (const char *name)
{
  size_t len;
  char **ep;

  if (name == NULL || name[0] == '\0')
    return NULL;

  len = strlen (name);
  ep = __environ;
  while (ep != NULL && *ep != NULL)
    {
      if (name[0] == (*ep)[0]
          && strncmp (name, *ep, len) == 0
          && (*ep)[len] == '=')
        return *ep + len + 1;
      ++ep;
    }

  return NULL;
}
libc_hidden_def (getenv)
