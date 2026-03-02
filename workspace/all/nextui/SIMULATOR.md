# NextUI UI Simulator

一个用于在桌面环境下测试 NextUI UI 结构的模拟器，支持自动遍历页面和截图功能，无需实际硬件即可开发和测试 UI，提高开发效率。

## 功能

- ✅ 游戏列表屏幕，带有可选择的菜单项
- ✅ 快速菜单覆盖层
- ✅ 游戏切换器（最近游戏轮播）
- ✅ 设置屏幕
- ✅ 状态指示器（电池、WiFi、时钟）
- ✅ 按键提示
- ✅ 键盘输入映射
- ✅ **自动遍历页面并截图**
- ✅ **手动截图功能**

## 构建和运行

### 构建

```bash
cd workspace/all/nextui
make -f makefile.test all
```

### 运行

#### 交互模式

```bash
./build/desktop/nextui_simulator
```

#### 自动遍历模式（推荐用于测试）

```bash
./build/desktop/nextui_simulator --auto --delay 100
```

这将自动遍历所有页面并截图保存。

#### 手动截图模式

```bash
./build/desktop/nextui_simulator --screenshot output_dir
```

在运行时按 `S` 键捕获截图。

### 清理

```bash
make -f makefile.test clean
```

## 命令行参数

| 参数 | 说明 |
|------|------|
| `--screenshot DIR` | 启用截图模式并保存到指定目录 |
| `--auto` | 启用自动遍历模式 |
| `--delay MS` | 自动模式下的截图延迟（毫秒，默认 500） |
| `--help` | 显示帮助信息 |

## 控制方式

| 按键 | 功能 |
|------|------|
| UP/DOWN/LEFT/RIGHT | 导航 |
| A | 选择 |
| B | 返回 |
| MENU/M | 切换快速菜单 |
| S | 手动截图 |
| ESC | 退出 |

## 屏幕界面

### 游戏列表屏幕
- 顶部状态栏：显示电池、WiFi 和时钟状态
- 主菜单：8 个可选项目，使用 UP/DOWN 键导航
- 底部按键提示：显示可用的操作

### 快速菜单
按 MENU 或 M 键打开快速菜单，包含：
- 保存状态
- 加载状态
- 设置
- 退出

### 游戏切换器
显示最近游玩的游戏轮播：
- LEFT/RIGHT：切换游戏
- A：继续游戏
- B：关闭

### 设置屏幕
系统设置选项：
- 亮度
- 音量
- 屏幕
- 关于

## 自动遍历模式

自动遍历模式会依次访问所有页面并捕获截图：

1. **游戏列表** (8 个项目)
2. **快速菜单** (4 个项目)
3. **游戏切换器** (3 个项目)
4. **设置** (4 个项目)

总共生成 19 张截图，文件名格式为：
```
000_gamelist_item0.bmp
001_gamelist_item1.bmp
...
018_settings_item3.bmp
```

## 截图格式

- 默认保存为 **BMP** 格式（不需要 SDL2_image）
- 如果安装了 SDL2_image，将保存为 **PNG** 格式

要启用 PNG 支持：

```bash
sudo apt-get install libsdl2-image-dev
```

然后修改 `makefile.test`，移除 `-DSTUB_SDL_IMAGE` 并添加 `-lSDL2_image`。

## 使用场景

### 1. 视觉回归测试

```bash
# 生成基线截图
./build/desktop/nextui_simulator --auto --delay 100

# 修改代码后再次生成
./build/desktop/nextui_simulator --auto --delay 100

# 比较截图差异
diff screenshots/*.bmp
```

### 2. UI 开发测试

```bash
# 交互模式测试 UI
./build/desktop/nextui_simulator

# 随时按 S 键捕获当前状态
```

### 3. CI/CD 集成

在 CI 流程中自动生成截图：

```yaml
- name: Run UI tests
  run: |
    cd workspace/all/nextui
    make -f makefile.test all
    ./build/desktop/nextui_simulator --auto --delay 100

- name: Upload screenshots
  uses: actions/upload-artifact@v4
  with:
    name: ui-screenshots
    path: workspace/all/nextui/screenshots/
```

## 注意事项

- 当前版本使用简单的图形绘制代替文本渲染
- 未来版本可以集成 SDL_ttf 实现更好的文本显示
- 模拟器使用 640x480 分辨率，可以根据需要调整
- BMP 格式截图文件较大，建议在支持 PNG 的环境中使用 PNG 格式

## 文件说明

- `simulator.c`: 主模拟器程序
- `makefile.test`: 构建配置
- `simulator_main.c`: 完整集成版本的入口点（预留）
- `simulator_adapter.c`: NextUI 适配器层（预留）
- `stub_msettings.h`: msettings 的 stub 实现
- `stub_sdl_image.h`: SDL2_image 的 stub 实现

## 扩展

这个模拟器提供了一个基本的 UI 框架，你可以：

1. 添加更多 UI 组件和屏幕
2. 集成实际的 NextUI 组件渲染代码
3. 添加更复杂的交互逻辑
4. 支持游戏缩略图显示
5. 实现实际的文件系统访问
6. 添加截图对比功能
7. 支持分辨率切换
8. 添加动画效果测试