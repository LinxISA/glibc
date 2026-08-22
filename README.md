# LinxISA glibc

## Scope
`lib/glibc` is the GNU libc fork for LinxISA Linux userspace and dynamic runtime bring-up.

## Upstream
- Repository: `https://github.com/LinxISA/glibc`
- Merge-back target branch: `master`

## What This Submodule Owns
- LinxISA glibc port and related build glue
- Shared libc bring-up path (`G1b`) used by integration gates

## Canonical Build and Test Commands
Run from `/Users/zhoubot/linx-isa`.

```bash
bash /Users/zhoubot/linx-isa/lib/glibc/tools/linx/build_linx64_glibc.sh
bash /Users/zhoubot/linx-isa/lib/glibc/tools/linx/build_linx64_glibc_g1b.sh
```

## LinxISA Integration Touchpoints
- Runtime convergence lane in `tools/bringup/run_runtime_convergence.sh`
- Strict integration gate in `tools/regression/strict_cross_repo.sh`
- Sysroot/runtime interoperability with LLVM and Linux bring-up

## PTO ISA Runtime Identity

The Linx dynamic loader accepts only the published PTO ISA v0.58.3 ELF note
identity on the main executable, every `DT_NEEDED` object, and every `dlopen`
object. The wire descriptor locks release `0.58.3`, encoding ABI
`pto-isa-0.58.3-mode-function-v1`, and projection SHA-256
`8a48b80e04484c70870f155bf9efc79d2a805cf99e809f4e4e8a7e6a7eb34172`.
The release provenance fixture also locks content SHA-256
`f299fe3d256c5d071e57bb4aaa2be2de2e4a386ae090048df1f73ae92d392678`;
the content hash is not an additional field in `.note.pto.isa`.

Missing, v0.58.1, malformed, conflicting, oversized, and trailing-NUL notes
fail closed before constructors or application code run. Duplicate identical
v0.58.3 notes remain valid.

## Related Docs
- `/Users/zhoubot/linx-isa/docs/project/navigation.md`
- `/Users/zhoubot/linx-isa/docs/bringup/libc_status.md`
- `/Users/zhoubot/linx-isa/docs/bringup/`
