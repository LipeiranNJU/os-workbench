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
// #define __DEBUG__
#define __DDEBUG__
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
#define beBMPHeader 1
#define beNotUsed 2
#define beBMPContent 3

inline int getClusterIndex(void* addr, void* start, int clusterSize) {
    return ((intptr_t)addr - (intptr_t) start) / clusterSize;
}

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

void lineCmp(uint8_t*, uint8_t*, uint8_t*, int);
void verifyFAT32Head(struct fat_header*);
void showFAT32HeadInfo(struct fat_header*);
bool isFATShortDirectory(struct FATShortDirectory*);
char* readInfoFromFATLongDirectory(struct FATLongDirectory* );

int main(int argc, char *argv[]) {
    assert(argc == 2);
    assert(sizeof(struct fat_header) == 512);
    assert(sizeof(struct FATShortDirectory) == 32);
    assert(sizeof(struct FATLongDirectory) == 32);
    char* fileName = argv[1];
    print("Filename is %s\n", fileName);


    struct stat statbuf;
    stat(fileName,&statbuf);
    int size = statbuf.st_size;
    print("img file size is %d\n", size);

    int fd = open(fileName, O_RDONLY, 0);
    assert(fd > 0);
    print("fd is %d\n", fd);
    struct fat_header* pfatheader =(struct fat_header*) mmap(NULL, size, PROT_READ, MAP_SHARED , fd, 0);
    print("SizoOf FATheader is %d\n",(int) sizeof(struct fat_header));
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
    print("Offset of initial clus is %d\n", offset);
    struct FATShortDirectory* pFATdir = (struct FATShortDirectory*)((intptr_t)pfatheader+offset);
    void* fatContentStart = (void*)((intptr_t)pfatheader+offset);
    int canBeUsed = 0;
    bool skip = false;
    print("Total Sec is %d\n", (int) pfatheader->BPB_TotSec32);
    for (; (intptr_t)(pFATdir) < (intptr_t)(pfatheader)+size;pFATdir++) {
        assert((intptr_t)pFATdir-(intptr_t)pfatheader < pfatheader->BPB_TotSec32*pfatheader->BPB_BytsPerSec);
        if (isFATShortDirectory(pFATdir) == true) {
            // printf("%lX\n", (long) (intptr_t)pFATdir-((intptr_t)pfatheader+offset));
            print("name:%s\n",pFATdir->DIR_Name);
            assert(pFATdir->DIR_FstClusHI == 0);
            char* magicNum = (char *) (offset + (uintptr_t)(pfatheader) + (pFATdir->DIR_FstClusLO - BPB_RootClus) * BPB_SecPerClus * BPB_BytsPerSec);
            assert(magicNum[0] == 'B');
            assert(magicNum[1] == 'M');
            struct BMPHeader* header = (struct BMPHeader*) magicNum;
            assert(header->bfReserved1 == 0);
            assert(header->bfReserved2 == 0);
            print("Size:%d\n",header->bfSize);
            print("Offbits:%d\n", header->bfOffBits);
            struct BMPInfoHeader* pBMInfoHeader = (struct BMPInfoHeader*) (header + 1);
            assert(pBMInfoHeader->biHeight>0);
            assert(pBMInfoHeader->biSize == 40);
            if (pBMInfoHeader->biCompression == 0) {
                print("will not be compressed.\n");
            } else {
                print("will be compressed\n");
                assert(0);
            }

            canBeUsed += 1;
            struct FATLongDirectory* pFATld = (struct FATLongDirectory*)(pFATdir - 1);
            char * picName = readInfoFromFATLongDirectory(pFATld);
            assert(picName != NULL);
            char* prefix = "/home/lpr/Downloads/lprlpr/";
            int size = strlen(prefix) + strlen(picName);
            char* abspath = malloc(sizeof(char) * (size + 1));
            memset(abspath, '\0', size + 1);
            strcat(strcat(abspath, prefix), picName);
            // free(picName);
            print("PicStoredPath:%s\n", abspath);
            // assert(0);
            skip = false;
            for (int i = 0; i < strlen(abspath); i++) {
                if (!isalnum(abspath[i]) && abspath[i] != '.' && abspath[i] != '/') {
                    skip = true;
                    // printf("invalid name:%s\n",abspath);
                    break;
                }
            }
            if (skip) {
                continue;
            }
            FILE* pfdpic = fopen(abspath, "w+");
            // assert(0);
            fwrite((void*) magicNum, 1, sizeof(*header), pfdpic);
            fclose(pfdpic);
            pfdpic = fopen(abspath, "a");
            fwrite((void*) pBMInfoHeader, 1, pBMInfoHeader->biSize, pfdpic);
            fclose(pfdpic);
            int lineWidthSize = pBMInfoHeader->biWidth*4;
            pfdpic = fopen(abspath, "a");
            // 位图数据区写入
            int picDataSize = header->bfSize - header->bfOffBits;
            void* picData = malloc(picDataSize);
            uint8_t* preLine = malloc(picDataSize);
            uint8_t* nowLine = malloc(picDataSize);
            uint8_t* laterLine = malloc(picDataSize);
            // printk("cluster index is%d\n", getClusterIndex(pFATdir, fatContentStart, 4*KB));
            void* picDataStart = (void*) ((uintptr_t)(header) + header->bfOffBits);
            // bool tempflag = true;
            int i = 0;
            for (; i < abs(pBMInfoHeader->biHeight); i++) {
                memcpy(nowLine, picDataStart+i*pBMInfoHeader->biWidth, lineWidthSize);
                memcpy(laterLine, picDataStart+(i+1)*pBMInfoHeader->biWidth, lineWidthSize);
                if (i != 0 && i != abs(pBMInfoHeader->biHeight) - 1&& strcmp(abspath, "/home/lpr/Downloads/lprlpr/0M15CwG1yP32UPCp.bmp") == 0) {
                    // printk("Bingo!\n");
                    // sleep(3);
                    ;
                    lineCmp(preLine, nowLine, laterLine, lineWidthSize);
                }
                memcpy(preLine, nowLine, lineWidthSize);
                memcpy(picData+i*pBMInfoHeader->biWidth,preLine, lineWidthSize);
                if (preLine[3] != preLine[7]) {

                    // printk("filename:%s\tThis is wrong!\n", abspath);
                    // assert(preLine[3] == preLine[7]);
                    // tempflag = false;
                    // break;
                } 
            }
            fwrite(picDataStart, 1, picDataSize/*(i+1)*lineWidthSize*/, pfdpic);
            fclose(pfdpic);
            free(picData);
            free(preLine);
            free(nowLine);
            if (((intptr_t) pFATdir - (intptr_t)pfatheader -offset) % (4*KB) != 0) {
                
            }
            char buf[41] = {};
            buf[40] = 0;
            char cmd[100] = {};
            int pipefds[2];
            if(pipe(pipefds) < 0){
		        perror("pipe");
                assert(0);
	        }
            int pid = fork();
            char* argv[3];
            argv[0] = "sha1sum",
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
                printf("%s    %s\n", buf, picName);
            }

        }

        assert((intptr_t) (pFATdir + 1) - (intptr_t)pFATdir == sizeof(struct FATShortDirectory));
        assert(pFATdir != NULL);
        fflush(stdout);
    }
    print("%d can be short name directory.\n",canBeUsed);
    close(fd);
    return 0;    
}

