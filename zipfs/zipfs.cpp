#include "zipfs.hpp"

#include "unzip.h"

namespace zipfs
{

static bool initialized = false;
unzFile fs = NULL;

void init(const std::string& path)
{
	if(initialized)
		return;
	initialized = true;
	fs = unzOpen64(path.c_str());
	atexit(done);
}

void done()
{
	if(!initialized)
		return;
	initialized = false;
	if(fs)
	{
		unzClose(fs);
		fs = NULL;
	}
}

static const file_info* getcurrentfileinfo()
{
	unz_file_info64 info;
	char file_name[1024];
	if(unzGetCurrentFileInfo64(fs, &info, file_name, sizeof(file_name), NULL, 0, NULL, 0) != UNZ_OK)
		return NULL;
	static file_info fi;
	fi.full_name = file_name;
	fi.size = info.uncompressed_size;
	fi.is_dir = !fi.full_name.empty() && fi.full_name[fi.full_name.size() - 1] == '/';
	return &fi;
}

std::string readfile(const std::string& file_name)
{
	std::string res;
	auto* info = getcurrentfileinfo();
	if((info && info->full_name == file_name) || unzLocateFile(fs, file_name.c_str(), 1) == UNZ_OK)
	{
		if(unzOpenCurrentFile(fs) == UNZ_OK)
		{
			unz_file_info64 zip_info;
			if(unzGetCurrentFileInfo64(fs, &zip_info, NULL, 0, NULL, 0, NULL, 0) == UNZ_OK)
			{
				res.resize(zip_info.uncompressed_size, 0);
				unzReadCurrentFile(fs, (void*) res.data(), res.size());
			}
			unzCloseCurrentFile(fs);
		}
	}
	return res;
}

const file_info* gotofirstfile()
{
	if(unzGoToFirstFile(fs) == UNZ_OK)
	{
		return getcurrentfileinfo();
	}
	return NULL;
}

const file_info* gotonextfile()
{
	if(unzGoToNextFile(fs) == UNZ_OK)
	{
		return getcurrentfileinfo();    
	}
	return NULL;
}

std::string file_info::name() const
{
	auto p = full_name.rfind('/');
	return p == std::string::npos ? full_name : full_name.substr(p + 1);
}

}
//namespace zipfs
