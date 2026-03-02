# NextUI Testing Guide

## Overview

This directory contains automated testing tools for NextUI to ensure visual consistency after refactoring.

## Test Suite

The test suite (`test_nextui.c`) provides:

- **Off-screen rendering**: Tests run without requiring a display
- **Screenshot comparison**: Compares output against baseline images
- **Diff generation**: Creates visual diff images for failed tests
- **Automated reporting**: Generates test summaries with pass/fail rates

## Building Tests

```bash
cd workspace/all/nextui
make -f makefile.test all
```

## Running Tests

### 1. Create Baseline Images

Before running regression tests, create baseline images from the original implementation:

```bash
# Build original NextUI from main branch
git checkout main
make

# Run the application and capture screenshots
# (manual process - take screenshots of key screens)

# Create baseline for test suite
git checkout refactor/remove-quickmenu
make -f makefile.test create-baseline
```

### 2. Run Regression Tests

After making changes, run tests to verify visual consistency:

```bash
make -f makefile.test test
```

### 3. Review Results

Test results are saved in `test_output/`:

- `*_output.png` - Current test output
- `baseline_*.png` - Baseline reference images
- `*_diff.png` - Difference visualization (red highlights differences)

## Test Coverage

The test suite covers:

1. **Empty screen rendering**
2. **Solid color patterns**
3. **Gradient rendering**
4. **Checkerboard patterns**
5. **Text rendering**
6. **Button component rendering**
7. **List component rendering**
8. **Status pill rendering**

## Customizing Tests

To add new tests:

1. Add a test function in `test_nextui.c`:

```c
void test_my_feature(SDL_Surface* surface) {
    // Your rendering code here
}
```

2. Register the test in `main()`:

```c
register_test(ctx, "my_feature", "baseline_my_feature.png", test_my_feature);
```

3. Rebuild and rerun:

```bash
make -f makefile.test clean
make -f makefile.test create-baseline
make -f makefile.test test
```

## Thresholds

The comparison threshold is set to 10% by default. Adjust this in `test_nextui.c`:

```c
int passed = compare_screenshots(baseline_path, output_path, diff_path, 0.1); // 10%
```

## CI/CD Integration

To integrate with GitHub Actions:

```yaml
- name: Run Visual Regression Tests
  run: |
    cd workspace/all/nextui
    make -f makefile.test test
```

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

1. Review the diff images to understand what changed
2. If the change is intentional, update baselines:

```bash
make -f makefile.test create-baseline
git add test_output/baseline_*.png
```

3. If the change is unintended, investigate the code

## Continuous Testing

For continuous testing during development:

```bash
# Watch mode (requires entr or similar)
make -f makefile.test all
while true; do
    find . -name "*.c" | entr -r make -f makefile.test test
done
```

## Notes

- Tests use 640x480 resolution by default
- All output is saved as PNG images
- Memory usage is minimal due to off-screen rendering
- Tests are platform-independent (run on any system with SDL2)

## Future Enhancements

Potential improvements:

- Add support for testing actual NextUI screens (game list, switcher, etc.)
- Implement font rendering tests
- Add animation frame comparison
- Support multiple screen resolutions
- Generate HTML test reports with embedded images
