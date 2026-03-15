/* Linx program startup for glibc.
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
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

extern int main (int, char **, char **);
extern int __libc_start_main (int (*)(int, char **, char **), int, char **,
			      void (*) (void), void (*) (void),
			      void (*) (void), void *);

/* The first piece of initialized data.  */
int __data_start = 0;
weak_alias (__data_start, data_start)

void _start_c (void (*rtld_fini) (void), long *sp)
  __attribute__ ((noreturn, used, visibility ("hidden")));

__attribute__ ((naked, noreturn, used))
void
_start (void)
{
  __asm__ volatile (
      "C.BSTART.STD\n"
      "c.movr\tsp,\t->a1\n"
      "BSTART\tCALL, _start_c, ra=1f\n"
      "C.BSTOP\n"
      "1:\n"
      "C.BSTART\tDIRECT, 1b\n"
      "C.BSTOP\n");
}

void
_start_c (void (*rtld_fini) (void), long *sp)
{
  int argc = (int) sp[0];
  char **argv = (char **) (sp + 1);

  __libc_start_main (main, argc, argv, 0, 0, rtld_fini, sp);
  __builtin_unreachable ();
}
