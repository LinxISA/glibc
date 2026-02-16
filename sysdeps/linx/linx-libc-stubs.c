/* Linx libc bring-up support stubs.
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
#include <stdbool.h>
#include <stdint.h>
/* Bring-up fallback for Linux syscall wrappers.  */
long __syscall_error (long err);
long
__syscall_error (long err)
{
  __set_errno (-err);
  return -1;
}
hidden_def (__syscall_error)

/* Bring-up 64-bit atomic helper fallbacks.
   Keep 32-bit helpers in fallback libgcc to avoid duplicate providers.  */
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

uint64_t
__atomic_exchange_8 (volatile void *ptr, uint64_t desired, int memorder)
{
  uint64_t old;
  (void) memorder;
  old = *(volatile uint64_t *) ptr;
  *(volatile uint64_t *) ptr = desired;
  return old;
}

bool
__atomic_compare_exchange_8 (volatile void *ptr, void *expected,
                             uint64_t desired, bool weak,
                             int success_memorder, int failure_memorder)
{
  uint64_t old;
  uint64_t *exp = (uint64_t *) expected;

  (void) weak;
  (void) success_memorder;
  (void) failure_memorder;

  old = *(volatile uint64_t *) ptr;
  if (old == *exp)
    {
      *(volatile uint64_t *) ptr = desired;
      return true;
    }

  *exp = old;
  return false;
}

uint64_t
__atomic_fetch_add_8 (volatile void *ptr, uint64_t val, int memorder)
{
  uint64_t old;
  (void) memorder;
  old = *(volatile uint64_t *) ptr;
  *(volatile uint64_t *) ptr = old + val;
  return old;
}

uint64_t
__atomic_fetch_sub_8 (volatile void *ptr, uint64_t val, int memorder)
{
  uint64_t old;
  (void) memorder;
  old = *(volatile uint64_t *) ptr;
  *(volatile uint64_t *) ptr = old - val;
  return old;
}

uint64_t
__atomic_fetch_and_8 (volatile void *ptr, uint64_t val, int memorder)
{
  uint64_t old;
  (void) memorder;
  old = *(volatile uint64_t *) ptr;
  *(volatile uint64_t *) ptr = old & val;
  return old;
}

uint64_t
__atomic_fetch_or_8 (volatile void *ptr, uint64_t val, int memorder)
{
  uint64_t old;
  (void) memorder;
  old = *(volatile uint64_t *) ptr;
  *(volatile uint64_t *) ptr = old | val;
  return old;
}

uint64_t
__atomic_fetch_xor_8 (volatile void *ptr, uint64_t val, int memorder)
{
  uint64_t old;
  (void) memorder;
  old = *(volatile uint64_t *) ptr;
  *(volatile uint64_t *) ptr = old ^ val;
  return old;
}
