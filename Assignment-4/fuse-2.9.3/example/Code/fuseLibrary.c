#include <stdio.h>
#include <sys/stat.h>
#define FUSE_USE_VERSION 26
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
	root = malloc(sizeof(directory));
	strcpy(root->directoryName,"/");
	startBlock = malloc(1024*sizeof(block));
}

/*
int pb_open(const char *path, int oflags)
{
	int fd = findFileId(path);
	return fd;
}
*/

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

	char* filename = getFileDirName(path);

	//printf("Parent - %s\n", parentPath);
	//printf("File - %s\n", filename);

	//printf("\nHere we are getting ready to create the new directory inside: %s", currDirectory->directoryName);
	fflush(stdout);
	directory* newDirectory = malloc(sizeof(directory));
	strcpy(newDirectory->directoryName, filename);
	//printf("\nTp1");
	fflush(stdout);
	newDirectory->siblingDirectory = currDirectory->subdirectory;
	currDirectory->subdirectory = newDirectory;
	//printf("\nCreated New Directory: %s", filename);
	return 1;

}


int pb_open(const char *path)
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

	char* filename = getFileDirName(path);

	//printf("Parent - %s\n", parentPath);
	//printf("File - %s\n", filename);

	//printf("\nHere we are getting ready to create the new directory inside: %s", currDirectory->directoryName);
	fflush(stdout);
	directory* newDirectory = malloc(sizeof(directory));
	strcpy(newDirectory->directoryName, filename);
	//printf("\nTp1");
	fflush(stdout);
	newDirectory->siblingDirectory = currDirectory->subdirectory;
	currDirectory->subdirectory = newDirectory;
	//printf("\nCreated New Directory: %s", filename);
	return 1;

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
	char* parent = getParentPath("/abc");
	char* file = getFileDirName("/abc");
	printf("Parent - %s\n", parent);
	printf("File - %s\n", file);
	int k = pb_mkdir("/abc");
	printf("Second Call\n");
	k = pb_mkdir("/abc/x");
	k = pb_mkdir("/abc/x/kanhu");
	return k;
}



