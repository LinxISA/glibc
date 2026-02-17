#+cflags is used by glibc's build rules for all C compilations.
# Disable auto-vectorization until Linx LLVM vector ISel reaches parity.
+cflags += -fno-vectorize -fno-slp-vectorize

ifeq ($(subdir),misc)
sysdep_routines += \
  linx-libc-stubs \
  # sysdep_routines
endif

ifeq ($(subdir),elf)
sysdep-rtld-routines += \
  linx-rtld-stubs \
  linx-rtld-sync \
  # sysdep-rtld-routines
endif