void verifyFAT32Head(struct fat_header* ptr) {
    print("hello\n");
    assert((memcmp(&ptr->BS_FilSysType0, "FAT32", 5) == 0));
    assert(ptr->Signature_word == 0xAA55);
    assert(ptr->BPB_RootEntCnt == 0);
    assert(ptr->BPB_TotSec16 == 0);
    assert(ptr->BPB_NumFATs == 2 || ptr->BPB_NumFATs == 1);
}

void showFAT32HeadInfo(struct fat_header* pfatheader) {
    print("SizoOf FATheader is %d\n",(int) sizeof(struct fat_header));
    print("jmpBoot[0] is %X\t", pfatheader->BS_jmpBoot[0]);
    print("jmpBoot[2] is %X\n", pfatheader->BS_jmpBoot[2]);
    print("BPB_BytsPerSec is %d\tBPB_SecPerClus is %d\n", pfatheader->BPB_BytsPerSec, pfatheader->BPB_SecPerClus); 
    print("BPB_RootClus is %d\n", pfatheader->BPB_RootClus);
    print("BPB_FATSz32 is %d\n", pfatheader->BPB_FATSz32);
    print("BPB_HiddSec is %d\n", pfatheader->BPB_HiddSec);
    print("BPB_RsvdSecCnt is %d\n", pfatheader->BPB_RsvdSecCnt);
    print("BPB_NumFATs is %d\n", pfatheader->BPB_NumFATs);
}

