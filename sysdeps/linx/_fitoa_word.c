/* Guarded integer-to-ASCII conversion.  Linx version.
   Copyright (C) 1994-2026 Free Software Foundation, Inc.
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

#include <_itoa.h>

char *
_itoa_word (_ITOA_WORD_TYPE value, char *buflim,
	    unsigned int base, int upper_case)
{
  const char *digits = (upper_case
			? _itoa_upper_digits
			: _itoa_lower_digits);

  if (base < 2 || base > 36)
    base = 10;

  for (unsigned int guard = 0; guard < sizeof (value) * CHAR_BIT; ++guard)
    {
      _ITOA_WORD_TYPE quotient = value / base;
      _ITOA_WORD_TYPE remainder = value - quotient * base;

      if (remainder >= base)
	remainder = 0;

      *--buflim = digits[remainder];

      if (quotient == 0)
	break;

      if (quotient >= value)
	quotient = 0;

      value = quotient;
    }

  return buflim;
}

char *
_fitoa_word (_ITOA_WORD_TYPE value, char *buf, unsigned int base,
	     int upper_case)
{
  char tmpbuf[sizeof (value) * CHAR_BIT];	      /* Worst case: base 2.  */
  char *cp = _itoa_word (value, tmpbuf + sizeof (tmpbuf), base, upper_case);

  while (cp < tmpbuf + sizeof (tmpbuf))
    *buf++ = *cp++;

  return buf;
}
