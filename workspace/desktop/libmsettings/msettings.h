#ifndef __msettings_h__
#define __msettings_h__

#define SETTINGS_DEFAULT_BRIGHTNESS 2
#define SETTINGS_DEFAULT_COLORTEMP 20
#define SETTINGS_DEFAULT_CONTRAST 0
#define SETTINGS_DEFAULT_SATURATION 0
#define SETTINGS_DEFAULT_EXPOSURE 0
#define SETTINGS_DEFAULT_VOLUME 8
#define SETTINGS_DEFAULT_HEADPHONE_VOLUME 4

#define SETTINGS_DEFAULT_MUTE_NO_CHANGE -69

void InitSettings(void);
void QuitSettings(void);
int InitializedSettings(void);

int GetBrightness(void);
int GetColortemp(void);
int GetContrast(void);
int GetSaturation(void);
int GetExposure(void);
int GetVolume(void);

void SetRawBrightness(int value); // 0-255
void SetRawVolume(int value); // 0-160

void SetBrightness(int value); // 0-10
void SetColortemp(int value); // 0-40
void SetContrast(int value); // -4-5
void SetSaturation(int value); // -5-5
void SetExposure(int value); // -4-5
void SetVolume(int value); // 0-20

int GetJack(void);
void SetJack(int value); // 0-1

#define AUDIO_SINK_DEFAULT 0 // use system default, usually speaker (or jack if plugged in)
#define AUDIO_SINK_BLUETOOTH 1 // software control via bluealsa, not a separate card
#define AUDIO_SINK_USBDAC 2 // assumes being exposed as card 1 to alsa
int GetAudioSink(void);
void SetAudioSink(int value);

int GetHDMI(void);
void SetHDMI(int value); // 0-1

int GetMute(void);
int GetFnToggle(void);
void SetFnToggle(int value); // 0-1

// custom FnToggle mode persistence layer

int GetFnToggleBrightness(void);
int GetFnToggleColortemp(void);
int GetFnToggleContrast(void);
int GetFnToggleSaturation(void);
int GetFnToggleExposure(void);
int GetFnToggleVolume(void);
int GetFnToggleDisablesDpad(void);
int GetFnToggleEmulatesJoystick(void);
int GetFnToggleTurboA(void);
int GetFnToggleTurboB(void);
int GetFnToggleTurboX(void);
int GetFnToggleTurboY(void);
int GetFnToggleTurboL1(void);
int GetFnToggleTurboL2(void);
int GetFnToggleTurboR1(void);
int GetFnToggleTurboR2(void);

void SetFnToggleBrightness(int);
void SetFnToggleColortemp(int);
void SetFnToggleContrast(int);
void SetFnToggleSaturation(int);
void SetFnToggleExposure(int);
void SetFnToggleVolume(int);
void SetFnToggleDisablesDpad(int);
void SetFnToggleEmulatesJoystick(int);
void SetFnToggleTurboA(int);
void SetFnToggleTurboB(int);
void SetFnToggleTurboX(int);
void SetFnToggleTurboY(int);
void SetFnToggleTurboL1(int);
void SetFnToggleTurboL2(int);
void SetFnToggleTurboR1(int);
void SetFnToggleTurboR2(int);

// Desktop platform stubs for vibration control (not supported)
void SetRawVibration(int value); // 0-100
void SetVibration(int value); // 0-10 (0=off, 10=100%)
void TestVibration(int value); // 0-100 (percentage), test vibration intensity
int GetVibration(void);
int GetFnToggleVibration(void);
void SetFnToggleVibration(int);

#endif  // __msettings_h__
