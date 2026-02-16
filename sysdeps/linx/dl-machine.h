/* Machine-dependent ELF dynamic relocation inline functions.  Linx version.
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

#ifndef dl_machine_h
#define dl_machine_h

#define ELF_MACHINE_NAME "LinxISA"

#include <entry.h>
#include <elf/elf.h>
#include <link.h>
#include <string.h>
#include <dl-tls.h>
#include <dl-irel.h>
#include <dl-static-tls.h>
#include <dl-machine-rel.h>

#ifndef EM_LINXISA
# define EM_LINXISA 233
#endif

/* Canonical Linx relocation numbers (keep in sync with LLVM/musl).  */
#ifndef R_LINX_NONE
# define R_LINX_NONE 0
#endif
#ifndef R_LINX_64
# define R_LINX_64 10
#endif
#ifndef R_LINX_32
# define R_LINX_32 11
#endif
#ifndef R_LINX_RELATIVE
# define R_LINX_RELATIVE 9
#endif
#ifndef R_LINX_COPY
# define R_LINX_COPY 14
#endif
#ifndef R_LINX_JUMP_SLOT
# define R_LINX_JUMP_SLOT 12
#endif
#ifndef R_LINX_GLOB_DAT
# define R_LINX_GLOB_DAT 13
#endif
#ifndef R_LINX_TLS_DTPMOD64
# define R_LINX_TLS_DTPMOD64 23
#endif
#ifndef R_LINX_TLS_DTPREL64
# define R_LINX_TLS_DTPREL64 24
#endif
#ifndef R_LINX_TLS_TPREL64
# define R_LINX_TLS_TPREL64 25
#endif
#ifndef R_LINX_TLSDESC
# define R_LINX_TLSDESC 26
#endif
#ifndef R_LINX_IRELATIVE
# define R_LINX_IRELATIVE 27
#endif

#define ELF_MACHINE_JMP_SLOT R_LINX_JUMP_SLOT

#define elf_machine_type_class(type)				\
  ((ELF_RTYPE_CLASS_PLT * ((type) == ELF_MACHINE_JMP_SLOT	\
			   || (type) == R_LINX_TLS_DTPREL64	\
			   || (type) == R_LINX_TLS_DTPMOD64	\
			   || (type) == R_LINX_TLS_TPREL64	\
			   || (type) == R_LINX_TLSDESC))	\
   | (ELF_RTYPE_CLASS_COPY * ((type) == R_LINX_COPY)))

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int __attribute_used__
elf_machine_matches_host (const ElfW(Ehdr) *ehdr)
{
  return ehdr->e_machine == EM_LINXISA;
}

/* Return the run-time load address of the shared object.  */
static inline ElfW(Addr)
elf_machine_load_address (void)
{
  extern const ElfW(Ehdr) __ehdr_start attribute_hidden;
  return (ElfW(Addr)) &__ehdr_start;
}

/* Return the link-time address of _DYNAMIC.  */
static inline ElfW(Addr)
elf_machine_dynamic (void)
{
  extern ElfW(Dyn) _DYNAMIC[] attribute_hidden;
  return (ElfW(Addr)) _DYNAMIC - elf_machine_load_address ();
}

/* Initial entry point code for the dynamic linker.
   Bring-up fallback: provide a C entry shim until Linx rtld start asm
   is finalized.  */
#define RTLD_START							\
static ElfW(Addr) _dl_start (void *arg);					\
void __attribute__ ((used, noreturn)) ENTRY_POINT (void)			\
{									\
  ElfW(Addr) user_entry = _dl_start (__builtin_frame_address (0));	\
  ((void (*) (void)) user_entry) ();					\
  __builtin_unreachable ();						\
}

/* Bias .got.plt entry by the offset requested by the PLT header.  */
#define elf_machine_plt_value(map, reloc, value) (value)

/* Names of architecture-specific auditing callback functions.  */
#define ARCH_LA_PLTENTER linx_gnu_pltenter
#define ARCH_LA_PLTEXIT  linx_gnu_pltexit

static inline ElfW(Addr)
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
		       const ElfW(Sym) *refsym, const ElfW(Sym) *sym,
		       const ElfW(Rela) *reloc,
		       ElfW(Addr) *reloc_addr, ElfW(Addr) value)
{
  return *reloc_addr = value;
}

