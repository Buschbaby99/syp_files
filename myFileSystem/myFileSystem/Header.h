#pragma once


typedef struct bootsector {
#define DEFAULT_BLOCKSIZE 4096
#define MAX_SUB_DIRECTORIES 100
	char label[32];
	int blocksize;
	int blockcount;
}bootsector;


typedef struct direntry {
	char filename[32];
	int size;
	int firstblock;
}direntry;

typedef struct subDir {
	char label[32];
	int blocksize;
	int blockcount;
}subDir;

typedef struct subDirEntrys {
#define MAX_SUB_ENTRIES 10
	subDir sub_entries[MAX_SUB_ENTRIES];
}subDirEntrys;

typedef struct dir {
#define MAX_DIR_ENTRIES 64
	direntry entries[MAX_DIR_ENTRIES];
}dir;

typedef struct fat {
#define MAX_BLOCK_COUNT 1024
	int f[MAX_BLOCK_COUNT];
}fat;

typedef struct admin {
	bootsector b;
	dir rootDir;
	fat fat;
	subDir subDirectories;
}admin;


typedef struct freeBlocks {
	int* arr;
	int cnt;
}freeBlocks;


void printStats(char* fileName);
int fileSize(char*);
int getNumberOfBlocks(int filesize, int blocksize);

int getNumberOfFreeBlocks(admin* a);
int getArrayWithFreeBlocks(admin* a, int filesize, freeBlocks* fb); //return pointer to first element in array
void printBlockArray(int* arr, int size);

int writeFileToPartition(admin* a, char* fileName, char* partName, freeBlocks* fb);


//////////////////     Üeben     //////////////////
void printMyAdmin(admin a);

/*
	For the menu
*/
void printMenu();
void myHelp();
void createPartition(char* myPartition);


