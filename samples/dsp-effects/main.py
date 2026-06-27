#!/usr/bin/env python
"""
DSP Effects Tester
==================
Interactive GUI for testing every FilterProperties DSP effect against tank.mp3
using the FMOD audio backend.

Controls
--------
  Left panel   : click an effect to select it
  Center panel : adjust parameter sliders, then press Apply to add to chain
  Right panel  : play/pause/stop, volume, active chain (click a slot to edit it)

  Space  - Play / Pause
  S      - Stop
  Enter  - Apply current effect (adds new slot, always rebuilds chain)
  U      - Update selected chain slot in-place (no FMOD chain rebuild)
  C      - Clear all effects
"""

# Must come before all other Panda3D imports.
from panda3d.core import loadPrcFileData
loadPrcFileData("", "audio-library-name p3fmod_audio")
loadPrcFileData("", "win-size 1280 800")
loadPrcFileData("", "window-title FMOD DSP Effects Tester")

import os

from panda3d.core import FilterProperties, TextNode
from direct.showbase.ShowBase import ShowBase
from direct.gui.DirectGui import (
    DirectButton, DirectSlider, DirectLabel, DirectFrame,
    DirectScrolledFrame,
)
from direct.gui import DirectGuiGlobals as DGG
from direct.gui.OnscreenText import OnscreenText
from panda3d.core import AudioSound

