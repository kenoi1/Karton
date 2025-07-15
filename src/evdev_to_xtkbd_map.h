// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#ifndef EVDEV_TO_XTKBD_MAP_H
#define EVDEV_TO_XTKBD_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#define CODE_MAP_LINUX_TO_XTKBD_LEN 525

extern const unsigned short code_map_linux_to_xtkbd[CODE_MAP_LINUX_TO_XTKBD_LEN];

#ifdef __cplusplus
}
#endif

#endif // EVDEV_TO_XTKBD_MAP_H