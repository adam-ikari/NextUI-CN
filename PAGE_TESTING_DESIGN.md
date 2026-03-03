# NextUI 页面测试方案设计

## 设计原则

### 核心理念
**分离关注点，工具适配目的**

1. **CI/CD 自动化测试** → 使用轻量级模拟器
2. **真实设备测试** → 使用真实框架在真实硬件上
3. **视觉回归测试** → 使用模拟器生成稳定截图
4. **功能集成测试** → 使用真实框架在有头环境下

### 成功标准
- ✅ 测试可重复执行，结果一致
- ✅ 无外部依赖（ROM、pak、字体等）
- ✅ 在无头环境下稳定运行
- ✅ 快速执行（< 30秒）
- ✅ 易于维护和扩展

## 方案一：轻量级模拟器方案（推荐）

### 架构设计

```
测试层次：
┌─────────────────────────────────────────┐
│         CI/CD 自动化测试层              │
│  (轻量级模拟器 + 基准截图对比)          │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│         UI 组件渲染层                   │
│  (模拟 NextUI 组件的外观和布局)         │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│         SDL2 基础渲染层                 │
│  (跨平台、无依赖、稳定可控)             │
└─────────────────────────────────────────┘
```

### 技术栈
- **渲染引擎**：SDL2（仅基础功能，不需要 SDL2_image、SDL2_ttf）
- **测试框架**：自定义简单测试框架
- **图片格式**：BMP（内置支持，无需外部库）
- **截图格式**：PNG（如果可用 SDL2_image）

### 组件设计

#### 1. 模拟器核心
```c
// simulator_core.c
typedef struct {
    SDL_Surface* screen;
    int width;
    int height;
    int current_screen;
    int current_item;
} SimulatorState;

// 屏幕类型
typedef enum {
    SCREEN_GAMELIST,
    SCREEN_QUICKMENU,
    SCREEN_GAMESWITCHER,
    SCREEN_SETTINGS,
    SCREEN_COUNT
} ScreenType;
```

#### 2. UI 组件模拟
```c
// ui_components.c
void draw_pill(SDL_Surface* screen, int x, int y, int w, int h, uint32_t color);
void draw_text(SDL_Surface* screen, int x, int y, const char* text, uint32_t color);
void draw_button(SDL_Surface* screen, int x, int y, const char* label);
void draw_status_bar(SDL_Surface* screen, int battery, int wifi, const char* time);
void draw_game_list_item(SDL_Surface* screen, int index, const char* title, int selected);
```

#### 3. 自动遍历系统
```c
// auto_traverse.c
typedef struct {
    int current_step;
    int total_steps;
    int delay_ms;
    Uint32 last_action_time;
    char screenshot_dir[512];
} AutoTraverse;

typedef struct {
    ScreenType screen;
    int item;
    const char* name;
} TraversalStep;
```

#### 4. 截图对比系统
```c
// screenshot_compare.c
int save_screenshot(SDL_Surface* screen, const char* filename);
int compare_screenshots(const char* file1, const char* file2, float threshold);
int generate_diff_image(const char* file1, const char* file2, const char* output);
```

### 测试场景定义

#### 标准测试套件（18 张截图）
```c
TraversalStep test_steps[] = {
    // 游戏列表（8 张）
    {SCREEN_GAMELIST, 0, "000_gamelist_item0"},
    {SCREEN_GAMELIST, 1, "001_gamelist_item1"},
    {SCREEN_GAMELIST, 2, "002_gamelist_item2"},
    {SCREEN_GAMELIST, 3, "003_gamelist_item3"},
    {SCREEN_GAMELIST, 4, "004_gamelist_item4"},
    {SCREEN_GAMELIST, 5, "005_gamelist_item5"},
    {SCREEN_GAMELIST, 6, "006_gamelist_item6"},
    {SCREEN_GAMELIST, 7, "007_gamelist_item7"},
    
    // 快速菜单（4 张）
    {SCREEN_QUICKMENU, 0, "008_quickmenu_item0"},
    {SCREEN_QUICKMENU, 1, "009_quickmenu_item1"},
    {SCREEN_QUICKMENU, 2, "010_quickmenu_item2"},
    {SCREEN_QUICKMENU, 3, "011_quickmenu_item3"},
    
    // 游戏切换器（3 张）
    {SCREEN_GAMESWITCHER, 0, "012_gameswitcher_item0"},
    {SCREEN_GAMESWITCHER, 1, "013_gameswitcher_item1"},
    {SCREEN_GAMESWITCHER, 2, "014_gameswitcher_item2"},
    
    // 设置菜单（3 张）
    {SCREEN_SETTINGS, 0, "015_settings_item0"},
    {SCREEN_SETTINGS, 1, "016_settings_item1"},
    {SCREEN_SETTINGS, 2, "017_settings_item2"},
};
```

