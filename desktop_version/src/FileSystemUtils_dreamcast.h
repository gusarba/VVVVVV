#ifndef FILESYSTEMUTILS_DREAMCAST_H
#define FILESYSTEMUTILS_DREAMCAST_H 1

int PLATFORM_initVmu(char* vmuPath, int port, int slot);

bool PLATFORM_saveToVmu(const char *name);
bool PLATFORM_loadFromVmu(const char *name);

// This is technically not a filesystem related operation, but what the hell
void PLATFORM_drawToVmu();

#endif /* FILESYSTEMUTILS_H */
