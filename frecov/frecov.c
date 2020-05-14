#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
struct fat_header {
  uint8_t  BS_jmpBoot[3];
  uint8_t  BS_OEMName[8];
  uint16_t BPB_BytsPerSec;
  uint8_t BPB_SecPerClus;
  uint16_t BPB_RsvdSecCnt;
  uint8_t BPB_NumFATs;
  uint16_t BPB_RootEntCnt;
  uint16_t BPB_TotSec16;
  uint8_t BPB_Media;
  uint16_t BPB_FATSz16;
  uint16_t BPB_SecPerTrk;
  uint16_t BPB_NumHeads;
  uint32_t BPB_HiddSec;
  uint32_t BPB_TotSec32;
  uint32_t BPB_FATSz32;
  uint16_t BPB_ExtFlags;
  uint16_t BPB_FSVer;
  uint32_t BPB_RootClus;
  uint16_t BPB_FSInfo;
  uint16_t BPB_BkBootSec;
  uint8_t BPB_Reserved[12];
  uint8_t BS_DrvNum;
  uint8_t BS_Reserved;
  uint8_t BS_BootSig;
  uint32_t BS_VolID; 
  uint8_t BS_VolLab[11];
  uint8_t BS_FilSysType[8];
  uint8_t  padding[420];
  uint16_t Signature_word;
} __attribute__ ((packed));

void verifyFAT32Head(struct fat_header*);
void showFAT32HeadInfo(struct fat_header*);
int main(int argc, char *argv[]) {
    assert(argc == 2);
    assert(sizeof(struct fat_header) == 512);
    char* fileName = argv[1];
    printf("Filename is %s\n", fileName);
    int fd = open(fileName, O_RDWR, 0);
    struct fat_header* pfatheader = mmap(NULL, 512 , PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED , fd , 0);
    assert(fd > 0);

    verifyFAT32Head(pfatheader);
    showFAT32HeadInfo(pfatheader);

    int BPB_BytsPerSec = pfatheader->BPB_BytsPerSec;
    int BPB_SecPerClus = pfatheader->BPB_SecPerClus;
    int offset = pfatheader->BPB_RsvdSecCnt;
    int BPB_RootClus = pfatheader->BPB_RootClus;
    int BPB_FATSz32 = pfatheader->BPB_FATSz32;
    int BPB_HiddSec =pfatheader->BPB_HiddSec;
    close(fd);
    return 0;    
}

void verifyFAT32Head(struct fat_header* ptr) {
    assert((memcmp(ptr->BS_FilSysType, "FAT32", 5) == 0));
    assert(ptr->Signature_word == 0xAA55);
    assert(ptr->BPB_RootEntCnt == 0);
    assert(ptr->BPB_TotSec16 == 0);
}

void showFAT32HeadInfo(struct fat_header* pfatheader) {
    printf("SizoOf FATheader is %d\n",(int) sizeof(struct fat_header));
    printf("jmpBoot[0] is %X\n", pfatheader->BS_jmpBoot[0]);
    printf("jmpBoot[2] is %X\n", pfatheader->BS_jmpBoot[2]);
    printf("BPB_BytsPerSec is %d\tBPB_SecPerClus is %d\n", pfatheader->BPB_BytsPerSec, pfatheader->BPB_SecPerClus); 
    printf("BPB_RootClus is %d\n", pfatheader->BPB_RootClus);
    printf("BPB_FATSz32 is %d\n", pfatheader->BPB_FATSz32);
    printf("BPB_HiddSec is %d\n", pfatheader->BPB_HiddSec);
}