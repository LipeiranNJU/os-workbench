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
#define Unknown (-1)
#define DirEntry 1
#define beNotUsed 2
#define BMPContent 3

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
    uint8_t bfType[2];
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
double* sobelY(uint8_t* lowerline, uint8_t* nowline, uint8_t* higherline, int pixels);
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

    BPB_BytsPerSec = 512;
    BPB_SecPerClus = 8;
    BPB_RootClus = pFATHeader->BPB_RootClus;
    BPB_FATSz32 = pFATHeader->BPB_FATSz32;
    BPB_HiddSec = pFATHeader->BPB_HiddSec;
    BPB_RsvdSecCnt = pFATHeader->BPB_RsvdSecCnt;
    BPB_NumFATs = pFATHeader->BPB_NumFATs;
    clusSize = BPB_BytsPerSec * BPB_SecPerClus;
    int imgOffset = (BPB_RsvdSecCnt+BPB_NumFATs*BPB_FATSz32+(BPB_RootClus-2)*BPB_SecPerClus+BPB_HiddSec)*BPB_BytsPerSec;
    struct FATShortDirectory* pFATshdir = (void* )pFATHeader + imgOffset;
    void* imgDataStart = (void* )pFATHeader + imgOffset;
    int imgDataSize = imgSize - imgOffset;
    int tmpi = 0;
    int clusNum = imgDataSize / clusSize;
    int* cluses = malloc(sizeof(int)*clusNum);
    for (int i = 0; i < clusNum; i++) 
        cluses[i] = Unknown;
    for (void* cluster = imgDataStart; inFile(cluster, imgDataStart, imgDataSize); cluster = nextClus(cluster)) {
        int countsh = 0;
        int countl = 0;
        for (struct FATShortDirectory* ptmp = cluster; inFile(ptmp, cluster, clusSize); ptmp++) {
            if (isFATShortDirectory(ptmp)) {
                char nameTmp[12];
                memcpy(nameTmp, ptmp->DIR_Name, 11);
                nameTmp[11] = '\0';
                tmpi++;
                countsh++;
                if (isFATLongDirectory((void*)(ptmp-1)))
                    countl++;
                if (isFATLongDirectory((void*)(ptmp-2)))
                    countl++;
            }
        }
        if (countsh > 6 && countl >= countsh) {
            int index = getClusterIndex(cluster, imgDataStart, clusSize);
            // printf("%p\n", cluster);
            cluses[index] = DirEntry;
        }
    }
    for (int i = 0; i < clusNum; i++) {
        if (cluses[i] == DirEntry) {
            // printf("index:%d\n", i);
            void* cluster = getClusterFromIndex(i, imgDataStart);
            // printf("%p\n", cluster);
            int j = 0;
            for (struct FATShortDirectory* ptmp = cluster; inFile(ptmp, cluster, clusSize); ptmp++) {
                if (isFATShortDirectory(ptmp) && isFATLongDirectory((void*)(ptmp-1))) {
                    char nameTmp[12];
                    memcpy(nameTmp, ptmp->DIR_Name, 11);
                    nameTmp[11] = '\0';
                    // printf("%s\t%d\n", nameTmp, ++j);
                    char* name = readCompleteInfoFromFATShortDirectory(ptmp);
                    if (name != NULL) {
                        char* prefix = "/tmp/";
                        int picNameSize = strlen(prefix)+strlen(name)+1;
                        char* abspath = malloc(sizeof(char) * picNameSize);
                        memset(abspath, '\0', picNameSize);
                        strcat(strcat(abspath, prefix), name);

                        struct BMPHeader* picStart = (void*) (imgOffset+(uintptr_t)pFATHeader+(ptmp->DIR_FstClusLO-BPB_RootClus)*clusSize);
                        FILE* pfdpic = fopen(abspath, "w+");
                        fwrite(picStart, 1, sizeof(*picStart), pfdpic);
                        fclose(pfdpic);
                        pfdpic = fopen(abspath, "a+");
                        struct BMPInfoHeader* picInfo = (struct BMPInfoHeader*)(picStart+1);
                        fwrite(picInfo, 1, picStart->bfOffBits-sizeof(*picStart), pfdpic);
                        // assert(picStart->bfOffBits%4 == 0);
                        // assert(picInfo->biSize+sizeof(*picStart) == picStart->bfSize - picStart->bfOffBits);
                        fclose(pfdpic);

                        pfdpic = fopen(abspath, "a+");
                        void* picData = (struct BMPInfoHeader*)(picInfo+1);
                        int picDataSize = picStart->bfSize-picStart->bfOffBits;
                        
                        int ByteperPixel = picInfo->biBitCount/8;
                        int picHeight = abs(picInfo->biHeight);
                        int realWidthSize = (picInfo->biWidth*picInfo->biBitCount+31)/32*4;
                        uint8_t* picture = malloc(picDataSize);
                        uint8_t* lowerline = malloc(realWidthSize);
                        uint8_t* nowline = malloc(realWidthSize);
                        uint8_t* higherline = malloc(realWidthSize);
                        uint8_t* tmpnowline = malloc(realWidthSize);
                        uint8_t* tmphigherline = malloc(realWidthSize);
                        bool blank = false;
                        void* source = NULL;
                        source = picData;
                        for (int i = 0; i < picHeight; i++) {
                            memcpy(nowline, source+i*realWidthSize, realWidthSize);
                            if (i != picHeight-1 && i!= 0) {
                                memcpy(higherline, source+(i+1)*realWidthSize, realWidthSize);
                                if (strcmp(name, "5YpvCYAOItJaxUBL.bmp") == 0) {
                                    if (getClusterIndex(source+i*realWidthSize, imgDataStart, clusSize) != getClusterIndex(source+(i-1)*realWidthSize, imgDataStart, clusSize)) {
                                        double* g = sobelY(lowerline, nowline, higherline, realWidthSize/ByteperPixel);
                                        if (*g>20000000) {
                                            int countUpper = 0;
                                            double tmpLow = *g;
                                            int tmpLowIndex = -1;
                                            for (int i = 0; i < clusNum; i++) {
                                                void* tmpcluster = getClusterFromIndex(i, imgDataStart);
                                                memcpy(tmpnowline, tmpcluster, realWidthSize);
                                                memcpy(tmphigherline, tmpcluster+realWidthSize, realWidthSize);
                                                double* tmpd = sobelY(lowerline,tmpnowline, tmphigherline, realWidthSize/ByteperPixel);
                                                if (*tmpd < tmpLow && *tmpd > 1) {
                                                    tmpLow = *tmpd;
                                                    tmpLowIndex = i;
                                                    countUpper++;
                                                }
                                                // assert(*tmpd >= *g);
                                            }
                                            if (countUpper > 5) {
                                                printf("CountUpper%d\n", countUpper);
                                                void* newCluster = getClusterFromIndex(tmpLowIndex, imgDataStart);
                                                source = newCluster - i*realWidthSize;
                                                *g = tmpLow;
                                                memcpy(nowline, source+i*realWidthSize, realWidthSize);
                                            }
                                            
                                        }
                                        printf("mean:%lf\n", *g);
                                        // for (int j = 0; j < realWidthSize/ByteperPixel-2; j++)
                                        //     printf("%lf\t", g[j]);
                                        // printf("\n");
                                        if (g[0]>190)
                                            blank = false;
                                        else
                                            blank = false;
                                    }
                                }
                            }
                            if (blank) {
                                memset(picture+i*realWidthSize, 0xff, picDataSize-i*realWidthSize);
                                break;
                            }
                            memcpy(picture+i*realWidthSize, nowline, realWidthSize);
                            memcpy(lowerline, nowline, realWidthSize);
                        }

                        fwrite(picture, 1, picDataSize, pfdpic);
                        // free(picture);
                        // free(lowerline);
                        // free(nowline);
                        // free(higherline);
                        picture = lowerline = nowline = higherline = NULL;
                        fclose(pfdpic);
                        char buf[41];
                        buf[40] = 0;
                        char cmd[100];
                        memset(cmd, 0, 100);
                        int pipefds[2];
                        if (pipe(pipefds) < 0) {
                            assert(0);
                        }
                        int pid = fork();
                        char* argv[3];
                        argv[0] = "sha1sum";
                        argv[1] = abspath;
                        argv[2] = NULL;
                        if (pid == 0) {
                            close(pipefds[0]);
                            dup2(pipefds[1], fileno(stderr));
                            dup2(pipefds[1], fileno(stdout));
                            execvp("sha1sum", argv);
                        } else {
                            close(pipefds[1]);
                            read(pipefds[0], buf, 40);
                            printf("%s   %s\n", buf, name);
                        }
                        
                        // for (int j = 0; j < 40; j++)
                        //     printf("a");
                        // printf("   ");
                        // fflush(stdout);
                        // printf("%s\n", name);
                    }
                }
            }
        }
    }
    return 0;
}

