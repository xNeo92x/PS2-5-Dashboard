# PS2-5 Dashboard

**An experimental, entirely procedural PS2 homebrew dashboard with a PS5-inspired navigation layout.**

Runs on a **real PlayStation 2**, not on a PSP. Designed with the PS2 Fat, SATA HDD/SSD adapters and **PixelFX Retro GEM Shiny** in mind. The interface uses PS2 Graphics Synthesizer primitives: flat-shaded rectangles, gradients, triangles, circles, lines, and a tiny in-code glyph program. **No image files, game art, shaders or Sony UI assets are embedded.**

> **Status: early developer preview / hardware unverified.** Source code and build setup are provided. A successful toolchain build or emulator screenshot does not prove HDMI compatibility. This dashboard is not an alternative PSBBN distribution and does not yet launch PS1/PS2 disc-image games.

## Features in this first revision

- A dark navy/blue PS5-*inspired* horizontal tile menu, with original vector-like symbols and abstract animated waves.
- Games, Apps, and System tabs. Select with D-pad and shoulder buttons; automatic controller reconnect.
- Custom 5x7 glyphs drawn as GS sprites at runtime; no texture allocation for the UI.
- Two independent builds: **480p safe** (704×480, conventional gsKit; suitable for PCSX2) and **720p experimental** (1280×720, 16-bit gsKit 3-pass HIRES scanout; **real PS2 only**).
- Manual read-only collection list at `mc0:/PS25/APPS.CFG` or `mc1:/PS25/APPS.CFG`.
- Experimental **.ELF** launch attempts from memory cards or the ps2client `host:` device.
- Exit to the stock PS2 browser; no changes to partitions, game ISOs, HDD settings, or PSBBN.

## Build

