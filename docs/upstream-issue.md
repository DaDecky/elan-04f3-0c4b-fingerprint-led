# Cross-reference issue body

The TOD workaround also works on a Lenovo Legion S7 15ACH6 with ELAN
`04f3:0c4b`, but the integrated power-button indicator is not exposed as a
Linux LED device. Enrollment and verification succeed with the TOD backend;
only the visual indicator is missing.

I made a small userspace `libusb` helper that sends the ELAN indicator commands
around the `fprintd` service lifecycle. The helper is documented here:

https://github.com/DaDecky/elan-04f3-0c4b-fingerprint-led

Observed behavior:

- `40 31` turns the indicator on (green on this laptop).
- `00 0b` turns it off.
- The helper is best-effort and does not block fingerprint authentication.
- No proprietary Lenovo driver binary is redistributed.

Would you consider adding this as an optional LED section to the workaround
documentation? The helper is intentionally separate from the proprietary TOD
backend.
