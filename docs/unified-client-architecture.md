# 融合客户端架构方案

> 基于实际代码，实现了一个整合 SPICE + Moonlight 双协议的远程桌面客户端
> 编写日期：2026-07-26

---

## 代码仓库

| 项目 | 路径 |
|------|------|
| **融合客户端** | `Z:\Desktop\desk\client\spice-gtk\tools\` (spicy + spicy-moonlight) |
| **Moonlight 协议库** | `Z:\Desktop\desk\client\moonlight-embedded\third_party\moonlight-common-c\` |
| **SPICE 服务端** | `Z:\Desktop\desk\server\spice-0.15.2\` |
| **Sunshine 服务端** | `Z:\Desktop\desk\server\Sunshine-2026.725.25407\` |

---

## 1. 架构概览

以 `spicy`（spice-gtk 的参考 GTK 客户端）为基础，新增 Moonlight 渲染引擎模块，实现**一个窗口同时处理 SPICE 和 Moonlight 两种协议**。

```
┌───────────────────────────────────────────────────────────────────┐
│                      spicy (spice-gtk/tools/)                     │
│                                                                   │
│  ┌──────────────────────────────┐ ┌────────────────────────────┐ │
│  │          SPICE 引擎          │ │      Moonlight 引擎        │ │
│  │       (spicy.c - 原始)       │ │  (spicy-moonlight.c - 新增)│ │
│  │                              │ │                            │ │
│  │  • Main Channel (显示+输入)   │ │  • Sunshine 端口检测        │ │
│  │  • Display Channel (视频)     │ │  • LiStartConnection       │ │
│  │  • Inputs Channel (键鼠)      │ │  • LiStopConnection        │ │
│  │  • USB Channel (设备重定向)    │ │  • fork 崩溃隔离           │ │
│  │  • Record Channel (录制)      │ │  • 视频解码回调            │ │
│  │                              │ │  • GtkDrawingArea 渲染     │ │
│  └──────────────────────────────┘ └────────────────────────────┘ │
│                                                                   │
│  ┌───────────────────────────────────────────────────────────┐   │
│  │                    模式切换逻辑                             │   │
│  │                                                           │   │
│  │  SPICE 连接后 2 秒 → 检测 47989 端口                      │   │
│  │     ├─ 有 Sunshine → fork 测试连接                        │   │
│  │     │   ├─ 子进程存活 → 真正连接 → Moonlight 全屏         │   │
│  │     │   └─ 子进程崩溃 → 回退 SPICE                        │   │
│  │     └─ 无 Sunshine → 纯 SPICE 模式（和原来一样）           │   │
│  └───────────────────────────────────────────────────────────┘   │
└───────────────────────────────────────────────────────────────────┘
```

---

## 2. 新增文件

### `tools/spicy-moonlight.h` — Moonlight 引擎 API

```c
// ========== 生命周期 ==========
SpiceMoonlight *spice_moonlight_new(SpiceSession *session);
GtkWidget      *spice_moonlight_get_widget(SpiceMoonlight *ml);
void            spice_moonlight_set_toplevel(SpiceMoonlight *ml, GtkWidget *w);

// ========== Sunshine 检测 ==========
gboolean spice_moonlight_detect(const char *host, int port, int timeout_ms);

// ========== 串流控制 ==========
gboolean spice_moonlight_connect(SpiceMoonlight *ml, const char *host,
                                  int width, int height, int fps);
void     spice_moonlight_disconnect(SpiceMoonlight *ml);
gboolean spice_moonlight_is_streaming(SpiceMoonlight *ml);
```

### `tools/spicy-moonlight.c` — 实现（~520 行）

五个核心模块：

| 模块 | 行号 | 功能 |
|------|------|------|
| Sunshine 检测 | 40-75 | 非阻塞 TCP 连接 + select 超时 |
| GLib 类型注册 | 100 | `G_DEFINE_TYPE(SpiceMoonlight, ..., G_TYPE_OBJECT)` |
| 视频解码回调 | 200-275 | `ml_setup / ml_cleanup / ml_submit_decode_unit` |
| 连接管理 | 380-505 | fork 隔离 + `LiStartConnection` |
| 渲染 | 105-191 | `GtkDrawingArea.draw` → Cairo 绘制 |

---

## 3. 修改文件

### `tools/spicy.c` — 改动点

| 位置 | 行号 | 改动 |
|------|------|------|
| include | 36 | `#include "spicy-moonlight.h"` |
| 全局变量 | 132-134 | `moonlight`, `moonlight_active`, `first_win` |
| `moonlight_auto_start` | 471-525 | ⭐ 核心：检测 + widget 创建 + 连接 + 显示切换 |
| `moonlight_check_cb` | 527-534 | 2 秒后触发的超时回调 |
| `main_channel_event` | 1369-1370 | `SPICE_CHANNEL_OPENED` 时注册超时 |
| `connection_destroy` | 1992-1996 | 清理 Moonlight 资源 |
| `cmd_entries` | 新增 | `--moonlight-host` 参数 |
| 全屏顺序修复 | ~1231 | `show_all` 在 `fullscreen` 之前调用 |

