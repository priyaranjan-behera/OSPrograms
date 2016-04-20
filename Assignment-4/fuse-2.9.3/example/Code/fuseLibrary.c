#include <stdio.h>
#include <sys/stat.h>
#define FUSE_USE_VERSION 26
#include <fuse.h>

#define FNSize 30


typedef struct block{
	int index;
	int nextBlock;
	char blockContent[4096];
	int status; //0 for free and 1 for occupied
} block;

typedef struct file{
	int fileId;
	char fileName[FNSize];
	int firstBlock;
	struct file * siblingFile;
} file;

typedef struct directory{
	int directoryId;
	char directoryName[FNSize];
	struct directory * subdirectory;
	struct directory * siblingDirectory;
	struct file * fileinDir;
} directory;

directory* root;
block* blocks;

int initilizeFileSystem()
{
	root = malloc(sizeof(directory));
	strcpy(root->directoryName,"/");
	blocks = malloc(1024*sizeof(block));
	printf("\nInitializing the blocks");
	for(int i=0; i<1024; i++)
	{
		blocks[i].status = 0;
		blocks[i].nextBlock = NULL;
	}
}

/*
int pb_open(const char *path, int oflags)
{
	int fd = findFileId(path);
	return fd;
}
*/

int getNextFreeBlockIndex()
{
	for(int i=0; i<1024; i++)
	{
		if(blocks[i].status == 0)
			return i;
	}
}

char* getFileDirName(char *path)
{
	char *fileName = strrchr(path, '/');

	if(NULL!=fileName)
	{
		if(fileName[0]!='\0')
		{
			return (char*)(fileName+1);
		}
	}
	
	return fileName;
}

char* getParentPath(char* path)
{
	
	char *parentPath=NULL;
	char *onlyFile=NULL;
	int size=0;
	onlyFile=(char*) malloc(sizeof(char)*255);

	//find last occurence of /
	onlyFile=strrchr(path,'/');
	
	if(onlyFile==NULL) //invalid path given
	{
		return NULL;
	}
	
	//Get the size of parentPath
	size=strlen(path)-strlen(onlyFile);
	if(size==0)//root is the parent
	{
		return "/";
	}
	parentPath=(char*) malloc((size+1)*sizeof(char));
	strncpy(parentPath,path,size);
	parentPath[size]='\0';
	return parentPath;
}

directory* traverseToDir(char* path)
{
	char *token = strtok(path, "/");
	int flag;
	directory *currDirectory;
	directory* sibDirectory;
	directory *newDirectory;
	currDirectory = root;
	while(token) {
		printf("\nProcessing Token: %s", token);
		currDirectory = currDirectory->subdirectory;
		flag = 0;
		if(strcmp(currDirectory->directoryName, token) == 0)
		{
			flag = 1;
		}
		else
		{

			sibDirectory = currDirectory->siblingDirectory;
			while(sibDirectory != NULL)
			{
				if(strcmp(sibDirectory->directoryName, token) == 0)
				{
					currDirectory = sibDirectory;
					flag = 1;
				}
				sibDirectory = sibDirectory->siblingDirectory;
			}
			if(flag == 0)
			{
				printf("\nFailed to find folder");
				//failure to find directory or file in the path, create the directory here.
				return NULL;
			}

		}
	    token = strtok(NULL, "/");
	}
	return currDirectory;
}


int pb_mkdir(const char *path)
{
	//printf("Tp1");
	fflush(stdout);
	char* parentPath = getParentPath(path);
	if(parentPath == NULL)
		return -1;

	//printf("Parent - %s\n", parentPath);
	directory* currDirectory = traverseToDir(parentPath);
	if(currDirectory == NULL)
		return -1;
	//printf("Tp1");

	char* fileName = getFileDirName(path);

	//printf("Parent - %s\n", parentPath);
	//printf("File - %s\n", fileName);

	//printf("\nHere we are getting ready to create the new directory inside: %s", currDirectory->directoryName);
	fflush(stdout);
	directory* newDirectory = malloc(sizeof(directory));
	strcpy(newDirectory->directoryName, fileName);
	//printf("\nTp1");
	fflush(stdout);
	newDirectory->siblingDirectory = currDirectory->subdirectory;
	currDirectory->subdirectory = newDirectory;
	//printf("\nCreated New Directory: %s", fileName);
	return 1;

}

