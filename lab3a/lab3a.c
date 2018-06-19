//NAME: Yanyin Liu,Shawye Ho


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/errno.h>
#include <unistd.h>
#include <time.h>
#include "ext2_fs.h"


#define SUPER_OFFSET 1024 
void * buffer;
struct ext2_super_block super;
int imgfd;
int num_block;
int num_inode;
int blocksize; 
int inodesize; 
unsigned int block_per_group;
unsigned int inode_per_group; 

int groupnum; 
struct ext2_group_desc* group = NULL;


char * timetoString(__u32 given_time) {
  
  int itim = (int) given_time;
  time_t tim = (time_t) itim;
  struct tm * time = gmtime(&tim);
  char* timeString = (char *) malloc(32);
  if (timeString == NULL)
    {
      fprintf(stderr, "Malloc Failed\n");
      exit(2);
    }
  
  sprintf(timeString, "%02d/%02d/%02d %02d:%02d:%02d",time->tm_mon + 1, time->tm_mday, time->tm_year % 100, time->tm_hour, time->tm_min, time->tm_sec);
  return timeString;
}
void Superblock_Summary()
{
	
	if(pread(imgfd, buffer, sizeof(struct ext2_super_block), SUPER_OFFSET) > 0)
	{
	  struct ext2_super_block * temp = (struct ext2_super_block *) buffer;
	  super = *temp;
	  num_block = super.s_blocks_count;
	  num_inode = super.s_inodes_count;
	  blocksize = 1024 << super.s_log_block_size;
	  inodesize = super.s_inode_size;
	  block_per_group = super.s_blocks_per_group;
	  inode_per_group = super.s_inodes_per_group;
	  int non_reserved_inode = super.s_first_ino;
	  
	  fprintf(stdout, "SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n", num_block, num_inode, blocksize, inodesize, block_per_group, inode_per_group, non_reserved_inode);

	}
	else
	{
		fprintf(stderr, "Read file block file return: %s\n", strerror(errno));
		exit(2);
	}
}


void Group_Summary()
{
	unsigned int rem_blocks = num_block % block_per_group;
	unsigned int rem_inodes = num_inode % inode_per_group;
	
	groupnum = 1 + (super.s_blocks_count-1) / super.s_blocks_per_group;
	int i;
	for (i=0; i<groupnum; i++)
	{
		ssize_t c = pread(imgfd, buffer, sizeof(struct ext2_group_desc), SUPER_OFFSET + 1024 + i*sizeof(struct ext2_group_desc));
		if(c<0)
		{
			fprintf(stderr, "Error on reading group: %s\n", strerror(errno));
			exit(2);
		}
		group = (struct ext2_group_desc *) buffer;
		unsigned int num_block_of_group = (i!=groupnum-1 || rem_blocks == 0) ? block_per_group : rem_blocks;
		unsigned int num_inode_of_group = (i!=groupnum-1 || rem_inodes == 0) ? inode_per_group : rem_inodes;
		fprintf(stdout, "GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n", i, num_block_of_group, num_inode_of_group, group[i].bg_free_blocks_count, group[i].bg_free_inodes_count, group[i].bg_block_bitmap, group[i].bg_inode_bitmap, group[i].bg_inode_table);
	}

}

void Free_Block_Entries()
{
	__u8 *blocks;
	blocks = malloc(blocksize);
	__u8 *inodes;
	inodes = malloc(blocksize);

	if (blocks == NULL)
	{
	  fprintf(stderr, "Error on malloc -- free blocks: %s\n", strerror(errno));
		exit(2);
	}

	if (inodes == NULL)
	{
		fprintf(stderr, "Error on malloc -- free inodes: %s\n", strerror(errno));
		exit(2);
	}

	int i;
	for (i = 0; i<groupnum; i++)
	{
		ssize_t c = pread(imgfd, blocks, blocksize, blocksize * group[i].bg_block_bitmap);
		ssize_t d = pread(imgfd, inodes, blocksize, blocksize * group[i].bg_inode_bitmap);
		if (c<0)
		{
			fprintf(stderr, "Error on reading group: %s\n", strerror(errno));
			exit(2);
		}
		if (d<0)
		{
			fprintf(stderr, "Error on reading inodes: %s\n", strerror(errno));
			exit(2);
		}
		int j;
		for (j = 0; j<blocksize; j++)
		{
			int checkbit = 1;
			__u8 checkbit_block = blocks[j];
			__u8 checkbit_inodes = inodes[j];
			int k; 
			for (k = 0; k<8; k++)
			{
				if ((checkbit & checkbit_block) == 0)
				{
					fprintf(stdout, "BFREE,%u\n", (k + 1) + (8 * j) + i * super.s_blocks_per_group);
				}
				if ((checkbit & checkbit_inodes) == 0)
				{
					fprintf(stdout, "IFREE,%u\n", (8 * j) + (k + 1) + i * super.s_blocks_per_group);
				}
				checkbit = checkbit << 1;

			}
		}
	}
	free(blocks);
	free(inodes);

}