# ---------------------------------------------------------------------------
# Effect catalogue
# Each row: (display_name, FilterProperties_method, [(label, min, max, default)])
# ---------------------------------------------------------------------------
EFFECTS = [
    ("Lowpass", "add_lowpass", [
        ("Cutoff Freq (Hz)", 10, 22000, 5000),
        ("Resonance Q",       1, 10,    1.0),   # FMOD range: 1–10
    ]),
    ("Highpass", "add_highpass", [
        ("Cutoff Freq (Hz)", 10, 22000, 5000),
        ("Resonance Q",       1, 10,    1.0),   # FMOD range: 1–10
    ]),
    ("Echo", "add_echo", [
        # conf._a → ECHO_DRYLEVEL (dB), conf._b → ECHO_WETLEVEL (dB)
        # conf._c → ECHO_DELAY (ms),    conf._d → ECHO_FEEDBACK (%)
        ("Dry Level (dB)", -80, 10,   0),
        ("Wet Level (dB)", -80, 10,   0),
        ("Delay (ms)",       1, 5000, 500),
        ("Feedback (%)",     0, 100,  50),
    ]),
    ("Flange", "add_flange", [
        # conf._a → FLANGE_MIX (%), conf._b unused, conf._c → DEPTH, conf._d → RATE
        ("Mix (%)",   0,    100, 50),   # FMOD FLANGE_MIX: 0–100 %
        ("Wet Mix",   0,    1,   0.5),  # conf._b not applied by FMOD
        ("Depth",     0.01, 1,   1.0),
        ("Rate (Hz)", 0,    20,  0.1),
    ]),
    ("Distort", "add_distort", [
        ("Level", 0, 1, 0.5),
    ]),
    ("Normalize", "add_normalize", [
        ("Fade Time (ms)", 0, 20000,  5000),
        ("Threshold",      0, 0.1,    0.1),
        ("Max Amp",        1, 100000, 20),
    ]),
    ("Parametric EQ", "add_parameq", [
        ("Center Freq (Hz)", 20,   22000, 8000),
        ("Bandwidth (oct)",  0.2,  5.0,   1.0),
        ("Gain",             0.05, 3.0,   1.0),
    ]),
    ("Pitch Shift", "add_pitchshift", [
        ("Pitch",    0.5, 2.0,  1.0),
        ("FFT Size", 256, 4096, 1024),
        ("Overlap",  1,   32,   4),
    ]),
    ("Chorus", "add_chorus", [
        # conf._a → CHORUS_MIX (%), conf._b/c/d (wet1/2/3) unused,
        # conf._e (delay) unused, conf._f → CHORUS_RATE, conf._g → CHORUS_DEPTH
        ("Mix (%)",    0,   100, 50),   # FMOD CHORUS_MIX: 0–100 %
        ("Wet 1",      0,   1,   0.5),  # not applied by FMOD
        ("Wet 2",      0,   1,   0.5),  # not applied by FMOD
        ("Wet 3",      0,   1,   0.5),  # not applied by FMOD
        ("Delay (ms)", 0.1, 100, 16),   # not applied by FMOD
        ("Rate (Hz)",  0,   20,  0.8),
        ("Depth",      0,   100, 3),    # FMOD CHORUS_DEPTH: 0–100, default 3
    ]),
    ("SFX Reverb", "add_sfxreverb", [
        # Old EAX param names map to FMOD 2.x SFX Reverb params (values passed as-is):
        # _a→DRYLEVEL, _b→WETLEVEL, _c→HIGHCUT, _d→DECAYTIME, _e→HFDECAYRATIO,
        # _f→EARLYLATEMIX, _g→EARLYDELAY, _h→(unused!), _i→LATEDELAY,
        # _j→DIFFUSION, _k→DENSITY, _l→HFREFERENCE, _m→LOWSHELFGAIN, _n→LOWSHELFFREQ
        ("Dry Level (dB)",      -80,   20,    0),     # SFXREVERB_DRYLEVEL
        ("Wet Level (dB)",      -80,   20,   -6),     # SFXREVERB_WETLEVEL
        ("High Cut (Hz)",        20,   20000, 20000), # SFXREVERB_HIGHCUT
        ("Decay Time (s)",       0.1,  20,    1.5),   # SFXREVERB_DECAYTIME
        ("HF Decay Ratio (%)",   10,   100,   50),    # SFXREVERB_HFDECAYRATIO
        ("Early/Late Mix (%)",    0,   100,   50),    # SFXREVERB_EARLYLATEMIX
        ("Early Delay (ms)",      0,   300,   20),    # SFXREVERB_EARLYDELAY
        ("(unused)",          -10000, 2000,    0),    # conf._h not applied by FMOD
        ("Late Delay (ms)",       0,   100,   40),    # SFXREVERB_LATEDELAY
        ("Diffusion (%)",         0,   100,  100),    # SFXREVERB_DIFFUSION
        ("Density (%)",           0,   100,  100),    # SFXREVERB_DENSITY
        ("HF Reference (Hz)",    20,   20000, 5000),  # SFXREVERB_HFREFERENCE
        ("Low Shelf Gain (dB)", -36,   12,    0),     # SFXREVERB_LOWSHELFGAIN
        ("Low Shelf Freq (Hz)",  20,   1000,  250),   # SFXREVERB_LOWSHELFFREQUENCY
    ]),
    ("Compressor", "add_compress", [
        ("Threshold (dB)",   -80, 0,    0),   # FMOD range: -80 to 0 dB
        ("Attack (ms)",      0.1, 500,  50),
        ("Release (ms)",     10,  5000, 50),
        ("Gain Makeup (dB)", 0,   30,   0),
    ]),
    ("Fader", "add_fader", [
        ("Gain (dB)", -80, 10, 0),
    ]),
    ("Limiter", "add_limiter", [
        ("Release Time (ms)",   1,  1000, 1000),  # C++ default: 1000 ms
        ("Ceiling (dB)",      -12,  0,    0),
        ("Maximizer Gain (dB)", 0,  12,   0),
        ("Mode",                0, [("Independent", 0), ("Linked", 1)]),
    ]),
    ("Pan", "add_pan", [
        # ── 2D ───────────────────────────────────────────────────────────
        ("Mode",              2, [("Mono", 0), ("Stereo", 1), ("Surround", 2)]),
        ("Stereo Position", -100, 100,   0),
        ("2D Direction",    -180, 180,   0),
        ("2D Extent",          0, 360, 360),
        ("2D Rotation",     -180, 180,   0),
        ("LFE Level (dB)",   -80,  20,   0),   # FMOD range: -80 to 20 dB
        # FMOD_DSP_PAN_2D_STEREO_MODE: 0=Distributed, 1=Discrete (C++ default=1)
        ("Stereo Mode",        1, [("Distributed", 0), ("Discrete", 1)]),
        ("Stereo Separation", -180, 180,  60),  # FMOD range: -180 to 180 deg
        ("Stereo Axis",     -180, 180,   0),
        # ── 3D ───────────────────────────────────────────────────────────
        ("Enabled Speakers",   0, 4095, 4095, int),
        ("3D Pos X",         -50,   50,    0),
        ("3D Pos Y",         -50,   50,    0),
        ("3D Pos Z",         -50,   50,    0),
        ("3D Rolloff",         0, [("LinSq", 0), ("Linear", 1), ("Inverse", 2), ("InvTap", 3), ("Custom", 4)]),
        ("3D Min Distance",    0, 1000,    1),
        ("3D Max Distance",    0, 1000,   20),
        ("3D Extent Mode",     0, [("Auto", 0), ("User", 1), ("Off", 2)]),
        ("3D Sound Size",      0,  360,    0),
        ("3D Min Extent",      0,  360,    0),
        ("Pan Blend",          0,    1,    0),
        ("LFE Upmix",          0, [("Off", 0), ("On", 1)]),
        ("Surround Mode",      0, [("Default", 0), ("Stereo", 2), ("5.1", 5), ("7.1", 6)]),
        ("Height Blend",      -1,    1,    0),
        ("Override Range",     1, [("Off", 0), ("On", 1)]),
    ]),
    ("Tremolo", "add_tremolo", [
        ("Frequency (Hz)", 0.1, 20, 5.0),
        ("Depth",          0,   1,  1.0),
        ("Shape",          0,   1,  0.0),
        ("Skew",          -1,   1,  0.0),
        ("Duty Cycle",     0,   1,  0.5),
        ("Square",         0,   1,  0.0),
        ("Phase",         -1,   1,  0.0),
        ("Spread",        -1,   1,  0.0),
    ]),
    ("Delay", "add_delay", [
        ("Ch0 Delay (ms)", 0, 10000, 0),
        ("Ch1 Delay (ms)", 0, 10000, 0),
        ("Max Delay (ms)", 0, 10000, 400),
    ]),
]

