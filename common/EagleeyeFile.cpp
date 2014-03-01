#include "EagleeyeFile.h"
#include "Print.h"
#include <stdio.h>
namespace eagleeye
{
bool EAGLEEYE_CopyFile(const char* src_file,const char* target_file)
{
	FILE* from_fd = NULL;
	FILE* to_fd = NULL;
	from_fd = fopen(src_file,"rb");
	to_fd = fopen(target_file,"wb+");

	if (!from_fd)
	{
		EAGLEEYE_ERROR("open %s error\n",src_file);
		return false;
	}
	if (!to_fd)
	{
		EAGLEEYE_ERROR("open %s error\n",target_file);
		return false;
	}

	char buf[EAGLEEYE_BUFFER_SIZE];

	while (!feof(from_fd))
	{
		size_t bytes_read = fread(buf, 1, EAGLEEYE_BUFFER_SIZE, from_fd);
		fwrite(buf, 1, bytes_read, to_fd);
	}

	fclose(from_fd);
	fclose(to_fd);

	return true;
}

bool EAGLEEYE_DeleteFile(const char* file_path)
{
	if (remove(file_path))
		return true;
	else
		return false;
}

bool EAGLEEYE_RenameFile(const char* old_file_name,const char* new_file_name)
{
	if (rename(old_file_name,new_file_name))
		return true;
	else
		return false;
}

}