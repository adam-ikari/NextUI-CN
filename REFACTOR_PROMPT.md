# NextUI 组件化重构任务

## 任务背景
NextUI 当前采用传统的命令式渲染架构，所有代码集中在一个 3400+ 行的单体文件中。为了提高可维护性、可测试性和真机/模拟器 UI 一致性，需要重构为组件化、状态驱动的架构。

## 重构目标
将 NextUI 重构为类似 React 的 SDL2 渲染框架：
- **组件化架构**：将 UI 元素提取为可复用的组件
- **状态驱动设计**：UI 状态改变自动触发重新渲染
- **模块化结构**：清晰的文件组织，易于理解和维护
- **双轨一致性**：真机和模拟器共享相同的组件定义

## 重构 Spec

### 架构设计

#### 1. 核心组件系统
```c
// components/ui_component.h
typedef struct UIComponent {
    void (*render)(struct UIComponent* self, SDL_Surface* screen, void* props);
    void (*destroy)(struct UIComponent* self);
    void* props;
} UIComponent;
```

#### 2. 状态管理系统
```c
// state/ui_state.h
typedef struct UIState {
    ScreenType current_screen;
    int selected_item;
    int show_setting;
    bool menu_key_held;
    bool show_game_switcher;
} UIState;

// 状态更新函数
void ui_state_set_screen(UIState* state, ScreenType screen);
void ui_state_set_selected(UIState* state, int selected);
void ui_state_toggle_menu_held(UIState* state);
void ui_state_set_setting(UIState* state, int setting);
```

#### 3. 基础组件定义
```c
// components/pill_component.c
typedef struct {
    SDL_Rect rect;
    uint32_t color;
    char* text;
} PillProps;

void render_pill(UIComponent* component, SDL_Surface* screen, void* props);

// components/button_component.c
typedef struct {
    char* label;
    SDL_Rect rect;
    bool primary;
    bool highlighted;
} ButtonProps;

void render_button(UIComponent* component, SDL_Surface* screen, void* props);

// components/list_component.c
typedef struct {
    int count;
    int selected;
    char** items;
    void* thumbnail; // 可选缩略图
} ListProps;

void render_list(UIComponent* component, SDL_Surface* screen, void* props);

// components/status_component.c
typedef struct {
    int battery_level;
    bool is_charging;
    bool has_wifi;
    char* time_string;
} StatusProps;

void render_status(UIComponent* component, SDL_Surface* screen, void* props);
```

#### 4. 屏幕模块系统
```c
// screens/game_list_screen.c
typedef struct {
    UIState* state;
    Array* entries;
    int selected;
} GameListScreen;

void game_list_init(GameListScreen* screen, UIState* state);
void game_list_render(GameListScreen* screen, SDL_Surface* surface);
void game_list_handle_input(GameListScreen* screen, int input);
void game_list_destroy(GameListScreen* screen);

// screens/quick_menu_screen.c
typedef struct {
    UIState* state;
    int selected;
} QuickMenuScreen;

void quick_menu_init(QuickMenuScreen* screen, UIState* state);
void quick_menu_render(QuickMenuScreen* screen, SDL_Surface* surface);
void quick_menu_handle_input(QuickMenuScreen* screen, int input);
void quick_menu_destroy(QuickMenuScreen* screen);

// screens/game_switcher_screen.c
// screens/settings_screen.c
```

#### 5. 渲染器系统
```c
// renderer.c
typedef struct {
    SDL_Surface* screen;
    UIState* state;
    Array* components;
    ScreenType current_screen;
    void* current_screen_instance;
} Renderer;

void renderer_init(Renderer* renderer, SDL_Surface* screen, UIState* state);
void renderer_add_component(Renderer* renderer, UIComponent* component);
void renderer_set_screen(Renderer* renderer, ScreenType screen_type);
void renderer_render(Renderer* renderer);
void renderer_destroy(Renderer* renderer);
```

#### 6. 主应用重构
```c
// app.c
typedef struct {
    UIState* state;
    Renderer* renderer;
    SDL_Surface* screen;
} NextUIApp;

void nextui_init(NextUIApp* app);
void nextui_run(NextUIApp* app);
void nextui_destroy(NextUIApp* app);
```

