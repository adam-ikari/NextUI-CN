# NextUI Testing Guide

## Overview

This directory contains automated testing tools for NextUI to ensure visual consistency after refactoring.

## Test Suite

### Basic Tests (`test_nextui.c`)

Provides fundamental rendering tests:

- **Off-screen rendering**: Tests run without requiring a display
- **Screenshot comparison**: Compares output against baseline images
- **Diff generation**: Creates visual diff images for failed tests
- **Automated reporting**: Generates test summaries with pass/fail rates

### Page Tests (`test_pages.c`)

Provides comprehensive page-level testing:

- **Empty directory screen**: Tests empty folder state
- **Game list screen**: Tests main game list rendering
- **Game list with thumbnail**: Tests list with preview images
- **Game switcher screen**: Tests recent games carousel
- **Game switcher empty**: Tests empty recent games state
- **Button hints**: Tests default and combined key hints
- **Status pill**: Tests status indicator rendering

## Building Tests

```bash
cd workspace/all/nextui
make -f makefile.test all
```

## Running Tests

### Basic Tests

```bash
# Run basic regression tests
make -f makefile.test test

# Create baseline for basic tests
make -f makefile.test create-baseline
```

### Page Tests

```bash
# Run page regression tests
make -f makefile.test test-pages

# Create baseline for page tests
make -f makefile.test create-baseline-pages
```

### All Tests

```bash
# Run all tests
make -f makefile.test test && make -f makefile.test test-pages

# Create all baselines
make -f makefile.test create-baseline && make -f makefile.test create-baseline-pages
```

## Test Coverage

### Basic Tests (8 tests)

1. **Empty screen rendering**
2. **Solid color patterns** (red, green, blue)
3. **Gradient rendering**
4. **Checkerboard patterns**
5. **Text rendering**
6. **Button component rendering**
7. **List component rendering**
8. **Status pill rendering**

### Page Tests (9 tests)

1. **Empty directory screen**
   - Empty folder message
   - Status pill (battery, wifi, clock)
   - Button hints

2. **Game list screen**
   - Multiple list items
   - Selected item highlighting
   - Status pill
   - Button hints (Open, Back, Resume)

3. **Game list with thumbnail**
   - Thumbnail preview on right
   - Adjusted list width
   - Status pill
   - Button hints

4. **Game switcher screen**
   - Title pill
   - Large preview area
   - Status pill
   - Button hints (Resume, Remove, Back)

5. **Game switcher empty**
   - Title pill
   - Empty message
   - Status pill
   - Back button

6. **Default button hints**
   - Primary hints (top)
   - Secondary hints (bottom)

7. **Combined key hints (menu held)**
   - Brightness adjustment
   - Color temperature adjustment

8. **Status pill only**
   - Battery indicator
   - WiFi indicator
   - Clock display

9. **Status pill with hints**
   - Combined rendering
   - Layout verification

## Reviewing Results

Test results are saved in `test_output/`:

- `*_output.png` - Current test output
- `baseline_*.png` - Baseline reference images
- `*_diff.png` - Difference visualization (red highlights differences)

## Creating Baselines

Before running regression tests, create baseline images:

```bash
# From main branch (original implementation)
git checkout main

# Run the application and capture screenshots manually
# Or use automated testing

# Create baselines
make -f makefile.test create-baseline
make -f makefile.test create-baseline-pages

# Commit baselines
git add test_output/baseline_*.png
git commit -m "Add test baselines for original implementation"

# Switch to refactor branch
git checkout refactor/remove-quickmenu
```

## Running Regression Tests

After making changes, run tests to verify visual consistency:

```bash
make -f makefile.test test
make -f makefile.test test-pages
```

## Thresholds

The comparison threshold is set to 10% by default:

```c
int passed = compare_screenshots(baseline_path, output_path, diff_path, 0.1); // 10%
```

Adjust this in `test_nextui.c` or `test_pages.c` if needed.

## Troubleshooting

### SDL2 Not Found

```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev libsdl2-image-dev

# Arch Linux
sudo pacman -S sdl2 sdl2_image

# macOS
brew install sdl2 sdl2_image
```

### Baseline Mismatch

If tests fail after expected changes:

1. Review diff images to understand what changed
2. If the change is intentional, update baselines:

```bash
make -f makefile.test create-baseline
make -f makefile.test create-baseline-pages
git add test_output/baseline_*.png
```

3. If the change is unintended, investigate the code

## CI/CD Integration

```yaml
# GitHub Actions example
- name: Run Visual Regression Tests
  run: |
    cd workspace/all/nextui
    make -f makefile.test test
    make -f makefile.test test-pages

- name: Upload Test Results
  if: failure()
  uses: actions/upload-artifact@v2
  with:
    name: test-results
    path: workspace/all/nextui/test_output/
```

## Notes

- Tests use 640x480 resolution by default
- All output is saved as PNG images
- Memory usage is minimal due to off-screen rendering
- Tests are platform-independent (run on any system with SDL2)
- Page tests use mock rendering to simulate actual UI components

## Test Workflow

### Development Phase

1. Make code changes
2. Run tests: `make -f makefile.test test-pages`
3. Review any failures
4. Fix issues or update baselines if intentional

### Release Phase

1. Ensure all tests pass
2. Run full test suite: `make -f makefile.test test && make -f makefile.test test-pages`
3. Review all baseline images
4. Tag release with passing tests

## Future Enhancements

Potential improvements:

- Integrate with actual NextUI screen components
- Test animation frame sequences
- Support multiple screen resolutions
- Generate HTML test reports with embedded images
- Add performance benchmarking
- Test user interactions (button presses, navigation)
- Automated screenshot capture from actual device
