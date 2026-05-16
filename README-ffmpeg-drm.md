# FFmpeg V4L2 Request + DRM PRIME Backend for moonlight-embedded

This adds a hardware-accelerated video backend for embedded Linux devices
that use the **V4L2 stateless Request API** for hardware decode and **KMS/DRM
atomic modesetting** for display. Tested on RK3399 (rkvdec) running LibreELEC.

## How it works

```
Network → moonlight-embedded decode thread
              ↓
         FFmpeg (codec: h264/hevc)
         hwaccel: AV_HWDEVICE_TYPE_DRM
         get_format → AV_PIX_FMT_DRM_PRIME
              ↓
         V4L2 Request API (rkvdec / hantro / cedrus)
         /dev/media0 + /dev/video2
              ↓
         AVDRMFrameDescriptor (NV12 dma-buf)
              ↓  av_frame_clone (holds buffer ref across vsync)
         Display thread (separate from decode thread)
              ↓
         drmModeAtomicCommit (blocking, overlay plane)
         card1 / Rockchip VOP / KMS atomic
```

The decode thread is non-blocking — it pushes frames to a single-slot queue
and immediately returns. The display thread blocks on vsync, ensuring the
network/control pipeline is never stalled by display timing.

`av_frame_clone` is used to hold a reference to the DRM PRIME buffer while it
is being scanned out by the display controller. Releasing the frame before the
next vsync causes the buffer to be returned to the V4L2 pool while it is still
on-screen, producing black flicker on every other frame.

On startup, all active KMS planes on the CRTC except the video overlay are
blanked (FB_ID=0 AND CRTC_ID=0 in the same atomic commit) to hide the Linux
framebuffer console.

## Hardware compatibility

Any device where:
- FFmpeg can use `AV_HWDEVICE_TYPE_DRM` with a V4L2 Request API decoder
- The kernel exposes a DRM/KMS atomic interface

Known working:
- **RK3399** (rkvdec) — H.264, HEVC via `/dev/media0` + `/dev/video2`
- Should work on any device supported by LibreELEC's patched FFmpeg with
  V4L2 Request hwaccel (RK3588, Allwinner, Amlogic with appropriate kernel)

The backend probes `/dev/media*` and `/dev/video*` at runtime to find a
device that supports the requested pixel format — no hardcoded paths.

## Build

Requires: `libdrm`, `libavcodec`, `libavutil` (with DRM hwaccel support)

```bash
cmake -DENABLE_FFMPEG_DRM=ON ...
```

CMakeLists.txt checks for `DRM_LIBRARY` and `AVCODEC_FOUND` before enabling
the backend. If the system FFmpeg lacks DRM hwaccel support, it will be
skipped silently.

## Performance (RK3399, 1080p60 HEVC @ 20 Mbps)

| Metric | Result |
|---|---|
| Decode time | ~2.7 ms average (rkvdec) |
| Frame drops | 0.00% |
| Network latency | 1 ms |

CPU load during streaming is minimal — all decode is handled by rkvdec,
display is a single drmModeAtomicCommit per frame.

## Files changed

- `src/video/ffmpeg_drm.c` — new backend (this file)
- `src/platform.h` — added `FFMPEG_DRM` to platform enum
- `src/platform.c` — dispatch for `FFMPEG_DRM` in `platform_check`,
  `platform_get_video`, `platform_prefers_codec`, `platform_name`
- `src/video/video.h` — extern declaration for `decoder_callbacks_ffmpeg_drm`
- `CMakeLists.txt` — `ENABLE_FFMPEG_DRM` option, updated `FATAL_ERROR` guard
  to not require legacy backends when `FFMPEG_DRM` is available

## Selecting the backend

```bash
moonlight stream -platform ffmpeg_drm -app "Desktop"
```

Or set it as the default in `moonlight.conf`:
```ini
[Moonlight]
platform = ffmpeg_drm
```
