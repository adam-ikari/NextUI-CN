#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>

#include "defines.h"
#include "api.h"
#include "utils.h"
#include "i18n.h"

// USB Gadget configuration
#define USB_VENDOR_ID  "0x2207"
#define USB_PRODUCT_ID "0xa4a5"
#define USB_MAX_POWER  "500"
#define USB_LANG_CODE  "0x409"
#define GADGET_NAME   "nextui"

// SD card paths
#define SD_CARD_DEVICE "/dev/mmcblk0p1"
#define SD_CARD_MOUNT  "/mnt/SDCARD"

// Constants
#define CONFIRMATION_TIMEOUT 30

// USB states
typedef enum {
    USB_STATE_IDLE,
    USB_STATE_CONFIRMING,
    USB_STATE_ENABLING,
    USB_STATE_ENABLED,
    USB_STATE_DISABLING,
    USB_STATE_ERROR
} USBState;

static USBState usb_state = USB_STATE_IDLE;
static bool quit = false;
static int error_code = 0;
static bool usb_enabled = false;

// Signal handler
static void sigHandler(int sig) {
    quit = true;
}

// Setup USB gadget
static int setup_usb_gadget(void) {
    char cmd[512];
    int ret;
    
    // Mount configfs if needed
    if (access("/sys/kernel/config/usb_gadget", F_OK) != 0) {
        system("mount -t configfs none /sys/kernel/config");
    }
    
    // Create gadget directory
    sprintf(cmd, "mkdir -p /sys/kernel/config/usb_gadget/%s", GADGET_NAME);
    system(cmd);
    
    // Set vendor and product IDs
    sprintf(cmd, "echo %s > /sys/kernel/config/usb_gadget/%s/idVendor", USB_VENDOR_ID, GADGET_NAME);
    ret = system(cmd);
    if (ret != 0) return -1;
    
    sprintf(cmd, "echo %s > /sys/kernel/config/usb_gadget/%s/idProduct", USB_PRODUCT_ID, GADGET_NAME);
    ret = system(cmd);
    if (ret != 0) return -2;
    
    // Set device strings
    sprintf(cmd, "mkdir -p /sys/kernel/config/usb_gadget/%s/strings/0x409", GADGET_NAME);
    system(cmd);
    
    sprintf(cmd, "echo \"NextUI\" > /sys/kernel/config/usb_gadget/%s/strings/0x409/manufacturer", GADGET_NAME);
    system(cmd);
    
    sprintf(cmd, "echo \"Trimui Brick\" > /sys/kernel/config/usb_gadget/%s/strings/0x409/product", GADGET_NAME);
    system(cmd);
    
    // Get serial number
    FILE *serial_file = fopen("/proc/cpuinfo", "r");
    if (serial_file) {
        char line[256];
        while (fgets(line, sizeof(line), serial_file)) {
            if (strstr(line, "Serial")) {
                char *serial = strchr(line, ':');
                if (serial && strlen(serial) > 2) {
                    sprintf(cmd, "echo \"%s\" > /sys/kernel/config/usb_gadget/%s/strings/0x409/serialnumber", 
                            serial + 2, GADGET_NAME);
                    system(cmd);
                    break;
                }
            }
        }
        fclose(serial_file);
    }
    
    // Create config
    sprintf(cmd, "mkdir -p /sys/kernel/config/usb_gadget/%s/configs/c.1/strings/0x409", GADGET_NAME);
    system(cmd);
    
    sprintf(cmd, "echo %s > /sys/kernel/config/usb_gadget/%s/configs/c.1/MaxPower", USB_MAX_POWER, GADGET_NAME);
    system(cmd);
    
    sprintf(cmd, "echo \"Mass Storage\" > /sys/kernel/config/usb_gadget/%s/configs/c.1/strings/0x409/configuration", GADGET_NAME);
    system(cmd);
    
    // Create mass storage function
    sprintf(cmd, "mkdir -p /sys/kernel/config/usb_gadget/%s/functions/mass_storage.0", GADGET_NAME);
    system(cmd);
    
    sprintf(cmd, "echo %s > /sys/kernel/config/usb_gadget/%s/functions/mass_storage.0/lun.0/file", 
            SD_CARD_DEVICE, GADGET_NAME);
    ret = system(cmd);
    if (ret != 0) return -3;
    
    // Link function to config
    sprintf(cmd, "ln -s /sys/kernel/config/usb_gadget/%s/functions/mass_storage.0 /sys/kernel/config/usb_gadget/%s/configs/c.1/mass_storage.0",
            GADGET_NAME, GADGET_NAME);
    system(cmd);
    
    // Find and activate UDC
    FILE *udc_file = fopen("/sys/class/udc/", "r");
    if (!udc_file) return -4;
    
    char udc[64];
    if (fscanf(udc_file, "%s", udc) == 1) {
        sprintf(cmd, "echo %s > /sys/kernel/config/usb_gadget/%s/UDC", udc, GADGET_NAME);
        ret = system(cmd);
        if (ret != 0) {
            fclose(udc_file);
            return -5;
        }
    }
    fclose(udc_file);
    
    return 0;
}