#endif /* !dl_machine_h */

#ifdef RESOLVE_MAP

static inline void
__attribute__ ((always_inline))
elf_machine_rela_relative (ElfW(Addr) l_addr, const ElfW(Rela) *reloc,
			   void *const reloc_addr)
{
  ElfW(Addr) value = l_addr + reloc->r_addend;
  memcpy (reloc_addr, &value, sizeof (value));
}

/* Perform a relocation described by R_INFO at RELOC_ADDR.  */
static inline void
__attribute__ ((always_inline))
elf_machine_rela (struct link_map *map, struct r_scope_elem *scope[],
		  const ElfW(Rela) *reloc, const ElfW(Sym) *sym,
		  const struct r_found_version *version,
		  void *const reloc_addr, int skip_ifunc)
{
  const unsigned long int r_type = ELFW (R_TYPE) (reloc->r_info);
  ElfW(Addr) *addr_field = (ElfW(Addr) *) reloc_addr;
  const ElfW(Sym) *const __attribute__ ((unused)) refsym = sym;
  struct link_map *sym_map = RESOLVE_MAP (map, scope, &sym, version, r_type);
  ElfW(Addr) value = reloc->r_addend;

  if (sym_map != NULL && sym != NULL)
    value += SYMBOL_ADDRESS (sym_map, sym, true);

  switch (r_type)
    {
    case R_LINX_NONE:
      break;
    case R_LINX_RELATIVE:
      elf_machine_rela_relative (map->l_addr, reloc, addr_field);
      break;
    case R_LINX_JUMP_SLOT:
    case R_LINX_GLOB_DAT:
    case R_LINX_64:
      *addr_field = value;
      break;
#ifndef RTLD_BOOTSTRAP
    case R_LINX_TLS_DTPMOD64:
      if (sym_map)
	*addr_field = sym_map->l_tls_modid;
      break;
    case R_LINX_TLS_DTPREL64:
      if (sym != NULL)
	*addr_field = TLS_DTPREL_VALUE (sym) + reloc->r_addend;
      break;
    case R_LINX_TLS_TPREL64:
      if (sym != NULL)
	{
	  CHECK_STATIC_TLS (map, sym_map);
	  *addr_field = TLS_TPREL_VALUE (sym_map, sym) + reloc->r_addend;
	}
      break;
    case R_LINX_COPY:
      if (__glibc_likely (sym != NULL))
	memcpy (reloc_addr, (void *) value, sym->st_size);
      break;
    case R_LINX_IRELATIVE:
      value = map->l_addr + reloc->r_addend;
      if (__glibc_likely (!skip_ifunc))
	value = elf_ifunc_invoke (value);
      *addr_field = value;
      break;
#endif
    default:
      _dl_reloc_bad_type (map, r_type, 0);
      break;
    }
}

static inline void
__attribute__ ((always_inline))
elf_machine_lazy_rel (struct link_map *map, struct r_scope_elem *scope[],
		      ElfW(Addr) l_addr, const ElfW(Rela) *reloc,
		      int skip_ifunc)
{
  ElfW(Addr) *const reloc_addr = (void *) (l_addr + reloc->r_offset);
  const unsigned int r_type = ELFW (R_TYPE) (reloc->r_info);

  if (__glibc_likely (r_type == R_LINX_JUMP_SLOT))
    *reloc_addr += l_addr;
  else if (__glibc_unlikely (r_type == R_LINX_IRELATIVE))
    {
      ElfW(Addr) value = l_addr + reloc->r_addend;
      if (__glibc_likely (!skip_ifunc))
	value = elf_ifunc_invoke (value);
      *reloc_addr = value;
    }
  else if (__glibc_unlikely (r_type != R_LINX_NONE))
    _dl_reloc_bad_type (map, r_type, 1);
}

/* Bring-up default: keep runtime setup minimal and non-lazy-specific.  */
static inline int
__attribute__ ((always_inline))
elf_machine_runtime_setup (struct link_map *l, struct r_scope_elem *scope[],
			   int lazy, int profile)
{
  (void) l;
  (void) scope;
  (void) profile;
  return lazy;
}

#endif /* RESOLVE_MAP */
