#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Header.h"


bootsector b = { "3AKIFT", DEFAULT_BLOCKSIZE, MAX_BLOCK_COUNT };
admin a = { 0 };

int numberForCase = 6;

char* fileName = NULL;
char* dirName = NULL;
char selectedPart[32];

//for ls
int myPartitionCnt = 0;
char myPartArr[MAX_PARTITONS][MAX_NAME_LENGHT];


int main(int argc, char** argv) {

	char str[50];
	myHelp();
	
	char** myInput = ' ';
	char** myBefehl = ' ';
	char** third_arg = ' ';
	char* myPointerArray[3];
	
	
	/*
	char myInput[20];
	char myBefehl[20];
	char third_arg[20];
	*/

	isMyPartSelected();
	
	while (gets(str)) {
		
		int cnt = arg_passer(myPointerArray, str);

		//exit(1);
		/*
		const char s[] = " ";
		char* token;
		token = strtok(str, s);

		int cnt = 0;
		
		
		while (token != NULL) {
			if (cnt == 0) {
				myBefehl = token;
				token = strtok(NULL, s);
				cnt++;
			}
			else if (cnt == 1) {
				myInput = token;
				token = strtok(NULL, s);
				cnt++;
			}
			else if (cnt == 2) {
				third_arg = token;
				token = strtok(NULL, s);
			}
		}
		*/
		

		if (cnt == 0) {
			numberForCase = 6;
		}
		else {
			if (strcmp("-help", myPointerArray[0]) == 0) { numberForCase = 0; }
			else if (strcmp("-create", myPointerArray[0]) == 0) { numberForCase = 1; }
			else if (strcmp("-partition", myPointerArray[0]) == 0) { numberForCase = 2; }
			else if (strcmp("-stats", myPointerArray[0]) == 0) { numberForCase = 3; }
			else if (strcmp("-add", myPointerArray[0]) == 0) { numberForCase = 4; }
			else if (strcmp("-exit", myPointerArray[0]) == 0) { numberForCase = 5; }
			else if (strcmp(" ", myPointerArray[0]) == 0) { numberForCase = 6; }
			else if (strcmp("-ls", myPointerArray[0]) == 0) { numberForCase = 8; }
			else if (strcmp("-del", myPointerArray[0]) == 0) { numberForCase = 9; }
			else if (strcmp("-boot", myPointerArray[0]) == 0) { numberForCase = 10; }
			else { numberForCase = 7; }
		}

		//exit(1);
		switch (numberForCase) {
			case 0: printMenu();
				isMyPartSelected();
				break;
			case 1: 
				createPartition(myPointerArray[1]);
				isMyPartSelected();
					break;
			case 2: 
				selectPartition(myPointerArray[1]);
				isMyPartSelected();
				break;
			case 3: 
				printStats();
				printf("\n");
				isMyPartSelected();
				break;
			case 4: 
				addThisImage(myPointerArray[1]);
				printf("\n\n");
				isMyPartSelected();
				break;
			case 5: 
				printf("Konsole wurde beendet beendet!\n");
				printf("\n==============================================\n");
				exit(0);
				break;
			case 6:
				isMyPartSelected();
				break;
			case 8:
				ls();
				isMyPartSelected();
				break;
			case 9:
				del(myPointerArray[1], a);
				isMyPartSelected();
				break;
			case 10:
				boot(myPointerArray[1], myPointerArray[2]);
				//isMyPartSelected();
				break;
			default: 
				printf("Wronge Input maybe try [-help] for info\n\n"); 
				printf("==============================================\n");
				isMyPartSelected();
				break;
		}
	}	
	return 0;
}

