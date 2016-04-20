#include <stdio.h>
#include <conio.h>
#include <fuse.h>

#define FNSize 30

typedef struct block{
	int index;
	struct block * nextBlock;
	char blockContent[4096];
} block;

typedef struct file{
	int fileId;
	char filename[FNSize];
	int firstBlock;
} file;

typedef struct directory{
	int directoryId;
	char directoryName[FNSize];
	struct directory * subdirectory;
	struct directory * siblingDirectory;
	struct file * fileinDir;
} directory;

directory* root;
block* startBlock;



int initilizeFileSystem()
{
	root = malloc(sizeof(directory))
}

int open(const char *path, int oflags)
{
	int fd = findFileId(path);
	return fd;
}

int mkdir(const char *path, mode_t mode)
{
	char *token = strtok(input, "/");
	int flag;
	directory *currDirectory;
	directory* sibDirectory;
	directory *newDirectory;
	while(token) {
    	if(strcmp(token, root->directoryName) == 0)
    		currDirectory = root;
    	else
    	{
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
    			//failure to find directory or file in the path, create the directory here.
    			newDirectory = malloc(sizeof(directory));
    			newDirectory->siblingDirectory = currDirectory;
    			currDirectory = newDirectory;
    			strcpy(currDirectory->directoryName, token);
    			return -1;
    		}
    	}
        puts(token);
        token = strtok(NULL, "/");
    }

}


int findFileId(const char *path)
{
	char* temp;
	strcpy(temp, path);
	char *token = strtok(input, "/");
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
    			if(strcmp(currFile.filename, token)
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