### 文件结构
```
workspace/all/nextui/
├── main.c                    # 主入口
├── app.c / app.h              # 应用程序核心
├── renderer.c / renderer.h      # 渲染器系统
├── state/
│   ├── ui_state.c / ui_state.h # 状态管理
│   └── screen_type.h         # 屏幕类型定义
├── components/
│   ├── ui_component.h        # 组件接口定义
│   ├── pill_component.c / pill_component.h
│   ├── button_component.c / button_component.h
│   ├── list_component.c / list_component.h
│   └── status_component.c / status_component.h
├── screens/
│   ├── game_list_screen.c / game_list_screen.h
│   ├── quick_menu_screen.c / quick_menu_screen.h
│   ├── game_switcher_screen.c / game_switcher_screen.h
│   └── settings_screen.c / settings_screen.h
└── legacy/                    # 原始代码备份
    └── nextui_original.c
```

### 数据流
```
用户输入 → UIState 更新 → 状态变化通知 → Renderer 检测到变化 → 重新渲染当前屏幕 → 组件树渲染
```

## 执行指南

### 第一步：查看 git 提交记录
1. 使用 `git log --oneline -20` 查看最近的提交
2. 理解项目历史和开发模式
3. 识别重要的提交模式和风格

### 第二步：理解重构目标
1. 重构目标是组件化架构，不是重写所有代码
2. 保持向后兼容，逐步迁移
3. 每个组件都应该能独立工作

### 第三步：开始实施（小步快跑）

#### 提交 1：创建基础架构
- [ ] 创建 `components/` 目录
- [ ] 创建 `components/ui_component.h` - 组件接口定义
- [ ] 创建 `state/` 目录
- [ ] 创建 `state/ui_state.h` - 状态管理结构
- [ ] 创建 `state/ui_state.c` - 状态管理实现
- [ ] 创建 `renderer.c` 和 `renderer.h` - 渲染器基础
- **测试**：确保代码编译通过
- **提交**：`git commit -m "feat: create component architecture foundation - define UI component interface and state management system"`

#### 提交 2：实现第一个组件
- [ ] 创建 `components/pill_component.h` - Pill 组件接口
- [ ] 创建 `components/pill_component.c` - Pill 组件实现
- [ ] 在 renderer 中注册 Pill 组件
- [ ] 创建简单的测试程序验证 Pill 组件渲染
- **测试**：确保 Pill 组件能正确渲染
- **提交**：`git commit -m "feat: implement Pill component - first reusable UI component for rounded rectangle elements"`

#### 提交 3：实现状态管理系统
- [ ] 完善 `ui_state.c` 中的状态更新函数
- [ ] 实现状态变化通知机制
- [ ] 添加状态订阅/取消订阅功能
- **测试**：确保状态更新能正确触发渲染
- **提交**：`git commit -m "feat: implement state management system - add state update functions and notification mechanism"`

#### 提交 4：提取 Button 组件
- [ ] 创建 `components/button_component.h` 和 `.c`
- [ ] 实现按钮渲染逻辑
- [ ] 支持主要/次要样式
- [ ] 支持高亮状态
- **测试**：确保按钮组件能正确渲染
- **提交**：`git commit -m "feat: implement Button component - support primary/secondary styles and highlight state"`

#### 提交 5：提取 List 组件
- [ ] 创建 `components/list_component.h` 和 `.c`
- [ ] 实现列表渲染逻辑
- [ ] 支持选中状态
- [ ] 支持可选缩略图
- **测试**：确保列表组件能正确渲染
- **提交**：`git commit -m "feat: implement List component - render selectable list with optional thumbnails"`

#### 提交 6：提取 Status 组件
- [ ] 创建 `components/status_component.h` 和 `.c`
- [ ] 实现状态栏渲染（电池、WiFi、时间）
- **测试**：确保状态组件能正确渲染
- **提交**：`git commit -m "feat: implement Status component - render battery, WiFi and clock indicators"`