### CI/CD 集成

#### GitHub Actions 配置
```yaml
name: UI Screenshot Tests

on: [push, pull_request]

jobs:
  ui-tests:
    runs-on: ubuntu-latest
    
    steps:
      - name: Checkout
        uses: actions/checkout@v4
      
      - name: Install SDL2
        run: |
          sudo apt-get update
          sudo apt-get install -y libsdl2-dev
      
      - name: Build Simulator
        run: |
          cd workspace/all/nextui
          make -f makefile.test clean
          make -f makefile.test all
      
      - name: Run Tests
        run: |
          cd workspace/all/nextui
          export SDL_VIDEODRIVER=dummy
          ./build/desktop/nextui_simulator --auto --delay 100
      
      - name: Compare Screenshots
        run: |
          cd workspace/all/nextui
          # 对比当前截图与基准截图
          python3 scripts/compare_screenshots.py screenshots/ baseline/
      
      - name: Upload Results
        if: always()
        uses: actions/upload-artifact@v4
        with:
          name: test-results
          path: workspace/all/nextui/screenshots/
```

### 优势
1. **零依赖**：不需要 ROM、pak、字体、资源图片
2. **快速执行**：整个测试套件在 5-10 秒内完成
3. **稳定可靠**：结果 100% 可重复，无随机性
4. **易于维护**：代码简洁，逻辑清晰
5. **跨平台**：Linux、macOS、Windows 都可以运行

### 局限性
1. 不是真实的 NextUI 框架
2. 需要手动同步 UI 变化
3. 不能测试实际的游戏运行功能

## 方案二：分层测试方案

### 测试金字塔

```
        /\
       /  \
      /    \     ← 真实设备测试（月度）
     /──────\
    /        \    ← 集成测试（周度）
   /__________\   ← 单元测试（每次提交）
```

### 测试层次定义

#### 1. 单元测试（每次提交）
- **工具**：轻量级模拟器
- **目的**：验证 UI 组件的基本渲染
- **覆盖范围**：
  - 颜色和主题
  - 字体和文本
  - 按钮和图标
  - 布局和对齐
- **执行时间**：< 10秒
- **结果**：通过/失败

#### 2. 集成测试（每周）
- **工具**：真实框架 + 测试 ROM
- **目的**：验证 UI 组件的交互和状态管理
- **覆盖范围**：
  - 页面切换动画
  - 按键响应
  - 状态同步
  - 边界情况
- **执行时间**：2-5分钟
- **结果**：通过/失败 + 详细日志

#### 3. 真实设备测试（月度）
- **工具**：真实硬件 + 真实游戏 ROM
- **目的**：验证完整的用户体验
- **覆盖范围**：
  - 实际游戏运行
  - 性能和响应时间
  - 电池和电源管理
  - 多平台兼容性
- **执行时间**：30-60分钟
- **结果**：完整测试报告

### 测试环境配置

#### 单元测试环境
```yaml
# GitHub Actions
- OS: Ubuntu Latest
- Display: SDL_VIDEODRIVER=dummy
- Dependencies: SDL2 only
- ROMs: None
- Timeout: 30s
```

#### 集成测试环境
```yaml
# 本地测试环境或专用 CI runner
- OS: 实际平台（Linux/macOS/Windows）
- Display: 虚拟显示器（Xvfb）
- Dependencies: SDL2, SDL2_image, SDL2_ttf, OpenGL
- ROMs: 测试 ROM 集合
- Timeout: 10m
```

#### 真实设备测试环境
- OS: 实际设备固件
- Display: 实际设备屏幕
- Dependencies: 完整系统环境
- ROMs: 完整游戏库
- Timeout: 60m
```

## 方案三：渐进式实现路径

### Phase 1: 基础模拟器（第 1-2 周）
**目标**：建立基本的模拟器框架

- [ ] 创建模拟器核心结构
- [ ] 实现基本的 SDL2 渲染
- [ ] 实现游戏列表屏幕
- [ ] 实现截图保存功能
- [ ] 实现 auto 模式

**交付物**：
- 可运行的基础模拟器
- 生成游戏列表截图
- CI/CD 基础集成

### Phase 2: 完整屏幕覆盖（第 3-4 周）
**目标**：实现所有主要屏幕

- [ ] 实现快速菜单屏幕
- [ ] 实现游戏切换器屏幕
- [ ] 实现设置菜单屏幕
- [ ] 实现自动遍历所有屏幕
- [ ] 生成 18 张基准截图

**交付物**：
- 完整的屏幕覆盖
- 18 张基准截图
- 自动化测试脚本

### Phase 3: 截图对比系统（第 5 周）
**目标**：实现视觉回归测试

- [ ] 实现截图对比算法
- [ ] 生成差异图片
- [ ] 创建测试报告
- [ ] 集成到 CI/CD

**交付物**：
- 截图对比工具
- 差异高亮显示
- 自动化测试报告

### Phase 4: 高级特性（第 6-8 周）
**目标**：增强测试能力

- [ ] 支持多个分辨率
- [ ] 支持多个主题
- [ ] 支持多语言测试
- [ ] 性能基准测试

**交付物**：
- 多分辨率测试
- 多主题测试
- 多语言测试
- 性能报告

## 配置管理

### 测试数据目录结构
```
workspace/all/nextui/
├── test_data/
│   ├── baseline/          # 基准截图
│   │   ├── 000_gamelist_item0.png
│   │   ├── 001_gamelist_item1.png
│   │   └── ...
│   ├── themes/            # 主题配置
│   │   ├── default.json
│   │   ├── dark.json
│   │   └── retro.json
│   ├── locales/           # 多语言配置
│   │   ├── en.json
│   │   ├── zh.json
│   │   └── ja.json
│   └── test_roms/         # 测试 ROM（可选）
│       └── test_game.gb
├── scripts/
│   ├── generate_baseline.py
│   ├── compare_screenshots.py
│   └── generate_report.py
└── makefile.test
```

### Makefile 配置
```makefile
# makefile.test
TARGET = nextui_simulator
BUILD_DIR = build/desktop

