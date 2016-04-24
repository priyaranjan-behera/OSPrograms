#include <stdio.h>
#include <sys/stat.h>
#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <string.h>
#include <errno.h>

#define FNSize 30
#define BlockSize 4096
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
int numBlocks;

int initializeFileSystem(int fsSize)
{
	numBlocks = (fsSize*1024)/4;
	//printf("Initializing the variables");
	root = malloc(sizeof(directory));
	strcpy(root->directoryName,"/");
	root->fileinDir = NULL;
	root->subdirectory = NULL;
	root->siblingDirectory = NULL;
	blocks = malloc(numBlocks*sizeof(block));
	//printf("\nInitializing the blocks");
	for(int i=0; i<numBlocks; i++)
	{
		blocks[i].status = 0;
		blocks[i].nextBlock = -1;
		blocks[i].blockContent = malloc(BlockSize);
	}
}

/*
int pb_open(const char *path, int oflags)
{
	int fd = findFileId(path);
	return fd;
}
*/


directory* traverseToDir(char* path)
{

	//printf("\nInside traverseToDir, traversing to: %s : length: %d", path, strlen(path));
	//printf("\nComparing to %s of length: %d", "/", strlen("/"));
	fflush(stdout);
	int flag;
	directory *currDirectory;
	directory* sibDirectory;
	directory *newDirectory;
	currDirectory = root;
	//printf("the output of strcmp is: %d", strcmp(path, "/"));
	fflush(stdout);
	if(strcmp(path,"/")==0)
	{
		//printf("\nReturning root");
		return currDirectory;
	}
	char *token = strtok(path, "/");
	while(token) {
		//printf( "\nProcessing Token: %s", token);
		fflush(stdout);
		currDirectory = currDirectory->subdirectory;
		if(currDirectory == NULL)
		{
			//printf("\nHandled NULL currDirectory");
			fflush(stdout);
			return NULL;
		}
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
				//printf("\nFailed to find folder");
				//failure to find directory or file in the path, create the directory here.
				return NULL;
			}

		}
	    token = strtok(NULL, "/");
	}
	//printf("\nreturning Current Directory:");
	return currDirectory;
}


int pb_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi)
{
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	directory* parentDirectory;
	directory* currDir;
	file* currFile;

	parentDirectory = traverseToDir(path);
	if(parentDirectory==NULL)
		return 0;

	currDir = parentDirectory->subdirectory;
	currFile = parentDirectory->fileinDir;

	while(currDir!=NULL)
	{
		filler(buf,currDir->directoryName, NULL, 0);
		currDir = currDir->siblingDirectory;
	}

	while(currFile!=NULL)
	{
		filler(buf, currFile->fileName, NULL, 0);
		currFile = currFile->siblingFile;
	}

	//printf("Inside Readdir");
	return 0;
}

int pb_access(const char* path, int mask)
{
	//printf("\n\nInside Access \n\n");
	fflush(stdout);
	return 0;
}

int freeSubsequentBlocks(int blockIndex)
{
	int currBlock, nextBlock;
	currBlock = blockIndex;

	while(currBlock > -1 && blocks[currBlock].status != 0)
	{
		nextBlock = blocks[currBlock].nextBlock;
		free(blocks[currBlock].blockContent);
		blocks[currBlock].blockContent = malloc(BlockSize*sizeof(char));
		blocks[currBlock].status = 0;
		blocks[currBlock].nextBlock = -1;
		currBlock = nextBlock;
	}
	
}

int getNextFreeBlockIndex()
{
	for(int i=0; i<numBlocks; i++)
	{
		if(blocks[i].status == 0)
		{
			//printf("\n\n\nNext Free Block = %d", i);
			return i;
		}
		//else
			//printf("\nblock: %d not free", i);
	}
	return -1;
}

