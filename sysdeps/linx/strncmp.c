/* Scalar strncmp implementation.  Linx version.
   Copyright (C) 2026 Free Software Foundation, Inc.
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

#include <string.h>

#undef strncmp

#ifndef STRNCMP
# define STRNCMP strncmp
#endif

int
STRNCMP (const char *s1, const char *s2, size_t n)
{
  const volatile unsigned char *p1 = (const volatile unsigned char *) s1;
  const volatile unsigned char *p2 = (const volatile unsigned char *) s2;

  while (n != 0)
    {
      const unsigned char c1 = *p1++;
      const unsigned char c2 = *p2++;

      if (c1 == '\0' || c1 != c2)
	return c1 - c2;

      --n;
    }

  return 0;
}
libc_hidden_builtin_def (STRNCMP)
