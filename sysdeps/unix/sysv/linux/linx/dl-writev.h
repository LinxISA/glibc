/* Message-writing for the dynamic linker.  Linx Linux version.
   Copyright (C) 2013-2026 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <sys/uio.h>
#include <sysdep.h>

/* Linx bring-up keeps rtld diagnostics on plain write(2) until writev(2)
   is validated across the kernel and QEMU path.  */
static inline void
_dl_writev (int fd, const struct iovec *iov, size_t niov)
{
  for (size_t i = 0; i < niov; ++i)
    if (iov[i].iov_len != 0)
      INTERNAL_SYSCALL_CALL (write, fd, iov[i].iov_base, iov[i].iov_len);
}