# ---------------------------------------------------------------------------
# Colour palette
# ---------------------------------------------------------------------------
BG        = (0.10, 0.10, 0.15, 1)
PANEL     = (0.13, 0.13, 0.20, 1)
ACCENT    = (0.28, 0.52, 0.88, 1)
BTN_DEF   = (0.20, 0.25, 0.38, 1)
BTN_GREEN = (0.18, 0.52, 0.28, 1)
BTN_RED   = (0.52, 0.18, 0.18, 1)
BTN_BLUE  = (0.18, 0.38, 0.68, 1)
BTN_ORG   = (0.52, 0.32, 0.10, 1)
T_BRIGHT  = (0.95, 0.95, 1.00, 1)
T_DIM     = (0.60, 0.60, 0.75, 1)
T_YELLOW  = (1.00, 0.95, 0.50, 1)
T_GREEN   = (0.45, 1.00, 0.55, 1)


class _RadioGroup:
    """A row of buttons that acts as a radio-button selector for enum params.

    Provides destroy() and get() so it can be stored alongside DirectSliders
    in self._widgets with the same interface.
    """
    def __init__(self, choices, default_val, y, parent=None):
        self._value = default_val
        self._btns  = []

        n       = len(choices)
        btn_w   = min(0.28, 0.84 / n)   # shrink equally if many choices
        gap     = 0.04
        total_w = n * btn_w + (n - 1) * gap
        x0      = -total_w / 2 + btn_w / 2  # centre the row

        kw = {} if parent is None else {"parent": parent}
        for j, (label, val) in enumerate(choices):
            x = x0 + j * (btn_w + gap)
            b = DirectButton(
                text=label,
                text_scale=0.036,
                text_fg=T_BRIGHT,
                frameSize=(-btn_w / 2, btn_w / 2, -0.028, 0.038),
                frameColor=ACCENT if val == default_val else BTN_DEF,
                relief=1,
                pos=(x, 0, y),
                **kw,
            )
            self._btns.append((b, val))

        for j, (b, val) in enumerate(self._btns):
            b["command"] = self._make_cmd(j, val)

    def _make_cmd(self, sel_j, sel_val):
        def _cmd():
            self._value = sel_val
            for k, (b, _) in enumerate(self._btns):
                b["frameColor"] = ACCENT if k == sel_j else BTN_DEF
        return _cmd

    def get(self):
        return self._value

    def set(self, val):
        self._value = val
        for k, (b, v) in enumerate(self._btns):
            b["frameColor"] = ACCENT if v == val else BTN_DEF

    def destroy(self):
        for b, _ in self._btns:
            b.destroy()


