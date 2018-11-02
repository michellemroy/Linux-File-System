#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


int file_ct = 3;
int dir_ct = 2;
int cont_ct = 2;

//inode structures
char files[1024][100] = {"trail.txt\0","flowers.txt\0"}; //file names
int fil_cont[1024][1000] = {{0},{1}};		//file mapping
int fil_cont_ct[1024] = {1,1};	//file block count
char content[1024][4096] = {"Hello World Trial.txt","Flowers Flowers Flowers"};				//file contents
int dir_files[100][100];			//2D array to map files to directories
int dir_fil_ct[100] = {1,1};						//counter for number of files in each directory
char dirs[100][100] = {"","new"};					//directory names array
int dir_dirs[100][63] = {{1}};					//2D array to handle nesting of directories
int dir_dir_ct[100] = {1,0};						//array to give nesting level of directory

//function to return file attributes for various file operations
static int do_getattr( const char *path, struct stat *st )
{
	printf("getattr-> path = %s\n",path);
	st->st_uid = getuid();				//user id
	st->st_gid = getgid();				//group id
	//st->st_atime = time( NULL );		//last access time
	//st->st_mtime = time( NULL );		//last modified time

	
	char *filename = malloc(sizeof(char)*100);
	char *dirname = malloc(sizeof(char)*100);
	memcpy(filename,path,100);			//copy path of file to memory
	
	int i;
	for(i=strlen(filename)-1;i>=0;i--)
		if(filename[i] == 47)
			break;
	strncpy(dirname,filename,i+1);		//split to directory, filename
	dirname[i+1] = '\0';
	dirname++;
	filename = filename+i+1;			

	printf("getattr dirname = %s , filename = %s\n",dirname, filename);		//print attributes for status in terminal
	char first[100] = "";
	char second[100];
	
	//loop to check for nested files and directories
	i=0;
	int j=0,l;
	while(strlen(dirname)>0)
	{

	
		for(i=0;i<strlen(dirname);i++)
		{
			if(dirname[i] == 47 || dirname[i+1] == '\0')
				{
					strncpy(second,dirname,i);
					second[i] = '\0';
					
					dirname += i+1;

					
					for(l=0;l<dir_dir_ct[j];l++)
						if(strcmp(second,dirs[dir_dirs[j][l]])==0)
							break;
					j = dir_dirs[j][l];
					strcpy(first,second);
					break;
					
				
				}

		}
		
	}
	/*if(strcmp(filename,"")==0)
	{

			st->st_mode = S_IFDIR | 0755;			//set permissions and file type constant as directory
			st->st_nlink = 2;						//symbolic link
			return 0;
	}*/
		

	//check if directory exists
	for(int i=0;i<dir_ct;i++)
	{
		if(strcmp(filename ,dirs[dir_dirs[j][i]])==0)
			{
			
			st->st_mode = S_IFDIR | 0755;			//set permissions and file type constant as directory
			st->st_nlink = 2;						//symbolic link
			return 0;
			}
	}

	
	//check if file exists
	for(int i=0;i<dir_fil_ct[j];i++)
	{
		if(strcmp(filename,files[dir_files[j][i]])==0)
			{
			
			st->st_mode = S_IFREG | 0777;			//set file permissions
			st->st_nlink = 1;
			st->st_size = 4096*fil_cont_ct[dir_files[j][i]];
			st->st_blocks=fil_cont_ct[dir_files[j][i]];						//set file size
			
			return 0;
			}
	}
	
	
	return -ENOENT;						//error flag returned if all cases fail, denotes no such file or directory
}


