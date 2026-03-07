#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libpad.h>
#include <fileXio_rpc.h>
#include <libfat.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <sbv_patches.h>
#include <unistd.h>

// Colors
#define COLOR_RESET "\033[0m"
#define COLOR_BLUE "\033[34m"
#define COLOR_WHITE "\033[37m"
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"

// Menu options
#define OPTION_REBUILD 1
#define OPTION_MERGE 2
#define OPTION_EXIT 3

// Function prototypes
void initPS2();
void initPad();
void initFileXio();
void initLibfat();
void displayMenu();
int getPadInput();
void rebuildUlCfg();
void mergeUlCfg();
void backupUlCfg();
int countUlFolders();
void parseUlCfg(const char* path, char gameIds[100][12], char ulPaths[100][20], int* count);
void writeUlCfg(char gameIds[100][12], char ulPaths[100][20], int count);
int isDuplicate(char gameIds[100][12], int count, const char* gameId);

int main() {
    initPS2();
    initPad();
    initFileXio();
    initLibfat();

    int choice = 0;
    while (choice != OPTION_EXIT) {
        displayMenu();
        choice = getPadInput();
        switch (choice) {
            case OPTION_REBUILD:
                rebuildUlCfg();
                break;
            case OPTION_MERGE:
                mergeUlCfg();
                break;
            case OPTION_EXIT:
                printf(COLOR_GREEN "Exiting ULCFG Manager Lite...\n" COLOR_RESET);
                break;
            default:
                printf(COLOR_RED "Invalid option. Please try again.\n" COLOR_RESET);
                break;
        }
        sleep(2); // Wait a bit before returning to menu
    }

    return 0;
}

void initPS2() {
    SifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:MCMAN", 0, NULL);
    SifLoadModule("rom0:MCSERV", 0, NULL);
}

void initPad() {
    padInit(0);
    padPortOpen(0, 0, NULL);
}

void initFileXio() {
    fileXioInit();
}

void initLibfat() {
    fatInitDefault();
}

void displayMenu() {
    printf("\033[2J\033[H"); // Clear screen
    printf(COLOR_BLUE "--------------------------------\n" COLOR_RESET);
    printf(COLOR_BLUE "      ULCFG MANAGER LITE\n" COLOR_RESET);
    printf(COLOR_BLUE "--------------------------------\n" COLOR_RESET);
    printf(COLOR_WHITE "USB: mass:/\n" COLOR_RESET);
    printf(COLOR_BLUE "--------------------------------\n" COLOR_RESET);
    printf(COLOR_WHITE "1 - Rebuild ul.cfg\n" COLOR_RESET);
    printf(COLOR_WHITE "2 - Merge ul.cfg\n" COLOR_RESET);
    printf(COLOR_WHITE "3 - Exit\n" COLOR_RESET);
    printf(COLOR_BLUE "--------------------------------\n" COLOR_RESET);
    printf(COLOR_WHITE "X = select, O = back\n" COLOR_RESET);
    printf(COLOR_BLUE "--------------------------------\n" COLOR_RESET);
}

int getPadInput() {
    struct padButtonStatus buttons;
    while (1) {
        padRead(0, 0, &buttons);
        if (buttons.btns & PAD_CROSS) {
            return OPTION_REBUILD;
        } else if (buttons.btns & PAD_SQUARE) {
            return OPTION_MERGE;
        } else if (buttons.btns & PAD_CIRCLE) {
            return OPTION_EXIT;
        }
        // Wait a bit
        usleep(10000);
    }
}

