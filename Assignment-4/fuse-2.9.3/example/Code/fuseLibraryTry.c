#include <stdio.h>
#include <sys/stat.h>
#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#define FNSize 30
//gcc -w -g -Wall fuseLibraryTry.c `pkg-config fuse --cflags --libs` -o fuseLibraryTry


typedef struct block{
	int index;
	int nextBlock;
	char* blockContent;
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

typedef struct directoryMap{
	int directoryIndex;
	directory* mappedDirectory;
} directoryMap;

directory* root;
block* blocks;
directoryMap dm[100];

int initializeFileSystem()
{
	syslog(LOG_INFO, "Initializing the variables");
	root = malloc(sizeof(directory));
	strcpy(root->directoryName,"/");
	root->fileinDir = NULL;
	root->subdirectory = NULL;
	root->siblingDirectory = NULL;
	blocks = malloc(1024*sizeof(block));
	syslog(LOG_INFO,"\nInitializing the blocks");
	for(int i=0; i<1024; i++)
	{
		blocks[i].status = 0;
		blocks[i].nextBlock = NULL;
		blocks[i].blockContent = malloc(1024);
	}
}

/*
int pb_open(const char *path, int oflags)
{
	int fd = findFileId(path);
	return fd;
}
*/

int freeSubsequentBlocks(int blockIndex)
{
	int currBlock, nextBlock;
	currBlock = blockIndex;

	while(blocks[currBlock].status != 0)
	{
		nextBlock = blocks[currBlock].nextBlock;
		free(blocks[currBlock].blockContent);
		blocks[currBlock].blockContent = malloc(1024);
		blocks[currBlock].status = 0;
		currBlock = nextBlock;
	}
	
}

int getNextFreeBlockIndex()
{
	for(int i=0; i<1024; i++)
	{
		if(blocks[i].status == 0)
		{
			syslog(LOG_INFO, "\nNext Free Block = %d", i);
			return i;
		}
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
	printf("Inside traverseToDir");
	fflush(stdout);
	int flag;
	directory *currDirectory;
	directory* sibDirectory;
	directory *newDirectory;
	currDirectory = root;
	if(strcmp(path,"/")==0)
		return currDirectory;
	char *token = strtok(path, "/");
	while(token) {
		syslog(LOG_INFO, "\nProcessing Token: %s", token);
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


int pb_mkdir(const char *path, mode_t mode)
{
	//printf("Tp1");
	fflush(stdout);
	char* parentPath = getParentPath(path);
	if(parentPath == NULL)
		return -ENOENT;

	//printf("Parent - %s\n", parentPath);
	directory* currDirectory = traverseToDir(parentPath);
	if(currDirectory == NULL)
		return -ENOENT;
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
	printf("Inside getFilePresentAtDir");
	fflush(stdout);
	file* currFile = currDirectory->fileinDir;
	printf("\nSearching for file inside the directory: %s", currDirectory->directoryName);
	fflush(stdout);
	while(currFile != NULL)
	{
		printf("Inside the Loop\n");
		fflush(stdout);
		printf("\nComparing the files: %s and %s", currFile->fileName, fileName);
		if(strcmp(currFile->fileName, fileName)==0)
		{
			return currFile;
		}
		currFile = currFile->siblingFile;
	}

	printf("\nGoing to return NULL\n");
	fflush(stdout);
	return NULL;
}

int getFileSize(file* currFile)
{
	int size = 0;
	block* currBlock = &blocks[currFile->firstBlock];
	while(currBlock)
	{
		size += strlen(currBlock->blockContent);
		if(currBlock->nextBlock == NULL || blocks[currBlock->nextBlock].status == 0)
			return size;
		currBlock = &blocks[currBlock->nextBlock];
	}		
	return 0;
}

directory* getDirPresentAtDir(directory* currDirectory, char* dirName)
{
	directory* travDirectory = currDirectory->subdirectory;
	syslog(LOG_INFO, "\nSearching for file inside the directory: %s", currDirectory->directoryName);
	while(travDirectory)
	{
		syslog(LOG_INFO, "\nComparing the dirs: %s and %s", travDirectory->directoryName, dirName);
		if(strcmp(travDirectory->directoryName, dirName)==0)
		{
			return travDirectory;
		}
		travDirectory = travDirectory->siblingDirectory;
	}

	return NULL;
}

int pb_open(const char *path,struct fuse_file_info* fi)
{
	//printf("Tp1");
	file* oldFile;
	file* newFile;
	fflush(stdout);
	char* parentPath = getParentPath(path);
	if(parentPath == NULL)
		return -ENOENT;

	printf("Parent - %s\n", parentPath);
	directory* currDirectory = traverseToDir(parentPath);
	if(currDirectory == NULL)
		return -ENOENT;
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
		fflush(stdout);
		newFile = malloc(sizeof(file));
		strcpy(newFile->fileName, fileName);
		newFile->siblingFile = currDirectory->fileinDir;
		currDirectory->fileinDir = newFile;
		newFile->firstBlock = getNextFreeBlockIndex();
		blocks[newFile->firstBlock].status = 1;
		return newFile->firstBlock;
		//todo: get the next available block and allocate it to the file.
	}
	else
	{
		syslog(LOG_INFO, "\nReturing the old file descriptor");
		return oldFile->firstBlock;
		//we have the file. Lets send the index of the block
	}



	return -ENOENT;

}

static int pb_opendir(const char *path, struct fuse_file_info *fi)
{
	return 0;
}

int pb_getattr(const char* path, struct stat* stbuf)
{
	//printf("Tp1");
	file* foundFile;
	directory* foundDirectory;
	fflush(stdout);
	char* parentPath = getParentPath(path);
	if(parentPath == NULL)
		return -ENOENT;

	printf("Parent - %s\n", parentPath);
	directory* currDirectory = traverseToDir(parentPath);
	if(currDirectory == NULL)
		return -ENOENT;
	printf("Tp1");

	char* fileName = getFileDirName(path);

	printf("Parent - %s\n", parentPath);
	printf("File - %s\n", fileName);

	foundDirectory = getDirPresentAtDir(currDirectory, fileName);
	if(foundDirectory!=NULL)
	{
		//Code here
		stbuf->st_dev = 1;
		stbuf->st_ino = 0;
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 1;
	}

	foundFile = getFilePresentAtDir(currDirectory, fileName);
	if(foundFile != NULL)
	{
		stbuf->st_dev = 1;
		stbuf->st_ino = 0;
		stbuf->st_mode = S_IFREG | 0755;
		stbuf->st_nlink = 1;
		stbuf->st_size = getFileSize(foundFile);
		//we have the file. Lets send the index of the block
	}

	return 1;

}

int pb_writefile(int startBlock, const void *buf, size_t count, off_t offset, struct fuse_file_info *fi)
{
	int blocksToOffset, offsetInFirstBlock, wroteBytes, bytesToWriteInFirst;
	int bytesToWriteInLast, blocksToWriteAfterFirst, offsetInBlock, offsetBlocks;

	//We will precalcute the offset blocks and the stats thereafter.

	offsetBlocks = offset/1024;
	offsetInFirstBlock = offset%1024;
	bytesToWriteInFirst = 1024 - offsetInFirstBlock;
	blocksToOffset = offset/1024;
	blocksToWriteAfterFirst = (count - offsetInFirstBlock)/1024;
	if((bytesToWriteInFirst + offsetInFirstBlock) < 1024)
		bytesToWriteInLast = 0;
	else
		bytesToWriteInLast = count - (bytesToWriteInFirst + (1024*(blocksToWriteAfterFirst-1)));


	wroteBytes = 0;
	char* tempBuf = malloc(1024);
	printf("\nTP1");
	block* firstBlock = &blocks[startBlock];
	block* currBlock;
	currBlock = firstBlock;


	for(int i=0; i<offsetBlocks; i++)
	{
		currBlock = &blocks[currBlock->nextBlock];
		if(currBlock == NULL)
		{
			return -ENOENT;
		}
	}

	//Now read the first block after the offset
	printf("\nTP2 - blocksToWriteAfterFirst: %d", blocksToWriteAfterFirst);
	printf("\nTP2 - bytesToWriteInLast: %d", bytesToWriteInLast);
	if(blocksToWriteAfterFirst == 0 && bytesToWriteInLast == 0)
	{
		memcpy(&currBlock->blockContent[offsetInFirstBlock],buf, count);
		printf("Wrote: %s,\nwrotesize:", &currBlock->blockContent[offsetInFirstBlock],strlen(&currBlock->blockContent[offsetInFirstBlock]));
		wroteBytes = count;
	}
	else
	{
		memcpy(&currBlock->blockContent[offsetInFirstBlock],buf, 1024-offsetInFirstBlock);
		printf("\nTP2Wrote: %s\n wrote size: %d", &currBlock->blockContent[offsetInFirstBlock],&currBlock->blockContent[offsetInFirstBlock]);
		wroteBytes = 1024-offsetInFirstBlock;
		buf += wroteBytes; //temp
		printf("\nWrote here: %d", wroteBytes);
	//Need to free all the blocks after this block first <free, malloc, status 0>

	}
	//todo:free subsequent blocks
	if(currBlock->nextBlock != NULL && blocks[currBlock->nextBlock].status != 0)
		freeSubsequentBlocks(currBlock->nextBlock);
	
	printf("\nTP2 - readBytes: %d", wroteBytes);
	fflush(stdout);
	//Now all the full blocks to be written
	for(int i = 0; i<blocksToWriteAfterFirst-1; i++)
	{
		printf("\nTP3 - readBytes: %d", wroteBytes);
		fflush(stdout);
		currBlock->nextBlock = getNextFreeBlockIndex();
		printf("\nTP3 - readBytes: %d", wroteBytes);
		fflush(stdout);
		currBlock = &blocks[currBlock->nextBlock];
		currBlock->status = 1;
		printf("\nTP3 - readBytes: %d", wroteBytes);
		fflush(stdout);
		memcpy(currBlock->blockContent,buf, 1024);
		printf("\nTP3 - readBytes: %d", wroteBytes);
		printf("\nWrote of Size: %d", strlen(currBlock->blockContent));
		fflush(stdout);
		wroteBytes += 1024;
		buf += 1024;
	}
	printf("\nTP3 - writeBytes: %d", wroteBytes);
	fflush(stdout);
	//this is the last block
	if(bytesToWriteInLast > 0)
	{
		currBlock->nextBlock = getNextFreeBlockIndex();
		currBlock = &blocks[currBlock->nextBlock];
		currBlock->status = 1;
		memcpy(currBlock->blockContent,buf, bytesToWriteInLast); //todo
		printf("\nWrote of Size: %d", strlen(currBlock->blockContent));
		wroteBytes += bytesToWriteInLast;
	}


	printf("\nTP1 - wroteBytes:%d", wroteBytes);
	return wroteBytes;
}

int pb_write(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
	int fd = pb_open(path,fi);
	if(fd<0)
		return -ENOENT;
	return pb_writefile(fd,buf,size,offset,fi);
}



int pb_readfile(int startBlock, const void *buf, size_t count, off_t offset, struct fuse_file_info *fi)
{
	buf = (char*)malloc(count+10);

	int blocksToOffset, offsetInFirstBlock, readBytes, bytesToWriteInFirst;
	int bytesToWriteInLast, blocksToWriteAfterFirst, offsetInBlock, offsetBlocks;
	//We will precalcute the offset blocks and the stats thereafter.

	offsetBlocks = offset/1024;
	offsetInFirstBlock = offset%1024;
	if(count < (1024 - offsetInFirstBlock))
		bytesToWriteInFirst = count;
	else
		bytesToWriteInFirst = 1024 - offsetInFirstBlock;
	blocksToOffset = offset/1024;
	blocksToWriteAfterFirst = ((count - offsetInFirstBlock)/1024);
	if((bytesToWriteInFirst + offsetInFirstBlock) < 1024)
		bytesToWriteInLast = 0;
	else
		bytesToWriteInLast = count - (bytesToWriteInFirst + (1024*(blocksToWriteAfterFirst-1)));

	readBytes = 0;
	char* tempBuf = malloc(1024);
	printf("\nTP1");
	block* firstBlock = &blocks[startBlock];
	block* currBlock;
	currBlock = firstBlock;

	for(int i=0; i<offsetBlocks; i++)
	{
		currBlock = &blocks[currBlock->nextBlock];
	}

	//Now read the first block after the offset
	printf("\nTP2 - blocksReadAfterFirst: %d", blocksToWriteAfterFirst);
	printf("\nTP3 - bytesToReadInLast: %d", bytesToWriteInLast);
	printf("\nTP3 - bytesToReadInFirst: %d", bytesToWriteInFirst);
	printf("\nTP3 - offsetInBlock: %d", offsetInBlock);
	if(blocksToWriteAfterFirst == 0 && bytesToWriteInLast == 0)
	{
		free(tempBuf);
		char* tempBuf = malloc(4096);
		printf("\nTrying to read: %s", &currBlock->blockContent[offsetInBlock]);
		memcpy(tempBuf,&currBlock->blockContent[offsetInFirstBlock],count);
		printf("Length of Temp Buf: %d\n", strlen(tempBuf));
		strcat(buf,tempBuf);
		readBytes = count;
		printf("Length of String: %d\n", strlen(buf));
	}
	else
	{
		free(tempBuf);
		char* tempBuf = malloc(1024);
		printf("\nTrying to read: %s", &currBlock->blockContent[offsetInBlock]);
		memcpy(tempBuf,&currBlock->blockContent[offsetInFirstBlock],1024-offsetInFirstBlock);
		printf("Length of Temp Buf: %d\n", strlen(tempBuf));
		strcat(buf,tempBuf);
		readBytes = 1024-offsetInFirstBlock; //temp
	//Need to free all the blocks after this block first <free, malloc, status 0>

		printf("Length of String: %d\n", strlen(buf));

	}
	
	
	printf("\nTP2 - readBytes: %d", readBytes);

	//Now all the full blocks to be written
	for(int i = 0; i<blocksToWriteAfterFirst-1; i++)
	{
		currBlock = &blocks[currBlock->nextBlock];
		free(tempBuf);
		tempBuf = malloc(1024);
		printf("Length of String: %d\n", strlen(buf));
		memcpy(tempBuf,currBlock->blockContent,1024);
		printf("Length of Temp Buf: %d\n", strlen(tempBuf));
		strcat(buf,tempBuf);
		readBytes += 1024;
		printf("\nTP2 - readBytes: %d", readBytes);

		printf("Length of String: %d\n", strlen(buf));
	}

	//this is the last block
	if(bytesToWriteInLast > 0)
	{
		currBlock = &blocks[currBlock->nextBlock];
		free(tempBuf);
		tempBuf = malloc(bytesToWriteInLast);
		memcpy(tempBuf,currBlock->blockContent,bytesToWriteInLast);
		printf("Content at last block: %s", currBlock->blockContent);
		printf("Length of Temp Buf: %d\n", strlen(tempBuf));
		strcat(buf,tempBuf);
		readBytes += bytesToWriteInLast;

		printf("Length of String: %d\n", strlen(buf));
	}
	

	printf("\nRead buf: %s", buf);
	printf("\nLength of buf: %d", strlen(buf));
	printf("\nTP1: readBytes: %d", readBytes);
	return readBytes;
}

int pb_read(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
	int fd = pb_open(path,fi);
	if(fd<0)
		return -ENOENT;
	return pb_readfile(fd, buf, size, offset, fi);
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
    			return -ENOENT;
    		}
    	}
        puts(token);
        token = strtok(NULL, "/");
    }

    return -ENOENT;
}