//function to read attributes of a directory
static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	printf( "ls --> Getting The List of Files of %s\n", path );
	printf("************************************************************\n");
		
	char *filename = malloc(sizeof(char)*100);
	char *dirname = malloc(sizeof(char)*100);
	strcpy(filename,path);
	int i;
	for(i=strlen(filename)-1;i>=0;i--)				
		if(filename[i] == 47)
			break;
	strcpy(dirname,filename);			//splits to directory,filename  -> filename here is empty
	//dirname[i+1] = '\0';
	dirname++;
	filename = filename+i+1;

	printf("dirname = %s , filename = %s\n",dirname, filename); 
	
	
	filler( buffer, ".", NULL, 0 );			// Current Directory
	filler( buffer, "..", NULL, 0 );		// Parent Directory

	char first[100] = "";
	char second[100];
	
	//loop to traverse nested directories
	int j=0,l;
	while(strlen(dirname)>0 && *dirname !=127)
	{

		for(i=0;i<=strlen(dirname);i++)
		{
			if(dirname[i] == 47 || dirname[i] == '\0')
				{
					strncpy(second,dirname,i);
					second[i] = '\0';
					
					dirname += i+1;

				
					for(l=0;l<dir_dir_ct[j];l++)
					{
						if(strcmp(second,dirs[dir_dirs[j][l]])==0)
							break;

					}
					j = dir_dirs[j][l];
					break;
					
				
				}

		}
			
	}

	
	for(int k =0;k<dir_dir_ct[j];k++)
		filler(buffer,dirs[dir_dirs[j][k]],NULL,0);				//set file table with directory contents



	for(int i=0;i<dir_fil_ct[j];i++)
	{
			filler( buffer, files[dir_files[j][i]], NULL,0 );			//set file table with file contents

	}


	
	return 0;
}


//function to read files
static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	printf( "cat/read--> Trying to read %s, %d, %u\n", path,(int)offset, size );		//printing status to terminal
	printf("************************************************************\n");

	char *filename = malloc(sizeof(char)*100);
	char *dirname = malloc(sizeof(char)*100);
	memcpy(filename,path,100);					//copy filename to memory
	int i;
	for(i=strlen(filename)-1;i>=0;i--)
		if(filename[i] == 47)
			break;
	strncpy(dirname,filename,i+1);
	dirname[i+1] = '\0';
	dirname++;
	filename = filename+i+1; 
	char *selectedText = NULL;

	char first[100] = "";
	char second[100];
	
	// loop to traverse nested files and directories
	int j=0,l;
	while(strlen(dirname)>0)
	{

	
		for(i=0;i<strlen(dirname);i++)
		{
			if(dirname[i] == 47 || dirname[i+1] == '\0')
				{
					strncpy(second,dirname,i);
					second[i] = '\0';
					
					dirname += i+1;

					
					for(l=0;l<dir_dir_ct[j];l++)
						if(strcmp(second,dirs[dir_dirs[j][l]])==0)
							break;
					j = dir_dirs[j][l];
					strcpy(first,second);
					break;
					
				
				}

		}
		
	}
	
	for(i=0;i<dir_fil_ct[j];i++)
	{
		if(strcmp(filename, files[dir_files[j][i]]) == 0)
			break;
	}


	
	//printf("Writing to file %s\n",filename); 
	//printf("contents = %s", buffer);

	int bl_ct = fil_cont_ct[dir_files[j][i]];

	char *temp = malloc(sizeof(char)*4096*bl_ct);
	//char *start =malloc(sizeof(char)*4096*bl_ct);
	
	//int bl_num = offset/4096;
	//printf("bl_num = %d",bl_num);
	

	//memcpy(buffer,content[dir_files[j][i]], size);
	FILE *fp = fopen("content.txt","r+");
	for (int k = 0;k<bl_ct;k++)
	{
	
	rewind(fp);
	fseek(fp,4096*fil_cont[dir_files[j][i]][k],SEEK_SET);
	printf("offset =%d\n",4096*fil_cont[dir_files[j][i]][k]);
	fread(temp+(4096*k),4096,1,fp);
	//temp+=4096;
	}
	fclose(fp);

	printf("contents = %s\n",temp);

	//buffer = realloc(buffer,sizeof(char)*4096*bl_ct);

	memcpy(buffer,temp,size);			//read content to buffer
	
	free(temp);
	return size;


	
}

