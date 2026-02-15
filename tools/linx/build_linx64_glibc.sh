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

mkdir -p "$BUILD_DIR" "$INSTALL_DIR" "$LOG_DIR"

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

ln -sf "$GMAKE_BIN" "$HOST_SHIM_DIR/gnumake"
ln -sf "$GSED_BIN" "$HOST_SHIM_DIR/sed"

cat > "$TOOL_WRAP_DIR/linx-clang" <<WRAP
#!/usr/bin/env bash
set -eo pipefail
CLANG_BIN="$CLANG"
TARGET="$TARGET"
SYSROOT="$SYSROOT"
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
  extra+=("-fuse-ld=lld")
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
  extra+=("-fuse-ld=lld")
fi
exec "\$CLANGXX_BIN" -target "\$TARGET" --sysroot="\$SYSROOT" "\${extra[@]}" "\$@"
WRAP
chmod +x "$TOOL_WRAP_DIR/linx-clang++"

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

set +e
gnumake -j"$JOBS" $MAKE_TARGETS >"$BUILD_LOG" 2>&1
mk_rc=$?
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
