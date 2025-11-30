#pragma once
#include <string>

struct Song
{
	int id;
	std::string title;
	std::string artist;
	std::string album;
	int duration;

	Song(int id, std::string t, std::string ar, std::string al, int d) : id(id), title(t), artist(ar), album(al), duration(d) {}
	Song() : id(0) {}
};
