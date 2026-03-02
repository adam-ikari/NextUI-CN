# CI/CD 集成指南

## UI 截图测试 CI 流程

NextUI UI 模拟器已集成到 CI/CD 流程中，用于自动化视觉回归测试。

## 工作流程

### 1. 触发条件

CI 工作流在以下情况下触发：
- 推送到 `main`、`develop` 分支
- 推送到任何 `feature/**`、`bugfix/**`、`refactor/**` 分支
- 创建或更新 Pull Request
- 手动触发（workflow_dispatch）

### 2. CI 执行步骤

#### 步骤 1: 安装依赖
```bash
sudo apt-get install -y \
  libsdl2-dev \
  libsdl2-image-dev \
  libsdl2-ttf-dev \
  libpng-dev \
  libjpeg-dev \
  zlib1g-dev
```

#### 步骤 2: 构建模拟器
```bash
cd workspace/all/nextui
make -f makefile.test all
```

#### 步骤 3: 运行自动遍历测试
```bash
./build/desktop/nextui_simulator --auto --delay 100
```

这会自动：
- 遍历所有 4 个页面
- 捕获 19 张截图
- 保存到 `screenshots/` 目录

#### 步骤 4: 创建基线（仅 main 分支）
```bash
mkdir -p baseline
cp screenshots/*.bmp baseline/
```

#### 步骤 5: 对比截图（非 main 分支）
```bash
# 比较每个截图与基线
for file in screenshots/*.bmp; do
  diff "baseline/$filename" "$file"
done
```

#### 步骤 6: 上传构建产物
- 截图：`ui-screenshots-{run_id}`
- 基线（仅 main）：`ui-baseline-{run_id}`
- 对比报告：`ui-comparison-report-{run_id}`

#### 步骤 7: PR 评论
在 Pull Request 上自动评论测试结果。

## 本地测试

### 运行 CI 测试
```bash
cd workspace/all/nextui
make -f makefile.test test
```

### 手动运行模拟器
```bash
./build/desktop/nextui_simulator
```

### 自动遍历模式
```bash
./build/desktop/nextui_simulator --auto --delay 100
```

## 构建产物

### 截图产物
包含 19 张截图：
- 000-007: 游戏列表（8 个项目）
- 008-011: 快速菜单（4 个项目）
- 012-014: 游戏切换器（3 个项目）
- 015-018: 设置（4 个项目）

### 对比报告
Markdown 格式，包含：
- 构建信息
- 每张截图的对比结果
- 匹配/不匹配统计
- 成功率

## 故障排查

### CI 测试失败

1. **查看对比报告**
   - 下载 `ui-comparison-report-{run_id}` 产物
   - 查找标记为 ❌ 的截图

2. **查看截图差异**
   - 下载 `ui-screenshots-{run_id}` 产物
   - 对比不匹配的截图与基线

3. **本地验证**
   ```bash
   # 下载基线
   gh run download {run_id} -n ui-baseline-{run_id}
   
   # 本地对比
   diff baseline/ screenshots/
   ```

### 截图文件过大

BMP 格式文件较大（约 900KB/张），19 张截图约 17MB。

**解决方案**：
1. 安装 SDL2_image 启用 PNG 支持
2. 修改 `makefile.test`：
   - 移除 `-DSTUB_SDL_IMAGE`
   - 添加 `-lSDL2_image`

## 扩展

### 添加更多测试用例

在 `simulator.c` 的 `auto_traverse_and_capture()` 函数中添加：

```c
case SCREEN_SETTINGS:
    max_items = 4;
    if (sim->selected_item >= max_items) {
        sim->selected_item = 0;
        // 添加新屏幕
        sim->current_screen = SCREEN_NEW_SCREEN;
    }
    break;
```

### 调整截图延迟

在 CI 工作流中：
```yaml
- name: Run auto-traversal screenshot tests
  run: |
    cd workspace/all/nextui
    ./build/desktop/nextui_simulator --auto --delay 200  # 增加延迟
```

### 添加视觉差异检测

使用 ImageMagick 检测像素差异：
```yaml
- name: Detect visual differences
  run: |
    cd workspace/all/nextui
    for file in screenshots/*.bmp; do
      compare -metric AE "baseline/$(basename $file)" "$file" "${file%.bmp}_diff.png"
    done
```

## 最佳实践

1. **定期更新基线**：当 UI 有意更改时，在 main 分支更新基线
2. **审查差异**：每次 PR 时仔细审查视觉差异
3. **本地测试**：推送前在本地运行测试验证
4. **保持稳定**：确保模拟器渲染与真机一致

## 性能优化

- 当前：19 张截图，约 17MB
- 优化方向：
  - 使用 PNG 格式（约 100KB/张）
  - 只保存变化的截图
  - 并行生成截图