static int do_open(const char *path, struct fuse_file_info *fi )
{
	printf("opening file %s\n", path);
	return 0;
}


//function to write to a file
static int do_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	char *filename = malloc(sizeof(char)*100);
	char *dirname = malloc(sizeof(char)*100);

	printf("size = %d,off = %d",size,offset);
	memcpy(filename,path,100);
	int i;
	for(i=strlen(filename)-1;i>=0;i--)			
		if(filename[i] == 47)
			break;
	strncpy(dirname,filename,i+1);				//split directory and filename
	dirname[i+1] = '\0';		
	dirname++;
	filename = filename+i+1;

	char first[100] = "";
	char second[100];
	
	// loop to traverse nested files and directories
	int j=0,k,l;
	while(strlen(dirname)>0)
	{

		for(i=0;i<strlen(dirname);i++)
		{
			if(dirname[i] == 47 || dirname[i+1] == '\0')
				{
					strncpy(second,dirname,i);
					second[i] = '\0';
					
					dirname += i+1;
					
					for(l=0;l<dir_dir_ct[j];l++)
						if(strcmp(second,dirs[dir_dirs[j][l]])==0)
							break;
					j = dir_dirs[j][l];
					strcpy(first,second);
					break;
					
				
				}

		}
		
	}
	

	for(i=0;i<dir_fil_ct[j];i++)
	{
		if(strcmp(filename, files[dir_files[j][i]]) == 0)
			break;
	}
	printf("Writing to file %s\n",filename); 
	printf("contents = %s, len=%d\n", buffer,strlen(buffer));
	char temp[4096];
	memcpy(temp, buffer, 4096);
	printf("len = %d\n",strlen(buffer));
	

	int bl_num = offset/4096;
	if(bl_num == 0)
		fil_cont_ct[dir_files[j][i]] = 1;
	printf("bl_num = %d",bl_num);
	if(bl_num > fil_cont_ct[dir_files[j][i]]-1)
		{
		printf("allocating more blocks\n");
		fil_cont_ct[dir_files[j][i]]++;
		fil_cont[dir_files[j][i]][fil_cont_ct[dir_files[j][i]]-1] = cont_ct;
		cont_ct++;
		}

	FILE *fp = fopen("content.txt","r+");

	fseek(fp,4096*fil_cont[dir_files[j][i]][bl_num],SEEK_SET);
	printf("offset =%d\n",4096*fil_cont[dir_files[j][i]][bl_num]);
	fwrite(temp,4096,1,fp);

	fclose(fp);

	
	FILE *fq = fopen("fil_cont.txt","w+");

	for(int k=0;k<file_ct;k++){
		for(int l=0;l<fil_cont_ct[k];l++)
			fprintf(fq,"%d\n",fil_cont[k][l]);
		
		fprintf(fq,"-5\n");
		}
	fclose(fq);
	return size;
}

//function to make directories
static int do_make(const char* path, mode_t mode)
{
	FILE *fp = fopen("dirs.txt","a+");	

	char *filename = malloc(sizeof(char)*100);
	char *dirname = malloc(sizeof(char)*100);

	memcpy(filename,path,100);
	
	int i;
	for(i=strlen(filename)-1;i>=0;i--)
		if(filename[i] == 47)
			break;
	strncpy(dirname,filename,i+1);			// split directory and filename 
	dirname[i+1] = '\0';
	dirname++;
	filename = filename+i+1;
	printf("making directory %s in %s\n",filename,dirname);


	char first[100] = "";
	char second[100];
	
	
	i=0;
	//loop to traverse nested directories and files
	int j=0,k,l;
	while(strlen(dirname)>0)
	{

		for(i=0;i<strlen(dirname);i++)
		{
			if(dirname[i] == 47 || dirname[i+1] == '\0')
				{
					strncpy(second,dirname,i);
					second[i] = '\0';
					
					dirname += i+1;
					
					for(l=0;l<dir_dir_ct[j];l++)
						if(strcmp(second,dirs[dir_dirs[j][l]])==0)
							break;
					j = dir_dirs[j][l];
					strcpy(first,second);
					break;
					
				
				}

		}
		
	}
	
	

	
	strcpy(dirs[dir_ct], filename);
	dir_ct++;			//create directory in memory					
	dir_dir_ct[j]++;								//increment number of nested directories
	dir_dirs[j][dir_dir_ct[j]-1] = dir_ct-1;  
	int z = fprintf(fp,"%s\n",filename);
	//printf
	fclose(fp);
	FILE *fq = fopen("dir_dirs.txt","w+");
	for(int i=0;i<dir_ct;i++)
		{for(int j=0;j<dir_dir_ct[i];j++)
			fprintf(fq,"%d\n",dir_dirs[i][j]);
		fprintf(fq,"-5\n");
		}
	fclose(fq);
	
	return 0;
	
}