bool isLegalInShort(char c) {
    if (c == 0x22 || c==0x2A || c==0x2B||c==0x2C ||c== 0x2E||c==0x2F ||c== 0x3A||c== 0x3B||c==0x3C ||c== 0x3D||c==0x3E ||c ==0x3F||c==0x5B||c==0x5C||c==0x5D||c==0x7C)
        return false;
    else
        return true;
    
}

bool isFATShortDirectory(struct FATShortDirectory* pFATdir) {
    // if (pFATdir->DIR_CrtTime % 2 == 1)
    //     return false;
    // else if (pFATdir->DIR_CrtTimeTenth > 199 || pFATdir->DIR_CrtTimeTenth < 0)
    //     return false;
    // else if (pFATdir->DIR_NTRes != 0)
    //     return false;
    // else if (pFATdir->DIR_FileSize > 2 * MB)
    //     return false;
    // else if (pFATdir->DIR_CrtDate > pFATdir->DIR_LstAccDate || pFATdir->DIR_CrtDate > pFATdir->DIR_WrtDate || pFATdir->DIR_LstAccDate > pFATdir->DIR_WrtDate)
    //     return false;
    // else {
        // int year = ((pFATdir->DIR_CrtDate & 0xfd00) >> 9);
        // int month = ((pFATdir->DIR_CrtDate & 0xf0) >> 4); 
        // int day = (pFATdir->DIR_CrtDate & 0xf);
        // if (year < 5 || year > 40)
        //     return false;
        // if (month < 1 || month > 12)
        //     return false;
        // if (day == 0)
        //     return false;
        
        // for (int i = 0; i < 11; i++) {
        //     if ((i != 0 && pFATdir->DIR_Name[i] < 0x20) || (i == 0 && pFATdir->DIR_Name[i] ==0x05) || (!isLegalInShort(pFATdir->DIR_Name[i])))
        //         return false;
        // }
        if (strncmp((char *)&pFATdir->DIR_Name[8], "BMP", 3) == 0 && isalnum(pFATdir->DIR_Name[0]))
            return true;

        return false;
    // }
    
}

