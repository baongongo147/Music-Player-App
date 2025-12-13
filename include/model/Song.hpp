#pragma once
#include <iostream>
#include <string>

namespace models{
struct Song
{
	int id;
	std::string title;
	std::string artist;
	std::string album;
	// int duration;
	std::string filePath;

	Song(int id, std::string t, std::string ar, std::string al, std::string path) : id(id), title(t), artist(ar), album(al), filePath(path) {}
	Song() : id(0) {}
};
}