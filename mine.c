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
/* Scratchpads like the Awesome setup. The scratchpad patch provides the
 * hiding (a tag bit no view ever shows, SCRATCHPAD_MASK); this replaces its
 * one-at-a-time show/hide with Awesome's behavior:
 *   Super+S        all scratchpad windows come up together on the focused
 *                  monitor, floating on top, centered at 60% of the screen;
 *                  Super+S again puts them all away.
 *   Super+Shift+S  makes the focused window a scratchpad, or a scratchpad
 *                  a normal tiled window again. */
#define MAXSCRATCHPADS 16
static Client *padwins[MAXSCRATCHPADS];
static int padwins_shown;

/* Forget scratchpad windows that have been closed since. */
static void
scratchpad_prune(void)
{
	Monitor *m;
	Client *c;
	int i, alive;

	for (i = 0; i < MAXSCRATCHPADS; i++) {
		if (!padwins[i])
			continue;
		alive = 0;
		for (m = mons; m && !alive; m = m->next)
			for (c = m->clients; c && !alive; c = c->next)
				alive = (c == padwins[i]);
		if (!alive)
			padwins[i] = NULL;
	}
}

static int
scratchpad_index(Client *c)
{
	int i;

	for (i = 0; i < MAXSCRATCHPADS; i++)
		if (padwins[i] == c)
			return i;
	return -1;
}

/* Bring one scratchpad window up on the focused monitor. */
static void
scratchpad_place(Client *c)
{
	Monitor *m = selmon;
	int w = m->ww * 6 / 10, h = m->wh * 6 / 10;

	if (c->mon != m)
		sendmon(c, m);
	c->tags = m->tagset[m->seltags];
	c->isfloating = 1;
	c->alwaysontop = 1;
	resize(c, m->wx + (m->ww - w) / 2, m->wy + (m->wh - h) / 2,
		w - 2 * c->bw, h - 2 * c->bw, 0);
}

static void
scratchpad_toggleall(const Arg *arg)
{
	Client *last = NULL;
	int i;

	scratchpad_prune();
	padwins_shown = !padwins_shown;
	for (i = 0; i < MAXSCRATCHPADS; i++) {
		if (!padwins[i])
			continue;
		if (padwins_shown) {
			scratchpad_place(padwins[i]);
			last = padwins[i];
		} else
			padwins[i]->tags = SCRATCHPAD_MASK;
	}
	arrange(NULL);
	focus(last);
	restack(selmon);
}

static void
scratchpad_toggleclient(const Arg *arg)
{
	Client *c = selmon->sel;
	int i;

	if (!c)
		return;
	scratchpad_prune();
	if ((i = scratchpad_index(c)) >= 0) {
		/* Back to a normal tiled window. */
		padwins[i] = NULL;
		c->alwaysontop = 0;
		c->isfloating = 0;
		arrange(c->mon);
		return;
	}
	if ((i = scratchpad_index(NULL)) < 0)
		return; /* all slots taken */
	padwins[i] = c;
	if (padwins_shown)
		scratchpad_place(c);
	else
		c->tags = SCRATCHPAD_MASK;
	arrange(NULL);
	focus(NULL);
	restack(selmon);
}
#endif // SCRATCHPAD_ALT_1_PATCH

#if VANITYGAPS_PATCH
/* "rows", for the rotated (portrait) monitor: two rows, one window in
 * each; the 3rd window splits the bottom one in half, the 4th splits the
 * top one, and so on, alternating. Windows never jump between rows when
 * another one opens:
 *   2: [1] / [2]    3: [1] / [2|3]    4: [1|4] / [2|3]    5: [1|4] / [2|3|5] */
static int
rows_istop(unsigned int i)
{
	return i == 0 || (i >= 3 && i % 2 == 1);
}

static void
rows(Monitor *m)
{
	unsigned int i, n, ntop = 0, nbot = 0, ti = 0, bi = 0, cnt, k;
	int oh, ov, ih, iv, x, y, w, h, th, bh, cw, cx, top;
	Client *c;

	getgaps(m, &oh, &ov, &ih, &iv, &n);
	if (n == 0)
		return;
	for (i = 0; i < n; i++) {
		if (rows_istop(i))
			ntop++;
		else
			nbot++;
	}

	x = m->wx + ov;
	y = m->wy + oh;
	w = m->ww - 2 * ov;
	h = m->wh - 2 * oh;
	th = nbot ? (h - ih) / 2 : h; /* top row height */
	bh = h - ih - th;

	for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
		top = rows_istop(i);
		cnt = top ? ntop : nbot;
		k = top ? ti++ : bi++;
		cw = (w - iv * (int)(cnt - 1)) / (int)cnt;
		cx = x + (int)k * (cw + iv);
		if (k == cnt - 1)
			cw = x + w - cx; /* the last one takes the rounding rest */
		resize(c, cx, top ? y : y + th + ih, cw - 2 * c->bw,
			(top ? th : bh) - 2 * c->bw, 0);
	}
}
#endif // VANITYGAPS_PATCH
