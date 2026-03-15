/* Get file status.  Linux/Linx version.
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

#define __fstatat __redirect___fstatat
#define fstatat __redirect_fstatat
#include <sys/stat.h>
#include <fcntl.h>
#ifndef __ARCH_WANT_NEW_STAT
# define __ARCH_WANT_NEW_STAT 1
#endif
#include <asm/unistd.h>
#include <string.h>
#include <sysdep.h>
#include <time.h>
#include <sys/sysmacros.h>
#include <internal-stat.h>

int
__fstatat64_time64 (int fd, const char *file, struct __stat64_t64 *buf,
                    int flag)
{
  struct kernel_stat kst;
  long int r = INTERNAL_SYSCALL_NCS_CALL (__NR3264_fstatat, fd, file,
                                          &kst, flag);
  if (!INTERNAL_SYSCALL_ERROR_P (r))
    {
    __cp_kstat_stat64_t64 (&kst, buf);
      return 0;
    }

  return INLINE_SYSCALL_ERROR_RETURN_VALUE (-r);
}
#if __TIMESIZE != 64
hidden_def (__fstatat64_time64)

int
__fstatat64 (int fd, const char *file, struct stat64 *buf, int flags)
{
  struct __stat64_t64 st_t64;
  return __fstatat64_time64 (fd, file, &st_t64, flags)
         ?: __cp_stat64_t64_stat64 (&st_t64, buf);
}
#endif

#undef __fstatat
#undef fstatat

hidden_def (__fstatat64)
weak_alias (__fstatat64, fstatat64)

strong_alias (__fstatat64, __fstatat)
weak_alias (__fstatat64, fstatat)
strong_alias (__fstatat64, __GI___fstatat);