// this function is required to implement write
static int do_truncate(const char* path, off_t size)
{
	printf("\nTruncate -> %s, %d\n", path,size);
	return size;
}

//function to create files
static int do_create(const char* path, mode_t mode, struct fuse_file_info *fi )
{
	FILE *fp = fopen("files.txt","a+");
	file_ct++;									//increment file count
	char *filename = malloc(sizeof(char)*100);
	char *dirname = malloc(sizeof(char)*100);
	memcpy(filename, path, 100);
	int i;
	for(i=strlen(filename)-1;i>=0;i--)
		if(filename[i] == 47)
			break;
	strncpy(dirname,filename,i+1);
	dirname[i+1] = '\0';
	dirname++;
	filename = filename+i+1;
	filename[strlen(filename)] = '\0';  


	char first[100] = "";
	char second[100];
	
	// loop to traverse nested files and directories
	int j=0,k,l;
	while(strlen(dirname)>0)
	{


		for(i=0;i<strlen(dirname);i++)
		{
			if(dirname[i] == 47 || dirname[i+1] == '\0')
				{
					strncpy(second,dirname,i);
					second[i] = '\0';
					
					dirname += i+1;

					
					for(l=0;l<dir_dir_ct[j];l++)
						if(strcmp(second,dirs[dir_dirs[j][l]])==0)
							break;
					j = dir_dirs[j][l];
					strcpy(first,second);
					break;
					
				
				}

		}
		
	}
	dir_fil_ct[j]++;							//increment count of files in directory
	
	printf("creating file %s in directory %s||\n", filename, dirname);
	strcpy(files[file_ct-1],filename);						//create file in memory
	dir_files[j][dir_fil_ct[j]-1] = file_ct-1; 					//setting new count of files in directory 
	printf("created %s\n", files[file_ct-1]);
	fprintf(fp,"%s\n",filename);
	fclose(fp);
	FILE *fq = fopen("dir_fils.txt","w+");
	for(int i=0;i<dir_ct;i++)
		{for(int j=0;j<dir_fil_ct[i];j++)
			fprintf(fq,"%d\n",dir_files[i][j]);
		fprintf(fq,"-5\n");
		}
	fclose(fq);
	return 0; 

}

