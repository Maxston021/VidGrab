# VidGrab GitHub Actions 自动编译指南

## 快速开始

### 1. 准备 GitHub 仓库

1. 注册 GitHub 账号（如果没有）
2. 创建新仓库（例如 `VidGrab`）
3. 把整个 `VidGrab-1.3.6c` 目录的内容上传到仓库

```bash
# 在 VidGrab-1.3.6c 目录下执行
git init
git add .
git commit -m "Initial commit"
git branch -M main
git remote add origin https://github.com/你的用户名/VidGrab.git
git push -u origin main
```

### 2. 触发编译

推送代码后，GitHub Actions 会自动开始编译。也可以手动触发：

1. 打开仓库页面
2. 点击 **Actions** 标签
3. 点击 **Build VidGrab IPA**
4. 点击 **Run workflow** → 选择 main 分支 → 点击 **Run workflow**

### 3. 下载 IPA

编译完成后（约 20-40 分钟，首次编译 FFmpeg 较慢）：

1. 在 Actions 页面点击对应的运行记录
2. 滚动到 **Artifacts** 区域
3. 点击 **VidGrab.ipa** 下载
4. 用 TrollStore / AltStore / Sideloadly 等工具安装到 iPhone

## 编译流程

```
检出代码 → 安装 Theos → 下载 iOS SDK → 编译 FFmpeg/LAME（缓存）
→ 下载 Python.xcframework → 安装 yt-dlp → make 编译 → assemble.py 打包 → 上传 IPA
```

### 缓存机制

- FFmpeg + LAME 编译结果会被缓存（key: `ffmpeg-7.1.1-lame-3.100-ios16.5-arm64`）
- 首次编译约 15-20 分钟，后续编译约 5-10 分钟
- 修改 `FFMPEG_VERSION` 或 `LAME_VERSION` 会自动重新编译

## 环境变量（可自定义）

在 workflow 的 `env` 段可以修改：

| 变量 | 默认值 | 说明 |
|------|--------|------|
| `PYTHON_VERSION` | 3.14 | Python 版本 |
| `FFMPEG_VERSION` | 7.1.1 | FFmpeg 版本 |
| `LAME_VERSION` | 3.100 | LAME 版本 |

## 本地编译（WSL / Linux）

如果你想在本地编译：

### 前置要求
- Linux 或 WSL2 (Ubuntu)
- Theos + iOS 16.5 SDK
- Python 3.14 + uv
- FFmpeg/LAME iOS 静态库

### 步骤

```bash
# 1. 安装 Theos
git clone --recursive https://github.com/theos/theos.git ~/theos
export THEOS=~/theos

# 2. 下载 iOS SDK 到 $THEOS/sdks/

# 3. 编译 FFmpeg/LAME（约15分钟）
bash scripts/build-ffmpeg.sh ./vendor/ffmpeg-ios

# 4. 下载 Python.xcframework
# 从 https://github.com/beeware/Python-Apple-support/releases 下载
# 解压后设置 PYTHON_XCFRAMEWORK 环境变量

# 5. 安装 yt-dlp
uv pip install --target app_packages --python 3.14 yt-dlp certifi requests urllib3 idna charset-normalizer websockets brotli

# 6. 编译
export FFMPEG_DIR=$(pwd)/vendor/ffmpeg-ios
export PYFW_DIR=$(pwd)/vendor/ios-arm64
make FINALPACKAGE=1

# 7. 打包 IPA
export PYTHON_XCFRAMEWORK=/path/to/Python.xcframework
export YTDLP_ZIP=/path/to/yt-dlp.zip
export APP_PACKAGES=$(pwd)/app_packages
export LDID=$THEOS/toolchain/linux/iphone/bin/ldid
python3 assemble.py
```

## 常见问题

### Q: 编译失败，找不到 SDK
A: 确认 `$THEOS/sdks/iPhoneOS16.5.sdk` 存在。可以从 https://github.com/xybp888/iOS-SDKs/releases 下载。

### Q: FFmpeg 编译失败
A: 检查 Theos 工具链是否完整。可以尝试清理缓存后重新编译。

### Q: IPA 安装后闪退
A: 可能是签名问题。用 TrollStore 安装不需要额外签名；用 AltStore/Sideloadly 需要用自己的 Apple ID 重签。

### Q: 如何修改代码后重新编译
A: 直接 push 到 main 分支，GitHub Actions 会自动编译。FFmpeg 缓存会加速后续编译。

## 文件结构

```
VidGrab-1.3.6c/
├── .github/workflows/build.yml    # GitHub Actions 配置
├── scripts/build-ffmpeg.sh        # FFmpeg/LAME 交叉编译脚本
├── Makefile                       # 已修改为可配置路径
├── assemble.py                    # 已修改为可配置路径
├── Sources/                       # Objective-C 源码
├── Resources/                     # 资源文件
├── app/                           # Python 代码
├── whisper/                       # whisper.cpp 源码
└── ...
```
