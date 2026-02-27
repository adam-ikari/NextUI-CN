#include "msettings.h"

// desktop
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
//#include <sys/mman.h>
//#include <sys/ioctl.h>
//#include <errno.h>
//#include <sys/stat.h>
//#include <dlfcn.h>
#include <string.h>

// Legacy MinUI settings
typedef struct SettingsV3 {
	int version; // future proofing
	int brightness;
	int headphones;
	int speaker;
	int mute;
	int unused[2];
	int jack;
} SettingsV3;

// First NextUI settings format
typedef struct SettingsV4 {
	int version; // future proofing
	int brightness;
	int colortemperature; // 0-20
	int headphones;
	int speaker;
	int mute;
	int unused[2];
	int jack; 
} SettingsV4;

// Current NextUI settings format
typedef struct SettingsV5 {
	int version; // future proofing
	int brightness;
	int colortemperature;
	int headphones;
	int speaker;
	int mute;
	int unused[2]; // for future use
	// NOTE: doesn't really need to be persisted but still needs to be shared
	int jack; 
} SettingsV5;


// Third NextUI settings format
typedef struct SettingsV6 {
	int version; // future proofing
	int brightness;
	int colortemperature;
	int headphones;
	int speaker;
	int mute;
	int contrast;
	int saturation;
	int exposure;
	int unused[2]; // for future use
	// NOTE: doesn't really need to be persisted but still needs to be shared
	int jack; 
} SettingsV6;

typedef struct SettingsV7 {
	int version; // future proofing
	int brightness;
	int colortemperature;
	int headphones;
	int speaker;
	int mute;
	int contrast;
	int saturation;
	int exposure;
	int mutedbrightness;
	int mutedcolortemperature;
	int mutedcontrast;
	int mutedsaturation;
	int mutedexposure;
	int unused[2]; // for future use
	// NOTE: doesn't really need to be persisted but still needs to be shared
	int jack; 
} SettingsV7;

typedef struct SettingsV8 {
	int version; // future proofing
	int brightness;
	int colortemperature;
	int headphones;
	int speaker;
	int mute;
	int contrast;
	int saturation;
	int exposure;
	int toggled_brightness;
	int toggled_colortemperature;
	int toggled_contrast;
	int toggled_saturation;
	int toggled_exposure;
	int toggled_volume;
	int unused[2]; // for future use
	// NOTE: doesn't really need to be persisted but still needs to be shared
	int jack; 
} SettingsV8;

typedef struct SettingsV9 {
	int version; // future proofing
	int brightness;
	int colortemperature;
	int headphones;
	int speaker;
	int mute;
	int contrast;
	int saturation;
	int exposure;
	int toggled_brightness;
	int toggled_colortemperature;
	int toggled_contrast;
	int toggled_saturation;
	int toggled_exposure;
	int toggled_volume;
	int disable_dpad_on_mute;
	int emulate_joystick_on_mute;
	int turbo_a;
	int turbo_b;
	int turbo_x;
	int turbo_y;
	int turbo_l1;
	int turbo_l2;
	int turbo_r1;
	int turbo_r2;
	int unused[2]; // for future use
	// NOTE: doesn't really need to be persisted but still needs to be shared
	int jack; 
} SettingsV9;

// When incrementing SETTINGS_VERSION, update the Settings typedef and add
// backwards compatibility to InitSettings!
#define SETTINGS_VERSION 9
typedef SettingsV9 Settings;
static Settings DefaultSettings = {
	.version = SETTINGS_VERSION,
	.brightness = SETTINGS_DEFAULT_BRIGHTNESS,
	.colortemperature = SETTINGS_DEFAULT_COLORTEMP,
	.headphones = SETTINGS_DEFAULT_HEADPHONE_VOLUME,
	.speaker = SETTINGS_DEFAULT_VOLUME,
	.mute = 0,
	.contrast = SETTINGS_DEFAULT_CONTRAST,
	.saturation = SETTINGS_DEFAULT_SATURATION,
	.exposure = SETTINGS_DEFAULT_EXPOSURE,
	.toggled_brightness = SETTINGS_DEFAULT_MUTE_NO_CHANGE,
	.toggled_colortemperature = SETTINGS_DEFAULT_MUTE_NO_CHANGE,
	.toggled_contrast = SETTINGS_DEFAULT_MUTE_NO_CHANGE,
	.toggled_saturation = SETTINGS_DEFAULT_MUTE_NO_CHANGE,
	.toggled_exposure = SETTINGS_DEFAULT_MUTE_NO_CHANGE,
	.toggled_volume = 0, // mute is default
	.disable_dpad_on_mute = 0,
	.emulate_joystick_on_mute = 0,
	.turbo_a = 0,
	.turbo_b = 0,
	.turbo_x = 0,
	.turbo_y = 0,
	.turbo_l1 = 0,
	.turbo_l2 = 0,
	.turbo_r1 = 0,
	.turbo_r2 = 0,
	.jack = 0,
};
static Settings* msettings;

