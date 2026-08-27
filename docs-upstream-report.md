# Upstream report template

## Hardware

- Laptop: Lenovo Legion S7 15ACH6
- Fingerprint reader: ELAN `04f3:0c4b`
- Desktop: Omarchy/Hyprland on Arch Linux

## Symptoms

The stock libfprint driver detects the reader but enrollment ends with
`enroll-disconnected`. The TOD stack makes enrollment and verification work,
but the integrated power-button indicator is not exposed through
`/sys/class/leds`.

## Working setup

- `libfprint-tod` plus Lenovo's `libfprint-2-tod1-elan` backend
- A udev override selecting `Elan Fingerprint Sensor`
- An optional libusb helper that sends the ELAN indicator commands around the
  `fprintd` service lifecycle

See the project README for installation and rollback instructions.
