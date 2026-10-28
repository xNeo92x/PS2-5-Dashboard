# PS2-5 Dashboard

**An experimental, entirely procedural PS2 homebrew dashboard with a PS5-inspired navigation layout.**

Runs on a **real PlayStation 2**, not on a PSP. Designed with the PS2 Fat, SATA HDD/SSD adapters and **PixelFX Retro GEM Shiny** in mind. The interface uses PS2 Graphics Synthesizer primitives: flat-shaded rectangles, gradients, triangles, circles, lines, and a tiny in-code glyph program. **No image files, game art, shaders or Sony UI assets are embedded.**

> **Status: early developer preview / hardware unverified.** Source code and build setup are provided. A successful toolchain build or emulator screenshot does not prove HDMI compatibility. This dashboard is not an alternative PSBBN distribution and does not yet launch PS1/PS2 disc-image games.

## Features in this first revision

- An original dark navy/blue PS5-*inspired* layout: category tabs, horizontal game/app rail, prominent selection detail panel and procedural symbols. All text is width-clipped to tile bounds.
- Games, Apps, and System tabs. Select with D-pad and shoulder buttons; automatic controller reconnect.
- Custom 5x7 glyphs drawn as GS sprites at runtime; no texture allocation for the UI.
- Three video backends: **480p** (704×480 normal gsKit), **720p direct** (1280×720, 16-bit double-buffered conventional gsKit), and **720p-hires** (legacy 3-pass gsKit HIRES retained as an experimental A/B test).
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
# Optional: compare the unstable original high-res backend:
make MODE=720p-hires
```

Build results (if the toolchain is installed):

```text
dist/PS25-480p.ELF
dist/PS25-720p.ELF
# Optional experimental comparison:
dist/PS25-720p-hires.ELF
```

`make clean` removes only local build outputs. `make test` requires only a native C compiler and tests the catalog parser. GitHub Actions builds both ELF variants using PS2SDK and gsKit. The workflow artifacts are **build outputs, not hardware-tested releases**.

## Install safely

1. **Back up your PSBBN SSD/HDD first**. Do not replace its boot files or partition table with this preview.
2. Build the **480p** ELF first and copy it to a USB stick or memory card.
3. Run it *manually* using an existing ELF launcher such as **wLaunchELF**. An already running launcher can load the ELF from USB; this dashboard itself does **not** initialize USB mass-storage drivers.
4. Test pad navigation and video. For the 720p experiment, keep the 480p ELF available to recover from an unsupported HDMI mode.
5. The Retro GEM Shiny can upscale a supported incoming mode to HDMI 1080p. If your television rejects 1440p, leave the Retro GEM output at 1080p; it does not mean the dashboard renders internally at 1080p.

**720p hardware warning:** The original `PS25-720p-hires.ELF` uses a timing-sensitive, three-pass gsKit HIRES renderer and produced severe distortion on a PS2 Fat with Retro GEM Shiny. It is retained **only for diagnostic comparison**. The revised `PS25-720p.ELF` instead uses normal gsKit GS double-buffered 16-bit scanout, avoiding HIRES. Its two framebuffer allocations are approximately 3.75 MiB **including gsKit page alignment**, out of 4 MiB GS eDRAM. This is still a **hardware-untested replacement** and may not solve every PS2 model/HDMI issue. If picture breaks, switch back to `PS25-480p.ELF`; no video mode is persisted.

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
| `PS25-720p.ELF` | **NEW:** Standard double-buffered 720p dashboard | 16-bit direct framebuffer; no HIRES threads/passes; test after 480p works |
| `PS25-720p-hires.ELF` | Original HIRES 720p (known to distort on test device) | Keep only as a comparison; not recommended for daily use |

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


## PS2 Fat + Retro GEM Shiny hardware findings (October 2026)

- 480p is **confirmed to launch and display** when run from an older uLaunchELF installed on a Free-MCBoot memory card.
- The same older 480p build does **not** launch reliably from PSBBN's bundled uLaunchELF. The reason for the differing IOP/loader environment needs investigation.
- Original three-pass HIRES 720p displayed corrupt text, a duplicated lower footer and lower-screen video noise. See: legacy `PS25-720p-hires.ELF`.
- The **new direct 720p build** avoids HIRES entirely and uses normal 16-bit GS double buffering. It is **not yet verified on real hardware**.
- Tile names now use width-bound truncation; the upper rail uses large PS5-inspired selectable tiles and a lower hero detail area.

### Recommended A/B test order

1. Keep the previously working 480p ELF on the USB stick as a fallback.
2. Launch `PS25-480p.ELF` **from the older Free-MCBoot uLaunchELF**, to check the new layout.
3. Launch the **new** `PS25-720p.ELF` by the same method. This build no longer uses multi-pass HIRES.
4. If distortion remains, save a photo of the screen and report what it looks like, including whether the Retro GEM output is locked at 1080p.
5. Do **not** auto-launch either build from PSBBN or replace your existing SATA SSD partition data.

**Development branch:** `fix/boot-diagnostics-and-safe-init` (draft PR #3). This branch intentionally contains hardware diagnostics and UI changes that require user testing before merging into `main`.

## 720p corruption isolation (hardware report: 28 October 2026)

**The original 720p HIRES and the revised 720p direct double-buffered
dashboard both exhibit repeated text and corrupted lower-screen image on the
same PS2 Fat + PixelFX Retro GEM Shiny.** This disproves the working
hypothesis that the HIRES scheduler *alone* caused the issue. Do not assume
the native 720p dashboard is fixed.

To isolate **video scanout vs. framebuffer memory layout vs. the complex UI**,
the following minimal diagnostics draw only *six full-width horizontal
colored bars*, two white vertical rails and one white middle square. No
font/UI engine, memory-card access, IOP modules or application launcher is
used. The **bottommost bar must be green**. All tests run indefinitely; use
the physical PS2 Reset button to exit.

**Test in the following order, using the older Free-MCBoot uLaunchELF that
successfully starts the working 480p dashboard:**

| ELF | GS framebuffer | Purpose |
| --- | --- | --- |
| `PS25-diag720p-scaled.ELF` | 640x360, 16-bit, single | 720p output timing with cheap 2x GS display magnification |
| `PS25-diag720p-single.ELF` | 1280x720, 16-bit, single | Native pixels without double buffering (~1.875 MiB aligned) |
| `PS25-diag720p-double.ELF` | 1280x720, 16-bit, double | Native pixels, two full framebuffers (~3.75 MiB aligned) |

**Optional diagnostic variants** if required to isolate GS pixel memory layout:

| ELF | GS framebuffer | Purpose |
| --- | --- | --- |
| `PS25-diag720p-16s.ELF` | 1280x720, CT16S, single | Test swizzled 16S layout |
| `PS25-diag720p-rgb32.ELF` | 1280x720, CT32, single | Test 32-bit GS framebuffer (~3.59 MiB aligned) |

A separate `PS25-720p-single.ELF` runs the complete dashboard with a
**single** 16-bit framebuffer. Do not run it until the simpler 720p bars are
known to work, and keep the proven 480p version as fallback.

**How to interpret the result:**

- If even `diag720p-scaled` is distorted, investigate the 720p signal path,
  mode registers and PixelFX firmware/scaling settings before tuning the UI.
- If scaled is clean but 1280x720 single is corrupted, the problem follows
  the full-resolution GS framebuffer rather than 720p output alone.
- If single is clean but double is corrupted, inspect the second buffer
  address, GS memory allocation and display-buffer flip.
- If all the simple diagnostics look clean but the full dashboard does not,
  investigate primitive queue usage, alpha blending and text rendering.

This is intentionally a **diagnostic experiment**, not a claim that the
issue is solved. Compile via `make diagnostics` and
`make MODE=720p-single`, or download the GitHub Actions test artifact.
Do not auto-start these files from PSBBN and do not alter the SATA SSD.

## 720p zoom and framebuffer findings (real-hardware report)

The Retro GEM Shiny OSD shows **1280x720p input** and **1080p60 output**. Its
zoom multiplier can crop the captured source before output: 2.000x gives
2560x1440 (cropped on a 1920x1080 output); 1.875x gives 2400x1350 (also
cropped); **1.500x maps 1280x720 to 1920x1080**, with the whole frame visible.
The zoom setting alone cannot prove corruption is created by the scaler: a
cropped scene can appear clean while hiding garbage below the visible area.

The owner reports the full-resolution **simple color-bar** diagnostics
`PS25-diag720p-single.ELF` and `PS25-diag720p-double.ELF` appear correct,
while the `PS25-diag720p-scaled.ELF` (640x360 magnified) is distorted.
The dashboard at 720p single-buffering appears clean when zoomed-in but
shows corruption when more of the screen is made visible. Until the *simple*
test image is evaluated at the same zoom, we cannot distinguish scaler
cropping from actual source framebuffer corruption.

**Recommended next hardware test (do not save changes to a working GEM profile):**

1. Run `PS25-diag720p-double.ELF` via the older FMCB uLaunchELF.
2. In Retro GEM Scaler, switch to **1.500 / 1.500**, 16:9 and 1080p output.
3. Check that **all six full-width horizontal bars are visible**, in
   particular the **green bottom bar**; if distorted, take a photo. If
   possible compare with 2.000 / 2.000 without changing anything else.
4. Run **`PS25-probe720p.ELF`** at the same 1.500 zoom. It automatically
   cycles through three stages, roughly every 420 frames:

   - **White central square:** six colored bars, no UI or font.
   - **Orange central square:** full UI *shapes*, all labels disabled.
   - **Cyan central square:** full UI *with procedural text*.

   This repeats. There is deliberately no controller/MC/IOP initialization.
   Report the first stage that becomes corrupted and whether the bottom of
   the picture is damaged. If colors are clean but only the full-font
   stage glitches, investigate the GS draw queue and text primitive traffic.
5. Exit the probe via the physical Reset button. Avoid treating a successful
   ELF compilation as proof of video correctness.

To compile the staged probe locally using the ps2dev toolchain:
`make MODE=probe720p`. CI uploads it alongside the older test executables.

The Retro GEM's OSD firmware version in the supplied photos is
**3.15.25.4**; do not update firmware only to debug this problem without
checking the official release notes and backing up your current profile.

## Why the Retro GEM menu hotkey failed in colorbar tests

The original `PS25-diag720p-double.ELF` was deliberately built with **no
controller initialization or pad polling**. On a PS2, the Pixel FX Retro GEM
menu hotkey can depend on seeing live controller transactions; if no game
or application polls the controller, the button combo might not be detected.

The default Pixel FX PS1/PS2 OSD combo is **L1 + R1 + D-pad Right + Circle**
([official controller hotkeys](https://docs.pixelfx.co/GEM-Details.html)).
If you have customized the shortcut, use your actual mapping instead.

To test OSD access while looking at a **720p colorbar screen**, start
`PS25-diag720p-double-pad.ELF` using the **older uLaunchELF from the FMCB
memory card**. The test renders the normal six bars in 1280x720 16-bit
double buffering, then starts `SIO2MAN`/`PADMAN` and polls controller port
1 every frame. A small indicator near the upper left turns **green**
when the PS2 has received valid controller data; **orange** means no valid
pad report so far. A green indicator does not guarantee Pixel FX will
recognize the OSD hotkey, but is a good sanity check.

Open the Retro GEM OSD with the combo above and temporarily set the
*Scaler → Zoom* to **1.500 / 1.500** with 16:9 and HDMI output at 1080p.
For 1280x720 this gives a nominal 1920x1080 picture (subject to actual
capture geometry and any overscan/crop offsets). Examine all **six**
stripes, especially the bottom green one. Avoid saving a problematic
profile; use **L1: Cancel** as shown in the Retro GEM OSD if appropriate.

This *pad-enabled* diagnostic is intentionally separated from the
no-pad version: it includes PS2 IOP drivers and can therefore expose
a distinct IOP/launcher issue. It does not read or write the SSD.
Exit with the PS2 Reset button.

If the OSD hotkey remains unavailable, the official
[Pixel FX web interface](https://docs.pixelfx.co/FX-Framework-WebUI.html)
can be accessed from a browser **if the GEM has already been paired with
Wi-Fi and its web UI credentials are known**; otherwise return to a
controller-polling application to configure the GEM before launching a
non-polling diagnostic.

**Interpretation update:** The previous report that `diag720p-single`
and `diag720p-double` were "clean" was based on a cropped 2.000x view.
At least the bottom two of six bars were outside the screen, so
the lower part of both 720p diagnostic images remains *unverified*.
A 2.000x crop can hide corruption below the visible region.