char* readInfoFromFATLongDirectory(struct FATLongDirectory* pFATld) {
    char c[140];
    int i = 0;
    memset(c, '\0', 140);
    c[i * 13 + 0] = (char) pFATld->LDIR_Name1[0];
    c[i * 13 + 1] = (char) pFATld->LDIR_Name1[1];
    c[i * 13 + 2] = (char) pFATld->LDIR_Name1[2];
    c[i * 13 + 3] = (char) pFATld->LDIR_Name1[3];
    c[i * 13 + 4] = (char) pFATld->LDIR_Name1[4];
    c[i * 13 + 5] = (char) pFATld->LDIR_Name2[0];
    c[i * 13 + 6] = (char) pFATld->LDIR_Name2[1];
    c[i * 13 + 7] = (char) pFATld->LDIR_Name2[2];
    c[i * 13 + 8] = (char) pFATld->LDIR_Name2[3];
    c[i * 13 + 9] = (char) pFATld->LDIR_Name2[4];
    c[i * 13 + 10] = (char) pFATld->LDIR_Name2[5];
    c[i * 13 + 11] = (char) pFATld->LDIR_Name3[0];
    c[i * 13 + 12] = (char) pFATld->LDIR_Name3[1];
    if (pFATld->LDIR_Ord > 0x40){
        int size = strlen(c);
        char * t = malloc(sizeof(char)*(size + 1));
        t[size] = 0;
        memcpy(t, c, size);
        return t;
    }
    else {
        pFATld = pFATld - 1;
        int i = 1;
        c[i * 13 + 0] = (char) pFATld->LDIR_Name1[0];
        c[i * 13 + 1] = (char) pFATld->LDIR_Name1[1];
        c[i * 13 + 2] = (char) pFATld->LDIR_Name1[2];
        c[i * 13 + 3] = (char) pFATld->LDIR_Name1[3];
        c[i * 13 + 4] = (char) pFATld->LDIR_Name1[4];
        c[i * 13 + 5] = (char) pFATld->LDIR_Name2[0];
        c[i * 13 + 6] = (char) pFATld->LDIR_Name2[1];
        c[i * 13 + 7] = (char) pFATld->LDIR_Name2[2];
        c[i * 13 + 8] = (char) pFATld->LDIR_Name2[3];
        c[i * 13 + 9] = (char) pFATld->LDIR_Name2[4];
        c[i * 13 + 10] = (char) pFATld->LDIR_Name2[5];
        c[i * 13 + 11] = (char) pFATld->LDIR_Name3[0];
        c[i * 13 + 12] = (char) pFATld->LDIR_Name3[1];
            if (pFATld->LDIR_Ord > 0x40) {
                int size = strlen(c);
                char * t = malloc(sizeof(char)*(size + 1));
                t[size] = 0;
                memcpy(t, c, size);
                return t;
            } else {
                pFATld = pFATld - 1;
                int i = 2;
                c[i * 13 + 0] = (char) pFATld->LDIR_Name1[0];
                c[i * 13 + 1] = (char) pFATld->LDIR_Name1[1];
                c[i * 13 + 2] = (char) pFATld->LDIR_Name1[2];
                c[i * 13 + 3] = (char) pFATld->LDIR_Name1[3];
                c[i * 13 + 4] = (char) pFATld->LDIR_Name1[4];
                c[i * 13 + 5] = (char) pFATld->LDIR_Name2[0];
                c[i * 13 + 6] = (char) pFATld->LDIR_Name2[1];
                c[i * 13 + 7] = (char) pFATld->LDIR_Name2[2];
                c[i * 13 + 8] = (char) pFATld->LDIR_Name2[3];
                c[i * 13 + 9] = (char) pFATld->LDIR_Name2[4];
                c[i * 13 + 10] = (char) pFATld->LDIR_Name2[5];
                c[i * 13 + 11] = (char) pFATld->LDIR_Name3[0];
                c[i * 13 + 12] = (char) pFATld->LDIR_Name3[1];
                if (pFATld->LDIR_Ord > 0x40) {
                    int size = strlen(c);
                    char * t = malloc(sizeof(char)*(size + 1));
                    t[size] = 0;
                    memcpy(t, c, size);
                    return t;
                }
            }
    }
    return NULL;
}
void lineCmp(uint8_t* preLine, uint8_t* nowLine,uint8_t* latterLine, int size) {
    int width = size/4;
    int sum[3] = {0};
    int Gx_B = 0;
    int Gy_B = 0;
    int G_B = 0;
    int Gx_G = 0;
    int Gy_G = 0;
    int G_G = 0;
    int Gx_R = 0;
    int Gy_R = 0;
    int G_R = 0;
    double linesum = 0;
    for (int i = 1; i < width-1; i++){
        Gx_B = 2*nowLine[(i+1)*4+0] - 2*nowLine[(i-1)*4+0] + preLine[(i+1)*4+0] - preLine[(i-1)*4+0] + latterLine[(i+1)*4+0] - latterLine[(i-1)*4+0];
        Gy_B = 2*latterLine[(i)*4+0] - 2*preLine[(i)*4+0] - preLine[(i+1)*4+0] - preLine[(i-1)*4+0] + latterLine[(i+1)*4+0] + latterLine[(i-1)*4+0];
        G_B = sqrt(1.0*Gx_B*Gx_B+1.0*Gy_B*Gy_B);
        sum[0]+=G_B;
        

        Gx_G = 2*nowLine[(i+1)*4+1] - 2*nowLine[(i-1)*4+1] + preLine[(i+1)*4+1] - preLine[(i-1)*4+1] + latterLine[(i+1)*4+1] - latterLine[(i-1)*4+1];
        Gy_G = 2*latterLine[(i)*4+1] - 2*preLine[(i)*4+1] - preLine[(i+1)*4+1] - preLine[(i-1)*4+1] + latterLine[(i+1)*4+1] + latterLine[(i-1)*4+1];
        G_G = sqrt(1.0*Gx_G*Gx_G+1.0*Gy_G*Gy_G);
        sum[1]+=G_G;

        
        Gx_R = 2*nowLine[(i+1)*4+2] - 2*nowLine[(i-1)*4+2] + preLine[(i+1)*4+2] - preLine[(i-1)*4+2] + latterLine[(i+1)*4+2] - latterLine[(i-1)*4+2];
        Gy_R = 2*latterLine[(i)*4+2] - 2*preLine[(i)*4+2] - preLine[(i+1)*4+2] - preLine[(i-1)*4+2] + latterLine[(i+1)*4+2] + latterLine[(i-1)*4+2];
        G_R = sqrt(1.0*Gx_R*Gx_R+1.0*Gy_R*Gy_R);
        sum[2]+=G_R;
        double length = sqrt(1.0*sum[0]*sum[0]+1.0*sum[1]*sum[1]+1.0*sum[2]*sum[2]);
        linesum += length;
    }
    printf("%lf\n", linesum/(width-2));
}