class DspTester(ShowBase):

    def __init__(self):
        ShowBase.__init__(self)
        self.setBackgroundColor(*BG[:3])
        self.disableMouse()

        # ── audio ──────────────────────────────────────────────────────────
        sound_path = os.path.join(os.path.dirname(__file__), "tank.mp3")
        self.sfx = self.loader.loadSfx(sound_path)
        self.sfx.setLoop(True)
        self._paused = False

        # ── state ──────────────────────────────────────────────────────────
        self._sel_idx     = 0     # index into EFFECTS
        self._widgets     = []    # [(widget, DirectLabel, choices_or_None, cast)]
        self._chain       = []    # [(display_name, method_name, [values])]
        self._btns        = []    # left-panel effect selector buttons
        self._chain_btns  = []    # right-panel active-chain slot buttons
        self._editing_slot: int | None = None  # None = add-new mode

        # ── build UI & select first effect ─────────────────────────────────
        self._build_ui()
        self._select(0)

        # ── keyboard shortcuts ──────────────────────────────────────────────
        self.accept("space", self._play_pause)
        self.accept("s",     self._stop)
        self.accept("S",     self._stop)
        self.accept("enter", self._apply)
        self.accept("u",     self._update_slot)
        self.accept("U",     self._update_slot)
        self.accept("c",     self._clear)
        self.accept("C",     self._clear)

    # -----------------------------------------------------------------------
    # UI construction
    # -----------------------------------------------------------------------

    def _build_ui(self):
        # Title bar
        OnscreenText(
            text="FMOD DSP Effects Tester",
            pos=(0, 0.93), scale=0.068,
            fg=T_YELLOW, align=TextNode.ACenter,
        )

        # ── LEFT PANEL: effect selector ─────────────────────────────────────
        # Panel background spans x: -1.60 → -0.70  (width 0.90)
        DirectFrame(
            frameSize=(0, 0.90, -0.97, 0.90),
            frameColor=PANEL, pos=(-1.60, 0, 0),
        )
        OnscreenText(
            text="DSP Effects", pos=(-1.15, 0.83),
            scale=0.047, fg=T_DIM, align=TextNode.ACenter,
        )

        BH     = 0.107   # vertical slot per button
        BTN_HW = 0.375   # button half-width
        BX     = -1.15   # button column centre x

        for i, effect in enumerate(EFFECTS):
            name = effect[0]
            y = 0.73 - i * BH
            btn = DirectButton(
                text=name,
                text_scale=0.039, text_fg=T_BRIGHT,
                frameSize=(-BTN_HW, BTN_HW, -0.033, 0.050),
                frameColor=BTN_DEF, relief=1,
                pos=(BX, 0, y),
                command=self._select, extraArgs=[i],
            )
            self._btns.append(btn)

        # ── CENTER PANEL: parameter sliders ─────────────────────────────────
        # Panel background spans x: -0.70 → 0.82  (width 1.52, flush with neighbours)
        DirectFrame(
            frameSize=(0, 1.52, -0.97, 0.90),
            frameColor=PANEL, pos=(-0.70, 0, 0),
        )

        self._effect_title = OnscreenText(
            text="", pos=(0.06, 0.82), scale=0.056,
            fg=T_GREEN, align=TextNode.ACenter,
        )

        # Scrollable region for parameter widgets.
        # Centre of panel is x=0.06; top raised to 0.77 (close to title at 0.82).
        self._scroll = DirectScrolledFrame(
            frameSize=(-0.70, 0.70, -0.82, 0.77),
            canvasSize=(-0.68, 0.65, -0.82, 0.77),
            frameColor=(0.07, 0.07, 0.11, 1),
            scrollBarWidth=0.04,
            pos=(0.06, 0, 0),
            verticalScroll_frameColor=BTN_DEF,
            verticalScroll_thumb_frameColor=ACCENT,
            verticalScroll_relief=1,
            horizontalScroll_frameSize=(0, 0, 0, 0),
        )

        # Apply / Update / Clear buttons sit below the scroll area.
        DirectButton(
            text="Apply  [Enter]",
            text_scale=0.038, text_fg=T_BRIGHT,
            frameSize=(-0.19, 0.19, -0.039, 0.055),
            frameColor=BTN_GREEN, relief=1,
            pos=(-0.38, 0, -0.876), command=self._apply,
        )
        self._update_btn = DirectButton(
            text="Update  [U]",
            text_scale=0.038, text_fg=T_BRIGHT,
            frameSize=(-0.19, 0.19, -0.039, 0.055),
            frameColor=BTN_DEF, relief=1,
            pos=(0.06, 0, -0.876), command=self._update_slot,
            state=DGG.DISABLED,
        )
        DirectButton(
            text="Clear All  [C]",
            text_scale=0.038, text_fg=T_BRIGHT,
            frameSize=(-0.19, 0.19, -0.039, 0.055),
            frameColor=BTN_RED, relief=1,
            pos=(0.50, 0, -0.876), command=self._clear,
        )

        # ── RIGHT PANEL: playback + chain list ───────────────────────────────
        # Panel background spans x: 0.82 → 1.60  (width 0.78, flush with centre)
        DirectFrame(
            frameSize=(0, 0.78, -0.97, 0.90),
            frameColor=PANEL, pos=(0.82, 0, 0),
        )

        RX = 1.21   # centre x of right panel

        OnscreenText(
            text="Playback", pos=(RX, 0.82),
            scale=0.049, fg=T_DIM, align=TextNode.ACenter,
        )
        DirectButton(
            text="Play / Pause  [Space]",
            text_scale=0.041, text_fg=T_BRIGHT,
            frameSize=(-0.31, 0.31, -0.037, 0.053),
            frameColor=BTN_BLUE, relief=1,
            pos=(RX, 0, 0.68), command=self._play_pause,
        )
        DirectButton(
            text="Stop  [S]",
            text_scale=0.041, text_fg=T_BRIGHT,
            frameSize=(-0.31, 0.31, -0.037, 0.053),
            frameColor=BTN_ORG, relief=1,
            pos=(RX, 0, 0.55), command=self._stop,
        )

        OnscreenText(
            text="Volume", pos=(RX, 0.41),
            scale=0.043, fg=T_DIM, align=TextNode.ACenter,
        )
        self._vol_lbl = OnscreenText(
            text="1.00", pos=(RX, 0.32),
            scale=0.039, fg=T_BRIGHT, align=TextNode.ACenter,
        )
        self._vol_sl = DirectSlider(
            range=(0, 1), value=1.0, pageSize=0.05,
            scale=0.30, pos=(RX, 0, 0.21),
            command=self._set_volume,
        )

        OnscreenText(
            text="Active Chain  (click to edit)",
            pos=(RX, 0.04),
            scale=0.040, fg=T_DIM, align=TextNode.ACenter,
        )

        self._status_lbl = OnscreenText(
            text="stopped", pos=(RX, -0.77),
            scale=0.035, fg=T_DIM, align=TextNode.ACenter,
        )

        self.taskMgr.add(self._tick_status, "tick_status")

    # -----------------------------------------------------------------------
    # Effect selection
    # -----------------------------------------------------------------------

    def _select(self, idx):
        self._sel_idx = idx
        self._editing_slot = None
        self._update_btn["state"]      = DGG.DISABLED
        self._update_btn["frameColor"] = BTN_DEF

        name = EFFECTS[idx][0]
        for i, btn in enumerate(self._btns):
            btn["frameColor"] = ACCENT if i == idx else BTN_DEF

        self._effect_title.setText(name)
        self._rebuild_sliders(idx)

    def _rebuild_sliders(self, idx):
        for widget, lbl, *_ in self._widgets:
            widget.destroy()
            lbl.destroy()
        self._widgets.clear()

        params  = EFFECTS[idx][2]
        slot_h  = 0.120          # comfortable fixed spacing
        y0      = 0.63
        canvas  = self._scroll.getCanvas()

        # Resize canvas to fit all params, then scroll back to top.
        canvas_bottom = min(-0.82, y0 - (len(params) - 1) * slot_h - 0.15)
        self._scroll['canvasSize'] = (-0.68, 0.65, canvas_bottom, 0.77)
        self._scroll.verticalScroll['value'] = 0

        for i, param in enumerate(params):
            label_text = param[0]
            y = y0 - i * slot_h

            if isinstance(param[2], list):
                # ── Radio-button group: (label, default_value, [(label, value)]) ──
                default_val, choices = param[1], param[2]

                lbl = DirectLabel(
                    parent=canvas,
                    text=label_text,
                    text_scale=0.034,
                    text_fg=T_DIM,
                    text_align=TextNode.ALeft,
                    frameColor=(0, 0, 0, 0),
                    pos=(-0.56, 0, y + 0.065),
                )
                widget = _RadioGroup(choices, default_val, y, parent=canvas)
                self._widgets.append((widget, lbl, choices, int))

            else:
                # ── Float slider: (label, min, max, default[, cast]) ────────────
                pmin, pmax, default = param[1], param[2], param[3]
                cast = param[4] if len(param) > 4 else float

                lbl = DirectLabel(
                    parent=canvas,
                    text=f"{label_text}: {default:.4g}",
                    text_scale=0.034,
                    text_fg=T_BRIGHT,
                    text_align=TextNode.ALeft,
                    frameColor=(0, 0, 0, 0),
                    pos=(-0.56, 0, y + 0.050),
                )
                widget = DirectSlider(
                    parent=canvas,
                    range=(pmin, pmax),
                    value=default,
                    pageSize=(pmax - pmin) / 20.0,
                    scale=0.40,
                    pos=(0.02, 0, y),
                    command=self._slider_moved,
                    extraArgs=[i, label_text, lbl],
                )
                self._widgets.append((widget, lbl, None, cast))

    def _slider_moved(self, idx, label_text, lbl):
        val = self._widgets[idx][0]["value"]
        lbl["text"] = f"{label_text}: {val:.4g}"

    # -----------------------------------------------------------------------
    # Filter chain
    # -----------------------------------------------------------------------

    def _apply(self):
        name, method = EFFECTS[self._sel_idx][:2]
        vals = []
        for widget, _, choices, cast in self._widgets:
            if choices is not None:
                vals.append(widget.get())        # _RadioGroup.get() returns int
            else:
                vals.append(cast(widget["value"]))
        self._chain.append((name, method, vals))
        # Apply always rebuilds the full chain.
        self._push_filters()
        # Exit edit mode (Apply adds a new slot, never overwrites).
        self._editing_slot = None
        self._update_btn["state"]      = DGG.DISABLED
        self._update_btn["frameColor"] = BTN_DEF
        self._rebuild_chain_buttons()

    def _clear(self):
        self._chain.clear()
        self._editing_slot = None
        self._update_btn["state"]      = DGG.DISABLED
        self._update_btn["frameColor"] = BTN_DEF
        if self.sfxManagerList:
            self.sfxManagerList[0].configure_filters(FilterProperties())
        self._rebuild_chain_buttons()

    def _push_filters(self):
        fp = FilterProperties()
        for _, method, vals in self._chain:
            getattr(fp, method)(*vals)
        if self.sfxManagerList:
            self.sfxManagerList[0].configure_filters(fp)

    def _load_vals_into_widgets(self, vals):
        """Override freshly-created widgets with saved chain-slot values."""
        params = EFFECTS[self._sel_idx][2]
        for i, (widget, lbl, choices, cast) in enumerate(self._widgets):
            if i >= len(vals):
                break
            val = vals[i]
            if choices is not None:          # _RadioGroup
                widget.set(val)
            else:                            # DirectSlider
                widget["value"] = val
                lbl["text"] = f"{params[i][0]}: {val:.4g}"

    def _select_chain_slot(self, i):
        """Click handler for an active-chain slot button."""
        name, method, vals = self._chain[i]
        eff_idx = next(j for j, e in enumerate(EFFECTS) if e[1] == method)
        self._select(eff_idx)              # rebuilds widgets with defaults; resets editing_slot
        self._load_vals_into_widgets(vals)
        self._editing_slot = i
        self._update_btn["state"]      = DGG.NORMAL
        self._update_btn["frameColor"] = BTN_BLUE
        self._rebuild_chain_buttons()      # re-draw to highlight selected slot

    def _update_slot(self):
        """Update the selected chain slot in-place (no FMOD chain rebuild)."""
        if self._editing_slot is None:
            return
        name, method, _ = self._chain[self._editing_slot]
        vals = [cast(w.get() if ch is not None else w["value"])
                for w, _, ch, cast in self._widgets]
        self._chain[self._editing_slot] = (name, method, vals)

        fp = FilterProperties()
        for _, m, v in self._chain:
            getattr(fp, m)(*v)

        mgr = self.sfxManagerList[0] if self.sfxManagerList else None
        if mgr:
            ok = mgr.update_filters(fp)
            if not ok:
                mgr.configure_filters(fp)  # fallback: structure mismatch

        self._rebuild_chain_buttons()

    def _rebuild_chain_buttons(self):
        """Recreate the clickable chain-slot buttons in the right panel."""
        for b in self._chain_btns:
            b.destroy()
        self._chain_btns.clear()

        RX     = 1.21
        slot_h = 0.072
        y0     = -0.08   # just below "Active Chain" header

        for i, (name, _, _) in enumerate(self._chain):
            is_sel = (i == self._editing_slot)
            btn = DirectButton(
                text=f"{i + 1}. {name}",
                text_scale=0.030, text_fg=T_BRIGHT,
                frameSize=(-0.33, 0.33, -0.022, 0.038),
                frameColor=ACCENT if is_sel else BTN_DEF,
                relief=1,
                pos=(RX, 0, y0 - i * slot_h),
                command=self._select_chain_slot,
                extraArgs=[i],
            )
            self._chain_btns.append(btn)

    # -----------------------------------------------------------------------
    # Playback
    # -----------------------------------------------------------------------

    def _play_pause(self):
        if self._paused:
            self.sfx.setPlayRate(1.0)
            self._paused = False
        elif self.sfx.status() == AudioSound.PLAYING:
            self.sfx.setPlayRate(0.0)
            self._paused = True
        else:
            self.sfx.play()

    def _stop(self):
        self.sfx.stop()
        self._paused = False

    def _set_volume(self):
        val = self._vol_sl["value"]
        self.sfx.setVolume(val)
        self._vol_lbl.setText(f"{val:.2f}")

    def _tick_status(self, task):
        s = self.sfx.status()
        if self._paused:
            txt = "paused"
        elif s == AudioSound.PLAYING:
            t = self.sfx.getTime()
            L = self.sfx.length()
            txt = f"playing  {t:.1f}s / {L:.1f}s"
        else:
            txt = "stopped"
        self._status_lbl.setText(txt)
        return task.cont


DspTester().run()
