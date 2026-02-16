/* Linux clone wrapper for LinxISA bring-up.
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
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sched.h>

/* Bring-up fallback:
   Linx clone child-entry ABI is not finalized yet.  Keep the symbol present
   so hosted libc can build, but report ENOSYS at runtime for now.  */
int
__clone (int (*fn) (void *), void *child_stack, int flags, void *arg, ...)
{
  __set_errno (ENOSYS);
  return -1;
}
libc_hidden_def (__clone)
weak_alias (__clone, clone)
