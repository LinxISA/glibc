/* longjmp for Linx.
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

#include <sysdep.h>

/* Restore the layout saved by setjmp.S: s0..s8, sp, ra.  */

ENTRY (__longjmp)
  C.BSTART.STD
  ldi [a0, 0], ->s0
  ldi [a0, 8], ->s1
  ldi [a0, 16], ->s2
  ldi [a0, 24], ->s3
  ldi [a0, 32], ->s4
  ldi [a0, 40], ->s5
  ldi [a0, 48], ->s6
  ldi [a0, 56], ->s7
  ldi [a0, 64], ->s8
  ldi [a0, 72], ->sp
  ldi [a0, 80], ->ra
  c.movr a1, ->a0

  C.BSTART COND, 1f
  setc.nei a1, 0

  C.BSTART.STD
  c.movi 1, ->a0

1:
  C.BSTART.STD RET
  c.setc.tgt ra
  C.BSTOP
END (__longjmp)