static char SettingsPath[256];

///////////////////////////////////////

int peekVersion(const char *filename) {
	int version = 0;
	FILE *file = fopen(filename, "r");
	if (file) {
		fread(&version, sizeof(int), 1, file);
		fclose(file);
	}
	return version;
}

void InitSettings(void){
	// We are not really using them, but we should be able to debug them
	sprintf(SettingsPath, "%s/msettings.bin", getenv("USERDATA_PATH"));
	//sprintf(SettingsPath, "%s/msettings.bin", SDCARD_PATH "/.userdata");
	msettings = (Settings*)malloc(sizeof(Settings));
	
	int version = peekVersion(SettingsPath);
	if(version > 0) {
		// fopen file pointer
		int fd = open(SettingsPath, O_RDONLY);
		if(fd) {
			if (version == SETTINGS_VERSION) {
				read(fd, msettings, sizeof(Settings));
			}
			else {
				// initialize with defaults
				memcpy(msettings, &DefaultSettings, sizeof(Settings));
				
				// overwrite with migrated data
				if(version==8) {
					printf("Found settings v8.\n");
					SettingsV8 old;
					read(fd, &old, sizeof(SettingsV8));

					msettings->toggled_volume = old.toggled_volume;

					msettings->toggled_brightness = old.toggled_brightness;
					msettings->toggled_colortemperature = old.toggled_colortemperature;
					msettings->toggled_contrast = old.toggled_contrast;
					msettings->toggled_exposure = old.toggled_exposure;
					msettings->toggled_saturation = old.toggled_saturation;
					
					msettings->saturation = old.saturation;
					msettings->contrast = old.contrast;
					msettings->exposure = old.exposure;

					msettings->colortemperature = old.colortemperature;

					msettings->brightness = old.brightness;
					msettings->headphones = old.headphones;
					msettings->speaker = old.speaker;
					msettings->mute = old.mute;
					msettings->jack = old.jack;
				}
				else if(version==7) {
					printf("Found settings v7.\n");
					SettingsV7 old;
					read(fd, &old, sizeof(SettingsV7));

					msettings->toggled_brightness = old.mutedbrightness;
					msettings->toggled_colortemperature = old.mutedcolortemperature;
					msettings->toggled_contrast = old.mutedcontrast;
					msettings->toggled_exposure = old.mutedexposure;
					msettings->toggled_saturation = old.mutedsaturation;

					msettings->saturation = old.saturation;
					msettings->contrast = old.contrast;
					msettings->exposure = old.exposure;

					msettings->colortemperature = old.colortemperature;

					msettings->brightness = old.brightness;
					msettings->headphones = old.headphones;
					msettings->speaker = old.speaker;
					msettings->mute = old.mute;
					msettings->jack = old.jack;
				}
				else if(version==6) {
					printf("Found settings v6.\n");
					SettingsV6 old;
					read(fd, &old, sizeof(SettingsV6));
					
					msettings->saturation = old.saturation;
					msettings->contrast = old.contrast;
					msettings->exposure = old.exposure;

					msettings->colortemperature = old.colortemperature;

					msettings->brightness = old.brightness;
					msettings->headphones = old.headphones;
					msettings->speaker = old.speaker;
					msettings->mute = old.mute;
					msettings->jack = old.jack;
				}
				else if(version==5) {
					printf("Found settings v5.\n");
					SettingsV5 old;
					read(fd, &old, sizeof(SettingsV5));

					msettings->colortemperature = old.colortemperature;

					msettings->brightness = old.brightness;
					msettings->headphones = old.headphones;
					msettings->speaker = old.speaker;
					msettings->mute = old.mute;
					msettings->jack = old.jack;
				}
				else if(version==4) {
					printf("Found settings v4.\n");
					SettingsV4 old;
					read(fd, &old, sizeof(SettingsV4));

					// colortemp was 0-20 here
					msettings->colortemperature = old.colortemperature * 2;

					msettings->brightness = old.brightness;
					msettings->headphones = old.headphones;
					msettings->speaker = old.speaker;
					msettings->mute = old.mute;
					msettings->jack = old.jack;
				}
				else if(version==3) {
					printf("Found settings v3.\n");
					SettingsV3 old;
					read(fd, &old, sizeof(SettingsV3));

					msettings->brightness = old.brightness;
					msettings->headphones = old.headphones;
					msettings->speaker = old.speaker;
					msettings->mute = old.mute;
					msettings->jack = old.jack;
				}
				else {
					printf("Found unsupported settings version: %i.\n", version);
				}
			}

			close(fd);
		}
		else {
			printf("Unable to read settings, using defaults\n");
			// load defaults
			memcpy(msettings, &DefaultSettings, sizeof(Settings));
		}
	}
	else {
		printf("No settings found, using defaults\n");
		// load defaults
		memcpy(msettings, &DefaultSettings, sizeof(Settings));
	}
}
static inline void SaveSettings(void) {
	FILE *file = fopen(SettingsPath, "w");
	if (file) {
		fwrite(msettings, sizeof(Settings), 1, file);
		fclose(file);
	}
}
void QuitSettings(void){
	SaveSettings();
	// dealloc settings
	free(msettings);
	msettings = NULL;
}
int InitializedSettings(void){
	return msettings != NULL;
}

