#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GLIBC_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
REPO_ROOT="$(cd "$GLIBC_ROOT/../.." && pwd)"

OUT_ROOT="${OUT_ROOT:-$REPO_ROOT/out/libc/glibc}"
BUILD_DIR="${BUILD_DIR:-$OUT_ROOT/build}"
LOG_DIR="${LOG_DIR:-$OUT_ROOT/logs}"
SUMMARY="${SUMMARY:-$LOG_DIR/g1b-summary.txt}"
MAKE_TARGETS="${MAKE_TARGETS:-csu/subdir_lib lib}"
ALLOW_BLOCKED="${GLIBC_G1B_ALLOW_BLOCKED:-0}"
READELF_BIN="${READELF_BIN:-/opt/homebrew/opt/binutils/bin/readelf}"

BASE_SCRIPT="$SCRIPT_DIR/build_linx64_glibc.sh"
BASE_SUMMARY="${GLIBC_G1A_SUMMARY:-$LOG_DIR/summary.txt}"
BASE_LOG="${GLIBC_G1A_LOG:-$LOG_DIR/03-make.log}"
G1B_BUILD_LOG="${G1B_BUILD_LOG:-$LOG_DIR/g1b-build.log}"
G1B_READELF_LOG="${G1B_READELF_LOG:-$LOG_DIR/g1b-readelf.log}"

mkdir -p "$LOG_DIR"
: > "$SUMMARY"

echo "[G1b] source: $GLIBC_ROOT" | tee -a "$SUMMARY"
echo "[G1b] build: $BUILD_DIR" | tee -a "$SUMMARY"
echo "[G1b] make targets: $MAKE_TARGETS" | tee -a "$SUMMARY"

if [[ ! -x "$BASE_SCRIPT" ]]; then
  echo "[G1b] status: blocked" | tee -a "$SUMMARY"
  echo "[G1b] classification: missing_base_script" | tee -a "$SUMMARY"
  echo "[G1b] detail: $BASE_SCRIPT" | tee -a "$SUMMARY"
  exit 2
fi

set +e
MAKE_TARGETS="$MAKE_TARGETS" \
SUMMARY="$BASE_SUMMARY" \
BUILD_DIR="$BUILD_DIR" \
LOG_DIR="$LOG_DIR" \
  bash "$BASE_SCRIPT" >"$G1B_BUILD_LOG" 2>&1
rc=$?
set -e

if [[ "$rc" -ne 0 ]]; then
  classification="build_failed"
  if [[ -f "$BASE_LOG" ]]; then
    if grep -Eiq "Cannot select: .*bswap|Linx DAG->DAG Pattern Instruction Selection" "$BASE_LOG"; then
      classification="llvm_isel_bswap_vector"
    elif grep -Eiq "redefinition of 'user_regs_struct'" "$BASE_LOG"; then
      classification="uapi_ptrace_redefinition"
    elif grep -Eiq "stamp\\.os: No such file|stamp\\.osT.*No such file" "$BASE_LOG"; then
      classification="parallel_stamp_race"
    fi
  fi

  echo "[G1b] status: blocked" | tee -a "$SUMMARY"
  echo "[G1b] classification: $classification" | tee -a "$SUMMARY"
  echo "[G1b] base_make_log: $BASE_LOG" | tee -a "$SUMMARY"
  echo "[G1b] wrapper_log: $G1B_BUILD_LOG" | tee -a "$SUMMARY"
  echo "[G1b] base_exit_code: $rc" | tee -a "$SUMMARY"

  if [[ "$ALLOW_BLOCKED" == "1" ]]; then
    echo "[G1b] policy: allow_blocked=1 (returning success with blocked status)" | tee -a "$SUMMARY"
    exit 0
  fi
  exit "$rc"
fi

artifact=""
for cand in \
  "$BUILD_DIR/linkobj/libc.so" \
  "$BUILD_DIR/libc.so" \
  "$BUILD_DIR/libc.so.6"; do
  if [[ -f "$cand" ]]; then
    artifact="$cand"
    break
  fi
done

if [[ -z "$artifact" ]]; then
  echo "[G1b] status: blocked" | tee -a "$SUMMARY"
  echo "[G1b] classification: artifact_missing" | tee -a "$SUMMARY"
  echo "[G1b] detail: expected libc.so artifact not found under $BUILD_DIR" | tee -a "$SUMMARY"
  if [[ "$ALLOW_BLOCKED" == "1" ]]; then
    echo "[G1b] policy: allow_blocked=1 (returning success with blocked status)" | tee -a "$SUMMARY"
    exit 0
  fi
  exit 3
fi

if [[ ! -x "$READELF_BIN" ]]; then
  echo "[G1b] status: blocked" | tee -a "$SUMMARY"
  echo "[G1b] classification: missing_readelf" | tee -a "$SUMMARY"
  echo "[G1b] detail: $READELF_BIN" | tee -a "$SUMMARY"
  if [[ "$ALLOW_BLOCKED" == "1" ]]; then
    echo "[G1b] policy: allow_blocked=1 (returning success with blocked status)" | tee -a "$SUMMARY"
    exit 0
  fi
  exit 4
fi

if ! "$READELF_BIN" -h "$artifact" >"$G1B_READELF_LOG" 2>&1; then
  echo "[G1b] status: blocked" | tee -a "$SUMMARY"
  echo "[G1b] classification: artifact_not_elf" | tee -a "$SUMMARY"
  echo "[G1b] artifact: $artifact" | tee -a "$SUMMARY"
  echo "[G1b] readelf_log: $G1B_READELF_LOG" | tee -a "$SUMMARY"
  if [[ "$ALLOW_BLOCKED" == "1" ]]; then
    echo "[G1b] policy: allow_blocked=1 (returning success with blocked status)" | tee -a "$SUMMARY"
    exit 0
  fi
  exit 5
fi

echo "[G1b] status: pass" | tee -a "$SUMMARY"
echo "[G1b] classification: shared_libc_so_built" | tee -a "$SUMMARY"
echo "[G1b] artifact: $artifact" | tee -a "$SUMMARY"
echo "[G1b] readelf_log: $G1B_READELF_LOG" | tee -a "$SUMMARY"
echo "ok: $SUMMARY"