.PHONY: all test clean baseline compare report

all: $(BUILD_DIR)/$(TARGET)

test: all
	@mkdir -p screenshots
	export SDL_VIDEODRIVER=dummy
	$(BUILD_DIR)/$(TARGET) --auto --delay 100
	@echo "Tests completed"

baseline: all
	@mkdir -p baseline
	export SDL_VIDEODRIVER=dummy
	$(BUILD_DIR)/$(TARGET) --auto --delay 100
	@cp screenshots/*.png baseline/
	@echo "Baseline created"

compare:
	@python3 scripts/compare_screenshots.py screenshots/ baseline/

report:
	@python3 scripts/generate_report.py screenshots/ baseline/

clean:
	rm -rf $(BUILD_DIR) screenshots baseline report.html
```

## 质量保证

### 代码质量
- ✅ 代码覆盖率 > 80%
- ✅ 零编译警告
- ✅ 符合项目代码风格
- ✅ 代码审查通过

### 测试质量
- ✅ 所有屏幕类型都有测试
- ✅ 测试结果 100% 可重复
- ✅ 测试执行时间 < 30 秒
- ✅ 截图质量清晰，无伪影

### CI/CD 质量
- ✅ 所有 PR 都必须通过测试
- ✅ 测试结果自动上传
- ✅ 测试失败自动阻止合并
- ✅ 测试报告自动生成

## 风险管理

### 技术风险
| 风险 | 影响 | 概率 | 缓解措施 |
|------|------|------|----------|
| SDL2 无头模式不稳定 | 高 | 中 | 使用虚拟显示器（Xvfb）作为备选 |
| 截图对比误报 | 中 | 低 | 调整阈值，人工审核关键差异 |
| UI 变化频繁 | 中 | 高 | 建立自动化更新基准的流程 |
| 跨平台兼容性 | 低 | 中 | 在所有目标平台上测试 |

### 维护风险
| 风险 | 影响 | 概率 | 缓解措施 |
|------|------|------|----------|
| 模拟器与真实 UI 不同步 | 高 | 中 | 建立定期同步机制 |
| 测试代码复杂度增加 | 中 | 中 | 保持代码简洁，及时重构 |
| 测试执行时间过长 | 中 | 低 | 优化测试并行化，分阶段执行 |

## 成功指标

### 短期指标（1-2 个月）
- [ ] 成功实现 18 张截图生成
- [ ] CI/CD 测试通过率 > 95%
- [ ] 测试执行时间 < 30 秒
- [ ] 零外部依赖

### 中期指标（3-6 个月）
- [ ] 覆盖 4 个主要平台
- [ ] 支持 3 个主题变体
- [ ] 支持 2 种语言
- [ ] 测试覆盖率 > 90%

### 长期指标（6-12 个月）
- [ ] 建立自动化基准更新流程
- [ ] 集成性能基准测试
- [ ] 建立完整的测试文档
- [ ] 团队熟练使用测试工具

## 总结

### 推荐方案
**方案一：轻量级模拟器方案**

### 核心价值
- 🎯 **简单**：代码简洁，易于理解
- ⚡ **快速**：执行时间短，适合 CI/CD
- 🔒 **稳定**：结果可靠，可重复
- 🛠️ **易维护**：修改成本低
- 🌐 **跨平台**：支持所有平台

### 实施建议
1. 采用渐进式实现路径
2. 优先实现核心功能
3. 建立质量保证机制
4. 持续优化和改进

### 关键成功因素
- 保持代码简洁
- 零外部依赖
- 测试结果稳定
- 易于维护扩展