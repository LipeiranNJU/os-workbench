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
#include <sys/stat.h>
#include <stdbool.h>

#define B 1
#define KB (1024 * B)
#define MB (1024 * KB)
#define GB (1024 * KB)

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
    uint8_t BS_FilSysType0;
    uint8_t BS_FilSysType1;
    uint8_t BS_FilSysType2;
    uint8_t BS_FilSysType3;
    uint8_t BS_FilSysType4;
    uint8_t BS_FilSysType5;
    uint8_t BS_FilSysType6;
    uint8_t BS_FilSysType7;
    uint8_t  padding[420];
    uint16_t Signature_word;
}__attribute__((packed));

struct FATdirectory {
    uint8_t DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
}__attribute__((packed));


void verifyFAT32Head(struct fat_header*);
void showFAT32HeadInfo(struct fat_header*);
bool isFATdirectory(struct FATdirectory*);

int main(int argc, char *argv[]) {
    assert(argc == 2);
    assert(sizeof(struct fat_header) == 512);
    assert(sizeof(struct FATdirectory) == 32);
    char* fileName = argv[1];
    printf("Filename is %s\n", fileName);


    struct stat statbuf;
    stat(fileName,&statbuf);
    int size = statbuf.st_size;
    printf("img file size is %d\n", size);

    int fd = open(fileName, O_RDONLY, 0);
    assert(fd > 0);
    printf("fd is %d\n", fd);
    struct fat_header* pfatheader =(struct fat_header*) mmap(NULL, size, PROT_READ, MAP_SHARED , fd, 0);
    printf("SizoOf FATheader is %d\n",(int) sizeof(struct fat_header));
    verifyFAT32Head(pfatheader);
    assert(pfatheader != NULL);
    showFAT32HeadInfo(pfatheader);

    int BPB_BytsPerSec = pfatheader->BPB_BytsPerSec;
    int BPB_SecPerClus = pfatheader->BPB_SecPerClus;
    int BPB_RootClus = pfatheader->BPB_RootClus;
    int BPB_FATSz32 = pfatheader->BPB_FATSz32;
    int BPB_HiddSec =pfatheader->BPB_HiddSec;
    int BPB_RsvdSecCnt = pfatheader->BPB_RsvdSecCnt;
    int BPB_NumFATs = pfatheader->BPB_NumFATs;
    int offset = (BPB_RsvdSecCnt + BPB_NumFATs * BPB_FATSz32 + (BPB_RootClus - 2) * BPB_SecPerClus + BPB_HiddSec) * BPB_BytsPerSec;
    printf("Offset of initial clus is %d\n", offset);
    struct FATdirectory* pFATdir = (struct FATdirectory*)((intptr_t)pfatheader+offset);
    int canBeUsed = 0;
    int rate = 0;
    printf("Total Sec is %d\n", (int) pfatheader->BPB_TotSec32);
    for (int i = 0; (intptr_t)(pFATdir) < (intptr_t)(pfatheader)+size; i++) {
        assert((intptr_t)pFATdir-(intptr_t)pfatheader < pfatheader->BPB_TotSec32*pfatheader->BPB_BytsPerSec);
        if (isFATdirectory(pFATdir) == true) {
            canBeUsed += 1;
        }

        assert((intptr_t) (pFATdir + 1) - (intptr_t)pFATdir == sizeof(struct FATdirectory));
        assert(pFATdir != NULL);
        fflush(stdout);
        pFATdir++;
    }
    printf("%d can be short name directory.\n",canBeUsed);
    close(fd);
    return 0;    
}

void verifyFAT32Head(struct fat_header* ptr) {
    printf("hello\n");
    // assert((memcmp(&ptr->BS_FilSysType0, "FAT32", 5) == 0));
    // assert(0);
    // assert(ptr->Signature_word == 0xAA55);
    // assert(0);
    // assert(ptr->BPB_RootEntCnt == 0);
    // assert(0);
    // assert(ptr->BPB_TotSec16 == 0);
    // assert(0);
    // assert(ptr->BPB_NumFATs == 2 || ptr->BPB_NumFATs == 1);
    // assert(0);
}

void showFAT32HeadInfo(struct fat_header* pfatheader) {
    printf("SizoOf FATheader is %d\n",(int) sizeof(struct fat_header));
    printf("jmpBoot[0] is %X\t", pfatheader->BS_jmpBoot[0]);
    printf("jmpBoot[2] is %X\n", pfatheader->BS_jmpBoot[2]);
    printf("BPB_BytsPerSec is %d\tBPB_SecPerClus is %d\n", pfatheader->BPB_BytsPerSec, pfatheader->BPB_SecPerClus); 
    printf("BPB_RootClus is %d\n", pfatheader->BPB_RootClus);
    printf("BPB_FATSz32 is %d\n", pfatheader->BPB_FATSz32);
    printf("BPB_HiddSec is %d\n", pfatheader->BPB_HiddSec);
    printf("BPB_RsvdSecCnt is %d\n", pfatheader->BPB_RsvdSecCnt);
    printf("BPB_NumFATs is %d\n", pfatheader->BPB_NumFATs);
}

bool isFATdirectory(struct FATdirectory* pFATdir) {
    if (pFATdir->DIR_CrtTime%2 == 1)
        return false;
    else if (pFATdir->DIR_CrtTimeTenth > 199 || pFATdir->DIR_CrtTimeTenth < 0)
        return false;
    else if (pFATdir->DIR_NTRes != 0)
        return false;
    else if (pFATdir->DIR_Attr & 0xB0 != 0)
        return false;
    else
        return true;
    
}