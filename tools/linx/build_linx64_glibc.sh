#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GLIBC_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
REPO_ROOT_DEFAULT="$(cd "$GLIBC_ROOT/../.." && pwd)"

TARGET="${TARGET:-linx64-unknown-linux-gnu}"
REPO_ROOT="${REPO_ROOT:-$REPO_ROOT_DEFAULT}"
OUT_ROOT="${OUT_ROOT:-$REPO_ROOT/out/libc/glibc}"
BUILD_DIR="${BUILD_DIR:-$OUT_ROOT/build}"
INSTALL_DIR="${INSTALL_DIR:-$OUT_ROOT/install}"
LOG_DIR="${LOG_DIR:-$OUT_ROOT/logs}"
SUMMARY="${SUMMARY:-$LOG_DIR/summary.txt}"
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 8)}"
FALLBACK_LIB_DIR="${FALLBACK_LIB_DIR:-$OUT_ROOT/fallback-libs}"
FALLBACK_LIBGCC="${FALLBACK_LIBGCC:-$FALLBACK_LIB_DIR/libgcc.a}"
FALLBACK_LIBGCC_EH="${FALLBACK_LIBGCC_EH:-$FALLBACK_LIB_DIR/libgcc_eh.a}"
FALLBACK_CRTBEGINS="${FALLBACK_CRTBEGINS:-$FALLBACK_LIB_DIR/crtbeginS.o}"
FALLBACK_CRTENDS="${FALLBACK_CRTENDS:-$FALLBACK_LIB_DIR/crtendS.o}"
FALLBACK_SRC_ROOT="${FALLBACK_SRC_ROOT:-$REPO_ROOT/avs/runtime/freestanding}"
FALLBACK_SOFTFP_SRC="${FALLBACK_SOFTFP_SRC:-$FALLBACK_SRC_ROOT/src/softfp/softfp.c}"
FALLBACK_ATOMIC_SRC="${FALLBACK_ATOMIC_SRC:-$FALLBACK_SRC_ROOT/src/atomic/atomic_builtins.c}"
FALLBACK_INC_DIR="${FALLBACK_INC_DIR:-$FALLBACK_SRC_ROOT/include}"

SYSROOT="${SYSROOT:-/Users/zhoubot/sysroots/linx64-linux-gnu}"
LINUX_HEADERS="${LINUX_HEADERS:-$OUT_ROOT/linux-headers/include}"

TOOLCHAIN_BIN="${TOOLCHAIN_BIN:-/Users/zhoubot/llvm-project/build-linxisa-clang/bin}"
CLANG="${CLANG:-$TOOLCHAIN_BIN/clang}"
CLANGXX="${CLANGXX:-$TOOLCHAIN_BIN/clang++}"
LD_BIN="${LD_BIN:-$TOOLCHAIN_BIN/ld.lld}"
AR_BIN="${AR_BIN:-$TOOLCHAIN_BIN/llvm-ar}"
RANLIB_BIN="${RANLIB_BIN:-$TOOLCHAIN_BIN/llvm-ranlib}"
NM_BIN="${NM_BIN:-$TOOLCHAIN_BIN/llvm-nm}"
OBJCOPY_BIN="${OBJCOPY_BIN:-$TOOLCHAIN_BIN/llvm-objcopy}"
OBJDUMP_BIN="${OBJDUMP_BIN:-$TOOLCHAIN_BIN/llvm-objdump}"
STRIP_BIN="${STRIP_BIN:-$TOOLCHAIN_BIN/llvm-strip}"
READELF_BIN="${READELF_BIN:-/opt/homebrew/opt/binutils/bin/readelf}"

GMAKE_BIN="${GMAKE_BIN:-/opt/homebrew/bin/gmake}"
GSED_BIN="${GSED_BIN:-/opt/homebrew/bin/gsed}"
BISON_BIN="${BISON_BIN:-/opt/homebrew/opt/bison/bin/bison}"

CFLAGS_USER="${CFLAGS_USER:--O2 -g}"
CPPFLAGS_USER="${CPPFLAGS_USER:--U_FORTIFY_SOURCE}"
MAKE_TARGETS="${MAKE_TARGETS:-csu/subdir_lib}"

CONFIG_LOG="${CONFIG_LOG:-$LOG_DIR/02-configure.log}"
BUILD_LOG="${BUILD_LOG:-$LOG_DIR/03-make.log}"

mkdir -p "$BUILD_DIR" "$INSTALL_DIR" "$LOG_DIR" "$FALLBACK_LIB_DIR"

if [[ ! -x "$CLANG" ]]; then
  echo "error: clang not found: $CLANG" >&2
  exit 1