bool isFATShortDirectory(const struct FATShortDirectory* ptmp) {
    if (strncmp((char*)&ptmp->DIR_Name[8], "BMP", 3) == 0) {
        return true;
    }
    return false;
}

bool isFATLongDirectory(const struct FATLongDirectory* pFATldir) {
    if ((pFATldir->LDIR_Ord >> 4) != 0x4 && (pFATldir->LDIR_Ord >> 4) != 0) {
        return false;
    }
    if (pFATldir->LDIR_FstClusLO != 0 || pFATldir->LDIR_Type != 0)
        return false;
    else    
        return true;
}
char* readCompleteInfoFromFATShortDirectory(struct FATShortDirectory* pFATsd) {
    
    char* name = malloc(sizeof(char) * 200);
    memset(name, '\0', 200);
    struct FATLongDirectory* pFATld = (struct FATLongDirectory*) (pFATsd-1);
    int i = -1;
    if ((pFATld->LDIR_Ord >> 4) != 0x4 && (pFATld->LDIR_Ord >> 4) != 0) {
        return NULL;
    }
    while((pFATld->LDIR_Ord >> 4)==0) {
        i += 1;
        name[i*13+0] = (char) pFATld->LDIR_Name1[0];
        name[i*13+1] = (char) pFATld->LDIR_Name1[1];
        name[i*13+2] = (char) pFATld->LDIR_Name1[2];
        name[i*13+3] = (char) pFATld->LDIR_Name1[3];
        name[i*13+4] = (char) pFATld->LDIR_Name1[4];
        name[i*13+5] = (char) pFATld->LDIR_Name2[0];
        name[i*13+6] = (char) pFATld->LDIR_Name2[1];
        name[i*13+7] = (char) pFATld->LDIR_Name2[2];
        name[i*13+8] = (char) pFATld->LDIR_Name2[3];
        name[i*13+9] = (char) pFATld->LDIR_Name2[4];
        name[i*13+10] = (char) pFATld->LDIR_Name2[5];
        name[i*13+11] = (char) pFATld->LDIR_Name3[0];
        name[i*13+12] = (char) pFATld->LDIR_Name3[1];
        pFATld = pFATld-1;
    }
    i += 1;
    name[i*13+0] = (char) pFATld->LDIR_Name1[0];
    name[i*13+1] = (char) pFATld->LDIR_Name1[1];
    name[i*13+2] = (char) pFATld->LDIR_Name1[2];
    name[i*13+3] = (char) pFATld->LDIR_Name1[3];
    name[i*13+4] = (char) pFATld->LDIR_Name1[4];
    name[i*13+5] = (char) pFATld->LDIR_Name2[0];
    name[i*13+6] = (char) pFATld->LDIR_Name2[1];
    name[i*13+7] = (char) pFATld->LDIR_Name2[2];
    name[i*13+8] = (char) pFATld->LDIR_Name2[3];
    name[i*13+9] = (char) pFATld->LDIR_Name2[4];
    name[i*13+10] = (char) pFATld->LDIR_Name2[5];
    name[i*13+11] = (char) pFATld->LDIR_Name3[0];
    name[i*13+12] = (char) pFATld->LDIR_Name3[1];
    for (int j = 12; j > -1; j--) {
        if (name[i*13+j] == 'p' && name[i*13+j-1] == 'm' && name[i*13+j-2] == 'b' && name[i*13+j-3] == '.' ) {
            name[i*13+j+1]= 0;
            break;
        }
    }
    for (int i = 0; i < strlen(name); i++) {
        if (!isprint(name[i]))
            return NULL;
    }
    return name;
}
int comp(const void* a, const void* b) {
    return *(double*)a-*(double*)b;
}
double* sobelY(uint8_t* lowerline, uint8_t* nowline, uint8_t* higherline, int pixels) {
    double r, g, b, sum;
    sum = r = g = b = 0;
    double* sobel = malloc(sizeof(double)*(pixels-2));
    for (int i = 0; i < pixels; i++) {
        r = higherline[i*3+0]+higherline[(i+1)*3+0]-lowerline[(i-1)*3+0]-2*lowerline[i*3+0]-lowerline[(i+1)*3+0];
        g = higherline[(i-1)*3+1]+2*higherline[i*3+1]+higherline[(i+1)*3+1]-lowerline[(i-1)*3+1]-2*lowerline[i*3+1]-lowerline[(i+1)*3+1];
        b = higherline[(i-1)*3+2]+2*higherline[i*3+2]+higherline[(i+1)*3+2]-lowerline[(i-1)*3+2]-2*lowerline[i*3+2]-lowerline[(i+1)*3+2];
        sum += sqrt(pow(r,2)+pow(g,2)+pow(b,2));
        sobel[i-1] = sqrt(pow(r,2)+pow(g,2)+pow(b,2));
    }
    double* mean = malloc(sizeof(double));
    *mean = sum/(pixels-2);
    qsort(sobel, pixels-2, sizeof(double), comp);
    return mean;
}