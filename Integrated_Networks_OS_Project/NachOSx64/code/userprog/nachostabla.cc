#include "nachostabla.h"

#define MAX_OPEN_FILES 100

NachosOpenFilesTable::NachosOpenFilesTable() {
    openFiles = new int[MAX_OPEN_FILES];
    openFilesMap = new BitMap(MAX_OPEN_FILES);
    for(int i = 0; i < MAX_OPEN_FILES ;i++) {
        openFiles[i] = -1;

    }
    // This is reserved for stdin, stdout and stderr
    openFilesMap->Mark(0);
    openFilesMap->Mark(1);
    openFilesMap->Mark(2);
    this->usage = 1;
    openFiles[0] = 0; // stdin
    openFiles[1] = 1; // stdout
    openFiles[2] = 2; // stderr
}

NachosOpenFilesTable::~NachosOpenFilesTable() {
    delete[] openFiles;
    delete openFilesMap;
}

int NachosOpenFilesTable::Open(int UnixHandle) {
    int index = this->openFilesMap->Find();
    if (index == -1) {
        return -1;
    }
    this->openFiles[index] = UnixHandle;
    return index;
}

int NachosOpenFilesTable::Close(int NachosHandle) {
    if(isOpened(NachosHandle)){
        int index = this->openFiles[NachosHandle];
        openFilesMap->Clear(NachosHandle);
        this->openFiles[index] = -1;
        return index;
    }
    return -1;
}

bool NachosOpenFilesTable::isOpened(int NachosHandle) {
    return this->openFilesMap->Test(NachosHandle) && this->openFiles[NachosHandle] != -1;
}

int NachosOpenFilesTable::getUnixHandle(int NachosHandle) {
    return this->openFiles[NachosHandle];
}

void NachosOpenFilesTable::addThread() {
    usage++;
}

void NachosOpenFilesTable::delThread() {
    usage--;
}

void NachosOpenFilesTable::Print() {
    printf("Open files table:\n");
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        printf("File %d: %d\n", i, openFiles[i]);
    }
}