// Cleanup USB gadget
static void cleanup_usb_gadget(void) {
    char cmd[512];
    
    // Disconnect UDC
    sprintf(cmd, "echo \"\" > /sys/kernel/config/usb_gadget/%s/UDC", GADGET_NAME);
    system(cmd);
    
    // Remove function link
    sprintf(cmd, "rm -f /sys/kernel/config/usb_gadget/%s/configs/c.1/mass_storage.0", GADGET_NAME);
    system(cmd);
    
    // Remove function directory
    sprintf(cmd, "rm -rf /sys/kernel/config/usb_gadget/%s/functions/mass_storage.0", GADGET_NAME);
    system(cmd);
    
    // Remove config directory
    sprintf(cmd, "rm -rf /sys/kernel/config/usb_gadget/%s/configs/c.1", GADGET_NAME);
    system(cmd);
    
    // Remove gadget directory
    sprintf(cmd, "rm -rf /sys/kernel/config/usb_gadget/%s", GADGET_NAME);
    system(cmd);
}

// Unmount SD card
static int unmount_sdcards(void) {
    if (access(SD_CARD_MOUNT, F_OK) != 0) return 0; // Already unmounted
    
    int ret = system("umount " SD_CARD_MOUNT);
    if (ret != 0) {
        return -1;
    }
    return 0;
}

// Mount SD card
static int mount_sdcards(void) {
    if (access(SD_CARD_MOUNT, F_OK) == 0) return 0; // Already mounted
    
    int ret = system("mount -t vfat " SD_CARD_DEVICE " " SD_CARD_MOUNT);
    if (ret != 0) {
        return -1;
    }
    return 0;
}

// Render warning screen
static void render_warning(void) {
    SDL_Surface *screen = GFX_getScreen();
    GFX_clear(screen);
    
    // Title
    char title[256];
    sprintf(title, TR("usbms.title"));
    
    int max_width = screen->w - SCALE1(PADDING * 2);
    char truncated[256];
    int text_width = GFX_truncateText(font.large, title, truncated, max_width, SCALE1(BUTTON_PADDING * 2));
    
    SDL_Surface *text = TTF_RenderUTF8_Blended(font.large, truncated, COLOR_WHITE);
    GFX_blitPill(ASSET_BLACK_PILL, screen, &(SDL_Rect){SCALE1(PADDING), SCALE1(PADDING), text_width, SCALE1(PILL_SIZE)});
    SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){SCALE1(PADDING + BUTTON_PADDING), SCALE1(PADDING + 4)});
    SDL_FreeSurface(text);
    
    // Warning messages
    int y = SCALE1(PADDING * 2 + PILL_SIZE + 20);
    
    const char* warnings[] = {
        TR("usbms.warning_1"),
        TR("usbms.warning_2"),
        TR("usbms.warning_3"),
        NULL
    };
    
    for (int i = 0; warnings[i]; i++) {
        text = TTF_RenderUTF8_Blended(font.medium, warnings[i], COLOR_YELLOW);
        SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){SCALE1(PADDING), y});
        SDL_FreeSurface(text);
        y += SCALE1(30);
    }
    
    // Hardware group
    GFX_blitHardwareGroup(screen, 0);
    
    // Buttons
    GFX_blitButtonGroup((char*[]) {"A", TR("usbms.enable"), "B", TR("common.cancel"), NULL}, 1, screen, 1);
    
    GFX_flip(screen);
}

// Render status screen
static void render_status(const char* status_text, const char* subtext) {
    SDL_Surface *screen = GFX_getScreen();
    GFX_clear(screen);
    
    // Title
    char title[256];
    sprintf(title, TR("usbms.title"));
    
    int max_width = screen->w - SCALE1(PADDING * 2);
    char truncated[256];
    int text_width = GFX_truncateText(font.large, title, truncated, max_width, SCALE1(BUTTON_PADDING * 2));
    
    SDL_Surface *text = TTF_RenderUTF8_Blended(font.large, truncated, COLOR_WHITE);
    GFX_blitPill(ASSET_BLACK_PILL, screen, &(SDL_Rect){SCALE1(PADDING), SCALE1(PADDING), text_width, SCALE1(PILL_SIZE)});
    SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){SCALE1(PADDING + BUTTON_PADDING), SCALE1(PADDING + 4)});
    SDL_FreeSurface(text);
    
    // Status text
    int y = SCALE1(PADDING * 2 + PILL_SIZE + 40);
    text = TTF_RenderUTF8_Blended(font.large, status_text, COLOR_GREEN);
    SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){SCALE1(PADDING), y});
    SDL_FreeSurface(text);
    
    if (subtext) {
        y += SCALE1(40);
        text = TTF_RenderUTF8_Blended(font.medium, subtext, COLOR_WHITE);
        SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){SCALE1(PADDING), y});
        SDL_FreeSurface(text);
    }
    
    // Hardware group
    GFX_blitHardwareGroup(screen, 0);
    
    GFX_flip(screen);
}