file* getFilePresentAtDir(directory* currDirectory, char* fileName)
{
	file* currFile = currDirectory->fileinDir;
	printf("\nSearching for file inside the directory: %s", currDirectory->directoryName);
	while(currFile)
	{
		printf("\nComparing the files: %s and %s", currFile->fileName, fileName);
		if(strcmp(currFile->fileName, fileName)==0)
		{
			return currFile;
		}
		currFile = currFile->siblingFile;
	}

	return NULL;
}

int pb_open(const char *path)
{
	//printf("Tp1");
	file* oldFile;
	file* newFile;
	fflush(stdout);
	char* parentPath = getParentPath(path);
	if(parentPath == NULL)
		return -1;

	printf("Parent - %s\n", parentPath);
	directory* currDirectory = traverseToDir(parentPath);
	if(currDirectory == NULL)
		return -1;
	printf("Tp1");

	char* fileName = getFileDirName(path);

	printf("Parent - %s\n", parentPath);
	printf("File - %s\n", fileName);

	printf("\nHere we are getting ready to create the new directory inside: %s", currDirectory->directoryName);
	fflush(stdout);

	oldFile = getFilePresentAtDir(currDirectory, fileName);
	if(oldFile == NULL)
	{
		//new file
		printf("\nCreating a new file here!");
		newFile = malloc(sizeof(file));
		strcpy(newFile->fileName, fileName);
		newFile->siblingFile = currDirectory->fileinDir;
		currDirectory->fileinDir = newFile;
		newFile->firstBlock = getNextFreeBlockIndex();
		//todo: get the next available block and allocate it to the file.
	}
	else
	{
		printf("\nReturing the old file descriptor");
		return oldFile->firstBlock;
		//we have the file. Lets send the index of the block
	}



	directory* newDirectory = malloc(sizeof(directory));
	strcpy(newDirectory->directoryName, fileName);
	//printf("\nTp1");
	fflush(stdout);
	newDirectory->siblingDirectory = currDirectory->subdirectory;
	currDirectory->subdirectory = newDirectory;
	//printf("\nCreated New Directory: %s", fileName);
	return 1;

}

int pb_write(int startBlock, const void *buf, size_t count, off_t offset, struct fuse_file_info *fi)
{
	int blocksToOffset, offsetInBlock, readBytes, remBytesInBlock;

	offsetInBlock = offset%1024;
	blocksToOffset = offset/1024;
	readBytes = 0;
	char* tempBuf = malloc(1024);
	printf("\nTP1");
	block* firstBlock = &blocks[startBlock];
	block* currBlock;
	currBlock = firstBlock;
	printf("\nTP1 - blocksToOffset: %d", blocksToOffset);
	for(int i=0; i<blocksToOffset; i++)
	{
		currBlock = &blocks[currBlock->nextBlock];
		if(currBlock == NULL)
		{
			return -1;
		}
	}

	remBytesInBlock = (count - readBytes)%1024;
	int init = 0;
	printf("\nTP1 - remBytesInBlock: %d", remBytesInBlock);
	fflush(stdout);
	while(remBytesInBlock > 0)
	{
		if(init == 1)
		{
			memcpy(tempBuf,currBlock->blockContent,remBytesInBlock);
			sprintf(buf, "%s", currBlock->blockContent+offsetInBlock);
			readBytes = offsetInBlock;
			remBytesInBlock = (count - readBytes)%1024;
			offsetInBlock = 0;
			init = 0;
		}
		else
		{
			memcpy(currBlock->blockContent,&buf[offsetInBlock], 1024);
			printf("Wrote the string: %s", currBlock->blockContent);
			readBytes += remBytesInBlock;
			remBytesInBlock = (count - readBytes)%1024;
			currBlock = &blocks[currBlock->nextBlock];
			buf = buf + 1024;
		}
	}
	printf("\nTP1");
	return readBytes;
}


