# ffplay-debug-helper

## 项目简介
`ffplay-debug-helper` 基于 FFmpeg 官方播放器 `ffplay` 的源码进行了结构化拆分与注释补充，方便在本地 IDE 中调试音视频播放流程。仓库将核心数据结构定义在 `include/datactl.h`，播放主循环和渲染管线拆分在 `src/` 目录下，帮助开发者快速理解并修改 ffplay 的多线程队列、时钟同步、Vulkan/SDL 渲染等模块。

## 目录结构
```
.
├── CMakeLists.txt        # CMake 构建脚本，集中维护 FFmpeg/SDL 路径
├── include/              # 头文件，包含数据结构(datactl.h)与渲染接口(ffplay_renderer.h)
├── src/
│   ├── main.cpp          # 程序入口，初始化日志/SDL 并调用 ffplay 主流程
│   ├── ffplay.cpp        # 主要的解码、时钟、线程调度逻辑
│   └── ffplay_renderer.c # Vulkan/SDL 渲染实现（源自 FFmpeg 项目）
├── ffmpeg-7.0-fdk_aac.zip# 预打包的 FFmpeg 依赖，可作为路径示例
└── README.md
```

## 依赖环境
- CMake ≥ 3.5
- C/C++17 编译器（GCC、Clang 或 MSVC）
- SDL2 开发库
- FFmpeg 7.x（需包含 `avcodec`、`avformat`、`avfilter`、`avdevice` 等组件）

> Windows 平台建议直接解压仓库附带的 `ffmpeg-7.0-fdk_aac.zip`，或使用你本地已有的预编译 FFmpeg + SDL2 包。Linux/macOS 用户请通过包管理器或源码方式安装对等版本，并调整路径。

## 构建步骤
1. **准备依赖路径**  
   编辑 `CMakeLists.txt` 中的 `FFMPEG_PATH`、`SDL_PATH` 变量，指向本地 FFmpeg 与 SDL2 的根目录。仓库给出的默认值示例位于 `D://FFmpeg/...`，请根据实际环境修改。

2. **生成构建目录**  
   ```bash
   cmake -B build -S .
   ```

3. **编译可执行文件**  
   ```bash
   cmake --build build
   ```
   成功后可执行文件默认输出到 `${PROJECT_ROOT}/bin/ffplay`。

4. **运行与调试**
   在 IDE 或命令行下运行 `bin/ffplay <媒体文件>`。源码在关键模块增加了注释与日志等级设置（`av_log_set_level(AV_LOG_DEBUG)`、`SDL_Log`），方便单步调试与观察线程/队列状态。

### 调试辅助功能

可执行文件还额外提供了若干辅助参数，帮助在调试前快速确认环境与媒体文件信息：

- `--check-deps`：打印已链接 FFmpeg/SDL 的版本号，并执行一次最小化初始化，检查依赖是否可用。
- `--probe <媒体路径>`：在不启动播放循环的情况下输出容器格式、时长、比特率以及各条流的编解码参数和元数据。支持重复传入以逐个探测多个文件；如需在探测后继续播放，请额外将媒体路径作为普通参数传入。
- `--help`：查看可用的辅助参数说明。

其余参数会原样透传给原始的 ffplay 入口，因此可自由组合调试选项，例如 `bin/ffplay --probe sample.mp4 -vf scale=1280:720 sample.mp4`。

## 常见问题
- **链接失败/找不到库**：确认 `FFMPEG_PATH/bin` 与 `SDL_PATH/bin` 下的动态库已在 `PATH`（Windows）或 `LD_LIBRARY_PATH`（Linux） 中，或手动复制到执行目录。
- **SDL 初始化失败**：请确保系统已安装对应平台的图形与音频驱动，并在无显示环境下配置虚拟显示（如 Linux 下的 `xvfb`）。
- **Vulkan 渲染可选**：`ffplay_renderer.c` 包含 Vulkan + libplacebo 的实现，若本地未启用相应扩展，可在 FFmpeg 编译时关闭 `--enable-vulkan`，或在运行时忽略相关路径。

## 参考资料
- [FFmpeg 官方文档](https://ffmpeg.org/documentation.html)
- [SDL2 Wiki](https://wiki.libsdl.org/FrontPage)

欢迎根据自身需求扩展调试工具、日志输出或图形界面，本仓库提供的结构化代码可作为深入学习 ffplay 的起点。