fi
if [[ ! -x "$CLANGXX" ]]; then
  echo "error: clang++ not found: $CLANGXX" >&2
  exit 1
fi
if [[ ! -x "$LD_BIN" ]]; then
  echo "error: ld.lld not found: $LD_BIN" >&2
  exit 1
fi
if [[ ! -x "$GMAKE_BIN" ]]; then
  echo "error: gmake not found: $GMAKE_BIN" >&2
  exit 1
fi
if [[ ! -x "$GSED_BIN" ]]; then
  echo "error: gsed not found: $GSED_BIN" >&2
  exit 1
fi
if [[ ! -x "$BISON_BIN" ]]; then
  echo "error: bison not found: $BISON_BIN" >&2
  exit 1
fi
if [[ ! -x "$READELF_BIN" ]]; then
  echo "error: readelf not found: $READELF_BIN" >&2
  exit 1
fi
if [[ ! -d "$LINUX_HEADERS" ]]; then
  echo "error: Linux headers not found: $LINUX_HEADERS" >&2
  exit 1
fi

TOOL_WRAP_DIR="$BUILD_DIR/.toolwrap"
HOST_SHIM_DIR="$BUILD_DIR/.host-tools"
mkdir -p "$TOOL_WRAP_DIR" "$HOST_SHIM_DIR"

# Provide a deterministic fallback libgcc archive for toolchains that only
# ship Clang/LLD binaries without a GCC runtime layout.
if [[ ! -f "$FALLBACK_LIBGCC" ]]; then
  if [[ -f "$FALLBACK_SOFTFP_SRC" && -f "$FALLBACK_ATOMIC_SRC" && -d "$FALLBACK_INC_DIR" ]]; then
    SOFTFP_OBJ="$FALLBACK_LIB_DIR/softfp.o"
    ATOMIC_OBJ="$FALLBACK_LIB_DIR/atomic_builtins.o"

    "$CLANG" -target "$TARGET" --sysroot="$SYSROOT" \
      -O2 -fPIC -ffreestanding -fno-stack-protector -fno-builtin \
      -I"$FALLBACK_INC_DIR" \
      -c "$FALLBACK_SOFTFP_SRC" -o "$SOFTFP_OBJ"
    "$CLANG" -target "$TARGET" --sysroot="$SYSROOT" \
      -O2 -fPIC -ffreestanding -fno-stack-protector -fno-builtin \
      -I"$FALLBACK_INC_DIR" \
      -c "$FALLBACK_ATOMIC_SRC" -o "$ATOMIC_OBJ"
    "$AR_BIN" cr "$FALLBACK_LIBGCC" "$SOFTFP_OBJ" "$ATOMIC_OBJ"
    "$RANLIB_BIN" "$FALLBACK_LIBGCC"
  else
    "$AR_BIN" cr "$FALLBACK_LIBGCC"
    "$RANLIB_BIN" "$FALLBACK_LIBGCC"
  fi
fi

if [[ ! -f "$FALLBACK_LIBGCC_EH" ]]; then
  cp -f "$FALLBACK_LIBGCC" "$FALLBACK_LIBGCC_EH"
  "$RANLIB_BIN" "$FALLBACK_LIBGCC_EH"
fi

if [[ ! -f "$FALLBACK_CRTBEGINS" ]]; then
  cat > "$FALLBACK_LIB_DIR/crtbeginS.c" <<'EOF'
void *__dso_handle = &__dso_handle;
EOF
  "$CLANG" -target "$TARGET" --sysroot="$SYSROOT" \
    -O2 -fPIC -ffreestanding -fno-stack-protector -fno-builtin \
    -c "$FALLBACK_LIB_DIR/crtbeginS.c" -o "$FALLBACK_CRTBEGINS"
fi

if [[ ! -f "$FALLBACK_CRTENDS" ]]; then
  cat > "$FALLBACK_LIB_DIR/crtendS.c" <<'EOF'
char __linx_crtend_anchor;
EOF
  "$CLANG" -target "$TARGET" --sysroot="$SYSROOT" \
    -O2 -fPIC -ffreestanding -fno-stack-protector -fno-builtin \
    -c "$FALLBACK_LIB_DIR/crtendS.c" -o "$FALLBACK_CRTENDS"
fi

ln -sf "$GMAKE_BIN" "$HOST_SHIM_DIR/gnumake"
ln -sf "$GSED_BIN" "$HOST_SHIM_DIR/sed"

