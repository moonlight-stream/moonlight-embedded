# Moonlight Embedded 综合文档

> 更新时间：2026-07-26
> 目标平台：Rockchip RK3568

---

## 目录

1. [moonlight-embedded vs moonlight-qt 对比](#1-moonlight-embedded-vs-moonlight-qt-对比)
2. [编译指南](#2-编译指南)
3. [Sunshine 分辨率自动切换](#3-sunshine-分辨率自动切换)
4. [YUV444 支持可行性分析](#4-yuv444-支持可行性分析)
5. [实时统计模块说明](#5-实时统计模块说明)
6. [文件结构](#6-文件结构)

---

## 1. moonlight-embedded vs moonlight-qt 对比

### 1.1 项目定位

| 项目 | 定位 |
|------|------|
| **moonlight-embedded** | 面向嵌入式 Linux 系统的轻量级命令行客户端，专为 SoC 类设备（RPi、ODROID、Rockchip）优化 |
| **moonlight-qt** | 面向桌面系统的全功能图形化客户端，跨平台支持（Windows、macOS、Linux） |

### 1.2 核心架构差异

| 维度 | moonlight-embedded | moonlight-qt |
|------|-------------------|-------------|
| **编程语言** | C (C99) | C++ (Qt6/Qt5) |
| **用户界面** | 命令行 (`moonlight stream -1080 -app Steam`) | 完整 Qt 图形界面 |
| **视频解码** | 平台原生 API（MPP、MMAL、V4L2 VPU） | FFmpeg（通过 hwaccel 使用 V4L2/VAAPI/Vulkan） |
| **显示输出** | DRM/KMS 直接扫描输出，无窗口系统 | Vulkan/OpenGL ES/DRM PRIME |
| **音频输出** | ALSA / PulseAudio / OSS / OMX | Qt Multimedia / ALSA / PulseAudio |
| **输入处理** | libevdev 直接读取 `/dev/input/event*` | Qt 事件 + SDL2 游戏控制器 API |
| **窗口系统** | 不需要 X11/Wayland | 需要 X11/Wayland（或 Qt EGLFS） |
| **CEC 遥控** | 支持 (libcec) | 不支持 |

### 1.3 软件栈对比

```
moonlight-embedded:

  [Moonlight CLI] → [libgamestream] → [Platform Backend]
                        ↓                       ↓
                [moonlight-common-c]    [rk.c / pi.c / aml.c / ...]
                        ↓                       ↓
                    [网络协议]            [MPP / MMAL / DRM-KMS]
```

```
moonlight-qt:

  [Qt GUI] → [FFmpeg] → [V4L2 / VAAPI / Vulkan / D3D] → [显示输出]
       ↓
  [moonlight-common-c] → [网络协议]
```

### 1.4 功能详细对比

#### 视频解码

| 功能 | moonlight-embedded | moonlight-qt |
|------|-------------------|-------------|
| H.264 硬解 | ✅ 平台原生 | ✅ FFmpeg 后端 |
| H.265/HEVC 硬解 | ✅ 平台原生 | ✅ FFmpeg 后端 |
| AV1 硬解 | ✅ 平台原生 | ✅ FFmpeg 后端 |
| 10-bit HDR 解码 | ✅ (rk.c 支持 NA12/NV15) | ✅ |
| 原子 KMS 提交 | ✅ (HDR 时启用) | ✅ |
| 显示旋转 | ✅ (0°/90°/180°/270°) | ✅ |
| YUV444 | ⚠️ 仅 x11/sdl 后端可行 | ✅ 多渲染器支持 |

#### 输入

| 功能 | moonlight-embedded | moonlight-qt |
|------|-------------------|-------------|
| evdev 手柄 | ✅ | ❌ |
| SDL2 手柄 | ✅ | ✅ |
| 键盘输入 | ✅ (evdev) | ✅ (Qt) |
| CEC 遥控器 | ✅ (libcec) | ❌ |
| 多输入设备 | ✅ (`-input` 参数指定) | 系统自动发现 |
| 触摸/多点触控 | ❌ | ✅ |

#### 音频

| 功能 | moonlight-embedded | moonlight-qt |
|------|-------------------|-------------|
| ALSA | ✅ | ✅ |
| PulseAudio | ✅ | ✅ |
| OSS | ✅ (FreeBSD) | ❌ |
| SDL 音频 | ✅ | ❌ |
| OMX (RPi) | ✅ | ❌ |
| 环绕声 5.1/7.1 | ✅ | ✅ |
| 音频设备选择 | ✅ (`-audio` 参数) | ✅ |

### 1.5 平台支持

| 平台 | moonlight-embedded | moonlight-qt |
|------|-------------------|-------------|
| Rockchip (RK3566/3568/3588) | ✅ **原生** (rk.c + MPP) | ⚠️ 通过 V4L2/FFmpeg 间接支持 |
| Raspberry Pi | ✅ **原生** (pi.c/MMAL/OMX) | ⚠️ 通过 V4L2/FFmpeg |
| Amlogic (S905/S912) | ✅ **原生** (aml.c) | ❌ 无专用后端 |
| i.MX6 | ✅ **原生** (imx.c) | ❌ 无专用后端 |
| x86 Linux | ✅ (X11/SDL 后端) | ✅ **原生** (VAAPI/VDPAU/Vulkan) |
| Windows | ❌ | ✅ **原生** (D3D11VA) |
| macOS | ❌ | ✅ **原生** (VideoToolbox) |
| Android | ❌ | ❌ (有独立 moonlight-android) |
| iOS | ❌ | ❌ (有独立 moonlight-ios) |

### 1.6 RK3568 适配分析

#### moonlight-embedded 在 RK3568 上有原生支持

```
moonlight-embedded/
├── cmake/FindRockchip.cmake   ← 查找 librockchip_mpp.so + libdrm
├── src/video/rk.c             ← Rockchip VPU 完整后端 (MPP + DRM/KMS)
├── src/platform.c             ← 自动检测并加载 rk 平台
│   ├── platform_check("rk")   ← dlopen("libmoonlight-rk.so")
│   └── dlsym("mpp_init")      ← 验证 MPP 可用性
└── src/main.c                 ← -platform rk 命令行参数
```

视频管线：

```
网络流 → LiStartConnection → rk_setup() → MPP解码 → DRM/KMS overlay显示
                                         ↓
                                MPP: mpp_create → mpp_init → decode_put_packet → decode_get_frame
                                DRM: drmModeSetPlane / drmModeAtomicCommit
```

- 直接使用 **Rockchip MPP API** (Media Process Platform)，零拷贝路径
- 支持 Legacy 和 Atomic KMS 两种提交模式
- 支持 8-bit (NV12) 和 10-bit (NA12/NV15) 像素格式

#### RK3568 性能参考

| 场景 | moonlight-embedded | moonlight-qt |
|------|-------------------|-------------|
| 1080p60 HEVC | ✅ 流畅，解码延迟 2-5ms | ✅ 流畅 |
| 4K30 HEVC | ✅ 可用 | ⚠️ 部分场景卡顿 |
| 4K60 HEVC | ⚠️ 受 RK3568 解码能力上限影响 | ❌ 通常不支持 |
| HDR 10-bit | ✅ 支持 | ⚠️ 视 V4L2 驱动情况 |
| 环绕声 | ✅ 原生支持 | ✅ 支持 |

### 1.7 总结推荐

| 对比项 | moonlight-embedded | moonlight-qt |
|--------|-------------------|-------------|
| 在 RK3568 上适配深度 | ⭐⭐⭐⭐⭐ **原生全栈** | ⭐⭐⭐ 通过 FFmpeg/V4L2 间接支持 |
| 延迟 | ⭐⭐⭐⭐⭐ **最低**（零拷贝 DRM/KMS） | ⭐⭐⭐ 多一层 FFmpeg 封装 |
| 资源占用 | ⭐⭐⭐⭐⭐ 极低 | ⭐⭐ 较高 |
| 功能完整性 | ⭐⭐⭐ 命令行、功能稳定 | ⭐⭐⭐⭐⭐ 桌面级功能完整 |
| 开发活跃度 | ⭐⭐⭐ 维护模式 | ⭐⭐⭐⭐⭐ 持续活跃 |
| 跨平台支持 | ⭐⭐ 仅嵌入式 Linux | ⭐⭐⭐⭐⭐ Windows/macOS/Linux |
| 易用性 | ⭐⭐ 需要命令行操作 | ⭐⭐⭐⭐⭐ GUI 开箱即用 |
| 可集成性 | ⭐⭐⭐⭐⭐ 脚本友好、轻量 | ⭐⭐ 依赖重 |

**在 RK3568 上，如果目标是嵌入式/专用流媒体设备，首选 moonlight-embedded**。如果需要在 RK3568 上运行完整桌面系统并用 GUI 操作，再考虑 moonlight-qt。

---

## 2. 编译指南

### 2.1 环境要求

**硬件**：
- x86 开发/测试：任意 x86_64 Linux 主机
- RK3568 目标部署：RK3566/RK3568/RK3588 设备

**软件**：
- Ubuntu 24.04 LTS（或其他 Debian 系 Linux）
- CMake ≥ 3.6
- GCC ≥ 11

### 2.2 依赖安装

```bash
sudo apt update
sudo apt install -y cmake gcc pkg-config \
  libopus-dev libevdev-dev libudev-dev \
  libsdl2-dev libavcodec-dev libavutil-dev \
  libpulse-dev libasound2-dev libssl-dev \
  libx11-dev libegl1-mesa-dev libgles2-mesa-dev \
  libva-dev libvdpau-dev libdrm-dev \
  libcurl4-openssl-dev libavahi-client-dev
```

#### RK3568 额外依赖

```bash
sudo apt install -y librga-dev librga2 rockchip-mpp rockchip-mpp-dev
```

依赖路径验证：

```bash
ls /usr/lib/aarch64-linux-gnu/librockchip_mpp.so*
ls /usr/include/rockchip/rk_mpi.h
```

### 2.3 编译步骤

#### x86_64 平台（功能验证）

```bash
git clone https://github.com/moonlight-stream/moonlight-embedded.git
cd moonlight-embedded
git submodule update --init --recursive
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
# 可选安装
sudo make install
```

编译产物：`build/moonlight` 可执行文件。

#### RK3568 平台（原生编译）

```bash
ssh root@192.168.x.x
git clone https://github.com/你的仓库/moonlight-embedded.git
cd moonlight-embedded
git submodule update --init --recursive
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
```

CMake 会自动检测 Rockchip MPP 并启用 `HAVE_ROCKCHIP` 和 `libmoonlight-rk.so`。

#### RK3568 交叉编译（从 x86 宿主机）

```bash
# 宿主机安装交叉编译工具链
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# 从 RK3568 设备同步 sysroot
ssh root@rk3568 "tar czf /tmp/sysroot.tar.gz /usr /lib" && \
  scp root@rk3568:/tmp/sysroot.tar.gz . && \
  tar xzf sysroot.tar.gz

# 交叉编译
cd moonlight-embedded
mkdir build-cross && cd build-cross
cmake .. \
  -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_FIND_ROOT_PATH=/path/to/sysroot \
  -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
  -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
  -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
  -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### 2.4 CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `ENABLE_SDL` | ON | SDL2 后端支持（输入、音频、视频） |
| `ENABLE_FFMPEG` | ON | FFmpeg 软解支持（X11/SDL 后端需要） |
| `ENABLE_X11` | ON | X11 窗口系统支持 |
| `ENABLE_CEC` | ON | HDMI-CEC 遥控器支持（需要 libcec） |
| `ENABLE_PULSE` | ON | PulseAudio 音频支持 |
| `DCMAKE_BUILD_TYPE` | Release | 编译类型：Release/Debug/RelWithDebInfo |

### 2.5 常见编译问题

#### `Could NOT find CURL`

```bash
sudo apt install -y libcurl4-openssl-dev
```

#### `Package 'avahi-client' not found`

```bash
sudo apt install -y libavahi-client-dev
```

#### `No SOURCES given to target: moonlight-common`

子模块未初始化：

```bash
git submodule update --init --recursive
```

#### `error: 'CLOCK_MONOTONIC' undeclared`

`-std=c99` 模式下 POSIX 时钟不可见。已在 CMakeLists.txt 中启用 `CMAKE_C_EXTENSIONS ON`（`-std=gnu99`），如果仍有问题：

```bash
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-D_GNU_SOURCE"
```

### 2.6 运行测试

```bash
# 查看帮助
./moonlight help

# 搜索局域网主机
./moonlight list

# 配对
./moonlight pair 192.168.x.x

# 流媒体（带实时统计）
./moonlight stream -1080 -fps 60 -stats -platform sdl 192.168.x.x

# RK3568 上用硬件解码（记得加 -stats）
./moonlight stream -1080 -fps 60 -stats -platform rk 192.168.x.x

# 使用配置文件
./moonlight /etc/moonlight/moonlight.conf
```

---

## 3. Sunshine 分辨率自动切换

### 3.1 需求

Windows 虚拟机做 Sunshine 宿主时，Moonlight 请求某个分辨率（如 1920x1080），希望**虚拟机的显示分辨率自动跟随切换**。这样流出来的画面就是 Moonlight 请求的准确分辨率，不需要虚拟机里手动改。

### 3.2 Sunshine 原生方案（推荐，Sunshine ≥ 0.21）

Sunshine Web UI → **Settings → Audio/Video → Advanced Display Options**：

```
Resolution: "Use resolution provided by the client" （下拉选择）
```

Sunshine 利用环境变量在流启动/停止时自动切换分辨率：

| 环境变量 | 说明 |
|---------|------|
| `SUNSHINE_CLIENT_WIDTH`  | 客户端请求的宽度 |
| `SUNSHINE_CLIENT_HEIGHT` | 客户端请求的高度 |
| `SUNSHINE_CLIENT_FPS`    | 客户端请求的帧率 |

启用此选项后，Sunshine 内部会调用 Windows API (`ChangeDisplaySettingsEx`) 自动切换分辨率，**无需额外脚本**。

> **Windows VM 注意事项**：虚拟机显卡驱动必须支持动态分辨率切换。VMware 的 `vmwgfx` 或 Hyper-V 的 `Basic Display Adapter` 可能限制较多，建议安装 VMware Tools / Hyper-V Integration Services 以获得完整支持。

### 3.3 QRes 手动脚本方案（通用，适合任意 Windows 版本）

如果 Sunshine 原生方案无效，可用 QRes 配合 Sunshine 的 Do/Undo 命令实现。

**步骤 1**：下载 [QRes](https://sourceforge.net/projects/qres/) 放到宿主机（如 `C:\tools\QRes.exe`）

**步骤 2**：Sunshine Web UI → **Advanced → Command Preparations**：

```
Do Command（流启动时执行）:
  cmd /C C:\tools\QRes.exe /x %SUNSHINE_CLIENT_WIDTH% /y %SUNSHINE_CLIENT_HEIGHT% /r %SUNSHINE_CLIENT_FPS%

Undo Command（流结束时恢复）:
  cmd /C C:\tools\QRes.exe /x 1920 /y 1080 /r 60
```

> 如果分辨率的显示模式在系统中不存在，QRes 会失败。可以用 CRU (Custom Resolution Utility) 提前添加需要的分辨率。

### 3.4 ResolutionAutomation 方案（第三方工具）

[Nonary/ResolutionAutomation](https://github.com/Nonary/ResolutionAutomation) 是一个开源 PowerShell 脚本，能自动：

- 读取 Sunshine 日志或环境变量获取客户端请求的分辨率
- 调用 Windows API 切换分辨率
- 流结束后自动恢复原始分辨率
- 支持超采样（客户端 720p，宿主机保持 1080p 渲染后缩放到 720p 传输）

用法：

```powershell
# 以管理员身份运行
.\ResolutionAutomation.ps1 -Action MonitorSunshine
```

### 3.5 Apollo 方案（Sunshine 增强分支）

[Apollo](https://github.com/ClassicOldSong/Apollo) 是 Sunshine 的社区分支，内置：

- **虚拟显示器管理** — 自动创建与 Moonlight 分辨率匹配的虚拟显示器
- 流结束后自动删除虚拟显示器
- 不需要物理显示器一直插着
- 适合无头（headless）Windows VM 场景

### 3.6 方案对比

| 方案 | 复杂度 | 适用场景 |
|------|--------|---------|
| Sunshine 内建 | ⭐ 零配置 | Sunshine ≥ 0.21，VM 显卡驱动支持 |
| QRes 脚本 | ⭐⭐ 两步配置 | 任何 Windows 版本，手动可控 |
| ResolutionAutomation | ⭐⭐ 开箱即用 | 需要超采样或自动降分辨率 |
| Apollo | ⭐⭐ 内置 | 无头 VM、虚拟显示器管理 |

### 3.7 Windows VM 特殊注意事项

1. **虚拟显卡驱动**：VMware 建议安装 VMware Tools；Hyper-V 建议安装 Integration Services；KVM/QEMU 建议使用 `virtio` 显卡驱动 + SPICE/FRP
2. **分辨率列表**：某些虚拟显卡只有有限的标准分辨率。用 QRes 前先用 `QRes.exe /l` 查看支持的分辨率列表
3. **无头 VM（没有物理显示器）**：部分虚拟显卡在没有物理显示器时无法创建高分辨率模式，建议用 **Apollo** 或 `IddSampleDriver`（虚拟显示器驱动）来绕开
4. **HDR**：QRes 不支持 HDR 切换。如果需要 HDR 自动开关，用 Sunshine 内建方案或搭配 `AutoHDRSwitch`

---

### 3.8 QXL / virtio 虚拟显卡排查

KVM/QEMU 中用 QXL 虚拟显卡时，Sunshine 日志中会出现：

```
Display refresh rate [1Hz]
Failed to set display mode(-s) completely!
Display device configuration failed with result: DisplayModePrepFailed
```

**原因**：QXL 是 QEMU 的基础 VGA 模拟，不支持 60Hz 刷新率切换，Sunshine 尝试调分辨率/刷新率会失败。

**解决办法**：

1. **临时绕过（不切换显示模式）**：Sunshine Web UI → **Settings → Advanced → Display Device**，将 Display Preparation 设为 `"Do not prepare"`（或取消勾选 Ensure Active），保存后重启 Sunshine
2. **换 virtio GPU**（推荐）：

   ```bash
   # 在宿主机上修改 VM 配置
   sudo virsh edit <vm-name>
   # 找到 <video> 段，将 model 从 qxl 改为 virtio
   ```

   然后在 Windows VM 里安装 virtio 驱动：

   ```powershell
   # 下载 virtio-win ISO
   # https://fedorapeople.org/groups/virt/virtio-win/direct-downloads/stable-virtio/virtio-win.iso
   # 挂载后安装 "viogpu" 驱动
   # 设备管理器 → 显示适配器 → 更新驱动 → 从挂载的 ISO 安装
   ```

3. **无头显示器方案**：用 [IddSampleDriver](https://github.com/ge9/IddSampleDriver)（虚拟显示器驱动），创建虚拟显示器后 Sunshine 可以正常捕获 60Hz

> **判断当前虚拟显卡类型**：在 Sunshine 日志中搜索 `friendly_name`，QXL 显示 `QXL0001`，virtio 显示 `Red Hat VirtIO GPU`


## 4. YUV444 支持可行性分析

### 3.1 概述

YUV444 是**色度全采样**格式（每像素保留完整色彩信息），相比 YUV420 优势在于文字更清晰、UI 边缘无彩色伪影。主要用于**远程桌面**场景（Sunshine + Moonlight）。

代价：码率约为 YUV420 的 **1.5~2 倍**，解码和渲染开销更高。

> YUV444 需要 **Sunshine 作为宿主**（NVIDIA GameStream 不支持）。

### 3.2 需要改动的层面

```
moonlight 协议层 → 编码格式协商 → 视频解码器 → 渲染/显示
    ①                      ②             ③           ④
```

### 3.3 各层面分析

#### ① 协议层 — moonlight-common-c

moonlight-qt 在 PR [#1282](https://github.com/moonlight-stream/moonlight-qt/pull/1282) 中通过配套的 common-c PR [#91](https://github.com/moonlight-stream/moonlight-common-c/pull/91) 添加了 YUV444 支持。

```c
config->enableYuv444 = true;
```

**当前状况**：本仓库的 `moonlight-common-c` 子模块引用 (`b126e48`) 较旧，**不含 YUV444 API**。

#### ② 编码协商 — main.c

需要增加：

```c
if (config->yuv444)
    config.stream.supportedVideoFormats |= VIDEO_FORMAT_FLAG_YUV444;
```

同时码率需要 ×2。

#### ③ 视频解码器 — 各后端现状

| 后端 | 能否解码 YUV444 | 说明 |
|------|---------------|------|
| **rk.c** (RK MPP) | ⚠️ **受限** | MPP 可解码 H.264/HEVC 4:4:4 码流，但输出通常仍是 NV12（硬件自动转换） |
| **x11.c** (FFmpeg) | ✅ **可行** | FFmpeg 解码器支持 4:4:4，输出 `AV_PIX_FMT_YUV444P` |
| **sdl.c** (FFmpeg + SDL) | ⚠️ **部分可行** | 可解码 4:4:4，但 SDL 纹理格式支持有限 |
| **aml.c / pi.c / mmal.c / imx.c** | ❌ **不支持** | 硬件解码器仅输出 YUV420 |

**关于 Rockchip MPP**：MPP 的 `MppFrameFormat` 定义了 `MPP_FMT_YUV444SP` 和 `MPP_FMT_YUV444P`，但 RK3568 的 VDPU 对 4:4:4 支持有限，主流 MPP 版本默认将 4:4:4 码流转换为 4:2:0 输出。

#### ④ 渲染/显示

| 后端 | 渲染方式 | YUV444 可行性 |
|------|---------|-------------|
| **rk.c** (DRM/KMS) | 硬件 overlay 平面 | ❌ RK3568 display controller 不支持 YUV444 的 DRM fourcc |
| **x11.c** (EGL/GLES2) | 着色器纹理采样 | ✅ 修改 `egl.c` 的 fragment shader，支持 3-plane YUV444 纹理 |
| **sdl.c** (SDL 纹理) | SDL_UpdateYUVTexture | ⚠️ SDL 的 `SDL_PIXELFORMAT_YV12` 是 420，444 需要 `SDL_PIXELFORMAT_I444` |

### 3.4 RK3568 综合结论

```
协议协商  ──>  需要更新 common-c     ⚠️ 可行
编码格式  ──>  H.264/HEVC 4:4:4     ✅ 可行
MPP 解码  ──>  不支持真正的 444      ❌ 硬件限制
DRM 显示  ──>  不支持 444 格式       ❌ 硬件限制
```

**在 RK3568 + 当前 rk.c 后端上，YUV444 可行性很低。** 主要原因不是协议层面，而是硬件管线限制：

1. **MPP 硬解** 4:4:4 后要么不支持，要么转为 NV12 输出
2. **RK3568 DRM 显示控制器**不支持 YUV444 的硬件 overlay

### 3.5 可行替代方案

| 方案 | 描述 | 可行性 |
|------|------|--------|
| **A: FFmpeg 软解 + EGL** | 不走 MPP，用 CPU 软解后 EGL 渲染 | ⚠️ RK3568 的 A55 核心性能不够，1080p60 吃力 |
| **B: 提高码率** | 不改变色度格式，提高 YUV420 码率到 50-80 Mbps | ✅ 折中效果好 |
| **C: x86 桌面端** | 在 x86 + x11 后端开启 YUV444 | ✅ 实际可行 |
| **D: 换 RK3588** | 性能更强，MPP 支持更好 | ⚠️ 仍需验证 |

### 3.6 需要改动的文件清单

| 文件 | 改动内容 |
|------|---------|
| `third_party/moonlight-common-c` (子模块) | 更新到含 YUV444 支持的版本，或自行添加 flag |
| `src/config.h` | 增加 `bool yuv444` 配置项 |
| `src/config.c` | 解析 `-yuv444` 命令行参数 + config 文件 |
| `src/main.c` | 协商时传递 YUV444 flag，码率 ×2，增加 help 文本 |
| `src/video/ffmpeg.c` | `ffmpeg_get_frame()` 处理 `AV_PIX_FMT_YUV444P` |
| `src/video/egl.c` | fragment shader 增加 4:4:4 的 3-plane 纹理采样分支 |
| `src/video/x11.c` | 适配 4:4:4 帧的管线提交 |
| `src/video/sdl.c` | 适配 4:4:4 的 SDL 纹理创建和更新 |
| `moonlight.conf` | 增加 `yuv444 = true` 配置示例 |

**工作量估算**：5-9 天。

### 3.7 建议

**RK3568 上不建议追 YUV444。** 硬件管线不支持，软解性能不够。推荐：

1. **提高码率** — 1080p 拉到 50-80 Mbps，YUV420 高码率下视觉损失很小
2. **换 RK3588** — 性能更强（需验证 MPP 的 4:4:4 支持情况）
3. **x86 桌面端用 moonlight-qt** — 官方已支持 YUV444，开箱即用

相关讨论：
- [moonlight-qt PR #1282 — YUV444 初始支持](https://github.com/moonlight-stream/moonlight-qt/pull/1282)
- [moonlight-qt Issue #1424 — YUV444 与 VRR 交互](https://github.com/moonlight-stream/moonlight-qt/issues/1424)
- [moonlight-qt Issue #1512 — HDR + YUV444 在 Intel Arc 上的冲突](https://github.com/moonlight-stream/moonlight-qt/issues/1512)

---

## 5. 实时统计模块说明

### 4.1 功能

新增 `-stats` 命令行开关，启用后每秒输出一行统计信息（控制台覆盖刷新）：

```
[STATS] 码率:   18.5 Mbps | 解码:  59.8 fps | 显示:  59.8 fps | 解码耗时:  3.21 ms | 帧大小:    39 KB
```

### 4.2 统计项

| 指标 | 说明 | 数据来源 |
|------|------|---------|
| 码率 | 每秒接收的视频数据量 (Mbps) | `submitDecodeUnit` 的 `fullLength` 汇总 |
| 解码帧率 | 每秒解码的帧数 | 各后端 `stats_frame_decoded()` 调用 |
| 显示帧率 | 每秒实际显示到屏幕的帧数 | `drmModeSetPlane` / `SDL_RenderPresent` 计数 |
| 解码耗时 | 平均每帧解码时间 (ms) | `decode_get_frame` 阻塞时间 / FFmpeg 解码时间 |
| 帧大小 | 平均每帧字节数 (KB) | 码率 / 帧数 换算 |
| 弱网告警 | 连接状态 `POOR` 触发次数 | `connectionStatusUpdate` 回调 |

### 4.3 使用方法

```bash
# 命令行
./moonlight stream -1080 -stats -platform sdl 192.168.x.x
./moonlight stream -1080 -stats -platform rk 192.168.x.x

# 配置文件
echo "stats = true" >> ~/.config/moonlight/moonlight.conf
```

### 4.4 架构说明

```
各视频后端 submitDecodeUnit → stats_submit_decode_unit()
各视频后端 解码完成          → stats_frame_decoded() + stats_decode_finished()
各视频后端 帧显示            → stats_frame_displayed()
连接状态更新                 → stats_connection_status()
                        ↓
               stats.c 内部 1 秒定时器
                        ↓
               stats_do_print() 格式化输出
```

- 线程安全：所有计数器通过 `pthread_mutex_t` 保护
- 零开销关闭：不加 `-stats` 时全部为 no-op
- 所有 7 个视频后端均已植入统计埋点（rk / x11 / sdl / aml / pi / mmal / imx）

### 4.5 改动清单

```
新增文件：
  src/stats.c         统计模块实现
  src/stats.h         统计模块 API

修改文件：
  src/main.c          集成 stats_init()
  src/connection.c    集成 stats_connection_status()
  CMakeLists.txt      启用 CMAKE_C_EXTENSIONS ON（解决 clock_gettime）
  moonlight.conf      增加 stats 配置项
  src/config.c/.h     增加 -stats 参数解析
  src/sdl.c           SDL 渲染统计
  src/video/rk.c      RK MPP 解码/显示统计
  src/video/x11.c     X11 FFmpeg 解码/显示统计
  src/video/sdl.c     SDL FFmpeg 解码统计
  src/video/aml.c     Amlogic 统计
  src/video/pi.c      Raspberry Pi OMX 统计
  src/video/mmal.c    Raspberry Pi MMAL 统计
  src/video/imx.c     i.MX6 VPU 统计
```

---

## 6. 文件结构

```
moonlight-embedded/
├── CMakeLists.txt              # 主构建文件
├── moonlight.conf              # 默认配置文件模板
├── src/
│   ├── main.c                  # 主入口（CLI 解析、流启动）
│   ├── connection.c/.h         # 连接回调（状态通知、震动、统计）
│   ├── config.c/.h             # 配置解析（命令行 + 配置文件）
│   ├── loop.c/.h               # 事件循环（poll + signalfd）
│   ├── platform.c/.h           # 平台检测和分发
│   ├── stats.c/.h              # ⭐ 实时统计模块（新增）
│   ├── sdl.c/.h                # SDL 窗口和渲染循环
│   ├── util.c/.h               # 工具函数
│   ├── cpu.c/.h                # CPU 特性检测（AES）
│   ├── audio/                  # 音频后端
│   │   ├── alsa.c              # ALSA 音频
│   │   ├── pulse.c             # PulseAudio 音频
│   │   ├── sdl.c               # SDL 音频
│   │   ├── omx.c               # RPi OMX 音频
│   │   └── oss.c               # FreeBSD OSS 音频
│   ├── input/                  # 输入后端
│   │   ├── evdev.c/.h          # evdev 输入
│   │   ├── udev.c/.h           # udev 设备热插拔
│   │   ├── mapping.c/.h        # 手柄映射
│   │   ├── sdl.c/.h            # SDL 输入
│   │   ├── x11.c/.h            # X11 输入
│   │   └── cec.c/.h            # CEC 遥控器输入
│   └── video/                  # 视频后端
│       ├── video.h             # 视频后端公共接口
│       ├── rk.c                # Rockchip MPP + DRM (⭐ RK3568 主用)
│       ├── x11.c               # X11 + FFmpeg 软解 / VAAPI / VDPAU
│       ├── egl.c/.h            # EGL/GLES2 YUV 渲染
│       ├── sdl.c               # SDL + FFmpeg 软解
│       ├── ffmpeg.c/.h         # FFmpeg 解码器封装
│       ├── ffmpeg_vaapi.c/.h   # VAAPI 硬件加速
│       ├── aml.c               # Amlogic VPU 硬解
│       ├── pi.c                # Raspberry Pi OMX 硬解
│       ├── mmal.c              # Raspberry Pi MMAL 硬解
│       └── imx.c / imx_vpu.c   # i.MX6 VPU 硬解
├── libgamestream/              # GameStream 协议实现
│   ├── client.c/.h             # 客户端协议
│   ├── discover.c/.h           # 主机发现 (mDNS)
│   └── ...
├── cmake/
│   ├── FindRockchip.cmake      # Rockchip MPP 检测
│   ├── FindAmlogic.cmake       # Amlogic 平台检测
│   ├── FindBroadcom-OMX.cmake  # RPi OMX 检测
│   ├── FindFreescale.cmake     # i.MX6 检测
│   └── ...
├── third_party/                # 第三方依赖（子模块）
│   ├── moonlight-common-c/     # Limelight 核心库
│   └── SDL_GameControllerDB/   # 手柄映射数据库
└── docs/
    ├── README.pod              # 原始 man page（Perl POD 格式）
    └── this-document.md        # 本综合文档
```
