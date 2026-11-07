# ClassWatch — Source Code Documentation

**ClassWatch by Levish Coders** — Classroom Monitoring & Management System

- Website: https://levishcoders.com
- Email: info.levishcoders@gmail.com
- Phone/WhatsApp: +256 743 199 780

ClassWatch is a professional computer-laboratory management platform for
schools, colleges, training centers, and IT labs. It is built on the
Veyon 4.11.4 open-source framework (GPL-2.0) and extends it with
classroom-oriented features, complete Windows/Linux/macOS packaging, and
a polished branded user interface.

---

## 1. Product Components

| Component | Binary | Purpose |
|---|---|---|
| **ClassWatch Master** | `classwatch-master.exe` | Teacher/admin console — monitoring grid, remote view/control, demo broadcast, screen lock, messages, sessions, screenshots, file transfer, power control |
| **ClassWatch Client** | `classwatch-client.exe` | Student-side tray application showing connection status |
| **ClassWatch Service** | `classwatch-service.exe` | Windows service / system daemon that runs the monitoring backend (auto-start) |
| **ClassWatch Server** | `classwatch-server.exe` | Per-session backend process spawned by the service for each logged-on user |
| **ClassWatch Worker** | `classwatch-worker.exe` | Helper process for elevated operations |
| **ClassWatch Configurator** | `classwatch-configurator.exe` | Configuration UI — access control, network, plugins, service settings |
| **ClassWatch CLI / WCLI** | `classwatch-cli.exe`, `classwatch-wcli.exe` | Command-line tools for scripting, deployment, and service control |

## 2. Source Tree Map

```
veyon-main/
├── core/            Shared framework: VeyonCore, configuration, crypto,
│                    connection protocol, service control, UI widgets
│                    (PasswordDialog, AboutDialog, toast notifications)
├── master/          ClassWatch Master application (Qt Widgets)
│   ├── src/         MainWindow, dashboard, panels, computer model
│   └── resources/   Icons, images, Qt resources (master.qrc)
├── client/          ClassWatch Client tray app
├── server/          Session backend (screen capture, input injection)
├── service/         OS service entry point
├── worker/          Elevated worker process
├── cli/             Command-line interface
├── configurator/    Configuration application
├── plugins/         Feature plugins loaded at runtime:
│   ├── authkeys/        Authentication keys
│   ├── demo/            Teacher screen broadcast (demo mode)
│   ├── desktopservices/ Power control, logout/login, session ops
│   ├── filetransfer/    File transfer between master and clients
│   ├── ldap/            LDAP directory integration
│   ├── powercontrol/    Wake-on-LAN / power management
│   ├── remoteaccess/    Remote desktop sessions (VNC/RDP)
│   ├── screenshot/      Screenshot capture
│   ├── screenlock/      Remote screen lock
│   ├── systemusergroups/ Windows/Linux user & group management
│   ├── textmessage/     Popup text messages to students
│   ├── usersessioncontrol/ Session enumeration/control
│   ├── vncserver/       VNC servers: ultravnc-builtin, external,
│   │                    headless, x11vnc-builtin, pipewire
│   ├── platform/        OS abstraction: windows/, linux/, common/
│   └── webapi/          HTTP/WebSocket API (optional)
├── 3rdparty/        Bundled third-party code:
│   ├── ultravnc/        UltraVNC server (Windows)
│   ├── libvncserver/    LibVNC server/client
│   ├── x11vnc/          X11 VNC server
│   ├── qthttpserver/    Qt HTTP server
│   ├── kldap/           KDE LDAP library
│   ├── interception/    Windows keyboard/mouse interception driver
│   ├── ddengine/        Desktop duplication engine (Windows)
│   └── libfakekey/      X11 fake key events
├── nsis/            Windows installer script (classwatch.nsi.in)
├── cmake/           CMake modules (WindowsInstaller, ConfigureFiles, ...)
├── .github/workflows/build.yml   CI: Windows/Linux/macOS builds + installers
└── docs/            This documentation
```

## 3. ClassWatch-Specific Extensions

Added on top of upstream Veyon (Stages 1–4 of the ClassWatch roadmap):

| File(s) | Purpose |
|---|---|
| `core/src/ClassWatchComputer.*` | Computer model extensions (classroom metadata, status) |
| `core/src/ClassWatchSessionManager.*` | Session enumeration & management for multi-session OS (RDP/terminal services) |
| `core/src/ClassWatchMessageManager.*` | Messaging system — private/announcement/broadcast messages, file transfer queue, chat rooms, delivery tracking |
| `core/src/ClassWatchIdGenerator.*` | Unique ID generation for ClassWatch entities |
| `master/src/ClassWatchComputerPanel.*` | "ClassWatch Computers" management panel in Master |
| `master/src/ClassWatchSessionPanel.*` | Session management panel |
| `master/src/ClassWatchMessagePanel.*` | Messaging UI — send message, history, file transfer, chat tabs |
| `master/src/ClassWatchComputerManager*` | Computer manager extension + migration from Veyon format |
| `client/src/ClassWatchClientApp.*` | Branded client application |
| `client/src/ClassWatchClientStatus.*` | Client tray status component |