void Dir_Indirect_Summary(struct ext2_inode * inode, unsigned int inode_num, int level_of_indirection, __u32 block_num, unsigned int size) {
  if (block_num == 0)
    return;
  int total = blocksize/sizeof(__u32); 
  __u32 buffert [1024];
  int readErr = pread(imgfd, buffert, blocksize, SUPER_OFFSET + (block_num -1)*blocksize);
  if (readErr == -1)
    {
      int err = errno;
      fprintf(stderr, "Error Reading Indirect Summary: %s\n", strerror(err));
      exit(2);
    }
  __u32 * block_ptr = (__u32 *) buffert;

  char direct[blocksize];
  struct ext2_dir_entry * direct_ptr;

  int l;
  for (l = 0; l < total; l++) {
    if (block_ptr[l] == 0)
      continue;   
    if (level_of_indirection > 1) 
      Dir_Indirect_Summary(inode, inode_num, level_of_indirection - 1, block_ptr[l], size);
    ssize_t c = pread(imgfd, &direct, blocksize, blocksize * (block_ptr[l] -1) + 1024);
    if (c<0)
      {
	fprintf(stderr, "Error on reading directory inode: %s\n", strerror(errno));
	exit(2);
      }
    
    direct_ptr = (struct ext2_dir_entry *) direct;
    
    for(;;)
      {
	if ((direct_ptr->inode == 0) || ((int)size >= blocksize)) 
	  break;
	
	int len = direct_ptr->name_len;
	char filename[len + 1]; 
	memcpy(filename, direct_ptr->name, len);
	filename[len] = '\0'; 
	if(direct_ptr != NULL) 
	  {
	    fprintf(stdout, "DIRENT,%u,%u,%u,%u,%u,'%s'\n", inode_num, size, direct_ptr->inode, direct_ptr->rec_len, direct_ptr->name_len, filename);
	  }
	size += direct_ptr->rec_len;
	direct_ptr = (void *) direct_ptr + direct_ptr->rec_len;    
      }
  }
}

void Directory_Summary(struct ext2_inode *dir_inode, unsigned int inode_num)
{
	int i;
	for(i=0; i < 15; i++)
	{
	  if (dir_inode->i_block[i] == 0)
	    continue;
	  unsigned int size = 0;
	  if(i<=11) 
		{
			char direct[blocksize];
			ssize_t c = pread(imgfd, &direct, blocksize, blocksize * (dir_inode->i_block[i] -1) + 1024);
			if (c<0)
			{
				fprintf(stderr, "Error on reading directory inode: %s\n", strerror(errno));
				exit(2);
			}
			struct ext2_dir_entry *direct_ptr = (struct ext2_dir_entry *) direct;
			for(;;)
			{
			  if ((direct_ptr->inode == 0) || ((int)size >= blocksize))
			    break;
			 
			  int len = direct_ptr->name_len;
			  char filename[len + 1];
			   memcpy(filename, direct_ptr->name, len);
			  filename[len] = '\0';
			  if(direct_ptr != NULL) 
			    {
			      fprintf(stdout, "DIRENT,%u,%u,%u,%u,%u,'%s'\n", inode_num, size, direct_ptr->inode, direct_ptr->rec_len, direct_ptr->name_len, filename);
			    }
			  
			  size += direct_ptr->rec_len;
			  direct_ptr = (void *) direct_ptr + direct_ptr->rec_len;
			}
		
		} 
	  else if (i == 12)
	    Dir_Indirect_Summary(dir_inode, inode_num, 1, dir_inode->i_block[12], size);
	  else if (i == 13)
	    Dir_Indirect_Summary(dir_inode, inode_num, 2, dir_inode->i_block[13], size);
	  else if (i == 14)
	    Dir_Indirect_Summary(dir_inode, inode_num, 3, dir_inode->i_block[14], size);
	}
}