### `tools/meson.build` — 编译配置

```python
spicy_sources += ['spicy-moonlight.c', 'spicy-moonlight.h']
# 依赖 moonlight-common 库
moonlight_dep = dependency('moonlight-common', required: false)
# fallback: 直接链接 .so
moonlight_lib = meson.get_compiler('c').find_library('moonlight-common',
    dirs: ['/usr/local/lib', '/usr/lib/x86_64-linux-gnu'], required: false)
```

---

## 4. 核心实现细节

### 4.1 Sunshine 检测 (`spice_moonlight_detect`)

```c
gboolean spice_moonlight_detect(const char *host, int port, int timeout_ms)
```

- TCP 连接 `host:port`，`O_NONBLOCK` + `select()` 超时
- 默认检测 `47989`（Sunshine HTTP API 端口）
- 3 秒超时
- 用于 `moonlight_auto_start` 判断是否进入融合模式

### 4.2 崩溃隔离（fork）

Moonlight 的 `LiStartConnection` 可能因各种原因崩溃，直接用 **fork 子进程** 测试：

```
moonlight_auto_start
  │
  ├─ spice_moonlight_detect(port 47989) → OK
  │
  ├─ fork()
  │   ├─ 子进程: LiStartConnection (无回调, 仅测试)
  │   │   ├─ 成功 → _exit(0)
  │   │   └─ 崩溃 → 子进程死亡，父进程不受影响
  │   │
  │   └─ 父进程: waitpid() 3 秒
  │       ├─ 子进程存活 → kill + 父进程真正 LiStartConnection
  │       └─ 子进程死亡 → 返回 FALSE, 保持 SPICE 模式
  │
  └─ 结果
      ├─ TRUE  → 隐藏 SPICE widget, 显示 Moonlight rendering
      └─ FALSE → 停留在 SPICE 模式
```

### 4.3 SERVER_INFORMATION 初始化（重要！）

`SERVER_INFORMATION.address` 是 `const char*`（指针），**不是 `char[]`**。

```c
// ✅ 正确做法
LiInitializeServerInformation(&serverInfo);
serverInfo.address = host;  // 直接赋值指针

// ❌ 错误做法（会导致 memcpy 写入 NULL 指针而崩溃）
memset(&serverInfo, 0, sizeof(serverInfo));
memcpy(serverInfo.address, host, ...);  // 崩溃！
```

### 4.4 视频解码回调的 context 陷阱

`DECODER_RENDERER_CALLBACKS` 的回调中，`void *context` 参数是 **moonlight-common-c 库内部指针**，不是用户自定义数据：

```c
// ❌ 错误做法 — context 不是 SpiceMoonlight*
static int ml_setup(int fmt, int w, int h, int rr, void *context, int f) {
    SpiceMoonlight *ml = SPICE_MOONLIGHT(context);  // 崩溃！
}

// ✅ 正确做法 — 使用静态全局变量
static SpiceMoonlight *g_ml = NULL;

static int ml_setup(int fmt, int w, int h, int rr, void *context, int f) {
    MoonlightRenderCtx *ctx = g_object_get_data(G_OBJECT(g_ml), "render-ctx");
    // ...
}
```

---

## 5. 编译和运行

### 5.1 编译

```bash
# 1. 先编译 moonlight-embedded（生成 libmoonlight-common.so）
cd ~/Desktop/desk/client/moonlight-embedded/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 2. 再编译 spice-gtk（含 spicy-moonlight 模块）
cd ~/Desktop/desk/client/spice-gtk/build
rm -rf *

CFLAGS="-I/home/work/Desktop/desk/client/moonlight-embedded/third_party/moonlight-common-c/src \
        -I/home/work/Desktop/desk/client/moonlight-embedded/third_party/moonlight-common-c/enet/include" \
meson setup .. -Dgtk=enabled

ninja
```

### 5.2 运行

```bash
# 纯 SPICE 模式（和原始 spicy 一样）
LD_LIBRARY_PATH=/home/work/Desktop/desk/client/moonlight-embedded/build/libgamestream \
./tools/spicy -h 192.168.201.133 -p 5900

# 融合模式（指定 Sunshine 地址）
LD_LIBRARY_PATH=/home/work/Desktop/desk/client/moonlight-embedded/build/libgamestream \
./tools/spicy -h 192.168.201.133 -p 5900 --moonlight-host 192.168.201.134

# 全屏融合模式
LD_LIBRARY_PATH=/home/work/Desktop/desk/client/moonlight-embedded/build/libgamestream \
./tools/spicy -h 192.168.201.133 -p 5900 --moonlight-host 192.168.201.134 -f
```