//function to rename files
static int do_rename(const char * old, const char * new)
{
	printf("old = %s, new = %s\n",old,new);
	char *filename = malloc(sizeof(char)*100);
	
	memcpy(filename, old, 100);
	
	int i;
	for(i=strlen(filename)-1;i>=0;i--)
		if(filename[i] == 47)
			break;

	filename = filename+i+1; 

	char *new_filename = malloc(sizeof(char)*100);
	char *dirname = malloc(sizeof(char)*100);
	memcpy(new_filename, new, 100);						
	memcpy(dirname, new_filename, 100);

	for(i=strlen(new_filename)-1;i>=0;i--)
		if(new_filename[i] == 47)
			break;
	strncpy(dirname,new_filename,i+1);
	dirname[i+1] = '\0';
	dirname++;
	new_filename = new_filename+i+1;

	
	char first[100] = "";
	char second[100];
	

	int j=0,k,l;
	while(strlen(dirname)>0)
	{

	
		for(i=0;i<strlen(dirname);i++)
		{
			if(dirname[i] == 47 || dirname[i+1] == '\0')
				{
					strncpy(second,dirname,i);
					second[i] = '\0';
					
					dirname += i+1;

					
					for(l=0;l<dir_dir_ct[j];l++)
						if(strcmp(second,dirs[dir_dirs[j][l]])==0)
							break;
					j = dir_dirs[j][l];
					strcpy(first,second);
					break;
					
				
				}

		}
		
	}

	

	for(i=0;i<dir_fil_ct[j];i++)
		if(strcmp(new_filename,files[dir_files[j][i]]) == 0)
			break;
	k = i;
	for(i;i<dir_fil_ct[j];i++)
		if(strcmp(filename,files[dir_files[j][i]]) == 0)
			break;


	printf("Copying from %s to %s\n",filename, new_filename); 

	memcpy(content[dir_files[j][k]],content[dir_files[j][i]],strlen(content[dir_files[j][i]]));

	

	

	return 0;
}


static int do_removedir(const char *path)
{
printf("remove dir :", path);
char *filename = malloc(sizeof(char)*100);
char *dirname = malloc(sizeof(char)*100);

	memcpy(filename,path,100);
	
	int i;
	for(i=strlen(filename)-1;i>=0;i--)
		if(filename[i] == 47)
			break;
	strncpy(dirname,filename,i+1);			// split directory and filename 
	dirname[i+1] = '\0';
	dirname++;
	filename = filename+i+1;
	

	char first[100] = "";
	char second[100];

	printf("remove %s from %s\n",filename,dirname);
	
	
	i=0;
	//loop to traverse nested directories and files
	int j=0,k,l;
	while(strlen(dirname)>0)
	{

		for(i=0;i<strlen(dirname);i++)
		{
			if(dirname[i] == 47 || dirname[i+1] == '\0')
				{
					strncpy(second,dirname,i);
					second[i] = '\0';
					
					dirname += i+1;

					for(l=0;l<dir_dir_ct[j];l++)
						if(strcmp(second,dirs[dir_dirs[j][l]])==0)
							break;
					j = dir_dirs[j][l];
					strcpy(first,second);
					break;
					
				
				}

		}
		
	}
	
	for (k = 0;k<dir_dir_ct[j];k++)
	{
		if(strcmp(dirs[dir_dirs[j][k]],filename)==0)

			break;
	} 	

	if(dir_fil_ct[dir_dirs[j][k]] > 0 || dir_dir_ct[dir_dirs[j][k]] > 0 )
		return -ENOTEMPTY;
	
	for (;k<dir_dir_ct[j];k++)
	{
		dir_dirs[j][k] = dir_dirs[j][k+1];
	}	
	dir_dir_ct[j]--;				
	
	FILE *fq = fopen("dir_dirs.txt","w+");
	for(int i=0;i<dir_ct;i++)
		{for(int j=0;j<dir_dir_ct[i];j++)
			fprintf(fq,"%d\n",dir_dirs[i][j]);
		fprintf(fq,"-5\n");
		}
	fclose(fq);

	return 0;

}

