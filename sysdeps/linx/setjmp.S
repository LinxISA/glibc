/* setjmp for Linx.
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

#include <sysdep.h>

ENTRY (_setjmp)
	C.BSTART.STD
	addiw zero, 0, ->a1
	j HIDDEN_JUMPTARGET (__sigsetjmp)
END (_setjmp)
libc_hidden_def (_setjmp)

ENTRY (setjmp)
	C.BSTART.STD
	addiw zero, 1, ->a1
	j HIDDEN_JUMPTARGET (__sigsetjmp)
END (setjmp)

ENTRY (__sigsetjmp)
	C.BSTART.STD
	sdi s0, [a0, 0]
	sdi s1, [a0, 8]
	sdi s2, [a0, 16]
	sdi s3, [a0, 24]
	sdi s4, [a0, 32]
	sdi s5, [a0, 40]
	sdi s6, [a0, 48]
	sdi s7, [a0, 56]
	sdi s8, [a0, 64]
	sdi sp, [a0, 72]
	sdi ra, [a0, 80]
#if IS_IN (rtld)
	c.movr zero, ->a0
	C.BSTART.STD RET
	c.setc.tgt ra
	C.BSTOP
#else
	j __sigjmp_save
#endif
END (__sigsetjmp)
hidden_def (__sigsetjmp)
weak_alias (_setjmp, __GI__setjmp)