Master UI was also restyled: compact icon-only panel toolbar with tooltips,
ClassWatch accent colors, dark-mode icon variants.

## 4. Networking & Protocol

- Master connects to clients over TCP using the Veyon RFB-based protocol
  with Veyon authentication extensions (`RfbVeyonAuth`).
- Service listens for master connections; per-user `classwatch-server`
  processes handle screen capture/control inside each session.
- Authentication methods: access-control password (shared secret),
  Windows/Linux system logon, LDAP, auth keys.
- Optional web API (`webapi` plugin) exposes HTTP/WebSocket endpoints.

## 5. Building

### Windows (recommended: MSYS2 UCRT64 + Qt 6)

```bash
pacman -S mingw-w64-ucrt-x86_64-{cmake,make,gcc,qt6-base,qt6-tools,openssl,libvncserver,qca-qt6,openldap,cyrus-sasl,lzo2,nsis}
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release \
      -DWITH_TESTS=OFF -DWITH_BUNDLED_LIBVNC=OFF -DWITH_WEBAPI=OFF \
      -DWITH_PCH=OFF -DWITH_TRANSLATIONS=OFF
cmake --build build --parallel 2
# installer payload + NSIS:
# see .github/workflows/build.yml "Package Windows Installer" step
```

### Linux

```bash
sudo apt install build-essential cmake qt6-base-dev libqt6core5compat6-dev \
  libqt6httpserver6-dev libvncserver-dev libldap-dev libsasl2-dev \
  libprocps-dev libfakekey-dev libxtst-dev xorg-dev libxrandr-dev \
  libxinerama-dev libxcursor-dev libxdamage-dev libxcomposite-dev libxfixes-dev
cmake -B build -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=OFF
cmake --build build
sudo cmake --install build   # .deb via CPack
```

### macOS

```bash
brew install cmake qt openldap cyrus-sasl
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DLDAP_INCLUDE_DIR=/opt/homebrew/include -DLDAP_LIBRARY=/opt/homebrew/lib/libldap.dylib \
  -DLBER_LIBRARY=/opt/homebrew/lib/liblber.dylib
cmake --build build
# .dmg produced by CI packaging step
```

### CI / Prebuilt Packages

GitHub Actions (`.github/workflows/build.yml`) builds all three platforms
on every push to `master` and uploads artifacts:

- `classwatch-windows-installer` — `classwatch-1.0.0.0-win32-setup.exe` (~35 MB, self-contained: all Qt/runtime DLLs, plugins, service, shortcuts)
- `classwatch-linux-packages` — `.deb` package
- `classwatch-macos-package` — `.dmg` disk image

Download from: https://github.com/infolevishcoders-web/classwatch/actions

## 6. Windows Installer Layout

The NSIS installer (`nsis/classwatch.nsi.in`) installs to
`C:\Program Files\ClassWatch` and creates:

- `ClassWatchService` Windows service (auto-start, LocalSystem)
- Start Menu folder "ClassWatch": Master, Configurator, Website, Uninstall
- Desktop shortcut for Master
- Sections: Service (required), Master (default on), Interception driver (default on, for advanced input control)

Payload includes: all `classwatch-*.exe`, `classwatch-core.dll`, 17 feature
plugins, Qt6 runtime + platform/imageformat/style/TLS plugins, QCA crypto
plugin, full MSYS2 runtime DLL closure (ICU, harfbuzz, gnutls, etc.),
UltraVNC hooks, ddengine, interception driver files.

## 7. Configuration

- **Configurator** (`classwatch-configurator.exe`): GUI for all settings —
  access control (password/logon/LDAP/keys), network (ports, firewall),
  service autostart, plugin settings, computer lists.
- **CLI**: `classwatch-cli` (user context) / `classwatch-wcli` (elevated) —
  `service register|unregister|start|stop|status`, `config get|set|import|export|clear`.
- Config stored per-user/per-machine under the ClassWatch config directory.

## 8. License & Commercial Use

ClassWatch is licensed under the **GNU GPL version 2** (see `COPYING`).

- Original Veyon code: Copyright (c) 2004-2026 Tobias Junghans / Veyon Solutions.
- ClassWatch modifications, branding, and new features: Copyright (c) 2026 Levish Coders.

Because ClassWatch is GPL-2.0, anyone who receives the binaries or source
may use, study, modify, and redistribute it under the GPL terms.

**Commercial licensing, customization, OEM branding, training, deployment,
and support contracts are available from Levish Coders** — contact
info.levishcoders@gmail.com or +256 743 199 780.

## 9. Roadmap / Stage Status

| Stage | Scope | Status |
|---|---|---|
| 1 | Foundation & branding | ✅ Done |
| 2 | Master & Client improvements | ✅ Done |
| 3 | Server & multi-session | ✅ Done |
| 4 | Communication (messages, file transfer, chat UI) | ✅ UI done; backend delivery in progress |
| 5 | Audio Classroom | ⏳ Planned |
| 6 | Web Filtering | ⏳ Planned |
| 7 | Timetable & administration | ⏳ Planned |
| 8 | Security, installer & release hardening | 🔄 In progress |
