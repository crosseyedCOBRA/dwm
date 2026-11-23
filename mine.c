/* My own additions to dwm-flexipatch. dwm.c includes this right after
 * patch/include.c, so everything from dwm.c and the enabled patches is
 * available here. Keeping them in one file keeps upstream merges simple. */

#if BAR_ANYBAR_PATCH
/* Whether a window says it is a dock (_NET_WM_WINDOW_TYPE_DOCK), the type
 * panels and bars set for themselves. */
static int
isdockwindow(Window w)
{
	Atom wtype = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	Atom dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
	Atom real, *types = NULL;
	int format, res = 0;
	unsigned long i, n, extra;

	if (XGetWindowProperty(dpy, w, wtype, 0L, 32L, False, XA_ATOM, &real, &format,
			&n, &extra, (unsigned char **)&types) == Success && types) {
		for (i = 0; i < n; i++)
			if (types[i] == dock)
				res = 1;
		XFree(types);
	}
	return res;
}

/* Which windows anybar takes as bars: the stock check is the window class
 * (altbarclass), but quickshell's bars set no class at all. They are typed
 * as docks, though, so take docks as bars too. */
static int
isaltbar(Window w)
{
	return wmclasscontains(w, altbarclass, "") || isdockwindow(w);
}
#endif // BAR_ANYBAR_PATCH

#if BAR_WINTITLEACTIONS_PATCH
/* Super+Ctrl+N, like Awesome's "restore minimized": bring back the most
 * recently focused minimized window on the current tag(s). */
static void
restoreminimized(const Arg *arg)
{
	Client *c;

	for (c = selmon->stack; c && !(ISVISIBLE(c) && HIDDEN(c)); c = c->snext);
	if (!c)
		return;
	show(c);
	focus(c);
	restack(selmon);
}
#endif // BAR_WINTITLEACTIONS_PATCH

#if SCRATCHPAD_ALT_1_PATCH
/* Super+Shift+S, like the Awesome setup: put the focused window away as a
 * scratchpad, or, if it is the scratchpad currently shown, make it a normal
 * tiled window again. */
static void
scratchpad_toggleclient(const Arg *arg)
{
	Client *c = selmon->sel;

	if (!c)
		return;
	if (c == scratchpad_last_showed) {
		scratchpad_last_showed = NULL;
		if (c->isfloating)
			togglefloating(NULL);
	} else
		scratchpad_hide();
}
#endif // SCRATCHPAD_ALT_1_PATCH
