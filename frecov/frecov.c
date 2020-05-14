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
} __attribute__((packed));

int main(int argc, char *argv[]) {
    assert(argc == 2);
    assert(sizeof(struct fat_header) == 512);
    char* fileName = argv[1];
    printf("Filename is %s\n", fileName);
    printf("SizoOf FATheader is %d\n",(int) sizeof(struct fat_header));
    int fd = open(fileName, O_RDWR, 0);
    struct fat_header* pfatheader = mmap(NULL, 512 , PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED , fd , 0);
    assert(pfatheader->Signature_word == 0xAA55);
    assert(memcmp(pfatheader->BS_FilSysType, "FAT32", 5) == 0);
    assert(fd > 0);

    printf("jmpBoot[0] is %X\n", pfatheader->BS_jmpBoot[0]);
    printf("jmpBoot[2] is %X\n", pfatheader->BS_jmpBoot[2]);
    int BPB_BytsPerSec = pfatheader->BPB_BytsPerSec;
    int BPB_SecPerClus = pfatheader->BPB_SecPerClus;
    printf("BPB_BytsPerSec is %d\tBPB_SecPerClus is %d\n", BPB_BytsPerSec, BPB_SecPerClus); 



    close(fd);
    return 0;    
}