### 5.3 运行依赖

运行需要：
- `libmoonlight-common.so.4`（来自 moonlight-embedded build）
- `libgamestream.so.4`（来自 moonlight-embedded build）
- SPICE 服务端（由 QEMU 或独立 spice-server 提供）
- Sunshine 服务端（运行在 Windows VM 上）

---

## 6. VM 配置

### 6.1 GPU 直通 VM（融合模式）

```xml
<domain>
  <devices>
    <!-- QXL 设备 — SPICE 服务端正常启动所有通道 -->
    <video><model type='qxl' ram='65536' heads='1'/></video>
    <!-- SPICE 通道（USB 重定向、录制等） -->
    <channel type='spicevmc'><target type='virtio' name='com.redhat.spice.0'/></channel>
    <redirdev bus='usb' type='spicevmc'/>
    <graphics type='spice' port='5900' autoport='yes' listen='0.0.0.0'/>
    <!-- 直通 GPU — Sunshine 通过 DXGI 捕获 -->
    <hostdev mode='subsystem' type='pci' managed='yes'>
      <source><address domain='0x0000' bus='0x01' slot='0x00' function='0x0'/></source>
    </hostdev>
  </devices>
</domain>
```

Windows VM 内：直通 GPU 设为主屏，QXL 为副屏。

### 6.2 非直通 VM（纯 SPICE）

保留 QXL 和 SPICE 配置，客户端自动使用纯 SPICE 模式。

### 6.3 虚拟显示器（IddSampleDriver）

直通 GPU 无物理显示器时，Sunshine 需要虚拟显示器：

```powershell
# Windows VM 内安装
# https://github.com/ge9/IddSampleDriver/releases
```

---

## 7. 状态说明

| 阶段 | 状态 | 说明 |
|------|------|------|
| SPICE USB/Record | ✅ | 原始功能，完全保留 |
| Sunshine 端口检测 | ✅ | TCP 47989，3s 超时 |
| fork 崩溃隔离 | ✅ | 子进程崩溃不影响主进程 |
| 全屏 `-f` | ✅ | 修复了 show_all 顺序 bug |
| `--moonlight-host` | ✅ | 手动指定 Sunshine 地址 |
| `LiStartConnection` | ⚠️ 返回 -1 | 缺少 `gs_init` RTSP 握手 |
| 视频渲染 | ⚠️ 灰度占位 | Cairo 绘制 Y 平面 |
| 键鼠转发 | ❌ | 未实现 Moonlight 输入 |
| 音频输出 | ❌ | 未实现 |
| 自动发现 VM IP | ❌ | 需要手动 `--moonlight-host` |

---

## 8. 已知问题和后续开发

### 问题

1. **`LiStartConnection` 返回 -1** — 当前不使用 `gs_init` 进行 RTSP 握手，直接调用 `LiStartConnection` 导致协议协商失败。需在连接前先调用 `gs_init` 或手动完成 RTSP handshake。
2. **Cairo 渲染效率低** — 当前使用 Cairo 在 CPU 端完成 YUV→RGB 转换和缩放，高分辨率下性能差。需改为 `GtkGLArea` + OpenGL shader。
3. **缺少配对流程** — 需要实现 `gs_pair` 完成 Moonlight 配对。

### 后续开发路线

| 优先级 | 任务 |
|--------|------|
| P0 | 使用 `gs_init` 完成 RTSP 握手，解决 `LiStartConnection` 返回 -1 |
| P0 | 实现配对界面或复用已有配对 |
| P1 | `GtkGLArea` + OpenGL 渲染替代 Cairo |
| P1 | GTK 键鼠事件 → `LiSendKeyboardEvent` / `LiSendMousePositionEvent` |
| P2 | PulseAudio 音频输出 |
| P2 | 自动发现 Windows VM IP（通过 SPICE agent 或 libvirt API） |
| P3 | 命令行参数支持自定义分辨率/码率 |

---

## 9. 改动清单总结

```
spice-gtk/tools/
├── spicy-moonlight.h     (新增, 97行)  Moonlight 引擎 API
├── spicy-moonlight.c     (新增, 523行) Moonlight 引擎实现
├── spicy.c               (修改)        集成 Moonlight 检测和切换
├── meson.build           (修改)        添加新源文件和依赖

moonlight-embedded/docs/
├── unified-client-architecture.md  (本文档)
```
