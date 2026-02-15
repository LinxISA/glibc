/* Thread-local storage handling in the ELF dynamic linker.  LinxISA version.
   This file is part of the GNU C Library.
 */

#ifndef _DL_TLS_H
#define _DL_TLS_H

typedef struct
{
  unsigned long int ti_module;
  unsigned long int ti_offset;
} tls_index;

#define TLS_TP_OFFSET 0
#define TLS_DTV_OFFSET 0x800

#define TLS_TPREL_VALUE(sym_map, sym) \
  ((sym_map)->l_tls_offset + (sym)->st_value - TLS_TP_OFFSET)

#define TLS_DTPREL_VALUE(sym) \
  ((sym)->st_value - TLS_DTV_OFFSET)

extern void *__tls_get_addr (tls_index *ti);

#define __TLS_GET_ADDR(__ti) (__tls_get_addr (__ti) - TLS_DTV_OFFSET)

#define TLS_DTV_UNALLOCATED ((void *) -1l)

#endif /* _DL_TLS_H */