void Indirect_Summary(unsigned int inode_num, __u32 totalprev, int level_of_indirection, __u32 block_num) {
  if (block_num == 0)
    return;
  int total = blocksize/sizeof(__u32); 
  __u32 buffert [1024];
  int readErr = pread(imgfd, buffert, blocksize, SUPER_OFFSET + (block_num -1)*blocksize);
  if (readErr == -1)
    {
      int err = errno;
      fprintf(stderr, "Error Reading Indirect Summary: %s\n", strerror(err));
      exit(2);
    }
  __u32 * block_ptr = (__u32 *) buffert; 
  int l; 
  for (l = 0; l < total; l++) {
    if (block_ptr[l] == 0)
      continue;   
    printf("INDIRECT,%u,%u,%u,%u,%u\n", inode_num, level_of_indirection, totalprev + l, block_num, block_ptr[l]);
    if (level_of_indirection > 1) //recursively dig
      Indirect_Summary(inode_num, totalprev + l, level_of_indirection - 1, block_ptr[l]);
  }
}
void Inode_Summary() {
  int i;
  for (i = 0; i < groupnum; i++) 
    {
      int grouploc = blocksize * group[i].bg_inode_table;
      void * buffa = malloc(1024); 
      if (buffa == NULL)
	{
	  fprintf(stderr, "Failed Malloc\n");
	  exit(2);
	}
      unsigned int j;
      for (j = 0; j < inode_per_group; j++) 
	{
	  int readErr = pread(imgfd, buffa, sizeof(struct ext2_inode), grouploc + (j * sizeof(struct ext2_inode)));
	  if (readErr == -1)
	    {
	      int err = errno;
	      fprintf(stderr, "Error reading image: %s\n", strerror(err));
	      exit(2);
	    }
	  struct ext2_inode * inode = (struct ext2_inode *) buffa;
	  if ((inode->i_mode != 0) && (inode->i_links_count != 0))
	    {
	      char filetype;
	      int mode = inode->i_mode;
	      int file = 0x8000;
	      int directory = 0x4000;
	      int symbolic_link = 0xA000;
	      int mode_upper = mode & 0xF000; 
	      if (mode_upper == file)
	      {
	      	filetype = 'f';
		Indirect_Summary(j + 1, 12, 1, inode->i_block[12]); 
		Indirect_Summary(j + 1, 12 + 256, 2, inode->i_block[13]);
		Indirect_Summary(j + 1, 12 + 256 + (256*256), 3, inode->i_block[14]); 
	      }
	      else if (mode_upper == directory)
	      {
	      	filetype = 'd';
	      	
	      	Directory_Summary(inode, j+1);
		Indirect_Summary(j + 1, 12, 1, inode->i_block[12]); 
		Indirect_Summary(j + 1, 12 + 256, 2, inode->i_block[13]); 
		Indirect_Summary(j + 1, 12 + 256 + (256*256), 3, inode->i_block[14]);
	      }
			
	      else if (mode_upper == symbolic_link)
		{
		  filetype = 's';
		}
			
	      else
		filetype = '?';
	      int mode_lower8 = mode & 0x0FFF; 
	      char * lastInodeChange = timetoString(inode->i_ctime); 
	      char * modificationTime = timetoString(inode->i_mtime); 
	      char * accessTime = timetoString(inode->i_atime);
		
	      if ((filetype == 'f') || (filetype == 'd')) 
		fprintf(stdout, "INODE,%d,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", j + 1, filetype, mode_lower8, inode->i_uid, inode->i_gid, inode->i_links_count, lastInodeChange, modificationTime, accessTime, inode->i_size, inode->i_blocks, inode->i_block[0], inode->i_block[1], inode->i_block[2], inode->i_block[3], inode->i_block[4], inode->i_block[5], inode->i_block[6], inode->i_block[7], inode->i_block[8], inode->i_block[9], inode->i_block[10], inode->i_block[11], inode->i_block[12], inode->i_block[13], inode->i_block[14]); 
	      else if (inode->i_size < 60)
		fprintf(stdout, "INODE,%d,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u,%u\n", j + 1, filetype, mode_lower8, inode->i_uid, inode->i_gid, inode->i_links_count, lastInodeChange, modificationTime, accessTime, inode->i_size, inode->i_blocks, inode->i_block[0]); 
	      else 
		fprintf(stderr, "Unknown type of large file (> 60 bytes)\n");
	      free(lastInodeChange);
	      free(modificationTime);
	      free(accessTime);
	    }
	}
      free(buffa);
    }
}

int main(int argc, char **argv)
{
	if(argc != 2)
	  {
	    fprintf(stderr, "Wrong number of argument!\n");
	    fprintf(stderr, "Usage: lab4b [file.img]\n");
	    exit(1);
	  }

	int arglen = strlen(argv[1]);
	if ((argv[1][arglen -1] != 'g') || (argv[1][arglen -2] != 'm') || (argv[1][arglen -3] != 'i') || (argv[1][arglen -4] != '.'))
	  {
	    fprintf(stderr, "Please use an image file (.img)\n");
	    exit(1);
	  }

	
	int opErr = open(argv[1], O_RDONLY);
	if (opErr == -1)
	  {
	    int err = errno;
	    fprintf(stderr, "Failure to open image file: %s\n", strerror(err));
	    exit(2);
	  }
	imgfd = opErr; 
	buffer = malloc(1024); 
	if (buffer == NULL)
	  {
	    int err = errno;
	    fprintf(stderr, "Error on malloc: %s\n", strerror(err));
	  }
	Superblock_Summary();
	Group_Summary();
	Free_Block_Entries();
	Inode_Summary();
	free(buffer);

	exit(0);
}
