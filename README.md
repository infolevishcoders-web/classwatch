# ClassWatch — Classroom Monitoring & Management System

**ClassWatch by Levish Coders**

[![license](https://img.shields.io/badge/license-GPLv2-green.svg)](LICENSE)

ClassWatch is a professional classroom computer laboratory management system:
real-time screen monitoring, remote control, lockdown, messaging, file
distribution, web filtering, audio classroom, timetables and administration —
built for schools and training centers on Windows, Linux and macOS.

ClassWatch is derived from the Veyon open-source framework and extends it with
school-focused features, a hardened security model and a commercial licensing
and web platform.

## Features

- **Monitoring** — live thumbnails of every student screen, wired or wireless
- **Remote control & demo** — view or fully control any machine; broadcast the
  teacher screen to the class
- **Student lockdown** — block Command Prompt, PowerShell and Task Manager;
  students cannot quit the client or reconfigure it; every violation is logged
  with local + UTC timestamps and shown as an on-screen warning
- **Messaging** — text announcements to the whole class or individuals
- **Sessions** — log students in/out, schedule lab time, power control
- **Files** — distribute teaching material and collect student work
- **Web filtering, audio classroom, timetable** — built-in school tools
- **Licensing** — signed offline license keys, 7-day free trial, seat limits
- **Administration** — audit logs, security event reporting, multi-location
  computer management

## Components

| Component | Purpose |
|---|---|
| `classwatch-master` | Teacher/admin console (monitoring, control, panels) |
| `classwatch-client` | Student tray application |
| `classwatch-service` | Background service (auto-starts at boot, applies lockdown) |
| `classwatch-configurator` | Configuration tool (password-locked after setup) |
| `classwatch-server` / `classwatch-worker` | Multi-session server components |
| `classwatch-wcli` | Command-line interface |
| `website/` | Netlify frontend + Render API + Supabase schema (accounts, subscriptions, payments) |

See [docs/CLASSWATCH.md](docs/CLASSWATCH.md) for the complete source-code
documentation for buyers and developers.

## Quick start

### Teachers (Windows — recommended)

1. Download `classwatch-*-setup.exe` from
   [GitHub Actions artifacts](https://github.com/infolevishcoders-web/classwatch/actions)
   or the ClassWatch website.
2. Run as administrator → installs Master + Service + Client.
3. Open **ClassWatch Configurator** → set the access-control password.
4. Open **ClassWatch Master** → activate your license key (or use the
   7-day free trial) → add student computers by IP/hostname.

### Student PCs

Run the same installer and **uncheck "ClassWatch Master"** on the components
page (or pass `/NoMaster` for silent installs). The Master console is never
installed on student machines.

### Linux

```bash
sudo dpkg -i classwatch_*_amd64.deb
```

### macOS

Open the `.dmg` and drag ClassWatch to Applications.

## Building from source

Requirements: CMake ≥ 3.20, Qt 6 (Core, Widgets, Network, Sql, WebSockets),
a C++20 compiler, and platform SDKs. Windows builds use MSYS2 UCRT64 + NSIS;
CI builds are defined in `.github/workflows/build.yml`.

```bash
git clone --recursive https://github.com/infolevishcoders-web/classwatch.git
cd classwatch
mkdir build && cd build
cmake ..
cmake --build . --parallel
```

See `docs/CLASSWATCH.md` for per-platform details.

## Website & accounts

The `website/` folder contains the complete web platform:

- **Frontend** (Netlify): landing, UGX pricing, trial signup, login
  (email/password, email OTP, Google, Apple), dashboard, admin console
- **Backend** (Render): `backend/server.js` — auth, subscriptions,
  device limits, Pawapat payment webhook, admin API
- **Database** (Supabase): `supabase/schema.sql`

Deployment steps are in [website/README.md](website/README.md).

## License & copyright

- Original Veyon code: Copyright (c) 2004-2026 Tobias Junghans / Veyon Solutions.
- ClassWatch modifications and branding: Copyright (c) 2026 Levish Coders.
- Licensed under GPL-2.0 — see [COPYING](COPYING).
- For commercial licensing, customization or deployment support:

**Levish Coders**
- Website: https://levishcoders.com
- Email: info.levishcoders@gmail.com
- Phone/WhatsApp: +256 743 199 780

## Acknowledgements

ClassWatch builds upon Veyon (https://veyon.io), created by Tobias Junghans
and contributors. All upstream copyright notices are preserved in the source
tree as required by GPL-2.0.
