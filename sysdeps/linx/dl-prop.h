/* Support for PTO ISA identity notes.  Linx version.
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
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _DL_PROP_H
#define _DL_PROP_H

#include <libintl.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* PTO ISA 0.58.3 ELF identity descriptor.  Keep this byte-for-byte in sync
   with PTO-ISA/pto-spec spec/release-manifest.json.  The manifest's content
   hash records release provenance and is not part of this wire descriptor.  */
#define LINX_PTO_ISA_IDENTITY_JSON					\
  "{\"encoding_abi\":\"pto-isa-0.58.3-mode-function-v1\","		\
  "\"encoding_projection_sha256\":"					\
  "\"8a48b80e04484c70870f155bf9efc79d2a805cf99e809f4e4e8a7e6a7eb34172\"," \
  "\"release\":\"0.58.3\"}"

#define LINX_PTO_NOTE_SCAN_MAX 4096

extern __typeof (__pread64) __pread64_nocancel;

static inline void
linx_pto_note_reject (struct link_map *l)
{
  l->l_mach.pto_isa_identity_invalid = 1;
  l->l_mach.pto_isa_identity_valid = 0;
}

static inline bool
linx_pto_add_overflow (ElfW(Addr) a, ElfW(Addr) b, ElfW(Addr) *result)
{
  *result = a + b;
  return *result < a;
}

static inline bool
linx_pto_note_range_loaded (struct link_map *l, ElfW(Addr) start,
			    ElfW(Addr) size)
{
  ElfW(Addr) end;

  if (linx_pto_add_overflow (start, size, &end))
    return false;

  for (unsigned int i = 0; i < l->l_phnum; ++i)
    {
      const ElfW(Phdr) *ph = &l->l_phdr[i];
      ElfW(Addr) segment_start;
      ElfW(Addr) segment_end;

      if (ph->p_type != PT_LOAD)
	continue;
      if (linx_pto_add_overflow (l->l_addr, ph->p_vaddr, &segment_start))
	continue;
      if (linx_pto_add_overflow (segment_start, ph->p_memsz, &segment_end))
	continue;
      if (start >= segment_start && end <= segment_end)
	return true;
    }

  return false;
}

static inline void
linx_pto_check_one (struct link_map *l, const char *program)
{
  if (__glibc_unlikely (!l->l_mach.pto_isa_identity_valid
			|| l->l_mach.pto_isa_identity_invalid))
    {
      const char *name = (l->l_name != NULL && *l->l_name != '\0'
			  ? l->l_name
			  : program != NULL ? program : "<main program>");
      if (program != NULL)
	_dl_fatal_printf ("%s: PTO ISA identity note missing or invalid\n",
			  name);
      else
	_dl_signal_error (0, name, "dlopen",
			  N_("PTO ISA identity note missing or invalid"));
    }
}

static inline void
linx_pto_check_searchlist (struct link_map *m, const char *program)
{
  for (unsigned int i = 0; i < m->l_searchlist.r_nlist; ++i)
    {
      struct link_map *l = m->l_searchlist.r_list[i]->l_real;

#ifdef SHARED
      if (is_rtld_link_map (l) || is_rtld_link_map (l->l_real))
	continue;
#endif

      linx_pto_check_one (l, program);
    }
}

static inline void __attribute__ ((always_inline))
_rtld_main_check (struct link_map *m, const char *program)
{
  linx_pto_check_searchlist (m, program);
}

static inline void __attribute__ ((always_inline))
_dl_open_check (struct link_map *m, int dlopen_mode)
{
  (void) dlopen_mode;
  linx_pto_check_searchlist (m, NULL);
}