// not implemented here

int GetBrightness(void) { return 0; }
int GetColortemp(void) { return 0; }
int GetContrast(void) { return 0; }
int GetSaturation(void) { return 0; }
int GetExposure(void) { return 0; }
int GetVolume(void) { return 0; }

int GetFnToggleBrightness(void) { return 0; }
int GetFnToggleColortemp(void) { return 0; }
int GetFnToggleContrast(void) { return 0; }
int GetFnToggleSaturation(void) { return 0; }
int GetFnToggleExposure(void) { return 0; }
int GetFnToggleVolume(void) { return 0; }
int GetFnToggleDisablesDpad(void) { return 0; }
int GetFnToggleEmulatesJoystick(void) { return 0; }
int GetFnToggleTurboA(void) { return 0; }
int GetFnToggleTurboB(void) { return 0; }
int GetFnToggleTurboX(void) { return 0; }
int GetFnToggleTurboY(void) { return 0; }
int GetFnToggleTurboL1(void) { return 0; }
int GetFnToggleTurboL2(void) { return 0; }
int GetFnToggleTurboR1(void) { return 0; }
int GetFnToggleTurboR2(void) { return 0; }

void SetFnToggleBrightness(int value){}
void SetFnToggleColortemp(int value){}
void SetFnToggleContrast(int value){}
void SetFnToggleSaturation(int value){}
void SetFnToggleExposure(int value){}
void SetFnToggleVolume(int value){}
void SetFnToggleDisablesDpad(int value) {}
void SetFnToggleEmulatesJoystick(int value) {}
void SetFnToggleTurboA(int value) {}
void SetFnToggleTurboB(int value) {}
void SetFnToggleTurboX(int value) {}
void SetFnToggleTurboY(int value) {}
void SetFnToggleTurboL1(int value) {}
void SetFnToggleTurboL2(int value) {}
void SetFnToggleTurboR1(int value) {}
void SetFnToggleTurboR2(int value) {}

void SetRawBrightness(int value) {}
void SetRawVolume(int value){}

void SetBrightness(int value) {}
void SetColortemp(int value) {}
void SetContrast(int value) {}
void SetSaturation(int value) {}
void SetExposure(int value) {}
void SetVolume(int value) {}

int GetJack(void) { return 0; }
void SetJack(int value) {}

int GetAudioSink(void) { return 0; }
void SetAudioSink(int value) {}

int GetHDMI(void) { return 0; }
void SetHDMI(int value) {}

int GetMute(void) { return 0; }
int GetFnToggle(void) { return 0; }
void SetFnToggle(int value) {}

// Desktop platform stubs for vibration control (not supported)
void SetRawVibration(int value) {}
void SetVibration(int value) {}
void TestVibration(int value) {}
int GetVibration(void) { return 0; }
int GetFnToggleVibration(void) { return 0; }
void SetFnToggleVibration(int value) {}