cat > "$TOOL_WRAP_DIR/linx-clang" <<WRAP
#!/usr/bin/env bash
set -eo pipefail
CLANG_BIN="$CLANG"
TARGET="$TARGET"
SYSROOT="$SYSROOT"
FALLBACK_LIB_DIR="$FALLBACK_LIB_DIR"
link=1
for a in "\$@"; do
  case "\$a" in
    -c|-S|-E|-M|-MM|-MMD|-MG|-MP|-fsyntax-only|-###|-v|-V|--version|-qversion|-version|--help)
      link=0
      ;;
  esac
done
extra=()
if [[ \$link -eq 1 ]]; then
  extra+=("-fuse-ld=lld" "-L\$FALLBACK_LIB_DIR" "-B\$FALLBACK_LIB_DIR")
fi
exec "\$CLANG_BIN" -target "\$TARGET" --sysroot="\$SYSROOT" "\${extra[@]}" "\$@"
WRAP
chmod +x "$TOOL_WRAP_DIR/linx-clang"

cat > "$TOOL_WRAP_DIR/linx-clang++" <<WRAP
#!/usr/bin/env bash
set -eo pipefail
CLANGXX_BIN="$CLANGXX"
TARGET="$TARGET"
SYSROOT="$SYSROOT"
FALLBACK_LIB_DIR="$FALLBACK_LIB_DIR"
link=1
for a in "\$@"; do
  case "\$a" in
    -c|-S|-E|-M|-MM|-MMD|-MG|-MP|-fsyntax-only|-###|-v|-V|--version|-qversion|-version|--help)
      link=0
      ;;
  esac
done
extra=()
if [[ \$link -eq 1 ]]; then
  extra+=("-fuse-ld=lld" "-L\$FALLBACK_LIB_DIR" "-B\$FALLBACK_LIB_DIR")
fi
exec "\$CLANGXX_BIN" -target "\$TARGET" --sysroot="\$SYSROOT" "\${extra[@]}" "\$@"
WRAP
chmod +x "$TOOL_WRAP_DIR/linx-clang++"