int pb_read(int startBlock, const void *buf, size_t count, off_t offset, struct fuse_file_info *fi)
{
	int blocksToOffset, offsetInBlock, readBytes, remBytesInBlock;
	buf = (char*)malloc(count);

	offsetInBlock = offset%1024;
	blocksToOffset = offset/1024;
	readBytes = 0;
	char* tempBuf = malloc(1024);
	printf("\nTP1");
	block* firstBlock = &blocks[startBlock];
	block* currBlock;
	currBlock = firstBlock;
	printf("\nTP1 - blocksToOffset: %d", blocksToOffset);
	for(int i=0; i<blocksToOffset; i++)
	{
		currBlock = &blocks[currBlock->nextBlock];
		if(currBlock == NULL)
		{
			return -1;
		}
	}

	remBytesInBlock = (count - readBytes)%1024;
	int init = 1;
	printf("\nTP1 - remBytesInBlock: %d", remBytesInBlock);
	fflush(stdout);
	while(remBytesInBlock > 0)
	{
		printf("\nTP1 - remBytesInBlock: %d", remBytesInBlock);
		if(init == 1)
		{
			printf("Trying to read: %s", &currBlock->blockContent[offsetInBlock]);
			sprintf(buf, "%s", &currBlock->blockContent[offsetInBlock]);
			readBytes = count; //temp
			remBytesInBlock = (count - readBytes)%1024;
			offsetInBlock = 0;
			init = 0;
			printf("\nTP2 - readBytes: %d", readBytes);
			printf("\nTP2 - remBytesInBlock: %d", remBytesInBlock);
		}
		else
		{
			currBlock = &blocks[currBlock->nextBlock];
			free(tempBuf);
			tempBuf = malloc(1024);
			memcpy(tempBuf,currBlock->blockContent,remBytesInBlock);
			sprintf(buf, "%s%s", buf, tempBuf);
			readBytes += remBytesInBlock;
			remBytesInBlock = (count - readBytes)%1024;
		}
	}
	printf("\nRead buf: %s", buf);
	printf("\nTP1");
	return readBytes;
}

/*
int findFileId(const char *path)
{
	char* temp;
	strcpy(temp, path);
	char *token = strtok(path, "/");
	int flag;
	directory *currDirectory;
	directory* sibDirectory;
	file* currFile;
    while(token) {
    	if(strcmp(token, root->directoryName) == 0)
    		currDirectory = root;
    	else
    	{
    		currFile = currDirectory->fileinDir;
    		while(currFile)
    		{
    			if(strcmp(currFile.fileName, token)
    			{
    				return currFile.firstBlock;
    			}
    		}
    		sibDirectory = currDirectory->siblingDirectory;
    		flag = 0;
    		while(sibDirectory != NULL)
    		{
    			if(strcmp(sibDirectory.directoryName, token) == 0)
    			{
    				currDirectory = sibDirectory;
    				flag = 1;
    			}
    			sibDirectory = sibDirectory->->siblingDirectory
    		}
    		if(flag == 0)
    		{
    			//failure to find directory or file in the path
    			return -1;
    		}
    	}
        puts(token);
        token = strtok(NULL, "/");
    }

    return -1;
}

int initializeBlocks(block* startBlock, int fileSystemSize)
{

}
*/


static struct fuse_operations pb_oper = {
	.mkdir		= pb_mkdir,
};

/*
int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &pb_oper, NULL);
}
*/


int main(int argc, char *argv[])
{
	initilizeFileSystem();
	char* buf = "hello how are you!";
	char* parent = getParentPath("/abc");
	char* file = getFileDirName("/abc");
	printf("Parent - %s\n", parent);
	printf("File - %s\n", file);
	int k = pb_mkdir("/abc");
	printf("Second Call\n");
	k = pb_mkdir("/abc/x");
	k = pb_mkdir("/abc/x/kanhu");
	k = pb_open("/abc/x/hello.txt");
	k = pb_open("/abc/x/hello.txt");
	printf("\nAt main: Write invoke for %s",buf);
	pb_write(k, buf, 8, 1, NULL);
	pb_read(k, buf, 8, 1, NULL);
	printf("\nAfter the read command");
	return k;
}