int pb_mknod(const char* path, mode_t mode, dev_t rdev)
{
	//printf("\nInside MakeNode");
	fflush(stdout);
	struct fuse_file_info fi;
	return pb_open(path,&fi);
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
void removeSpaces(char *str)
{
    // To keep track of non-space character count
    int count = 0;
 
    // Traverse the given string. If current character
    // is not space, then place it at index 'count++'
    for (int i = 0; str[i]; i++)
        if (str[i] != ' ')
            str[count++] = str[i]; // here count is
                                   // incremented
    str[count] = '\0';
}



int pb_mkdir(const char *path, mode_t mode)
{
	//printf("\nInside MakeDir");
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
	return 0;

}

file* getFilePresentAtDir(directory* currDirectory, char* fileName)
{
	//printf("Inside getFilePresentAtDir");
	fflush(stdout);
	file* currFile = currDirectory->fileinDir;
	//printf("\nSearching for file inside the directory: %s", currDirectory->directoryName);
	fflush(stdout);
	while(currFile != NULL)
	{
		//printf("Inside the Loop\n");
		fflush(stdout);
		//printf("\nComparing the files: %s and %s", currFile->fileName, fileName);
		if(strcmp(currFile->fileName, fileName)==0)
		{
			return currFile;
		}
		currFile = currFile->siblingFile;
	}

	//printf("\nGoing to return NULL\n");
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
		if(currBlock->nextBlock == NULL || currBlock->nextBlock == -1 || blocks[currBlock->nextBlock].status == 0)
			return size;
		currBlock = &blocks[currBlock->nextBlock];
	}		
	return 0;
}

directory* getDirPresentAtDir(directory* currDirectory, char* dirName)
{
	directory* travDirectory = currDirectory->subdirectory;
	//printf("\nSearching for directory inside the directory: %s", currDirectory->directoryName);
	fflush(stdout);
	while(travDirectory != NULL)
	{
		//printf("\nComparing the dirs: %s and %s", travDirectory->directoryName, dirName);
		if(strcmp(travDirectory->directoryName, dirName)==0)
		{
			//printf("Yes. Found it");
			return travDirectory;
		}
		travDirectory = travDirectory->siblingDirectory;
	}

	return NULL;
}

int removeDirectoryFromDirectory(directory* currDirectory, char* dirName)
{
	directory* travDirectory = currDirectory->subdirectory;
	directory* oldDir;
	//printf("\nSearching for directory inside the directory: %s", currDirectory->directoryName);
	fflush(stdout);
	if(currDirectory->subdirectory != NULL && strcmp(currDirectory->subdirectory->directoryName, dirName) == 0)
	{
		oldDir = currDirectory->subdirectory;
		currDirectory->subdirectory = currDirectory->subdirectory->siblingDirectory;
		free(oldDir);
		return 0;
	}
	else
	{
		while(travDirectory->siblingDirectory != NULL)
		{
			//printf("\nComparing the dirs: %s and %s", travDirectory->directoryName, dirName);
			if(strcmp(travDirectory->siblingDirectory->directoryName, dirName)==0)
			{
				//printf("Yes. Found it");
				travDirectory->siblingDirectory = travDirectory->siblingDirectory->siblingDirectory;
				return 0;
			}
			travDirectory = travDirectory->siblingDirectory;
		}
	}

	return NULL;
}

int pb_openfd(const char *path,struct fuse_file_info* fi)
{
	//printf("\nInside Open");
	file* oldFile;
	file* newFile;
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
	//fflush(stdout);

	oldFile = getFilePresentAtDir(currDirectory, fileName);
	//printf("Returned File as NULL");
	fflush(stdout);
	if(oldFile == NULL)
	{
		//new file
		
		//printf("\nCreating a new file here!");
		fflush(stdout);
		newFile = malloc(sizeof(file));
		strcpy(newFile->fileName, fileName);
		newFile->siblingFile = currDirectory->fileinDir;
		currDirectory->fileinDir = newFile;
		newFile->firstBlock = getNextFreeBlockIndex();
		if(newFile->firstBlock == -1)
		{
			pb_unlink(path);
			return -ENOSPC;
		}
		//printf("Creating new file and allocating block: %d", newFile->firstBlock);
		fflush(stdout);
		blocks[newFile->firstBlock].status = 1;
		blocks[newFile->firstBlock].nextBlock = -1;
		return newFile->firstBlock;
		
		//todo: get the next available block and allocate it to the file.
	}
	else
	{
		//printf("\nReturing the old file descriptor");
		return oldFile->firstBlock;
		//we have the file. Lets send the index of the block
	}



	return -ENOENT;

}