int do_remove(const char *path)
{
char *filename = malloc(sizeof(char)*100);
	char *dirname = malloc(sizeof(char)*100);
	memcpy(filename, path, 100);
	int i;
	for(i=strlen(filename)-1;i>=0;i--)
		if(filename[i] == 47)
			break;
	strncpy(dirname,filename,i+1);
	dirname[i+1] = '\0';
	dirname++;
	filename = filename+i+1;
	filename[strlen(filename)] = '\0';  


	char first[100] = "";
	char second[100];
	
	// loop to traverse nested files and directories
	int j=0,k,l;
	while(strlen(dirname)>0)
	{


		for(i=0;i<strlen(dirname);i++)
		{
			if(dirname[i] == 47 || dirname[i+1] == '\0')
				{
					strncpy(second,dirname,i);
					second[i] = '\0';
					
					dirname += i+1;

					
					for(l=0;l<dir_dir_ct[j];l++)
						if(strcmp(second,dirs[dir_dirs[j][l]])==0)
							break;
					j = dir_dirs[j][l];
					strcpy(first,second);
					break;
					
				
				}

		}
		
	}
	
	for (k = 0;k<dir_fil_ct[j];k++)
	{
		if(strcmp(files[dir_files[j][k]],filename)==0)

			break;
	} 	


	
	for (;k<dir_fil_ct[j];k++)
	{
		dir_files[j][k] = dir_files[j][k+1];
	}	
	dir_fil_ct[j]--;

	FILE *fq = fopen("dir_fils.txt","w+");
	for(int i=0;i<file_ct;i++)
		{for(int j=0;j<dir_fil_ct[i];j++)
			fprintf(fq,"%d\n",dir_files[i][j]);
		fprintf(fq,"-5\n");
		}
	fclose(fq);	

	return 0;	

}



//setting fuse macros to redirect to our handlers
static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read		= do_read,
    .open		= do_open,
    .write		= do_write,
	.mkdir		= do_make,
	.truncate	= do_truncate,
	.create		= do_create,
	.rename		= do_rename,
	.rmdir		= do_removedir,
	.unlink		= do_remove 

	
};




int main( int argc, char *argv[])
{
	
	FILE *fp = fopen("files.txt","r+");
	int i=0,r;
	while(r = fscanf(fp,"%s",files[i]) != EOF)
	{	
		i++;
		
	}
	file_ct = i;
	fclose(fp);
	
	FILE *fq = fopen("dirs.txt","r+");
	i=0;
	while(r = fscanf(fq,"%s",dirs[i+1]) != EOF)
	{	
		
		printf("i= %d, r = %d, %s\n",i,r,dirs[i+1]);
		i++;
		
	}
	dir_ct = i+1;
	printf("i= %d, |%s|\n",i,dirs[0]);
	fclose(fq);
	FILE *fr = fopen("dir_dirs.txt","r+");
	i=0;
	int j=0;
	while(r = fscanf(fr,"%d",&dir_dirs[i][j]) != EOF)
	{	
		printf("i= %d, r = %d, %d\n",i,r,dir_dirs[i][j]);
		
		if(dir_dirs[i][j] == -5)
			{
			dir_dir_ct[i]=j;
			i++;			
			j=-1;}
		j++;
		
	}
	fclose(fr);
	fr = fopen("dir_fils.txt","a+");
	i=0;
	j=0;
	while(r = fscanf(fr,"%d",&dir_files[i][j]) != EOF)
	{	
		printf("i= %d, r = %d, %d\n",i,r,dir_files[i][j]);
		
		if(dir_files[i][j] == -5)
			{
			dir_fil_ct[i]=j;
			i++;			
			j=-1;}
		j++;
		
	}
	
	fr = fopen("fil_cont.txt","a+");
	i=0;
	j=0;
	while(r = fscanf(fr,"%d",&fil_cont[i][j]) != EOF)
	{	
		printf("i= %d, r = %d, %d\n",i,r,fil_cont[i][j]);
		
		if(fil_cont[i][j] == -5)
			{
			fil_cont_ct[i]=j;
			i++;			
			j=-1;}
		j++;
		
	}
	cont_ct = i;	
	
	fclose(fr);
	for(int i=0;i<file_ct;i++)
		printf("->%s\n",files[i]);
	for(int i=0;i<dir_ct;i++)
		printf("->%s\n",dirs[i]);
	for(int i=0;i<file_ct;i++){
		for(int j=0;j<dir_fil_ct[i];j++)
			printf("->%d\t",dir_files[i][j]);
	printf("\n");
}

	return fuse_main( argc, argv, &operations, NULL );
}


