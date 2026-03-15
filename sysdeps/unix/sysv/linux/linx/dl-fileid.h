/* File identity for the dynamic linker.  Linux/Linx version.
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

#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <sysdep.h>
#include <arch-syscall.h>
#include <kernel_stat.h>

/* Linx currently reuses the generic syscall table, but asm-generic only
   exposes the new-stat numbers when __ARCH_WANT_NEW_STAT is defined.  The
   dynamic linker's file-id helper is built in contexts that do not set that
   flag, so provide the known generic fallback locally.  */
#ifndef __NR_fstat
# define __NR_fstat 80
#endif

struct r_file_id
  {
    dev_t dev;
    ino64_t ino;
  };

static inline bool
_dl_get_file_id (int fd, struct r_file_id *id)
{
  struct kernel_stat st;
  long int r = INTERNAL_SYSCALL_NCS_CALL (__NR_fstat, fd, &st);

  if (INTERNAL_SYSCALL_ERROR_P (r))
    {
      __set_errno (-r);
      return false;
    }

  id->dev = st.st_dev;
  id->ino = st.st_ino;
  return true;
}

static inline bool
_dl_file_id_match_p (const struct r_file_id *a, const struct r_file_id *b)
{
  return a->dev == b->dev && a->ino == b->ino;
}
