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

## Related Docs
- `/Users/zhoubot/linx-isa/docs/project/navigation.md`
- `/Users/zhoubot/linx-isa/docs/bringup/libc_status.md`
- `/Users/zhoubot/linx-isa/docs/bringup/`