static inline void __attribute__ ((always_inline))
linx_pto_process_note_bytes (struct link_map *l, const unsigned char *notes,
				     size_t size)
{
  size_t offset = 0;

  while (offset < size)
    {
      if (__glibc_unlikely (size - offset < sizeof (ElfW(Nhdr))))
	{
	  linx_pto_note_reject (l);
	  return;
	}

      const ElfW(Nhdr) *note = (const void *) (notes + offset);
      size_t name_offset = offset + sizeof (*note);
      size_t name_size = ALIGN_UP (note->n_namesz, 4);
      size_t desc_offset;
      size_t desc_size = ALIGN_UP (note->n_descsz, 4);
      size_t next;

      if (__glibc_unlikely (name_size < note->n_namesz
			    || __builtin_add_overflow (name_offset, name_size,
						       &desc_offset)
			    || desc_size < note->n_descsz
			    || __builtin_add_overflow (desc_offset, desc_size,
						       &next)
			    || next <= offset || next > size
			    || desc_offset > size
			    || note->n_descsz > size - desc_offset))
	{
	  linx_pto_note_reject (l);
	  return;
	}

      if (note->n_namesz == 4 && note->n_type == PTO_NT_ISA_IDENTITY
		  && memcmp (notes + name_offset, ELF_NOTE_PTO, 4) == 0)
		{
	  const char *desc = (const char *) notes + desc_offset;
	  const size_t expected_len = sizeof (LINX_PTO_ISA_IDENTITY_JSON) - 1;
	  bool matches = (note->n_descsz == expected_len
			  && memcmp (desc, LINX_PTO_ISA_IDENTITY_JSON,
				     expected_len) == 0);

	  if (__glibc_unlikely (!matches))
	    {
	      linx_pto_note_reject (l);
	      return;
	    }

	  l->l_mach.pto_isa_identity_valid = 1;
	}

      offset = next;
    }
}

static inline void __attribute__ ((always_inline))
_dl_process_pt_note (struct link_map *l, int fd, const ElfW(Phdr) *ph)
{
  /* Other ELF note owners may use a different segment alignment.  Ignore
     those segments here: a PTO identity is accepted only from a four-byte
     aligned segment, and the final searchlist check rejects a missing one.  */
  if (ph->p_align != 4)
    return;

  if (fd != -1)
    {
      unsigned char buf[LINX_PTO_NOTE_SCAN_MAX]
	__attribute__ ((aligned (__alignof__ (ElfW(Nhdr)))));

      if (__glibc_unlikely (ph->p_filesz > sizeof (buf)
			    || ph->p_offset > INT64_MAX))
	{
	  linx_pto_note_reject (l);
	  return;
	}

      size_t size = ph->p_filesz;
      if (size == 0)
	return;

      if (__glibc_unlikely (__pread64_nocancel (fd, buf, size, ph->p_offset)
			    != (ssize_t) size))
	{
	  linx_pto_note_reject (l);
	  return;
	}

	  linx_pto_process_note_bytes (l, buf, size);
      return;
    }

  if (__glibc_unlikely (ph->p_filesz > LINX_PTO_NOTE_SCAN_MAX))
    {
      linx_pto_note_reject (l);
      return;
    }

  ElfW(Addr) start;
  if (__glibc_unlikely (linx_pto_add_overflow (l->l_addr, ph->p_vaddr,
				       &start)
				|| !linx_pto_note_range_loaded (l, start,
						       ph->p_filesz)))
    {
      linx_pto_note_reject (l);
      return;
    }

  linx_pto_process_note_bytes (l, (const void *) start, ph->p_filesz);
}

/* Called for each property in the NT_GNU_PROPERTY_TYPE_0 note of L,
   processing of the properties continues until this returns 0.  */
static inline int __attribute__ ((always_inline))
_dl_process_gnu_property (struct link_map *l, int fd, uint32_t type,
			  uint32_t datasz, void *data)
{
  /* Continue until GNU_PROPERTY_1_NEEDED is found.  */
  if (type == GNU_PROPERTY_1_NEEDED)
    {
      if (datasz == 4)
	l->l_1_needed = *(unsigned int *) data;
      return 0;
    }
  return 1;
}

#endif /* _DL_PROP_H */
