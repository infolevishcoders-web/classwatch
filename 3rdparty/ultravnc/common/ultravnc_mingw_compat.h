// This file is part of UltraVNC
// https://github.com/ultravnc/UltraVNC
// https://uvnc.com/
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// SPDX-FileCopyrightText: Copyright (C) 2002-2025 UltraVNC Team Members. All Rights Reserved.
//
// mingw-w64 headers no longer define the legacy GWL_*/DWL_* constants.
// The GWLP_*/DWLP_* replacements are numerically identical on both
// 32-bit and 64-bit Windows, so we provide the legacy names here.

#ifndef ULTRAVNC_MINGW_COMPAT_H
#define ULTRAVNC_MINGW_COMPAT_H

#ifndef GWL_WNDPROC
#define GWL_WNDPROC (-4)
#endif
#ifndef GWL_HINSTANCE
#define GWL_HINSTANCE (-6)
#endif
#ifndef GWL_STYLE
#define GWL_STYLE (-16)
#endif
#ifndef GWL_EXSTYLE
#define GWL_EXSTYLE (-20)
#endif
#ifndef GWL_USERDATA
#define GWL_USERDATA (-21)
#endif
#ifndef DWL_MSGRESULT
#define DWL_MSGRESULT 0
#endif

#ifndef GWLP_WNDPROC
#define GWLP_WNDPROC (-4)
#endif
#ifndef GWLP_HINSTANCE
#define GWLP_HINSTANCE (-6)
#endif
#ifndef GWLP_STYLE
#define GWLP_STYLE (-16)
#endif
#ifndef GWLP_EXSTYLE
#define GWLP_EXSTYLE (-20)
#endif
#ifndef GWLP_USERDATA
#define GWLP_USERDATA (-21)
#endif
#ifndef DWLP_MSGRESULT
#define DWLP_MSGRESULT 0
#endif
#ifndef DWLP_DLGPROC
#define DWLP_DLGPROC 0
#endif

#endif // ULTRAVNC_MINGW_COMPAT_H
