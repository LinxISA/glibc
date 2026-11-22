#!/usr/bin/env python3
"""Check Linx glibc PTO ISA identity wiring.

This is a source-level guard for the dynamic loader contract that is hard to
exercise in a normal host build before the Linx runtime image exists.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import re
import struct
import sys


EXPECTED_DESCRIPTOR = (
    '{"encoding_abi":"pto-isa-0.59.1-mode-function-v1",'
    '"encoding_projection_sha256":'
    '"d90c881754d66f2cd26f6014d0b582cba88307e70123583d87fc59a918229c28",'
    '"release":"0.59.1"}'
)

V0581_DESCRIPTOR = (
    '{"encoding_abi":"pto-isa-0.58.1-mode-function-v1",'
    '"encoding_projection_sha256":'
    '"89b872d6eaf0252200bc9349d49b9346e2a69d894cdcc2dcd0fd71911c1e0b8c",'
    '"release":"0.58.1"}'
)

V0583_DESCRIPTOR = (
    '{"encoding_abi":"pto-isa-0.58.3-mode-function-v1",'
    '"encoding_projection_sha256":'
    '"8a48b80e04484c70870f155bf9efc79d2a805cf99e809f4e4e8a7e6a7eb34172",'
    '"release":"0.58.3"}'
)

EXPECTED_AUTHORITY = {
    "release": "0.59.1",
    "encoding_abi": "pto-isa-0.59.1-mode-function-v1",
    "encoding_projection_sha256":
        "d90c881754d66f2cd26f6014d0b582cba88307e70123583d87fc59a918229c28",
    "content_sha256":
        "11d63af3718a14b5ace927efda3dd0d3b7a8af7251a6dc3d4cfbfa673e9ecd60",
    "source_commit": "a89a53d48f5708328be609fe2e6296ac8bdb4bc2",
    "source_tree": "13807a8af41c4481ebcc88bbf33357cda8041f5d",
}

EXPECTED_RELOCS = {
    "R_LINX_TLS_DTPMOD64": 28,
    "R_LINX_TLS_DTPREL64": 29,
    "R_LINX_TLS_TPREL64": 30,
    "R_LINX_TLSDESC": 31,
    "R_LINX_IRELATIVE": 32,
}

NOTE_ALIGN = 4
NOTE_TYPE = 1
NOTE_NAME = b"PTO\0"
NOTE_SCAN_MAX = 4096


def align_up(value: int, align: int = NOTE_ALIGN) -> int:
    return (value + align - 1) & ~(align - 1)


def make_note(name: bytes, note_type: int, desc: bytes) -> bytes:
    header = struct.pack("<III", len(name), len(desc), note_type)
    return (
        header
        + name
        + b"\0" * (align_up(len(name)) - len(name))
        + desc
        + b"\0" * (align_up(len(desc)) - len(desc))
    )


def parse_fixture(notes: bytes, segment_align: int = NOTE_ALIGN) -> tuple[bool, bool]:
    if len(notes) > NOTE_SCAN_MAX:
        return False, True
    if segment_align != NOTE_ALIGN:
        return False, False

    valid = False
    invalid = False
    offset = 0
    while offset < len(notes):
        if len(notes) - offset < 12:
            return False, True
        namesz, descsz, note_type = struct.unpack_from("<III", notes, offset)
        name_offset = offset + 12
        desc_offset = name_offset + align_up(namesz)
        next_offset = desc_offset + align_up(descsz)
        if (
            next_offset <= offset
            or next_offset > len(notes)
            or desc_offset > len(notes)
            or descsz > len(notes) - desc_offset
        ):
            return False, True
        name = notes[name_offset:name_offset + namesz]
        desc = notes[desc_offset:desc_offset + descsz]
        if namesz == len(NOTE_NAME) and note_type == NOTE_TYPE and name == NOTE_NAME:
            if desc != EXPECTED_DESCRIPTOR.encode("utf-8"):
                return False, True
            valid = True
        offset = next_offset
    return valid, invalid


def check_elf_artifact(path: pathlib.Path) -> None:
    image = path.read_bytes()
    require(
        len(image) >= 64
        and image[:6] == b"\x7fELF\x02\x01"
        and struct.unpack_from("<H", image, 18)[0] == 233,
        f"artifact must be a little-endian Linx ELF64 image: {path}",
    )
    phoff = struct.unpack_from("<Q", image, 32)[0]
    phentsize, phnum = struct.unpack_from("<HH", image, 54)
    require(phentsize >= 56 and phoff + phentsize * phnum <= len(image),
            f"invalid program header table: {path}")

    found = False
    for number in range(phnum):
        offset = phoff + number * phentsize
        p_type, _, p_offset, _, _, p_filesz, _, p_align = struct.unpack_from(
            "<IIQQQQQQ", image, offset
        )
        if p_type != 4 or p_align != NOTE_ALIGN:
            continue
        require(p_offset + p_filesz <= len(image),
                f"PT_NOTE exceeds file bounds: {path}")
        valid, invalid = parse_fixture(image[p_offset:p_offset + p_filesz])
        require(not invalid, f"invalid PTO ISA identity in {path}")
        found = found or valid
    require(found, f"exact PTO ISA 0.59.1 identity missing from {path}")


def check_fixture_cases() -> None:
    good = make_note(NOTE_NAME, NOTE_TYPE, EXPECTED_DESCRIPTOR.encode("utf-8"))
    v0581 = make_note(NOTE_NAME, NOTE_TYPE, V0581_DESCRIPTOR.encode("utf-8"))
    v0583 = make_note(NOTE_NAME, NOTE_TYPE, V0583_DESCRIPTOR.encode("utf-8"))
    mismatched = json.loads(EXPECTED_DESCRIPTOR)
    mismatched["encoding_projection_sha256"] = "0" * 64
    wrong_projection = make_note(
        NOTE_NAME,
        NOTE_TYPE,
        json.dumps(mismatched, separators=(",", ":")).encode("utf-8"),
    )
    other = make_note(b"GNU\0", 3, b"build-id")
    cases = {
        "valid": (good, (True, False)),
        "missing": (other, (False, False)),
        "legacy-v0.58.1": (v0581, (False, True)),
        "legacy-v0.58.3": (v0583, (False, True)),
        "wrong-projection": (wrong_projection, (False, True)),
        "conflict": (good + v0583, (False, True)),
        "duplicate-identical": (good + good, (True, False)),
        "malformed": (good + b"\x01\x02", (False, True)),
        "trailing-nul": (
            make_note(NOTE_NAME, NOTE_TYPE,
                      EXPECTED_DESCRIPTOR.encode("utf-8") + b"\0"),
            (False, True),
        ),
        "oversized": (good + b"x" * NOTE_SCAN_MAX, (False, True)),
    }
    for name, (payload, expected) in cases.items():
        actual = parse_fixture(payload)
        require(actual == expected,
                f"fixture {name} expected {expected}, got {actual}")

    require(parse_fixture(other, segment_align=8) == (False, False),
            "non-PTO note segments may use a non-PTO alignment")
    require(parse_fixture(good, segment_align=8) == (False, False),
            "non-4-byte segments must not provide a valid PTO identity")


def read(repo: pathlib.Path, rel: str) -> str:
    return (repo / rel).read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"error: {message}")


def extract_c_string_macro(source: str, name: str) -> str:
    lines = source.splitlines()
    for index, line in enumerate(lines):
        if line.startswith(f"#define {name}"):
            macro_lines = [line]
            while macro_lines[-1].rstrip().endswith("\\"):
                index += 1
                require(index < len(lines), f"{name} macro continuation is open")
                macro_lines.append(lines[index])
            break
    else:
        raise SystemExit(f"error: {name} macro missing")

    literal_text = "\n".join(macro_lines)
    parts = re.findall(r'"(?:\\.|[^"\\])*"', literal_text)
    require(parts, f"{name} has no string literal")
    return "".join(bytes(part[1:-1], "utf-8").decode("unicode_escape")
                   for part in parts)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--artifact", action="append", type=pathlib.Path, default=[])
    args = parser.parse_args()
    repo = pathlib.Path(__file__).resolve().parents[2]

    authority = json.loads(read(repo, "tools/linx/pto-isa-0591-authority.json"))
    require(authority == EXPECTED_AUTHORITY,
            "PTO ISA 0.59.1 authority fixture is not exact")
    wire_identity = {
        key: authority[key]
        for key in ("encoding_abi", "encoding_projection_sha256", "release")
    }
    require(json.dumps(wire_identity, sort_keys=True, separators=(",", ":"))
            == EXPECTED_DESCRIPTOR,
            "authority fixture does not derive the canonical wire descriptor")

    elf_h = read(repo, "elf/elf.h")
    dl_prop = read(repo, "sysdeps/linx/dl-prop.h")
    dl_machine = read(repo, "sysdeps/linx/dl-machine.h")
    linkmap = read(repo, "sysdeps/linx/linkmap.h")

    require("#define ELF_NOTE_PTO" in elf_h, "ELF_NOTE_PTO is missing")
    require("#define PTO_NT_ISA_IDENTITY\t1" in elf_h,
            "PTO_NT_ISA_IDENTITY must be owner-local type 1")

    descriptor = extract_c_string_macro(dl_prop, "LINX_PTO_ISA_IDENTITY_JSON")
    require(descriptor == EXPECTED_DESCRIPTOR,
            "PTO ISA identity descriptor is not byte-exact")
    require("\\0" not in descriptor and "\x00" not in descriptor,
            "PTO ISA identity descriptor must not include a trailing NUL")

    require("if (ph->p_align != 4)\n    return;" in dl_prop,
            "non-4-byte note segments must be skipped without rejection")
    require("linx_pto_process_note_bytes (l, (const void *) start, ph->p_filesz"
            in dl_prop,
            "main executable PT_NOTE parser must scan p_filesz bytes")
    require("LINX_PTO_NOTE_SCAN_MAX 4096" in dl_prop,
            "PTO note parser must cap scanning at 4KiB")
    require("__pread64_nocancel" in dl_prop and "ph->p_offset" in dl_prop
            and "ph->p_filesz" in dl_prop,
            "DSO PT_NOTE parser must use bounded fd reads")
    require("linx_pto_note_range_loaded" in dl_prop and "PT_LOAD" in dl_prop,
            "main executable PT_NOTE parser must validate mapped ranges")
    require("linx_pto_add_overflow" in dl_prop
            and "__builtin_add_overflow" in dl_prop,
            "PTO note parser must use checked arithmetic")
    require("_rtld_main_check" in dl_prop and "_dl_open_check" in dl_prop,
            "startup and dlopen checks must be wired")
    require("l_searchlist.r_nlist" in dl_prop,
            "DT_NEEDED dependencies must be checked through the searchlist")
    require("pto_isa_identity_valid" in linkmap
            and "pto_isa_identity_invalid" in linkmap,
            "link_map_machine must store PTO note state")

    for name, value in EXPECTED_RELOCS.items():
        match = re.search(rf"# define {name} (\d+)", dl_machine)
        require(match is not None, f"{name} definition missing")
        require(int(match.group(1)) == value,
                f"{name} must be {value}, found {match.group(1)}")

    check_fixture_cases()
    for artifact in args.artifact:
        check_elf_artifact(artifact)

    print("ok: Linx glibc PTO ISA identity wiring matches 0.59.1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
