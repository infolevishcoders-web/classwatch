# ClassWatch - Classroom Monitoring & Management System

**ClassWatch by Levish Coders**

Based on Veyon 4.11.4 (https://github.com/veyon/veyon)
Copyright (c) 2004-2026 Tobias Junghans / Veyon Solutions

[![license](https://img.shields.io/badge/license-GPLv2-green.svg)](LICENSE)

## What is ClassWatch?

ClassWatch is a professional classroom computer laboratory management system
built on the Veyon open-source framework. It provides monitoring and control
of computers across multiple platforms, supporting digital learning environments,
virtual training, and remote support.

The following features are available in ClassWatch:

  * Overview: monitor all computers in one or multiple locations or classrooms
  * Remote access: view or control computers to watch and support users
  * Demo: broadcast the teacher's screen in realtime (fullscreen/window)
  * Screen lock: draw attention to what matters right now
  * Communication: send text messages to students
  * Start and end lessons: log in and log out users all at once
  * Screenshots: record learning progress and document infringements
  * Programs & websites: launch programs and open website URLs remotely
  * Teaching material: distribute and open documents, images and videos easily
  * Administration: power on/off and reboot computers remotely

## License

Copyright (c) 2004-2026 Tobias Junghans / Veyon Solutions.
Copyright (c) 2026 Levish Coders.

See the file COPYING for the GNU GENERAL PUBLIC LICENSE.
This project is licensed under GPL-2.0.


## Installation and configuration

Please refer to the official ClassWatch Administrator Manual for information
on the installation and configuration of ClassWatch.

For reference, the original Veyon Administrator Manual is available at
https://docs.veyon.io/en/latest/admin/index.html


## Usage

Please refer to the official ClassWatch User Manual for information on how
to use ClassWatch.

For reference, the original Veyon User Manual is available at
https://docs.veyon.io/en/latest/user/index.html


## ClassWatch on Linux

### Downloading sources

First grab the latest sources by cloning the Git repository and fetching all submodules:

	git clone --recursive https://github.com/levishcoders/classwatch.git && cd classwatch


### Installing dependencies

Requirements for Debian-based distributions:

- Build tools: g++ libc6-dev make cmake dpkg-dev
- Qt5: qtbase5-dev qtbase5-private-dev qtbase5-dev-tools qttools5-dev qttools5-dev-tools
- X11: xorg-dev libxtst-dev libfakekey-dev
- libjpeg: libjpeg-dev provided by libjpeg-turbo8-dev or libjpeg62-turbo-dev
- zlib: zlib1g-dev
- OpenSSL: libssl-dev
- PAM: libpam0g-dev
- procps: libprocps-dev
- LZO: liblzo2-dev
- QCA: libqca-qt5-2-dev
- LDAP: libldap2-dev
- SASL: libsasl2-dev

As root you can run

	apt install g++ libc6-dev make cmake qtbase5-dev qtbase5-private-dev \
	            qtbase5-dev-tools qttools5-dev qttools5-dev-tools \
	            xorg-dev libxtst-dev libfakekey-dev libjpeg-dev zlib1g-dev libssl-dev libpam0g-dev \
	            libprocps-dev liblzo2-dev libqca-qt5-2-dev libldap2-dev \
	            libsasl2-dev



Requirements for RedHat-based distributions:

- Build tools: gcc-c++ make cmake rpm-build
- Qt5: qt5-devel qt5-qtbase-private-devel
- X11: libXtst-devel libXrandr-devel libXinerama-devel libXcursor-devel libXrandr-devel libXdamage-devel libXcomposite-devel libXfixes-devel libfakekey-devel
- libjpeg: libjpeg-turbo-devel
- zlib: zlib-devel
- OpenSSL: openssl-devel
- PAM: pam-devel
- procps: procps-devel
- LZO: lzo-devel
- QCA: qca-devel qca-qt5-devel
- LDAP: openldap-devel
- SASL: cyrus-sasl-devel

As root you can run

	dnf install gcc-c++ make cmake rpm-build qt5-devel libXtst-devel libXrandr-devel libXinerama-devel libXcursor-devel \
             libXrandr-devel libXdamage-devel libXcomposite-devel libXfixes-devel libjpeg-turbo-devel zlib-devel \
             openssl-devel pam-devel procps-devel lzo-devel qca-devel qca-qt5-devel openldap-devel cyrus-sasl-devel


### Configuring and building sources

Run the following commands:

	mkdir build
	cd build
	cmake ..
	make -j4

NOTE: If you want to build a .deb or .rpm package for this software, instead of the provided cmake command, you should use:

	cmake -DCMAKE_INSTALL_PREFIX=/usr ..

to install package files in /usr instead of /usr/local.

If some requirements are not fullfilled, CMake will inform you about it and
you will have to install the missing software before continuing.

You can now generate a package (.deb or .rpm depending what system you are in).

For generating a package you can run

	fakeroot make package

Then you'll get something like classwatch_x.y.z_arch.deb or classwatch-x.y.z.arch.rpm

Alternatively you can install the built binaries directly (not recommended for
production systems) by running the following command as root:

	make install

### Arch linux

A PKGBUILD can be found in the [AUR](https://aur.archlinux.org/packages/veyon/).

### PPA

This PPA contains official Veyon packages for Ubuntu suitable for use both on desktop computers and ARM boards (e.g. Raspberry Pi). Even though only packages for LTS releases are available they should work for subsequent non-LTS releases as well.

	sudo add-apt-repository ppa:veyon/stable
	sudo apt-get update

## Join development

If you are interested in ClassWatch, its programming, artwork, testing or something like that, you're welcome to participate in the development of ClassWatch!

For contributions to the original Veyon project, please refer to https://github.com/veyon/veyon/issues.

## Join translation team

ClassWatch and its documentation are translated at the Transifex platform. Please go to https://app.transifex.com/veyon-solutions/veyon and join the corresponding translation team. Please DO NOT submit pull requests for modified translation files since this would require manual Transifex synchronizations on our side.

## More information

* https://levishcoders.com/
* https://github.com/levishcoders/classwatch
* Original Veyon project: https://veyon.io/
