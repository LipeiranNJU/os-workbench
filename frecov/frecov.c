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
#include <ctype.h>
#include <math.h>
#define B 1
//#define __DEBUG__
//#define __DDEBUG__
#define KB (1024 * B)
#define MB (1024 * KB)
#define GB (1024 * KB)
#ifdef __DEBUG__
#define print(...) printf(__VA_ARGS__)
#else
#define print(...) 
#endif

#ifdef __DDEBUG__
#define printk(...) printf(__VA_ARGS__)
#else
#define printk(...) 
#endif
#define beNotKown 0
#define isBMPDir 1
#define beNotUsed 2
#define beBMPContent 3

int BPB_BytsPerSec;
int BPB_SecPerClus;
int BPB_RootClus;
int BPB_FATSz32;
int BPB_HiddSec;
int BPB_RsvdSecCnt;
int BPB_NumFATs;
int offset;
int clusSize;
inline bool inFile(const void* nowAddr, const void* fileStart, int fileSize) {
    return ((intptr_t)(nowAddr) - (intptr_t)(fileStart)) < fileSize ? true : false;
}
inline int getClusterIndex(const void* addr, const void* start, int clusterSize) {
    return ((intptr_t)addr - (intptr_t) start) / clusterSize;
}
inline void* nextClus(const void* Clus) {
    return (void*)((intptr_t)(Clus) + BPB_BytsPerSec * BPB_SecPerClus);
}

struct FATHeader {
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

struct BMPHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
}__attribute__((packed));

struct BMPInfoHeader {
    uint32_t biSize;
    int32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
}__attribute__((packed));

struct FATShortDirectory {
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

struct FATLongDirectory {
    uint8_t LDIR_Ord;
    uint16_t LDIR_Name1[5];
    uint8_t LDIR_Attr;
    uint8_t LDIR_Type;
    uint8_t LDIR_Chksum;
    uint16_t LDIR_Name2[6];
    uint16_t LDIR_FstClusLO;
    uint16_t LDIR_Name3[2];
}__attribute__((packed));
bool isFATLongDirectory(const struct FATLongDirectory* pFATldir);
bool isFATShortDirectory(const struct FATShortDirectory*);
char* readCompleteInfoFromFATShortDirectory(struct FATShortDirectory* pFATsd);
static inline struct FATLongDirectory* nextLongDirectory(struct FATLongDirectory* longDirectory){
    return (struct FATLongDirectory*)((intptr_t)(longDirectory) + sizeof(struct FATLongDirectory));
}
static inline struct FATShortDirectory* nextShortDirectory(struct FATShortDirectory* shortDirectory){
    return (struct FATShortDirectory*)((intptr_t)(shortDirectory) + sizeof(struct FATShortDirectory));
}
bool isValidFileName(char* name);
int dirClus[100];

void* getClusterFromIndex(int index_from_zero, void* initialClusterAddr) {
    return initialClusterAddr+index_from_zero*clusSize;
}
void initAttr(struct FATHeader* pfatheader) {
    BPB_BytsPerSec = pfatheader->BPB_BytsPerSec;
    BPB_SecPerClus = pfatheader->BPB_SecPerClus;
    BPB_RootClus = pfatheader->BPB_RootClus;
    BPB_FATSz32 = pfatheader->BPB_FATSz32;
    BPB_HiddSec = pfatheader->BPB_HiddSec;
    BPB_RsvdSecCnt = pfatheader->BPB_RsvdSecCnt;
    BPB_NumFATs = pfatheader->BPB_NumFATs;
    clusSize = BPB_SecPerClus * BPB_BytsPerSec;
}
void dirClusAdd(int index) {
    assert(index >= 0);
    for (int i = 0; i < 100; i++) {
        if (dirClus[i] == -1) {
            dirClus[i] = index;
            return;
        }
    }
    assert(0);
}
int main (int argc, char* argv[]) {
    char* imgName = argv[1];
    struct stat statbuf;
    stat(imgName, &statbuf);
    int imgSize = statbuf.st_size;
    int imgFd = open(imgName, O_RDONLY, 0);
    struct FATHeader* pFATHeader = (struct FATHeader *) mmap(NULL, imgSize, PROT_READ, MAP_SHARED, imgFd, 0);

    BPB_BytsPerSec = pFATHeader->BPB_BytsPerSec;
    BPB_SecPerClus = pFATHeader->BPB_SecPerClus;
    BPB_RootClus = pFATHeader->BPB_RootClus;
    BPB_FATSz32 = pFATHeader->BPB_FATSz32;
    BPB_HiddSec = pFATHeader->BPB_HiddSec;
    BPB_RsvdSecCnt = pFATHeader->BPB_RsvdSecCnt;
    BPB_NumFATs = pFATHeader->BPB_NumFATs;

    int imgOffset = (BPB_RsvdSecCnt+BPB_NumFATs*BPB_FATSz32+(BPB_RootClus-2)*BPB_SecPerClus+BPB_HiddSec)*BPB_BytsPerSec;
    struct FATShortDirectory* pFATshdir = (void* )pFATHeader + imgOffset;
    int imgDataSize = imgSize - imgOffset;
    for (struct FATShortDirectory* ptmp = pFATshdir; inFile(ptmp, pFATshdir, imgDataSize); ptmp++) {
        if (ptmp->DIR_NTRes == 0 && (ptmp->DIR_Attr >> 6) == 0) {
            for (int i = 0; i < 11; i++) {
                if (isprint(ptmp->DIR_Name[i]) || (i!=0 && ptmp->DIR_Name[i] == '\0'))
                    printf("%c", ptmp->DIR_Name[i]);
                if (i == 10)
                    printf("\n");
            }
        }

    }
    return 0;
}