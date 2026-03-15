/* Definition of `struct stat' used by the Linx kernel.
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

#ifndef _LINUX_LINX_KERNEL_STAT_H
#define _LINUX_LINX_KERNEL_STAT_H 1

struct kernel_stat
{
  __dev_t st_dev;
  __ino64_t st_ino;
  __mode_t st_mode;
  __nlink_t st_nlink;
  __uid_t st_uid;
  __gid_t st_gid;
  __dev_t st_rdev;
  unsigned long int __pad;
  __off64_t st_size;
  __blksize_t st_blksize;
  int __pad2;
  __blkcnt64_t st_blocks;
  __syscall_slong_t st_atime_sec;
  __syscall_slong_t st_atime_nsec;
  __syscall_slong_t st_mtime_sec;
  __syscall_slong_t st_mtime_nsec;
  __syscall_slong_t st_ctime_sec;
  __syscall_slong_t st_ctime_nsec;
  unsigned int __unused[2];
};

#define STAT_IS_KERNEL_STAT 0
#define XSTAT_IS_XSTAT64 1
#define STATFS_IS_STATFS64 __STATFS_MATCHES_STATFS64

#endif /* _LINUX_LINX_KERNEL_STAT_H  */
