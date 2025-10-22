# ffplay Debug Helper

一个用于实验和调试 FFmpeg `ffplay` 播放器的精简示例工程。项目在官方 `ffplay.c` 的基础上进行了裁剪与注释，方便在本地环境中编译、调试，并观察 FFmpeg/SDL 日志输出。

## 项目亮点

- 🌱 结构简单：只包含 `ffplay` 运行所需的最小代码与头文件，便于阅读与修改。
- 🧪 调试友好：在 `main.cpp` 中预置了 FFmpeg 与 SDL 的日志输出示例，方便快速验证开发环境。
- 🔧 可扩展：通过 `include/` 与 `src/` 目录划分头文件和实现文件，支持继续对 `ffplay` 进行定制。

## 目录结构

```
.
├── CMakeLists.txt        # CMake 构建脚本（需配置 FFmpeg/SDL 路径）
├── include/              # 头文件，包含 ffplay 所需的数据结构与声明
├── src/                  # 源码，含 main 入口与 ffplay 主要实现
└── ffmpeg-7.0-fdk_aac.zip # 参考用的 FFmpeg 预编译包（Windows 路径示例）
```

## 环境依赖

- C/C++ 编译器（推荐 MSVC 或 clang/clang-cl，需支持 C++17）
- [CMake](https://cmake.org/) ≥ 3.5
- [FFmpeg 7.0](https://ffmpeg.org/)（需要包含 `avcodec`、`avformat`、`avfilter`、`swresample`、`swscale` 等动态库）
- [SDL2 2.30](https://github.com/libsdl-org/SDL)（动态库与头文件）

> **提示**：仓库中附带的 `ffmpeg-7.0-fdk_aac.zip` 仅作为路径示例。实际编译时请在本地解压或替换为你自己的 FFmpeg/SDL 安装目录。

## 构建步骤

1. 安装好 FFmpeg 与 SDL2，并记录其包含头文件与库文件的路径。
2. 编辑根目录下的 `CMakeLists.txt`，将以下变量改为本地实际路径：
   ```cmake
   set(FFMPEG_PATH D://FFmpeg//ffmpeg-7.0-fdk_aac)
   set(SDL_PATH    D://FFmpeg//SDL-release-2.30.6//build64)
   ```
3. 生成与构建：
   ```bash
   cmake -B build -S .
   cmake --build build --config Release
   ```
4. 编译成功后，目标可执行文件将位于 `bin/ffplay`（或对应平台的后缀）。

## 运行与调试

- 运行可执行文件前，请确保 FFmpeg 与 SDL 的动态库能够被系统找到（Windows 下可将 DLL 放在可执行文件同目录，或添加到 `PATH`）。
- 通过命令行执行：
  ```bash
  ./bin/ffplay <media-file>
  ```
- 程序会输出 FFmpeg 与 SDL 的调试信息，便于确认播放器初始化流程。

## 常见问题

- **链接失败 / 找不到库**：检查 `FFMPEG_PATH` 与 `SDL_PATH` 是否指向正确的 `include`、`bin` 目录；或在 CMake 中追加 `link_directories`。
- **运行时提示缺少 DLL**：将 FFmpeg 与 SDL 的动态库放在可执行文件所在目录，或加入系统环境变量。
- **想在 macOS/Linux 编译？**：修改 `CMakeLists.txt` 中的库名称/后缀，并确保系统安装了对应版本的 FFmpeg 与 SDL2。

## 下一步计划

- 将 Windows 风格的硬编码路径抽离为 CMake 选项，便于跨平台使用。
- 为 `ffplay` 核心模块补充单元测试与更多注释。

欢迎提交 issue 或 PR 来完善这个调试项目！
