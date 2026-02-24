#ifndef ZIPFS_HPP
#define ZIPFS_HPP

#include <string>

#pragma once

namespace zipfs
{

void init(const std::string& path);
void done();

std::string readfile(const std::string& file_name);

struct file_info
{
	std::string name() const;
	std::string full_name;
	uint64_t size = 0;
	bool is_dir = false;
};
const file_info* gotofirstfile();
const file_info* gotonextfile();

}
//namespace zipfs

#endif//ZIPFS_HPP
