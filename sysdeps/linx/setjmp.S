/* setjmp for Linx.
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

/* Bring-up layout must match bits/setjmp.h:
   s0..s8, sp, ra.  Signal-mask saving is intentionally deferred; rtld needs
   a real register snapshot so _dl_catch_exception can recover from errors.  */

#define SAVE_JMPBUF()        \
  C.BSTART.STD;              \
  sdi s0, [a0, 0];           \
  sdi s1, [a0, 8];           \
  sdi s2, [a0, 16];          \
  sdi s3, [a0, 24];          \
  sdi s4, [a0, 32];          \
  sdi s5, [a0, 40];          \
  sdi s6, [a0, 48];          \
  sdi s7, [a0, 56];          \
  sdi s8, [a0, 64];          \
  sdi sp, [a0, 72];          \
  sdi ra, [a0, 80];          \
  addi zero, 0, ->a0;        \
  C.BSTART.STD RET;          \
  c.setc.tgt ra;             \
  C.BSTOP

ENTRY (_setjmp)
  SAVE_JMPBUF ()
END (_setjmp)

ENTRY (setjmp)
  SAVE_JMPBUF ()
END (setjmp)

ENTRY (__sigsetjmp)
  SAVE_JMPBUF ()
END (__sigsetjmp)

hidden_def (__sigsetjmp)
weak_alias (_setjmp, __GI__setjmp)
