/* SPDX-License-Identifier: AGPL-3.0-only */
#include "utils/x11.h"

bool get_screen_resolution(size_t &width, size_t &height) {

    Display *d = XOpenDisplay(NULL);
    if (d == NULL) {
        return false;
    }

    Screen *s = DefaultScreenOfDisplay(d);
    if (s == NULL) {
        return false;
    }

    width = s->width;
    height = s->height;

    XCloseDisplay(d);

    return true;
}

void switch_to_desktop(size_t desktop) {

    std::string wmctrl_cmd("wmctrl -s ");
    wmctrl_cmd += std::to_string(desktop);

    run(wmctrl_cmd);
}

int divRoundClosest(const int n, const int d) { return ((n < 0) == (d < 0)) ? ((n + d / 2) / d) : ((n - d / 2) / d); }