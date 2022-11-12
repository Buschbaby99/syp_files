#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Header.h"

bootsector b = { "3AKIFT", DEFAULT_BLOCKSIZE, MAX_BLOCK_COUNT };
admin a = { 0 };
subDir sub = { 0 };

int numberForCase = 6;

char* fileName = NULL;
char* dirName = NULL;

int main(int argc, char** argv) {

	char str[50];
	myHelp();
	char** myInput = ' ';
	char** myBefehl = ' ';
	
	printf("Eingabe: ");
	while (gets(str)) {
		//printf("Eingabe: ");

		const char s[2];
		char* token;

		token = strtok(str, " ");

		//printf("%s", str);
		//exit(-1);

		int cnt = 0;
		while (token != NULL) {
			if (cnt == 0) {
				myBefehl = token;
				token = strtok(NULL, s);
				cnt++;
			}
			else if(cnt == 1) {
				myInput = token;
				token = strtok(NULL, s);
			}
		}
		

		if (strcmp("-help", myBefehl) == 0) {numberForCase = 0;}
		else if (strcmp("-create", myBefehl) == 0) {numberForCase = 1;}
		else if (strcmp("-partition", myBefehl) == 0) {numberForCase = 2;}
		else if (strcmp("-stats", myBefehl) == 0) {numberForCase = 3;}
		else if (strcmp("-add", myBefehl) == 0) {numberForCase = 4;}
		else if (strcmp("-exit", myBefehl) == 0) {numberForCase = 5;}
		else {numberForCase = 7;}
		
		switch (numberForCase) {
			case 0: printMenu();
				break;
			case 1: 
				//printf("%s", myInput);
				createPartition(myInput);
				printf("Eingabe: ");
					break;
			case 2: printf("a ist drei\n"); 
				break;
			case 3: printf("a ist drei\n");
				break;
			case 4: printf("a ist drei\n");
				break;
			case 5: printf("a ist drei\n");
				break;
			case 6:
				printf("Eingabe: ");
				break;
			default: printf("Wronge Input\n"); 
				break;
		}

	}
	int i = 0;
		if (strcmp("-partition", argv[i]) == 0) {
			fileName = argv[i + 1];
			//printf("\nName: %s", fileName);
			FILE* fd = fopen(fileName, "rb");
			fread(&a, 1, sizeof(admin), fd);
			fclose(fd);
			//printMyAdmin(a);
			//exit(-1);
		}
		if (strcmp("-mkdir", argv[i]) == 0) {
			
			//char** myPath = argv[i+1];
			dirName = argv[i + 1];
			
			FILE* fd = fopen(dirName, "wb");
			int totalSize = sizeof(subDir) + (sub.blockcount * sub.blocksize);

			fwrite(&sub, 1, sizeof(subDir), fd);
			void* pt = malloc(sub.blockcount * sub.blocksize);
			if (!pt) return;
			memset(pt, 0, sub.blockcount * sub.blocksize);
			fwrite(pt, 1, sub.blockcount * sub.blocksize, fd);
			int cursorPos = ftell(fd);
			fclose(fd);
		}
		if (strcmp("-stats", argv[i]) == 0) {
			printStats(fileName);
		}
		if (strcmp("-add", argv[i]) == 0) {

			if (fileName == NULL) {
				printf("\n###Error: no partition specified!");
				exit(-1);
			}

			char* userFile = argv[i + 1];

			dir* d = &(a.rootDir);

			for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
				if (strcmp(d->entries[i].filename, userFile) == 0) {
					printf("\n###Error: [%s] exists already ... exit now", userFile);
					exit(-1);
				}
			}

			for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
				if (d->entries[i].filename[0] == '\0') {
					strcpy(&d->entries[i].filename[0], userFile);
					int _filesize = d->entries[i].size = fileSize(userFile);
					int bCnt = getNumberOfBlocks(d->entries[i].size, b.blocksize);
					int _freeBlocks = getNumberOfFreeBlocks(&a);
					printf("\nadded [%s] to partition [%s] size=[%d] #blocks=[%d] free=[%d]",
						userFile, fileName, d->entries[i].size, bCnt, _freeBlocks);

					if (bCnt <= _freeBlocks) {
						freeBlocks fb;
						getArrayWithFreeBlocks(&a, _filesize, &fb);
						writeFileToPartition(&a, userFile, fileName, &fb);
						// write used blocks to FAT
						printf("\nblocks used: ");
						for (int j = 0; j < fb.cnt; j++) {
							printf("%d, ", fb.arr[j]);
							if (j == 0) {
								// write filename + filesize + firstblock to root dir
								strcpy(a.rootDir.entries[i].filename, userFile);
								a.rootDir.entries[i].size = _filesize;
								a.rootDir.entries[i].firstblock = fb.arr[j];
							}
							if(j < fb.cnt-1) {
								a.fat.f[fb.arr[j]] = fb.arr[j + 1];
							}
							else {
								a.fat.f[fb.arr[j]] = EOF;
							}
						}

						// write entire admin struct back to file 
						FILE* fd = fopen(fileName, "rb+");
						fwrite(&a, 1, sizeof(admin), fd);
						fclose(fd);


					}

					break;
				}
			}
		}
	return 0;
}

