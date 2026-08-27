# ELAN 04f3:0c4b fingerprint LED bridge

This repository contains a small `libusb` helper that controls the integrated
fingerprint/power-button indicator on ELAN readers with USB ID `04f3:0c4b`.
The helper is wired into `fprintd` with a systemd drop-in, so the indicator
turns on when `fprintd` starts an authentication session and turns off when the
service exits.

## Why this is needed

On the Lenovo Legion S7 15ACH6, the stock libfprint driver can detect the
reader but enrollment ends with `enroll-disconnected`. The
[`libfprint-tod` ELAN workaround](https://github.com/Abishek-Pechiappan/libfprint-elan-04f3-0c4b-tod/)
makes enrollment and verification work, but its proprietary TOD backend does
not expose the power-button indicator as a Linux LED device.

This project is only the indicator bridge. It does **not** replace the TOD
fingerprint backend and does not include any proprietary Lenovo binary.

The two commands used by the helper are the commands defined by the open-source
ELAN driver:

| Action | Bytes |
| --- | --- |
| Indicator on | `40 31` |
| Indicator off | `00 0b` |

## Tested hardware

| Laptop | Reader | Enrollment/verification | Indicator |
| --- | --- | --- | --- |
| Lenovo Legion S7 15ACH6 | ELAN `04f3:0c4b` | Working with TOD backend | Green while authenticating |

Other laptops may use a different ELAN protocol. Treat this as hardware-
specific until independently tested.

## Requirements

- Linux with `fprintd` and systemd
- `libusb-1.0` development files (`libusb` and `pkgconf` on Arch)
- A working fingerprint stack for the reader. For this device, that means
  `libfprint-tod` and the matching `libfprint-2-tod1-elan` package.
- Root access for installation and for the systemd service to access the USB
  interface

Install the TOD stack by following the existing
[ELAN 04f3:0c4b workaround](https://github.com/Abishek-Pechiappan/libfprint-elan-04f3-0c4b-tod/).
Do not download or redistribute proprietary driver files from this repository.

## Build

On Arch Linux:

```sh
sudo pacman -S --needed base-devel libusb pkgconf
make
```

The resulting binary is `build/elan-led`.

## Test manually

Stop `fprintd` before opening the USB interface directly:

```sh
sudo systemctl stop fprintd
sudo ./build/elan-led on
sudo ./build/elan-led off
sudo systemctl start fprintd
```

Expected output is `sent 40 31 (on)` and `sent 00 0b (off)`.

## Install automatic indicator control

```sh
sudo install -Dm755 build/elan-led /usr/local/libexec/elan-led
sudo install -Dm644 systemd/fprintd.service.d/led.conf \
  /etc/systemd/system/fprintd.service.d/led.conf
sudo systemctl daemon-reload
```

Then test an authentication request, for example:

```sh
sudo -k
sudo true
```

The helper is best-effort. If the indicator cannot be accessed, the `fprintd`
service continues to work and fingerprint authentication is not blocked.

## Optional udev rule

The `udev/60-libfprint-2-tod1-elan.rules` file is an optional override for the
TOD backend. Install it only if your TOD setup does not already provide an
equivalent rule:

```sh
sudo install -Dm644 udev/60-libfprint-2-tod1-elan.rules \
  /etc/udev/rules.d/60-libfprint-2-tod1-elan.rules
sudo udevadm control --reload-rules
sudo udevadm trigger --attr-match=idVendor=04f3 --attr-match=idProduct=0c4b
sudo systemctl restart fprintd
```

## Uninstall / rollback

```sh
sudo rm -f /etc/systemd/system/fprintd.service.d/led.conf
sudo rm -f /usr/local/libexec/elan-led
sudo systemctl daemon-reload
```

Remove the optional udev rule only if it was installed by this project. The
fingerprint backend and its packages are managed separately.

## Limitations and safety

- Tested only with ELAN `04f3:0c4b` on a Lenovo Legion S7 15ACH6.
- The helper needs exclusive access to USB interface 0. It normally runs from
  `ExecStartPre`/`ExecStopPost`, before and after `fprintd` owns the device.
- There is no kernel LED class device involved; this is a userspace protocol
  bridge.
- This repository contains no Lenovo proprietary firmware or shared library.
  Review any third-party TOD package before installing it.

## License

The helper and packaging files are MIT-licensed. Third-party fingerprint
backends remain under their own licenses.
