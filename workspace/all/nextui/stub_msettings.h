// Stub for msettings.h (for simulator use)
#ifndef STUB_MSETTINGS_H
#define STUB_MSETTINGS_H

// Stub functions
static inline void InitSettings(void) {
}

static inline void QuitSettings(void) {
}

static inline int InitializedSettings(void) {
    return 1;
}

static inline int GetBrightness(void) {
    return 5;
}

static inline int GetColortemp(void) {
    return 20;
}

static inline int GetContrast(void) {
    return 0;
}

static inline int GetSaturation(void) {
    return 0;
}

static inline int GetExposure(void) {
    return 0;
}

static inline int GetVolume(void) {
    return 10;
}

static inline void SetBrightness(int value) {
    (void)value;
}

static inline void SetColortemp(int value) {
    (void)value;
}

static inline void SetContrast(int value) {
    (void)value;
}

static inline void SetSaturation(int value) {
    (void)value;
}

static inline void SetExposure(int value) {
    (void)value;
}

static inline void SetVolume(int value) {
    (void)value;
}

static inline int GetJack(void) {
    return 0;
}

static inline void SetJack(int value) {
    (void)value;
}

static inline int GetAudioSink(void) {
    return 0;
}

static inline void SetAudioSink(int value) {
    (void)value;
}

static inline int GetHDMI(void) {
    return 0;
}

static inline void SetHDMI(int value) {
    (void)value;
}

static inline int GetMute(void) {
    return 0;
}

static inline int GetFnToggle(void) {
    return 0;
}

static inline void SetFnToggle(int value) {
    (void)value;
}

// FnToggle stubs
static inline int GetFnToggleBrightness(void) { return 0; }
static inline int GetFnToggleColortemp(void) { return 0; }
static inline int GetFnToggleContrast(void) { return 0; }
static inline int GetFnToggleSaturation(void) { return 0; }
static inline int GetFnToggleExposure(void) { return 0; }
static inline int GetFnToggleVolume(void) { return 0; }
static inline int GetFnToggleDisablesDpad(void) { return 0; }
static inline int GetFnToggleEmulatesJoystick(void) { return 0; }
static inline int GetFnToggleTurboA(void) { return 0; }
static inline int GetFnToggleTurboB(void) { return 0; }
static inline int GetFnToggleTurboX(void) { return 0; }
static inline int GetFnToggleTurboY(void) { return 0; }
static inline int GetFnToggleTurboL1(void) { return 0; }
static inline int GetFnToggleTurboL2(void) { return 0; }
static inline int GetFnToggleTurboR1(void) { return 0; }
static inline int GetFnToggleTurboR2(void) { return 0; }

static inline void SetFnToggleBrightness(int v) { (void)v; }
static inline void SetFnToggleColortemp(int v) { (void)v; }
static inline void SetFnToggleContrast(int v) { (void)v; }
static inline void SetFnToggleSaturation(int v) { (void)v; }
static inline void SetFnToggleExposure(int v) { (void)v; }
static inline void SetFnToggleVolume(int v) { (void)v; }
static inline void SetFnToggleDisablesDpad(int v) { (void)v; }
static inline void SetFnToggleEmulatesJoystick(int v) { (void)v; }
static inline void SetFnToggleTurboA(int v) { (void)v; }
static inline void SetFnToggleTurboB(int v) { (void)v; }
static inline void SetFnToggleTurboX(int v) { (void)v; }
static inline void SetFnToggleTurboY(int v) { (void)v; }
static inline void SetFnToggleTurboL1(int v) { (void)v; }
static inline void SetFnToggleTurboL2(int v) { (void)v; }
static inline void SetFnToggleTurboR1(int v) { (void)v; }
static inline void SetFnToggleTurboR2(int v) { (void)v; }

// Vibration stubs
static inline void SetRawVibration(int value) { (void)value; }
static inline void SetVibration(int value) { (void)value; }
static inline void TestVibration(int value) { (void)value; }
static inline int GetVibration(void) { return 0; }
static inline int GetFnToggleVibration(void) { return 0; }
static inline void SetFnToggleVibration(int v) { (void)v; }

#endif // STUB_MSETTINGS_H