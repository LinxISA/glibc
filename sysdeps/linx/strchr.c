/* Scalar strchr implementation.  Linx version.
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

#undef strchr
#undef index

#ifdef STRCHR
# define strchr STRCHR
#endif

char *
strchr (const char *s, int c_in)
{
  const volatile unsigned char *p = (const volatile unsigned char *) s;
  const unsigned char c = (unsigned char) c_in;

  while (*p != '\0')
    {
      if (*p == c)
	return (char *) p;
      ++p;
    }

  return c == '\0' ? (char *) p : NULL;
}

#ifndef STRCHR
weak_alias (strchr, index)
libc_hidden_builtin_def (strchr)
#endif
