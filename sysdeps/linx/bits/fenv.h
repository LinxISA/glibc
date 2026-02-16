/* Copyright (C) 1997-2026 Free Software Foundation, Inc.
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

#ifndef _FENV_H
# error "Never use <bits/fenv.h> directly; include <fenv.h> instead."
#endif

/* Linx bring-up baseline:
   - No floating-point exception flags are exposed yet.
   - No selectable rounding modes are exposed yet.
   glibc still requires FE_TONEAREST to represent the default mode.  */
#define FE_ALL_EXCEPT 0
#define FE_TONEAREST 0

typedef unsigned int fexcept_t;

typedef struct
{
  fexcept_t __excepts;
}
fenv_t;

#define FE_DFL_ENV ((const fenv_t *) -1l)

#if __GLIBC_USE (IEC_60559_BFP_EXT_C23)
typedef unsigned int femode_t;
# define FE_DFL_MODE ((const femode_t *) -1L)
#endif