Install the [ps2dev toolchain / PS2SDK](https://ps2dev.github.io/#getting-started) and [gsKit](https://github.com/ps2dev/gsKit) with the development headers and libraries. A normal shell setup looks like:

```sh
export PS2DEV=/path/to/ps2dev
export PS2SDK="$PS2DEV/ps2sdk"
export GSKIT="$PS2DEV/gsKit"
export PATH="$PS2DEV/bin:$PATH"
make test
make MODE=480p
make MODE=720p
```

Build results (if the toolchain is installed):

```text
dist/PS25-480p.ELF
dist/PS25-720p.ELF
```

`make clean` removes only local build outputs. `make test` requires only a native C compiler and tests the catalog parser. GitHub Actions builds both ELF variants using PS2SDK and gsKit. The workflow artifacts are **build outputs, not hardware-tested releases**.

## Install safely

1. **Back up your PSBBN SSD/HDD first**. Do not replace its boot files or partition table with this preview.
2. Build the **480p** ELF first and copy it to a USB stick or memory card.
3. Run it *manually* using an existing ELF launcher such as **wLaunchELF**. An already running launcher can load the ELF from USB; this dashboard itself does **not** initialize USB mass-storage drivers.
4. Test pad navigation and video. For the 720p experiment, keep the 480p ELF available to recover from an unsupported HDMI mode.
5. The Retro GEM Shiny can upscale a supported incoming mode to HDMI 1080p. If your television rejects 1440p, leave the Retro GEM output at 1080p; it does not mean the dashboard renders internally at 1080p.

**Important 720p caveat:** The gsKit HIRES path only stores a fraction of the frame in GS eDRAM and relies on precisely timed multi-pass scanout. The official gsKit examples warn that PCSX2 only shows part of the image in this mode. Test `PS25-720p.ELF` on **real hardware**. If the image breaks, restart using `PS25-480p.ELF`. No video mode is persisted to the console.

## Manual collection

Copy `example/APPS.CFG` to `mc0:/PS25/APPS.CFG` or `mc1:/PS25/APPS.CFG`, and replace the example comments with your actual installed ELF paths:

```text
# Title | GAME or APP | filesystem path to an ELF
wLaunchELF | APP | mc0:/BOOT/BOOT.ELF
My Homebrew | GAME | mc0:/PS25/MYGAME.ELF
```

Only `mc0:/`, `mc1:/`, and `host:` ELF paths are accepted; paths must end in `.ELF` and have a readable ELF header. The parser rejects unsupported prefixes and overlong entries. Limit 32 entries; maximum config read length 8191 bytes.

The **launch operation is experimental**: `LoadExecPS2()` relies on the system and IOP filesystem environment. A valid ELF is **not** a guarantee the PS2 can start it by that path. Use an established launcher for recovery. Do not expect direct PSBBN PFS partition, SATA ISO, OPL, POPStarter, or PS1 disc-image integration yet.

## Controls

| Button | Behavior |
| --- | --- |
| Left / Right | Move through tiles |
| L1 / R1 | Switch Games / Apps / System |
| Cross or Start | Open selected ELF / system option |
| Triangle | System tab |
| Circle | Close overlay or return to Games |
| Select | Help / controls |
| System → Exit to Browser | Confirm before leaving via `ExecOSD` |

## Design and engineering

The PS2 GS has **4 MiB eDRAM**. It cannot page a framebuffer from the SATA SSD. Our procedural drawing eliminates most **texture memory**, but not framebuffer memory. We use conventional double-buffered 480p or experimental multipass 720p without textures or a Z-buffer. 16-bit color with hardware dithering is used to keep VRAM pressure down.

This project is an original design, not a port of **08Weber/psp5** (that program is for the PSP and ARK), and contains **no psp5 source code**. The PSP project inspired the broad UX direction; ps2dev/gsKit examples informed the PS2 graphics backend. PSBBN Definitive and Free-BBN are relevant community projects, but their binaries have not been bundled.

## Planned work

1. Verify ELF build, 480p/720p scanout, controller and fonts on a real PS2 Fat + Retro GEM Shiny.
2. Integrate existing homebrew loaders and an optional read-only PSBBN title index (without mutating partitions).
3. Improve focus transitions and text rasterization; add controller mapping, settings persistence, memory/GS diagnostics.
4. Explore optional SATA/APA/PFS browsing with clear user opt-in and read-only mounting before ever changing PSBBN.

## Credits and legal

Created with assistance from **ChatGPT (OpenAI)**. Thanks to [ps2dev](https://github.com/ps2dev), [gsKit](https://github.com/ps2dev/gsKit), [CosmicScale's PSBBN Definitive Project](https://github.com/CosmicScale/PSBBN-Definitive-Project), [08Weber's psp5](https://github.com/08Weber/psp5), and [Free-BBN](https://github.com/jegesmedve09/Free-BBN) for their open community work.

Code in this repository is MIT-licensed (see LICENSE). The project is **unofficial**, not sponsored or endorsed by Sony Interactive Entertainment. 'PS5' and 'PlayStation' are Sony trademarks. No Sony graphics, system files or proprietary SDK files are included.

## Black screen / reboot: staged boot diagnostics

The original two variants were reported to show a black screen and a reset on a
real PS2 with Retro GEM Shiny. **These additional test executables are not proof
that the issue is solved.** They isolate individual parts of startup.

Test these files **in order** using wLaunchELF directly from a FAT32 USB stick
(`mass:/`), **not** PSBBN's built-in app launcher or OPL, so we can exclude a
launcher-specific reset.

| ELF | Expected screen | Purpose |
| --- | --- | --- |
| `PS25-diag480i.ELF` | Blue background, orange rectangle, white bars | Basic interlaced NTSC video. NO pad, MC, IOP or filesystem |
| `PS25-diag576i.ELF` | Blue background, orange rectangle, white bars | Same test with PAL video |
| `PS25-diag480p.ELF` | Green/teal background, green rectangle, white bars | Basic 480p video, still without IOP |
| `PS25-demo480p.ELF` | PS5-like dashboard, no controller input | Full procedural UI with **NO IOP/pad/MC** |
| `PS25-480p.ELF` | Same dashboard, now with controller input | IOP and pad initialization; memory card loads only when Square is pressed |
| `PS25-720p.ELF` | Experimental high-resolution dashboard | Advanced multipass scanout; test only after 480p works |

The first three test pictures run indefinitely. **There is no automatic reset
in these diagnostic binaries.** You must use the physical Reset button when
done. The demo has no controller functionality by design; it also runs indefinitely.

The full dashboard now shows its first frame **before** initializing any IOP
drivers. For safety, the memory-card catalog is **not read at startup**: press
**Square** if the basic dashboard and pad already work. If the last visible
status says `INITIALIZING CONTROLLER` or `READING MEMORY CARD`, report this
exactly.

Compile all diagnostic and dashboard variants using:

```sh
make diagnostics
make demo
make MODE=480p
make MODE=720p
```

Please report exactly which file showed colors, whether the picture stayed on
screen, and whether the console visibly power-cycled/restarted or simply
returned to the launcher. Do **not** overwrite the PSBBN SSD or install the
experimental ELF as an autostart application.
