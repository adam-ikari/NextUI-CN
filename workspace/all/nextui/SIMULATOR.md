# NextUI UI Simulator

一个用于在桌面环境下测试 NextUI UI 结构的模拟器，无需实际硬件即可开发和测试 UI，提高开发效率。

## 当前状态

这是一个**简化版本**的模拟器，它展示了 NextUI UI 的基本结构，但**不包含完整的 NextUI 代码集成**。

### 为什么使用简化版本？

完整的 NextUI 集成需要：
1. 构建所有 NextUI 源代码
2. 处理复杂的依赖项（GFX、PAD、platform、msettings 等）
3. 模拟硬件功能（WiFi、蓝牙、电源管理等）
4. 处理多线程和资源加载

当前简化版本提供了：
- ✅ 可以快速构建和运行
- ✅ 展示 UI 基本结构
- ✅ 支持基本的交互测试
- ❌ 不使用实际的 NextUI 组件渲染
- ❌ 不支持完整的 NextUI 功能

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
./build/desktop/nextui_simulator
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

## 完整集成计划

要实现完整的 NextUI 集成（使用实际 NextUI 代码），需要：

1. **依赖项管理**
   - 安装 SDL2_image 和 SDL2_ttf
   - 构建 libmsettings
   - 配置所有编译路径

2. **代码适配**
   - 将 NextUI 的 main 函数分解为初始化、主循环和清理函数
   - 创建离屏渲染表面
   - 适配硬件相关的 API 调用

3. **功能模拟**
   - 模拟文件系统访问
   - 模拟网络状态
   - 模拟电源管理
   - 模拟输入设备

4. **测试框架**
   - 创建测试用例
   - 实现截图对比
   - 添加性能监控

## 文件说明

- `simulator.c`: 简化版模拟器主程序
- `simulator_main.c`: 完整集成版本的入口点（待实现）
- `simulator_adapter.c`: NextUI 适配器（待实现）
- `stub_msettings.h`: msettings 的 stub 实现
- `stub_sdl_image.h`: SDL_image 的 stub 实现
- `makefile.test`: 构建配置

## 扩展

这个模拟器提供了一个基本的 UI 框架，你可以：

1. 添加更多 UI 组件和屏幕
2. 集成实际的 NextUI 组件渲染代码
3. 添加更复杂的交互逻辑
4. 支持游戏缩略图显示
5. 实现实际的文件系统访问

## 注意事项

- 当前版本使用简单的图形绘制代替文本渲染
- 未来版本可以集成 SDL_ttf 实现更好的文本显示
- 模拟器使用 640x480 分辨率，可以根据需要调整