void printStats(char* fileName)
{
	admin a;
	FILE* fd = fopen(fileName, "rb");
	fread(&a, 1, sizeof(admin), fd);
	fclose(fd);

	printf("\n\n=========================");
	printf("\npartition:\t[%s]", fileName);
	printf("\nlabel:\t\t[%s]", a.b.label);
	printf("\n# of blocks:\t[%d]", a.b.blockcount);
	printf("\nblock size:\t[%d]", a.b.blocksize);
	printf("\n=========================\n\n");

	for (int i = 0; i < MAX_DIR_ENTRIES; i++)
	{
		direntry* e = &a.rootDir.entries[i];
		if (e->filename[0] == 0)
			continue;

		printf("\n[%d]=>[%s] size=[%d] fb=[%d]", i, e->filename, e->size, e->firstblock);
		printf("\n\tblocks=[");
		//printf("%d, ", e->firstblock);

		int blnr = e->firstblock;

		while (a.fat.f[blnr] != EOF) {
			printf("%d, ", blnr);
			blnr = a.fat.f[blnr];
		}
		printf("%d]", blnr); //last blocknumber (EOF)
	}
}


int fileSize(char* fileName) {
	FILE* fd = fopen(fileName, "rb");
	fseek(fd, 0, SEEK_END);
	int size = ftell(fd);
	fclose(fd);

	return size;
}


int getNumberOfBlocks(int filesize, int blocksize) {

	int q = filesize / blocksize;

	if (filesize % blocksize != 0)
		return q + 1;
	else
		return q;
}


int getNumberOfFreeBlocks(admin* a) {
	int free = 0;
	for (int i = 0; i < a->b.blockcount; i++) {
		if (a->fat.f[i] == '\0') {
			free++;
		}
	}
	return free;
}

int getArrayWithFreeBlocks(admin* a, int filesize, freeBlocks* fb) {
	int _blockCount = getNumberOfBlocks(filesize, a->b.blocksize);
	fb->cnt = _blockCount; //return this info to the caller
	int* blockArray = malloc(sizeof(int) * _blockCount);
	fb->arr = blockArray;
	for (int i = 0, j = 0; i < a->b.blockcount; i++) {
		if (a->fat.f[i] == '\0') {
			blockArray[j++] = i;
		}
		if (j >= _blockCount) {
			break;
		}
	}
	printBlockArray(blockArray, _blockCount);
	return blockArray;
}

void printBlockArray(int* arr, int size) {
	for (int i = 0; i < size; i++) {
		printf("\n%d => block=[%d]", i, arr[i]);
	}
}

int writeFileToPartition(admin* a, char* fileName, char* partName, freeBlocks* fb) {
	unsigned char* buffer = malloc(a->b.blocksize);

	FILE* fdUserFile = fopen(fileName, "rb");
	FILE* fdPart = fopen(partName, "rb+");
	int wCnt;
	int rCnt;
	for (int i = 0; i < fb->cnt; i++) {
		rCnt = fread(buffer, 1, a->b.blocksize, fdUserFile);
		int offset = sizeof(admin) + a->b.blocksize * fb->arr[i];
		fseek(fdPart, offset, SEEK_SET);
		wCnt = fwrite(buffer, 1, a->b.blocksize, fdPart);

	}
	free(buffer);
	fclose(fdUserFile);
	fclose(fdPart);
}


//////////////////     Üeben     //////////////////

void printMyAdmin(admin a) {
	printf("\nadmin-bootsection-label : %s", a.b.label);
	printf("\nadmin-bootsection-blocksize : %d", a.b.blocksize);
	printf("\nadmin-bootsection-blockcount : %d", a.b.blockcount);

	printf("\nadmin-rootdir-entries : %d", a.rootDir.entries);
	printf("\nadmin-fat-f : %d", a.fat.f);
}

void printMenu() {
	printf("\n\n==============================================\n");
	printf("\n-create..................erstellt eine Partition");
	printf("\n-partition...............wählt die Partition aus");
	printf("\n-stats...................zeigt alle Statistiken an");
	printf("\n-add.....................fügt File hinzu");
	printf("\n-exit....................beendet Programm\n\n");
	printf("==============================================\n");
	printf("Eingabe: ");
}

void myHelp() {
	printf("\n\n==============================================\n");
	printf("*                                            *\n");
	printf("*  -help...........aufruf der Funktionen     *\n");
	printf("*                                            *\n");
	printf("==============================================\n");
	//printf("Eingabe: ");
}


void createPartition(char* myPartition){
		
		FILE* fd = fopen(myPartition, "wb");
		
		int totalSize = sizeof(admin) + (b.blockcount * b.blocksize);
		printf("\nTotoal size: [%d]\n", totalSize);

		fwrite(&b, 1, sizeof(bootsector), fd);
		fwrite(&a.fat, 1, sizeof(fat), fd);
		fwrite(&a.rootDir, 1, sizeof(dir), fd);
		void* pt = malloc(b.blockcount * b.blocksize);
		if (!pt) return;
		memset(pt, 0, b.blockcount * b.blocksize);
		fwrite(pt, 1, b.blockcount * b.blocksize, fd);
		int cursorPos = ftell(fd);

		printf("Created [%s] file with [%d] bytes ... cursor=[%d]\n",
			myPartition, totalSize, cursorPos);
		printf("==============================================\n\n");
		fclose(fd);
		numberForCase = 6;
}