// Render error screen
static void render_error(const char* error_text) {
    SDL_Surface *screen = GFX_getScreen();
    GFX_clear(screen);
    
    // Title
    char title[256];
    sprintf(title, TR("usbms.error"));
    
    int max_width = screen->w - SCALE1(PADDING * 2);
    char truncated[256];
    int text_width = GFX_truncateText(font.large, title, truncated, max_width, SCALE1(BUTTON_PADDING * 2));
    
    SDL_Surface *text = TTF_RenderUTF8_Blended(font.large, truncated, COLOR_RED);
    GFX_blitPill(ASSET_BLACK_PILL, screen, &(SDL_Rect){SCALE1(PADDING), SCALE1(PADDING), text_width, SCALE1(PILL_SIZE)});
    SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){SCALE1(PADDING + BUTTON_PADDING), SCALE1(PADDING + 4)});
    SDL_FreeSurface(text);
    
    // Error text
    int y = SCALE1(PADDING * 2 + PILL_SIZE + 40);
    text = TTF_RenderUTF8_Blended(font.large, error_text, COLOR_RED);
    SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){SCALE1(PADDING), y});
    SDL_FreeSurface(text);
    
    // Hardware group
    GFX_blitHardwareGroup(screen, 0);
    
    // Buttons
    GFX_blitButtonGroup((char*[]) {"B", TR("common.exit"), NULL}, 1, screen, 1);
    
    GFX_flip(screen);
}

int main(int argc, char *argv[]) {
    // Initialize systems
    GFX_init();
    PAD_init();
    PWR_init();
    
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    
    bool dirty = true;
    
    while (!quit) {
        PAD_poll();
        
        if (PAD_justPressed(BTN_MENU)) {
            // Ignore menu button
        }
        else {
            switch (usb_state) {
                case USB_STATE_IDLE:
                    // Go to confirmation
                    usb_state = USB_STATE_CONFIRMING;
                    dirty = true;
                    break;
                    
                case USB_STATE_CONFIRMING:
                    if (PAD_justPressed(BTN_A)) {
                        // Confirm - start enabling
                        usb_state = USB_STATE_ENABLING;
                        dirty = true;
                    }
                    else if (PAD_justPressed(BTN_B)) {
                        // Cancel - exit
                        quit = true;
                    }
                    break;
                    
                case USB_STATE_ENABLING:
                    // Perform enable steps
                    if (unmount_sdcards() == 0) {
                        if (setup_usb_gadget() == 0) {
                            usb_state = USB_STATE_ENABLED;
                            usb_enabled = true;
                        } else {
                            error_code = 1;
                            usb_state = USB_STATE_ERROR;
                            // Remount on error
                            mount_sdcards();
                        }
                    } else {
                        error_code = 2;
                        usb_state = USB_STATE_ERROR;
                    }
                    dirty = true;
                    break;
                    
                case USB_STATE_ENABLED:
                    if (PAD_justPressed(BTN_A) || PAD_justPressed(BTN_B)) {
                        // Any button to disable
                        usb_state = USB_STATE_DISABLING;
                        dirty = true;
                    }
                    break;
                    
                case USB_STATE_DISABLING:
                    // Perform disable steps
                    cleanup_usb_gadget();
                    if (mount_sdcards() == 0) {
                        usb_enabled = false;
                        quit = true;
                    } else {
                        error_code = 3;
                        usb_state = USB_STATE_ERROR;
                    }
                    dirty = true;
                    break;
                    
                case USB_STATE_ERROR:
                    if (PAD_justPressed(BTN_B)) {
                        quit = true;
                    }
                    break;
            }
        }
        
        // Update power
        PWR_update(&dirty, NULL, NULL, NULL);
        
        if (dirty) {
            switch (usb_state) {
                case USB_STATE_CONFIRMING:
                    render_warning();
                    break;
                    
                case USB_STATE_ENABLING:
                    render_status(TR("usbms.enabling"), TR("usbms.please_wait"));
                    break;
                    
                case USB_STATE_ENABLED:
                    render_status(TR("usbms.enabled"), TR("usbms.press_to_disable"));
                    break;
                    
                case USB_STATE_DISABLING:
                    render_status(TR("usbms.disabling"), TR("usbms.please_wait"));
                    break;
                    
                case USB_STATE_ERROR:
                    switch (error_code) {
                        case 1:
                            render_error(TR("usbms.error_gadget"));
                            break;
                        case 2:
                            render_error(TR("usbms.error_unmount"));
                            break;
                        case 3:
                            render_error(TR("usbms.error_mount"));
                            break;
                        default:
                            render_error(TR("usbms.error_unknown"));
                            break;
                    }
                    break;
                    
                default:
                    break;
            }
            
            dirty = false;
        }
        else {
            GFX_sync();
        }
        
        // Frame limiting
        SDL_Delay(16);
    }
    
    // Cleanup on exit
    if (usb_enabled) {
        cleanup_usb_gadget();
        mount_sdcards();
    }
    
    PWR_quit();
    PAD_quit();
    GFX_quit();
    
    return EXIT_SUCCESS;
}