repair_degenerate_stamp_os() {
  local subdir="$1"
  local stamp_o="$BUILD_DIR/$subdir/stamp.o"
  local stamp_os="$BUILD_DIR/$subdir/stamp.os"
  local repaired=()
  local entry base

  [[ -f "$stamp_o" && -f "$stamp_os" ]] || return 0

  # Some Linx bring-up builds generate stamp.os with only .oS entries.
  # Reconstruct the PIC object list from stamp.o to keep rtld/libc_pic complete.
  if tr ' ' '\n' < "$stamp_os" | grep -q '\.os$'; then
    return 0
  fi

  while IFS= read -r entry; do
    [[ -n "$entry" ]] || continue
    if [[ "$entry" == *.o ]]; then
      base="${entry%.o}"
    elif [[ "$entry" == *.oS ]]; then
      base="${entry%.oS}"
    else
      continue
    fi

    if [[ -f "$BUILD_DIR/$base.os" ]]; then
      repaired+=("$base.os")
    elif [[ -f "$BUILD_DIR/$base.oS" ]]; then
      repaired+=("$base.oS")
    fi
  done < <(tr ' ' '\n' < "$stamp_o")

  if [[ ${#repaired[@]} -eq 0 ]]; then
    return 0
  fi

  printf '%s ' "${repaired[@]}" > "$stamp_os"
  printf '\n' >> "$stamp_os"
  echo "[G1] repaired $subdir/stamp.os from stamp.o (${#repaired[@]} entries)" >> "$BUILD_LOG"
}

run_make_with_stamp_retry() {
  local label="$1"
  local safe_label
  shift
  local tmp_log
  local rc
  safe_label="${label//\//_}"
  safe_label="${safe_label//[^A-Za-z0-9_.-]/_}"
  tmp_log="$(mktemp "${BUILD_DIR}/.gmake-${safe_label}.XXXXXX.log")"

  gnumake -j"$JOBS" "$@" >"$tmp_log" 2>&1
  rc=$?
  cat "$tmp_log" >>"$BUILD_LOG"

  if [[ $rc -ne 0 ]] && grep -Eiq "stamp\\.os: No such file|stamp\\.osT.*No such file" "$tmp_log"; then
    echo "[G1] detected stamp race for $label; retrying serial (-j1)" >>"$BUILD_LOG"
    gnumake -j1 "$@" >"$tmp_log" 2>&1
    rc=$?
    cat "$tmp_log" >>"$BUILD_LOG"
  fi

  rm -f "$tmp_log"
  return $rc
}

echo "[glibc] target: $TARGET" | tee "$SUMMARY"
echo "[glibc] source: $GLIBC_ROOT" | tee -a "$SUMMARY"
echo "[glibc] build: $BUILD_DIR" | tee -a "$SUMMARY"
echo "[glibc] install: $INSTALL_DIR" | tee -a "$SUMMARY"
echo "[glibc] sysroot: $SYSROOT" | tee -a "$SUMMARY"
echo "[glibc] linux headers: $LINUX_HEADERS" | tee -a "$SUMMARY"
echo "[glibc] jobs: $JOBS" | tee -a "$SUMMARY"

export PATH="$HOST_SHIM_DIR:$(dirname "$BISON_BIN"):$(dirname "$GMAKE_BIN"):$(dirname "$GSED_BIN"):/opt/homebrew/opt/binutils/bin:$PATH"

rm -f "$CONFIG_LOG" "$BUILD_LOG"

pushd "$BUILD_DIR" >/dev/null

set +e
"$GLIBC_ROOT/configure" \
  --host="$TARGET" \
  --build="$(/usr/bin/uname -m)-apple-darwin$(/usr/bin/uname -r)" \
  --prefix=/usr \
  --with-headers="$LINUX_HEADERS" \
  --disable-werror \
  --disable-nscd \
  --disable-timezone-tools \
  CC="$TOOL_WRAP_DIR/linx-clang" \
  CXX="$TOOL_WRAP_DIR/linx-clang++" \
  LD="$LD_BIN" \
  AR="$AR_BIN" \
  RANLIB="$RANLIB_BIN" \
  NM="$NM_BIN" \
  OBJCOPY="$OBJCOPY_BIN" \
  OBJDUMP="$OBJDUMP_BIN" \
  STRIP="$STRIP_BIN" \
  READELF="$READELF_BIN" \
  CFLAGS="$CFLAGS_USER" \
  CPPFLAGS="$CPPFLAGS_USER" \
  >"$CONFIG_LOG" 2>&1
cfg_rc=$?
set -e

if [[ $cfg_rc -ne 0 ]]; then
  echo "glibc gate G1 blocked during configure" | tee -a "$SUMMARY"
  echo "Command exit code: $cfg_rc" | tee -a "$SUMMARY"
  echo "See: $CONFIG_LOG" | tee -a "$SUMMARY"
  popd >/dev/null
  exit $cfg_rc
fi

echo "[G1] configure passed" | tee -a "$SUMMARY"

# Avoid stale archive members from prior failed/incremental runs.
rm -f \
  "$BUILD_DIR/libc_pic.a" \
  "$BUILD_DIR/libc_nonshared.a" \
  "$BUILD_DIR/elf/rtld-libc.a" \
  "$BUILD_DIR/elf/librtld.os"

set +e
mk_rc=0
	for target in $MAKE_TARGETS; do
	  if [[ "$target" == "lib" ]]; then
	    echo "[G1] gnumake prewarm start: nptl/subdir_lib stdlib/subdir_lib debug/subdir_lib" >>"$BUILD_LOG"
	    run_make_with_stamp_retry "prewarm_lib" nptl/subdir_lib stdlib/subdir_lib debug/subdir_lib
	    rc=$?
	    echo "[G1] gnumake prewarm done: rc=$rc" >>"$BUILD_LOG"
	    if [[ $rc -ne 0 ]]; then
	      mk_rc=$rc
      break
    fi
    repair_degenerate_stamp_os "nptl"
    repair_degenerate_stamp_os "stdlib"
    repair_degenerate_stamp_os "debug"
  fi

	  echo "[G1] gnumake target start: $target" >>"$BUILD_LOG"
	  run_make_with_stamp_retry "$target" "$target"
	  rc=$?
	  echo "[G1] gnumake target done: $target rc=$rc" >>"$BUILD_LOG"
  if [[ $rc -ne 0 ]]; then
    mk_rc=$rc
    break
  fi
done
set -e

if [[ $mk_rc -ne 0 ]]; then
  echo "glibc gate G1 blocked during make targets: $MAKE_TARGETS" | tee -a "$SUMMARY"
  echo "Command exit code: $mk_rc" | tee -a "$SUMMARY"
  echo "See: $BUILD_LOG" | tee -a "$SUMMARY"
  popd >/dev/null
  exit $mk_rc
fi

echo "[G1] make targets passed: $MAKE_TARGETS" | tee -a "$SUMMARY"

crt1_path="$BUILD_DIR/csu/crt1.o"
if [[ -f "$crt1_path" ]]; then
  echo "[G1] artifact: $crt1_path" | tee -a "$SUMMARY"
else
  echo "[G1] artifact missing: $crt1_path" | tee -a "$SUMMARY"
fi

popd >/dev/null

echo "ok: $SUMMARY"
