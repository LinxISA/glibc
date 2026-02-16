/* Linx PLT runtime trampoline stubs.
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
   <https://www.gnu.org/licenses/>.

   Bring-up note:
   This provides placeholder resolver/profile symbols so rtld links while
   Linx lazy-binding trampolines are under development.  */

	.text
	.align 2

	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, @function
_dl_runtime_resolve:
	.long 0
	.size _dl_runtime_resolve, . - _dl_runtime_resolve

	.globl _dl_runtime_profile
	.type _dl_runtime_profile, @function
_dl_runtime_profile:
	.long 0
	.size _dl_runtime_profile, . - _dl_runtime_profile
