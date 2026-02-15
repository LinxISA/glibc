/* Architecture-specific procfs register set types for LinxISA (bring-up).
   This file is part of the GNU C Library.
 */

#ifndef _LINX_BITS_PROCFS_H
#define _LINX_BITS_PROCFS_H 1

#include <stdint.h>

/* General-purpose register type. */
typedef unsigned long int elf_greg_t;

/* General-purpose register set.
   LinxISA currently models 24 architectural GPRs (R0-R23) in the bring-up ABI.
   Keep extra slots for future expansion and for debugger friendliness.  */
typedef elf_greg_t elf_gregset_t[32];

/* Floating-point register set (placeholder for bring-up).  */
typedef struct
{
  uint64_t fpregs[32];
} elf_fpregset_t;

#endif /* _LINX_BITS_PROCFS_H */

