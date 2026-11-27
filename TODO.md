# My dwm: what's left before it replaces Awesome

Awesome stays the daily driver until dwm covers the everyday basics.
Work through these one at a time on dwm days.

## Done
- [x] The quickshell bar as dwm's bar (BAR_ANYBAR + IPC; mine.c takes dock
      windows as bars, dwm-bar-state in the nixos repo feeds the bar dwm's
      tags and layout)
- [x] Minimize/restore, maximize, always on top, Awesome-style scratchpads
- [x] Keybinding sheet (Super+F1) and main menu (Super+Shift+W, right-click
      the desktop) as rofi scripts in the nixos repo
- [x] A first patch of my own (mine.c: docks as bars, restore minimized,
      scratchpad toggle)
- [x] Float the same apps Awesome floats (window role rules on for the
      Thunderbird/Chromium ones)
- [x] Awesome-style scratchpads: Super+S shows all of them together,
      centered at 60% of the focused monitor (mine.c)
- [x] Drag a tiled window into a new spot (placemouse on Super+drag)
- [x] Per-monitor layouts: dwindle 50/50 on the side-by-side monitors,
      my own "rows" layout on the rotated one (mine.c)
- [x] Cursor theme (Bibata) for dwm's own cursors (drw.c via Xcursor)

## Next
- [ ] Try the bar on all three monitors: tag clicks on the rotated
      DisplayPort-1 and on DisplayPort-2, the bar hiding for fullscreen games
- [ ] Check every keybinding against the Awesome rc.lua in daily use
- [ ] Check window rules (Steam, Vesktop, YouTube Music) land on the right
      monitor and tag
- [ ] Check wallpaper and color changes (awesome-colors) restart dwm cleanly

## Upstream ideas (first contributions)
- [ ] patch/ipc/ipc.c logged every IPC connection to stderr with fputs instead
      of its DEBUG() macro: send the one-line fix to bakkeby/dwm-flexipatch
- [ ] Anybar only recognizes bars by window class; quickshell sets none.
      Taking _NET_WM_WINDOW_TYPE_DOCK windows as bars (mine.c) could be an
      option for the anybar patch
