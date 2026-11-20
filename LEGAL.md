# Legal Policy — KirbyRecomp

**Last updated: 2025**

---

## 1. Zero Nintendo Copyrighted Material

This repository contains **zero Nintendo copyrighted material**. Specifically:

- ❌ No game ROMs, NSPs, XCIs, NCAs
- ❌ No game assets (models, textures, audio, shaders, scripts)
- ❌ No firmware, system modules, or encryption keys
- ❌ No prod.keys, title keys, or master keys
- ❌ No game executables (NSOs, NROs)

All game files must be **user-supplied** from their own legally owned copy.

---

## 2. What This Project Contains (Original Work Only)

| Component | License | Ownership |
|-----------|---------|-----------|
| Translator pipeline (AArch64 → IR → C) | MIT | Project authors |
| Shader translator (Maxwell → SPIR-V) | MIT | Project authors |
| NVN command buffer translator | MIT | Project authors |
| HLE service implementations | MIT | Project authors |
| Runtime (memory, scheduling, syscalls) | MIT | Project authors |
| Build system (CMake, generator) | MIT | Project authors |
| SwitchRecomp IR/codegen | GPL-3.0 | Upstream (kaiyao7) |

**No Nintendo code is incorporated.** All implementations are clean-room from public documentation, RE research, and open specs.

---

## 3. User Requirements (You Must Own the Game)

To build and run, you must provide **your own legally obtained** files:

| File | Source |
|------|--------|
| `ExeFS` / `RomFS` | Dump from **your own cartridge/eShop download** |
| `prod.keys` | Dump from **your own Switch console** |

**You must personally own the game and console.** Downloading these files from the internet is **not** legal ownership.

---

## 4. Prohibited Activities

| Prohibited | Reason |
|------------|--------|
| Distributing game ROMs/NSOs/NROs | Copyright infringement |
| Sharing prod.keys / title keys | Anti-circumvention (DMCA 1201 / EUCD Art 6) |
| Distributing game assets (textures, audio, models) | Copyright infringement |
| Providing pre-built binaries with assets | Facilitates infringement |
| Hosting ROM/key download links | Contributory infringement |
| Bundling Nintendo firmware/BIOS | Copyright + anti-circumvention |

---

## 5. Anti-Circumvention Compliance (DMCA 1201 / EUCD Art 6)

This project **does not** provide or facilitate:

- Key extraction from consoles
- DRM bypass tools
- Encryption key derivation
- Firmware decryption utilities

**Users must legally obtain their own keys** from their own hardware.

---

## 6. Clean-Room Implementation Policy

All Nintendo functionality is implemented via **clean-room reverse engineering**:

| Separation | Purpose |
|------------|---------|
| **Researchers** | Study binaries, document behavior, produce specs |
| **Implementers** | Write code from specs only, never view original binaries |

No Nintendo source code, leaked SDKs, or proprietary documentation is referenced.

---

## 7. Asset Distribution Policy

**All game assets remain user-owned:**

- Shaders → translated to SPIR-V at build time from user's RomFS
- Models/textures/audio → loaded at runtime from user's RomFS
- No assets bundled, cached, or redistributed by this project

---

## 7. Provenance & Attribution

Each build generates `provenance.toml` recording:

- Generator version, timestamp, SwitchRecomp version
- Source file hashes (SHA-256 of user's NSOs)
- No asset content recorded

---

## 8. DMCA / Takedown Policy

| If you receive a notice | Action |
|-------------------------|--------|
| Valid DMCA for Nintendo assets | Remove immediately; we do not host them |
| DMCA for our original code | Counternotify (we own it) |
| Anti-circumvention claim | Review with counsel; we don't distribute circumvention tools |

**Contact:** Open a GitHub issue or email the maintainers.

---

## 7. Jurisdictional Notes

| Jurisdiction | Key Provisions |
|--------------|----------------|
| **US (DMCA)** | §1201 anti-circumvention; §107 fair use; §117 backup copies |
| **EU (EUCD)** | Art 6 anti-circumvention; Art 5-6 decompilation for interoperability |
| **Japan** | Copyright Act Art 47-2 (interoperability); Unfair Competition Prevention Law |
| **Clean-room** | Recognized defense in US (Sega v. Accolade), EU (SAS Institute v. WPL) |

---

## 8. Precedents & References

- **Sega v. Accolade** (1992) — Reverse engineering for interoperability = fair use
- **Sony v. Connectix** (2000) — BIOS reverse engineering for emulator = fair use
- **SAS Institute v. WPL** (EU) — Observing program behavior for interoperability permitted
- **Sega v. Accolade** (9th Cir.) — Intermediate copying for interoperability = fair use
- **Lexmark v. Static Control** — Interoperability exceptions to DMCA 1201
- **Sega v. Accolade** — Disassembly for compatibility = fair use

---

## 9. No Warranty / Liability

This software is provided **"AS IS"** without warranty. The authors are not liable for any legal consequences arising from misuse. **Users are solely responsible for ensuring their use complies with local laws.**

---

## 10. Reporting Violations

If you find Nintendo copyrighted material in this repository, **open a GitHub issue immediately** with:

- File path(s)
- Description of the material
- Evidence of Nintendo ownership

We will remove it immediately.

---

**Sources:**
- EFF Reverse Engineering FAQ — https://www.eff.org/issues/coders/reverse-engineering-faq
- Recompedia Legality — https://recompedia.com/legality
- Sega v. Accolade, 977 F.2d 1510 (9th Cir. 1992)
- Sony v. Connectix, 203 F.3d 596 (9th Cir. 2000)
- DMCA 17 USC §1201 — Anti-circumvention
- EU Software Directive 2009/24/EC Arts 5-6
- EUCD 2001/29/EC Art 6 — Anti-circumvention
- Japan Copyright Act Art 47-2
- Nintendo DMCA Notices (2024) — https://github.com/github/dmca

---

**TL;DR:** We ship code/tools only. You bring your own legally owned game. No Nintendo IP leaves your machine.