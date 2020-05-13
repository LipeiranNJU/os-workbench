#include <stdio.h>
#include <assert.h>
#include <stdint.h>
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
  uint16_t signature;
} __attribute__((packed));

int main(int argc, char *argv[]) {
    assert(argc == 2);
    assert(sizeof(struct fat_header) == 512);
    printf("Filename is %s\n",argv[1]);
    return 0;    
}
