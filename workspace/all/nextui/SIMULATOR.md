# NextUI UI Simulator

一个用于在桌面环境下测试 NextUI UI 组件的模拟器，无需实际硬件即可开发和测试 UI，提高开发效率。

## 功能

- 游戏列表屏幕，带有可选择的菜单项
- 快速菜单覆盖层
- 状态指示器（电池、WiFi、时钟）
- 按键提示
- 键盘输入映射

## 构建和运行

### 构建

```bash
cd workspace/all/nextui
make -f makefile.test all
```

### 运行

```bash
./nextui_simulator
```

或者使用 makefile：

```bash
make -f makefile.test run
```

### 清理

```bash
make -f makefile.test clean
```

## 控制方式

| 按键 | 功能 |
|------|------|
| UP/DOWN | 导航菜单 |
| A | 选择项目 |
| B | 返回 |
| MENU/M | 切换快速菜单 |
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

## 扩展

这个模拟器提供了一个基本的 UI 框架，你可以：

1. 添加更多 UI 组件和屏幕
2. 集成实际的 NextUI 组件渲染代码
3. 添加更复杂的交互逻辑
4. 支持游戏缩略图显示
5. 实现实际的文件系统访问

## 技术细节

- 使用 SDL2 进行渲染
- 纯 C 实现
- 跨平台支持（Linux、macOS、Windows）
- 60 FPS 渲染循环
- 基本的键盘输入处理

## 注意事项

- 当前版本使用简单的图形绘制代替文本渲染
- 未来版本可以集成 SDL_ttf 实现更好的文本显示
- 模拟器使用 640x480 分辨率，可以根据需要调整