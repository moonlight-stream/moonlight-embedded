# Moonlight Embedded

 [![Build](https://img.shields.io/github/actions/workflow/status/moonlight-stream/moonlight-embedded/build.yml?branch=master)](https://github.com/moonlight-stream/moonlight-embedded/actions/workflows/build.yml?query=branch%3Amaster) [Nightly Build Downloads](https://nightly.link/moonlight-stream/moonlight-embedded/workflows/build/master)

Moonlight Embedded is an open source client for [Sunshine](https://github.com/LizardByte/Sunshine) and NVIDIA GameStream for embedded Linux systems, like Raspberry Pi, CuBox-i and ODROID. Moonlight allows you to stream your full collection of games and applications from your PC to other devices to play them remotely.

Moonlight also has [PC](https://github.com/moonlight-stream/moonlight-qt), [Android](https://github.com/moonlight-stream/moonlight-android), and [iOS](https://github.com/moonlight-stream/moonlight-ios) clients.

## Documentation

More information about installing and running Moonlight Embedded is available on the [wiki](https://github.com/moonlight-stream/moonlight-embedded/wiki).

## Bugs

Please check the wiki and old bug reports before submitting a new bug report.

Bugs can be reported to the [issue tracker](https://github.com/moonlight-stream/moonlight-embedded/issues).

## See also

[Moonlight-common-c](https://github.com/moonlight-stream/moonlight-common-c) is the shared codebase between different Moonlight implementations

## Contribute

1. Fork us
2. Write code
3. Send Pull Requests

## Native GameStream microphone

On builds with SDL 2.0.5 or later and a working default SDL recording device,
pass `-microphone` explicitly for the stream:

```sh
moonlight stream -microphone HOST
```

The option applies to this invocation only. Configuration files cannot enable
it. View-only mode disables forwarding. The client reports when forwarding
starts or encounters an error. End the stream to stop microphone capture.
A device or negotiation failure leaves ordinary streaming available.

Use an NVIDIA GameStream host with the compatible
[Windows host setup](https://github.com/Bargs/native-gamestream-microphone-host)
on a trusted private IPv4 network or secured tunnel. The native legacy CBC mode
has no packet authentication. IPv6 and Sunshine microphone forwarding are
outside this implementation. Device and live host delivery still need testing.