int pb_open(const char *path,struct fuse_file_info* fi)
{
	//printf("\nInside Open");
	file* oldFile;
	file* newFile;
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
	//fflush(stdout);

	oldFile = getFilePresentAtDir(currDirectory, fileName);
	//printf("Returned File as NULL");
	fflush(stdout);
	if(oldFile == NULL)
	{
		//new file
		
		//printf("\nCreating a new file here!");
		fflush(stdout);
		newFile = malloc(sizeof(file));
		strcpy(newFile->fileName, fileName);
		newFile->siblingFile = currDirectory->fileinDir;
		currDirectory->fileinDir = newFile;
		newFile->firstBlock = getNextFreeBlockIndex();
		if(newFile->firstBlock == -1){
			pb_unlink(path);
			return -ENOSPC;
		}
		//printf("Creating new file and allocating block: %d", newFile->firstBlock);
		blocks[newFile->firstBlock].status = 1;
		blocks[newFile->firstBlock].nextBlock = -1;
		return 0;
		
		//todo: get the next available block and allocate it to the file.
	}
	else
	{
		//printf("\nReturing the old file descriptor");
		return 0;
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
	//printf("\nInside Getattr");
	int res;
	if(strcmp(path,"/")==0)
	{
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_ino = 0;
		stbuf->st_nlink = 2;
		stbuf->st_dev = 0;
		return 0;
	}


	//printf("Triggering getattr for %s", path);
	file* foundFile;
	directory* foundDirectory;
	fflush(stdout);
	char* parentPath = getParentPath(path);
	//printf("\nParent Path: %s", parentPath);
	fflush(stdout);
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

	//printf("\nWill Invoke getDirPresentAtDir");
	foundDirectory = getDirPresentAtDir(currDirectory, fileName);
	if(foundDirectory!=NULL)
	{
		//Code here
		//printf("\nFound Dir");
		stbuf->st_dev = 1;
		stbuf->st_ino = 0;
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 1;
		return 0;
	}
	else
	{
		foundFile = getFilePresentAtDir(currDirectory, fileName);
		if(foundFile != NULL)
		{
			//printf("Found File");
			fflush(stdout);
			stbuf->st_mode = S_IFREG | 0777;
			stbuf->st_nlink = 1;
			stbuf->st_size = getFileSize(foundFile);
			//printf("\nFile Size: %d", getFileSize(foundFile) );
			return 0;
			//we have the file. Lets send the index of the block
		}
	}
	/*
	//printf("Creating new file: %s", path);
	fflush(stdout);
	struct fuse_file_info fi;
	pb_open(path, &fi);
	*/
	//printf("Returning Error");
	fflush(stdout);
	return -ENOENT;

}

int pb_rmdir(const char* path)
{



	//printf("Triggering rmdir for %s", path);
	file* foundFile;
	char* filePath = malloc(100);
	directory* foundDirectory;
	directory* processingDir;
	directory* oldDir;
	directory* subDirectory;
	fflush(stdout);
	char* parentPath = getParentPath(path);
	//printf("\nParent Path: %s", parentPath);
	fflush(stdout);
	if(parentPath == NULL)
		return -ENOENT;

	//printf("Parent - %s\n", parentPath);
	directory* parentDirectory = traverseToDir(parentPath);
	if(parentDirectory == NULL)
		return -ENOENT;
	//printf("Tp1");

	char* fileName = getFileDirName(path);

	//printf("Parent - %s\n", parentPath);
	//printf("File - %s\n", fileName);

	//printf("\nWill Invoke getDirPresentAtDir");
	foundDirectory = getDirPresentAtDir(parentDirectory, fileName);
	processingDir = foundDirectory;
	if(foundDirectory!=NULL)
	{
		//processingDir = traverseToDir(path);
		//deleting files inside the folder
		while(processingDir->fileinDir!=NULL)
		{

			strcpy(filePath, path);
			strcat(filePath, "/");
			strcat(filePath, processingDir->fileinDir->fileName);
			pb_unlink(filePath);
			//printf("LOOP1");

		}
		//deleting the folders inside the folder
		processingDir->subdirectory;
		while(processingDir->subdirectory!=NULL)
		{
			strcpy(filePath, path);
			strcat(filePath, "/");
			strcat(filePath, processingDir->subdirectory->directoryName);
			pb_rmdir(filePath);
			processingDir->subdirectory = processingDir->subdirectory->siblingDirectory;
			//printf("LOOP2");
		}
		//oldDir = processingDir;
		//processingDir = processingDir->siblingDirectory;
		//free(oldDir); //some error here. Skipping it for a while.
		//parentDirectory->subdirectory = foundDirectory->siblingDirectory;
		removeDirectoryFromDirectory(parentDirectory, fileName);
		return 0;
	}

	
	/*
	//printf("Creating new file: %s", path);
	fflush(stdout);
	struct fuse_file_info fi;
	pb_open(path, &fi);
	*/
	//printf("Deleted the Folder");
	free(filePath);
	free(parentPath);
	//printf("Returning Error");
	fflush(stdout);
	return -ENOENT;

}


int pb_unlink(const char* path)
{

	//printf("Triggering unlink for %s", path);
	file* prevFile;
	file* currFile;
	file* oldFile;
	directory* processingDir;
	fflush(stdout);
	char* parentPath = getParentPath(path);
	//printf("\nParent Path: %s", parentPath);
	fflush(stdout);
	if(parentPath == NULL)
		return -ENOENT;

	//printf("Parent - %s\n", parentPath);
	processingDir = traverseToDir(parentPath);
	if(processingDir == NULL)
		return -ENOENT;

	char* fileName = getFileDirName(path);

	//printf("Parent - %s\n", parentPath);
	//printf("File - %s\n", fileName);

	if(processingDir->fileinDir!=NULL && strcmp(processingDir->fileinDir->fileName, fileName)==0)
	{
		freeSubsequentBlocks(processingDir->fileinDir->firstBlock);
		oldFile = processingDir->fileinDir;
		processingDir->fileinDir = processingDir->fileinDir->siblingFile;
		free(oldFile);
		return 0;

	}
	else if(processingDir->fileinDir!=NULL)
	{
		prevFile = processingDir->fileinDir;
		currFile = prevFile->siblingFile;
		while(currFile!=NULL)
		{
			if(strcmp(currFile->fileName, fileName)==0)
			{
				freeSubsequentBlocks(currFile->firstBlock);
				oldFile = currFile;
				prevFile->siblingFile = currFile->siblingFile;
				free(oldFile);
				return 0;
			}
			prevFile = currFile;
			currFile = currFile->siblingFile;
		}
	}

	/*
	//printf("Creating new file: %s", path);
	fflush(stdout);
	struct fuse_file_info fi;
	pb_open(path, &fi);
	*/
	//printf("Returning Error");
	fflush(stdout);
	return -ENOENT;

}



int pb_writefile(const char* path, int startBlock, const void *buf, size_t count, off_t offset, struct fuse_file_info *fi)
{
	int blocksToOffset, offsetInFirstBlock, wroteBytes, bytesToWriteInFirst;
	int bytesToWriteInLast, blocksToWriteAfterFirst, offsetInBlock, offsetBlocks;

	//We will precalcute the offset blocks and the stats thereafter.

	offsetBlocks = offset/BlockSize;
	offsetInFirstBlock = offset%BlockSize;
	if(count <= BlockSize - offsetInFirstBlock)
	{
		bytesToWriteInFirst = count;
		blocksToWriteAfterFirst = 0;

	}	
	else{
		bytesToWriteInFirst = BlockSize - offsetInFirstBlock;
		blocksToWriteAfterFirst = (count - bytesToWriteInFirst)/BlockSize;
	}
	offsetInBlock = offset/BlockSize;
	if((bytesToWriteInFirst + offsetInFirstBlock) < BlockSize)
		bytesToWriteInLast = 0;
	else
		bytesToWriteInLast = count - (bytesToWriteInFirst + (BlockSize*(blocksToWriteAfterFirst)));


	wroteBytes = 0;
	char* tempBuf = malloc(BlockSize);
	//printf("\nTP1");
	block* firstBlock = &blocks[startBlock];
	block* currBlock;
	currBlock = firstBlock;

	//printf("\nBefore Iterating: ");
	//printf("\nStart Block: %d", startBlock);
	//printf("\nOffset Blocks Required:%d", offsetBlocks);

	for(int i=0; i<offsetBlocks; i++)
	{
		//printf("Next Block listed here is: %d", currBlock->nextBlock);
		if(currBlock->nextBlock == NULL || currBlock->nextBlock < 0)
		{
			//printf("\n Need to allocate new block");
			currBlock->nextBlock = getNextFreeBlockIndex();
			if(currBlock->nextBlock == -1){
				pb_unlink(path);
				return -ENOSPC;	
			}
			//printf("\nIterating to block: %d", currBlock->nextBlock);
			currBlock = &blocks[currBlock->nextBlock];
			currBlock->nextBlock = -1;
		}
		else
		{
			//printf("\nIterating to block: %d", currBlock->nextBlock);
			currBlock = &blocks[currBlock->nextBlock];
		}
		currBlock->status = 1;
	}

	//Now read the first block after the offset
	//printf("\nTP2 - blocksToWriteAfterFirst: %d", blocksToWriteAfterFirst);
	//printf("\nTP2 - bytesToWriteInLast: %d", bytesToWriteInLast);
	//printf("\n Offset: %d", offset);
	//printf("\n Count: %d", count);
	//printf("\n offsetBlocks: %d", offsetBlocks);
	//printf("\n offsetInFirstBlock: %d", offsetInFirstBlock);
	//printf("\n Offset: %d", offset);
	//printf("\n Count: %d", count);
	//printf("\nInside - Write: Starting Block: %d", startBlock);
	if(blocksToWriteAfterFirst == 0 && bytesToWriteInLast == 0)
	{
		memcpy(&currBlock->blockContent[offsetInFirstBlock],buf, count);
		//printf("Wrote: %s,\nwrotesize:", &currBlock->blockContent[offsetInFirstBlock],strlen(&currBlock->blockContent[offsetInFirstBlock]));
		wroteBytes = count;
	}
	else
	{
		memcpy(&currBlock->blockContent[offsetInFirstBlock],buf, BlockSize-offsetInFirstBlock);
		//printf("\nTP2Wrote: %s\n wrote size: %d", &currBlock->blockContent[offsetInFirstBlock],strlen(&currBlock->blockContent[offsetInFirstBlock]));
		wroteBytes = BlockSize-offsetInFirstBlock;
		buf += wroteBytes; //temp
		//printf("\nWrote here: %d", wroteBytes);
	//Need to free all the blocks after this block first <free, malloc, status 0>

	}
	//todo:free subsequent blocks
	if((currBlock->nextBlock != NULL && currBlock->nextBlock != -1))
		freeSubsequentBlocks(currBlock->nextBlock);
	
	//printf("\nTP2 - readBytes: %d", wroteBytes);
	fflush(stdout);
	//Now all the full blocks to be written
	for(int i = 0; i<blocksToWriteAfterFirst; i++)
	{
		//printf("\nTP3 - readBytes: %d", wroteBytes);
		fflush(stdout);
		currBlock->nextBlock = getNextFreeBlockIndex();
		if(currBlock->nextBlock == -1){
			pb_unlink(path);
			return -ENOSPC;
		}
		//printf("\nTP3 - readBytes: %d", wroteBytes);
		fflush(stdout);
		currBlock = &blocks[currBlock->nextBlock];
		currBlock->status = 1;
		currBlock->nextBlock = -1;
		//printf("\nTP3 - readBytes: %d", wroteBytes);
		fflush(stdout);
		memcpy(currBlock->blockContent,buf, BlockSize);
		//printf("\nTP3 - readBytes: %d", wroteBytes);
		//printf("\nWrote of Size: %d", strlen(currBlock->blockContent));
		fflush(stdout);
		wroteBytes += BlockSize;
		buf += BlockSize;
	}
	//printf("\nTP3 - writeBytes: %d", wroteBytes);
	fflush(stdout);
	//this is the last block
	if(bytesToWriteInLast > 0)
	{
		currBlock->nextBlock = getNextFreeBlockIndex();
		if(currBlock->nextBlock == -1){
			pb_unlink(path);
			return -ENOSPC;
		}
		currBlock = &blocks[currBlock->nextBlock];
		currBlock->status = 1;
		memcpy(currBlock->blockContent,buf, bytesToWriteInLast); //todo
		//printf("\nWrote of Size: %d", strlen(currBlock->blockContent));
		wroteBytes += bytesToWriteInLast;
	}

	currBlock->nextBlock = -1;
	//printf("\nTP1 - wroteBytes:%d", wroteBytes);
	//printf("\nShould have written: %d",count);
	return wroteBytes;
}

int pb_write(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
	//printf("\nInside Write\n");
	int fd = pb_openfd(path,fi);
	//printf("Inside pb_write");
	fflush(stdout);
	if(fd<0)
		return -ENOENT;
	return pb_writefile(path,fd,buf,size,offset,fi);
}

file* getFileFromPath(char* path)
{
	char* parentPath =  getParentPath(path);
	char* fileName = getFileDirName(path);
	directory* parentDirectory = traverseToDir(parentPath);
	return getFilePresentAtDir(parentDirectory, fileName);
}


int pb_readfile(const char* path, int startBlock, const void *buf, size_t count, off_t offset, struct fuse_file_info *fi)
{
	printf("\nCount given: %d", count);
	fflush(stdout);
	file* queriedFile = getFileFromPath(path);
	printf("\nThe Size of the queried file is: %d", getFileSize(queriedFile));
	printf("\nInitial Size of buffer: %d", strlen(buf));
	printf("\nWhat is inside the buffer: %s", buf);
	//allocating sufficient memory for buffer
	int currFileSize = getFileSize(queriedFile);

	if(offset + count > currFileSize)
	{
		count = currFileSize - offset;
	}
	//buf = realloc(buf,count+10000);
	printf("\nAdjusted Size of buffer: %d", strlen(buf));

	int blocksToOffset, offsetInFirstBlock, readBytes, bytesToWriteInFirst;
	int bytesToWriteInLast, blocksToWriteAfterFirst, offsetInBlock, offsetBlocks;
	//We will precalcute the offset blocks and the stats thereafter.

	offsetBlocks = offset/BlockSize;
	offsetInFirstBlock = offset%BlockSize;
	if(count <= BlockSize - offsetInFirstBlock)
	{
		bytesToWriteInFirst = count;
		blocksToWriteAfterFirst = 0;

	}	
	else{
		bytesToWriteInFirst = BlockSize - offsetInFirstBlock;
		blocksToWriteAfterFirst = (count - bytesToWriteInFirst)/BlockSize;
	}
	offsetInBlock = offset/BlockSize;
	if((bytesToWriteInFirst + offsetInFirstBlock) < BlockSize)
		bytesToWriteInLast = 0;
	else
		bytesToWriteInLast = count - (bytesToWriteInFirst + (BlockSize*(blocksToWriteAfterFirst)));

	printf("\n*****PRINTING THE STATS BEFORE READING*****\n");
	printf("\nTP2 - blocksToWriteAfterFirst: %d", blocksToWriteAfterFirst);
	printf("\nTP2 - bytesToWriteInLast: %d", bytesToWriteInLast);
	printf("\n Offset: %d", offset);
	printf("\n Count: %d", count);
	printf("\n offsetBlocks: %d", offsetBlocks);
	printf("\n offsetInFirstBlock: %d", offsetInFirstBlock);
	printf("\nInside - Write: Starting Block: %d", startBlock);

	readBytes = 0;
	char* tempBuf = malloc(BlockSize);
	//printf("\nTP1");
	block* firstBlock = &blocks[startBlock];
	block* currBlock;
	currBlock = firstBlock;

	//printf("\nBefore Iterating: ");
	//printf("\nStart Block: %d", startBlock);
	//printf("\nOffset Blocks Required:%d", offsetBlocks);

	for(int i=0; i<offsetBlocks; i++)
	{
		printf("\nSkipping OFFSET: %d", i);
		if(currBlock->nextBlock == NULL || currBlock->nextBlock < 0)
			return -EINVAL;
		currBlock = &blocks[currBlock->nextBlock];
	}

	//Now read the first block after the offset
	//printf("\nTP2 - blocksToWriteAfterFirst: %d", blocksToWriteAfterFirst);
	//printf("\nTP2 - bytesToWriteInLast: %d", bytesToWriteInLast);
	//printf("\n Offset: %d", offset);
	//printf("\n Count: %d", count);
	//printf("\n offsetBlocks: %d", offsetBlocks);
	//printf("\n offsetInFirstBlock: %d", offsetInFirstBlock);
	//printf("\n Offset: %d", offset);
	//printf("\n Count: %d", count);
	//printf("\nInside - Write: Starting Block: %d", startBlock);
	if(blocksToWriteAfterFirst == 0 && bytesToWriteInLast == 0 && count < BlockSize)
	{
		free(tempBuf);
		tempBuf = malloc(BlockSize);
		printf("\nTrying to from only block", &currBlock->blockContent[offsetInBlock]);
		memcpy(tempBuf,&currBlock->blockContent[offsetInFirstBlock],count);
		//strcpy(tempBuf,&currBlock->blockContent[offsetInFirstBlock]);
		printf("\nLength of Temp Buf: %d\n", strlen(tempBuf));
		memcpy(buf,tempBuf,count);
		//strcpy(buf,tempBuf);
		readBytes = count;
		printf("\nLength of String: %d\n", strlen(buf));
	}
	else
	{
		free(tempBuf);
		tempBuf = malloc(BlockSize);
		printf("\nTrying to read from first of many block:", &currBlock->blockContent[offsetInBlock]);
		memcpy(tempBuf,&currBlock->blockContent[offsetInFirstBlock],BlockSize-offsetInFirstBlock);
		//strcpy(tempBuf,&currBlock->blockContent[offsetInFirstBlock]);
		printf("\nLength of Temp Buf: %d\n", strlen(tempBuf));
		memcpy(buf,tempBuf,BlockSize-offsetInFirstBlock);
		//strcpy(buf, tempBuf);
		readBytes = BlockSize-offsetInFirstBlock; //temp
	//Need to free all the blocks after this block first <free, malloc, status 0>

		printf("\nLength of String: %d\n", strlen(buf));

	}
	
	
	//printf("\nTP2 - readBytes: %d", readBytes);

	//Now all the full blocks to be written
	for(int i = 0; i<blocksToWriteAfterFirst; i++)
	{
		printf("\nReading from full blocks");
		printf("\nWill call malloc for tempbuf of size: %d", BlockSize);
		fflush(stdout);
		if(currBlock->nextBlock == NULL || currBlock->nextBlock < 0){
			printf("\nCould not find further block");
			fflush(stdout);
			return readBytes;
		}
		currBlock = &blocks[currBlock->nextBlock];
		//free(tempBuf);
		tempBuf = malloc(BlockSize);
		
		//printf("\nBlock Size is: BlockSize %d", BlockSize);
		//printf("Length of String: %d\n", strlen(buf));
		memcpy(tempBuf,currBlock->blockContent,BlockSize);
		//strcpy(tempBuf,currBlock->blockContent);
		printf("\nLength of Temp Buf: %d\n", strlen(tempBuf));
		printf("\nBefore: Length of String: %d\n", strlen(buf));
		fflush(stdout);
		memcpy(buf+readBytes,tempBuf,BlockSize);
		printf("\nAfter: Length of String: %d\n", strlen(buf));
		//strcat(buf,tempBuf);
		readBytes += BlockSize;
		//printf("TP2 - readBytes: %d", readBytes);

		
	}

	//this is the last block
	if(bytesToWriteInLast > 0)
	{
		printf("/nLast Block Reading:");
		if(currBlock->nextBlock == NULL || currBlock->nextBlock < 0)
			return -EINVAL;
		currBlock = &blocks[currBlock->nextBlock];
		free(tempBuf);
		tempBuf = malloc(bytesToWriteInLast);
		memcpy(tempBuf,currBlock->blockContent,bytesToWriteInLast);
		//strcpy(tempBuf,currBlock->blockContent);
		printf("\nContent at last block: ", currBlock->blockContent);
		printf("\nLength of Temp Buf: %d\n", strlen(tempBuf));
		printf("\nValue of bytesToWriteInLast: %d",bytesToWriteInLast);
		//strcat(buf,tempBuf);
		memcpy(buf+readBytes,tempBuf,strlen(tempBuf));
		readBytes += bytesToWriteInLast;

		printf("Length of String: %d\n", strlen(buf));
	}
	

	//printf("\nRead buf: %s", buf);
	//printf("\nLength of buf: %d", strlen(buf));
	//printf("\nTP1: readBytes: %d", readBytes);
	//strcat(buf,"\0");
	//change readbytes if you face problem
	//readBytes = strlen(buf);
	//printf("\nTP2: readBytes: %d", readBytes);
	return readBytes;
}

int pb_read(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
	//printf("Inside Read");
	int fd = pb_openfd(path,fi);
	//printf("Inside pb_read");
	fflush(stdout);
	if(fd<0)
		return -ENOENT;
	return pb_readfile(path, fd, buf, size, offset, fi);
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
/*
void* pb_init(struct fuse_conn_info *conn)
{
	initializeFileSystem();
}

*/
static struct fuse_operations pb_oper = {
	.mkdir		= pb_mkdir,
	.open 		= pb_open,
	.read 		= pb_read,
	.write 		= pb_write,
	.getattr 	= pb_getattr,
	.opendir 	= pb_opendir,
	.readdir 	= pb_readdir,
	.mknod 		= pb_mknod,
//	.init 		= pb_init,
	.access 	= pb_access,
	.unlink 	= pb_unlink,
	.rmdir 		= pb_rmdir,
};

/*
int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &pb_oper, NULL);
}
*/
/*

int main(int argc, char *argv[])
{
	mode_t m;
	struct fuse_file_info fi;
	openlog("slog", LOG_PID|LOG_CONS, LOG_USER);
 	syslog(LOG_INFO, "A different kind of Hello world ... ");
 	struct stat* stbuf = malloc(sizeof(struct stat));
 	//printf("Root Folder: %s", argv[1]);
	initializeFileSystem();
	char* buf = "hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!hello how are you!";
	char* parent = getParentPath("/abc");
	char* file = getFileDirName("/abc");
	//printf("\nParent - %s\n", parent);
	//printf("\nFile - %s\n", file);
	int k = pb_mkdir("/abc",m);
	//printf("Second Call\n");
	k = pb_mkdir("/abc/x",m);
	k = pb_mkdir("/abc/x/kanhu",m);
	//printf("Check from here");
	k = pb_open("/hello.txt", &fi);
	//printf("\nAt main: Write invoke for %s",buf);
	//printf("\nLength of input: %d : k= %d", strlen(buf), k);
	pb_write("/hello.txt", buf, strlen(buf), 0, NULL);
	pb_read("/hello.txt", buf, strlen(buf), 0, NULL);

	closelog();
	return k;
}
*/


int main(int argc, char *argv[])
{
	initializeFileSystem(atoi(argv[argc-1]));
	return fuse_main(argc-1, argv, &pb_oper, NULL);
}