void rebuildUlCfg() {
    printf(COLOR_GREEN "Rebuilding ul.cfg...\n" COLOR_RESET);
    backupUlCfg();

    // Scan for ul.* folders
    int ulCount = 0;
    char gameIds[100][12];
    char ulPaths[100][20];

    // Assume ul.000 to ul.099 or something, but scan directory
    // For simplicity, assume ul.000 to ul.050
    for (int i = 0; i < 50; i++) {
        char ulPath[20];
        sprintf(ulPath, "mass:/ul.%03d", i);
        char gameidPath[30];
        sprintf(gameidPath, "%s/gameid.txt", ulPath);
        int fd = fileXioOpen(gameidPath, O_RDONLY, 0);
        if (fd >= 0) {
            char gameId[12];
            int read = fileXioRead(fd, gameId, sizeof(gameId) - 1);
            gameId[read] = '\0';
            // Trim newline
            char* nl = strchr(gameId, '\n');
            if (nl) *nl = '\0';
            if (!isDuplicate(gameIds, ulCount, gameId)) {
                strcpy(gameIds[ulCount], gameId);
                strcpy(ulPaths[ulCount], ulPath + 6); // Remove mass:/
                ulCount++;
            }
            fileXioClose(fd);
        }
    }

    writeUlCfg(gameIds, ulPaths, ulCount);
    printf(COLOR_GREEN "Rebuild complete. Games found: %d\n" COLOR_RESET, ulCount);
}

void mergeUlCfg() {
    printf(COLOR_GREEN "Merging ul.cfg...\n" COLOR_RESET);
    backupUlCfg();

    // Scan for .cfg files
    // For simplicity, assume ul.cfg, ul(1).cfg, etc.
    char gameIds[100][12];
    char ulPaths[100][20];
    int totalCount = 0;

    for (int i = 0; i < 10; i++) {
        char cfgPath[20];
        if (i == 0) {
            strcpy(cfgPath, "mass:/ul.cfg");
        } else {
            sprintf(cfgPath, "mass:/ul(%d).cfg", i);
        }
        int fd = fileXioOpen(cfgPath, O_RDONLY, 0);
        if (fd >= 0) {
            parseUlCfg(cfgPath, gameIds, ulPaths, &totalCount);
            fileXioClose(fd);
        }
    }

    writeUlCfg(gameIds, ulPaths, totalCount);
    printf(COLOR_GREEN "Merge complete. Files merged: %d\n" COLOR_RESET, totalCount);
}

void backupUlCfg() {
    if (fileXioExists("mass:/ul.cfg")) {
        fileXioRename("mass:/ul.cfg", "mass:/ul.cfg.bak");
        printf(COLOR_GREEN "Backed up ul.cfg to ul.cfg.bak\n" COLOR_RESET);
    }
}

void parseUlCfg(const char* path, char gameIds[100][12], char ulPaths[100][20], int* count) {
    int fd = fileXioOpen(path, O_RDONLY, 0);
    if (fd < 0) return;

    char buffer[256];
    int read;
    while ((read = fileXioRead(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[read] = '\0';
        char* line = strtok(buffer, "\n");
        while (line) {
            char gameId[12], ulPath[20];
            if (sscanf(line, "%11s = %19s", gameId, ulPath) == 2) {
                if (!isDuplicate(gameIds, *count, gameId)) {
                    strcpy(gameIds[*count], gameId);
                    strcpy(ulPaths[*count], ulPath);
                    (*count)++;
                }
            }
            line = strtok(NULL, "\n");
        }
    }
    fileXioClose(fd);
}

void writeUlCfg(char gameIds[100][12], char ulPaths[100][20], int count) {
    int fd = fileXioOpen("mass:/ul.cfg", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        printf(COLOR_RED "Failed to create ul.cfg\n" COLOR_RESET);
        return;
    }
    for (int i = 0; i < count; i++) {
        char line[40];
        sprintf(line, "%s = %s\n", gameIds[i], ulPaths[i]);
        fileXioWrite(fd, line, strlen(line));
    }
    fileXioClose(fd);
}

int isDuplicate(char gameIds[100][12], int count, const char* gameId) {
    for (int i = 0; i < count; i++) {
        if (strcmp(gameIds[i], gameId) == 0) return 1;
    }
    return 0;
}