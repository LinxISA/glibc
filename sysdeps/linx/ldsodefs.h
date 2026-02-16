/* Run-time dynamic linker data structures for loaded ELF shared objects.
   Linx version.
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

#ifndef _LINX_LDSODEFS_H
#define _LINX_LDSODEFS_H 1

#include <elf.h>

struct La_linx_regs;
struct La_linx_retval;

#define ARCH_PLTENTER_MEMBERS                                              \
    ElfW(Addr) (*linx_gnu_pltenter) (ElfW(Sym) *, unsigned int,            \
                                     uintptr_t *, uintptr_t *,              \
                                     const struct La_linx_regs *,           \
                                     unsigned int *, const char *,          \
                                     long int *);

#define ARCH_PLTEXIT_MEMBERS                                               \
    unsigned int (*linx_gnu_pltexit) (ElfW(Sym) *, unsigned int,           \
                                      uintptr_t *, uintptr_t *,             \
                                      const struct La_linx_regs *,          \
                                      struct La_linx_retval *,              \
                                      const char *);

#include_next <ldsodefs.h>

#endif