#### 提交 7：创建屏幕系统
- [ ] 创建 `screens/` 目录
- [ ] 创建 `screens/game_list_screen.h` 和 `.c`
- [ ] 创建 `screens/quick_menu_screen.h` 和 `.c`
- [ ] 创建 `screens/game_switcher_screen.h` 和 `.c`
- [ ] 创建 `screens/settings_screen.h` 和 `.c`
- **测试**：确保每个屏幕模块能独立编译
- **提交**：`git commit -m "feat: create screen module system - implement GameList, QuickMenu, GameSwitcher and Settings screens"`

#### 提交 8：迁移渲染逻辑
- [ ] 将 `nextui.c` 中的渲染逻辑迁移到对应的屏幕模块
- [ ] 使用新的组件系统替换直接调用 GFX_blit
- [ ] 保持原有视觉效果不变
- **测试**：确保迁移后的视觉效果与原版一致
- **提交**：`git commit -m "refactor: migrate rendering logic to screen modules - use component system instead of direct GFX_blit calls"`

#### 提交 9：集成渲染器
- [ ] 完善 renderer.c 的组件管理
- [ ] 实现屏幕切换逻辑
- [ ] 实现状态变化检测和重新渲染
- **测试**：确保整个系统正常工作
- **提交**：`git commit -m "feat: integrate renderer with screen modules - implement screen switching and state-driven re-rendering"`

#### 提交 10：创建主应用
- [ ] 创建 `app.c` 和 `app.h`
- [ ] 重构 main 函数使用新架构
- [ ] 保持命令行参数支持
- **测试**：确保应用程序正常启动和运行
- **提交**：`git commit -m "refactor: create main application structure - refactor main function to use new component architecture"`

#### 提交 11：验证和修复
- [ ] 运行完整的应用程序
- [ ] 验证所有屏幕正常显示
- [ ] 验证状态切换正常工作
- [ ] 修复发现的 bug
- **测试**：确保功能完整性
- **提交**：`git commit -m "fix: fix issues found during integration testing - ensure all screens render correctly and state transitions work properly"`

#### 提交 12：清理和优化
- [ ] 移除旧的未使用代码
- [ ] 优化组件渲染性能
- [ ] 添加必要的注释
- **提交**：`git commit -m "refactor: cleanup and optimization - remove unused code and optimize component rendering performance"`

## 重要注意事项

### 代码规范
- 遵循项目现有的代码风格
- 使用 4 空格缩进
- 函数命名使用 `模块_动作` 格式（如 `game_list_render`）
- 类型命名使用 `模块_name_t` 格式（如 `GameListScreen`）
- 添加必要的注释说明组件用途

### 测试要求
- 每个组件创建后都要有测试验证
- 确保视觉效果与原版一致
- 确保状态变化触发正确的重新渲染
- 在有头和无头环境下都能运行

### 提交规范
- 每完成一个小目标就提交一次
- 提交信息使用清晰的格式：
  - `feat:` 新功能
  - `refactor:` 重构改进
  - `fix:` bug 修复
  - `docs:` 文档更新
- 提交信息应该清晰描述做了什么和为什么

### 向后兼容
- 重构过程中保持现有功能不中断
- 使用渐进式迁移，不要一次性改变太多
- 保留旧的代码作为参考（legacy/ 目录）

### 风险控制
- 每个提交后都要编译测试
- 遇到问题及时回滚到上一个工作版本
- 保持与主线分支同步，定期合并主分支的更新

## 成功标准
- [ ] 所有组件能独立工作
- [ ] 状态变化自动触发重新渲染
- [ ] 视觉效果与原版一致
- [ ] 代码组织清晰，易于理解
- [ ] 新架构支持未来扩展
- [ ] 在真机和模拟器上都能运行

## 调试提示
如果遇到问题：
1. 使用 `git log --oneline` 查看最近的提交历史
2. 使用 `git diff` 对比代码变化
3. 使用 `git revert <commit>` 回滚到特定提交
4. 保持小步快跑，每个提交都是可验证的

开始执行重构任务，从创建基础架构开始。