int initializeBlocks(block* startBlock, int fileSystemSize)
{

}
*/

void* pb_init(struct fuse_conn_info *conn)
{
	initializeFileSystem();
}
static struct fuse_operations pb_oper = {
	.mkdir		= pb_mkdir,
	.open 		= pb_open,
	.read 		= pb_read,
	.write 		= pb_write,
	.getattr 	= pb_getattr,
	.opendir 	= pb_opendir,
	.init 		= pb_init,
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
	umask(0);
	return fuse_main(argc, argv, &pb_oper, NULL);
}


/*
int main(int argc, char *argv[])
{
	initilizeFileSystem();
	char* buf = "hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!";
	char* parent = getParentPath("/abc");
	char* file = getFileDirName("/abc");
	printf("\nParent - %s\n", parent);
	printf("\nFile - %s\n", file);
	int k = pb_mkdir("/abc");
	printf("Second Call\n");
	k = pb_mkdir("/abc/x");
	k = pb_mkdir("/abc/x/kanhu");
	k = pb_open("/abc/x/hello.txt");
	printf("\nAt main: Write invoke for %s",buf);
	printf("\nLength of input: %d", strlen(buf));
	pb_write(k, buf, strlen(buf), 0, NULL);
	pb_write(k, buf, strlen(buf), 0, NULL);
	pb_read(k, buf, strlen(buf), 0, NULL);
	printf("\nAfter the read command");
	return k;
}
*/