void printStats()
{
	if (selectedPart[0] == '\0') {
		printf("You have to select a Partition with [-partition filename]\n\n");
	}
	else {
		fileName = selectedPart;
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
	numberForCase = 6;
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
		//last EOF
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


void printMyAdmin(admin a) {
	printf("\nadmin-bootsection-label : %s", a.b.label);
	printf("\nadmin-bootsection-blocksize : %d", a.b.blocksize);
	printf("\nadmin-bootsection-blockcount : %d", a.b.blockcount);

	printf("\nadmin-rootdir-entries : %d", a.rootDir.entries);
	printf("\nadmin-fat-f : %d", a.fat.f);
}

void printMenu() {
	printf("\n==============================================\n");
	printf("-create..................erstellt eine Partition");
	printf("\n-partition...............wählt die Partition aus");
	printf("\n-stats...................zeigt alle Statistiken an");
	printf("\n-add.....................fügt File hinzu");
	printf("\n-exit....................beendet Programm\n");
	printf("==============================================\n");
}

void myHelp() {
	printf("\n==============================================\n");
	printf("*                                            *\n");
	printf("*  -help...........aufruf der Funktionen     *\n");
	printf("*                                            *\n");
	printf("==============================================\n");
	//printf("Eingabe: ");
}


void createPartition(char* myPartition){
		
		FILE* fd = fopen(myPartition, "wb");
		
		int totalSize = sizeof(admin) + (b.blockcount * b.blocksize);
		printf("Totoal size: [%d]\n", totalSize);

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
		printf("==============================================\n");
		fclose(fd);
		
		strcpy(myPartArr[myPartitionCnt], myPartition);
		myPartitionCnt++;
		numberForCase = 6;
}

void selectPartition(char* myPartition) {
	
	FILE* fd = fopen(myPartition, "rb");
	fread(&a, 1, sizeof(admin), fd);
	fclose(fd);
	strcpy(selectedPart, myPartition);
	numberForCase = 6;
}

void isMyPartSelected() {
	if (selectedPart[0] == '\0') {
		printf("[NO PARTITION SELECTED]>");
	}
	else {
		printf("[%s]>", selectedPart);
	}
	printf("Eingabe: ");
}


void addThisImage(char* image) {

	char* userFile = image;
	fileName = selectedPart;

	//printf("userFile: %s\n fileName: %s\n", userFile, fileName);

	dir* d = &(a.rootDir);

	if (fileName == NULL) {
		printf("\n###Error: no partition specified!");
		exit(-1);
	}

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
					if (j < fb.cnt - 1) {
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

void ls() {

	if (myPartitionCnt == 0) {
		printf("Es gibt noch keine Partition\nBitte lege mit [-create filename] zuerst eine an.\n\n");
	}
	else {
		for (int i = 0; i < myPartitionCnt; i++) {
			printf("%s\n", myPartArr[i]);
		}
		printf("\n");
	}
}


void del(char* myInput) {

	char* userFile = myInput;
	fileName = selectedPart;

	//printf("userFile: %s\n", userFile);
	//printf("fileName: %s\n", fileName);

	if (fileName == NULL) {
		printf("\n###Error: no partition specified!");
		exit(-1);
	}

	dir* d = &(a.rootDir);

	int myFirstBlock = 0;
	int myIndex = 0;
	int myBlockSize = 0;

	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
		if (strcmp(d->entries[i].filename, userFile) == 0) {
			myFirstBlock = d->entries[i].firstblock;
			myBlockSize = d->entries[i].size;
			myIndex = i;
			break;
		}
		else {
			printf("\n###Error:[%s] doesn't exists ... exit now", userFile);
			exit(-1);
		}
	}
	
	int i = myFirstBlock;

	while (i != EOF) {
		i = a.fat.f[i];
		a.fat.f[i] = 0;
	}
	
	a.fat.f[myFirstBlock] = 0;
	a.rootDir.entries[myIndex].filename[0] = '\0';
	a.rootDir.entries[myIndex].firstblock = 0;
	a.rootDir.entries[myIndex].size = 0;

	FILE* fd = fopen(fileName, "rb+");
	fwrite(&a, 1, sizeof(admin) + a.b.blocksize * a.b.blockcount, fd);
	fclose(fd);

	printf("[%s] wurde gelöscht!\n", userFile);
}

int arg_passer(char* ptr[], char* str) {

	const char s[] = " ";
	char* token;
	token = strtok(str, s);

	int cnt = 0;

	while (token != NULL){
		ptr[cnt] = token;
		token = strtok(NULL, s);
		cnt++;
	}

	for (int i = 0; i < cnt; i++) {
		printf("%s\n", ptr[i]);
	}

	return cnt;
}


void boot(char* ptr[]) {

	admin a;
	FILE* fd = fopen(selectedPart, "rb");
	fread(&a, 1, sizeof(admin), fd);

	printf("Hallo: %s\n", (a.rootDir.entries[0].filename));

}