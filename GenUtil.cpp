#include <iostream>
#include <fstream>


std::string loadText(const std::string &filename)
{
	std::ifstream in(filename.c_str());
	if(!in) {
		return "";
	}
	std::string source, s;
	while(std::getline(in, s))
	    source += s + "\n";
    return source;
}
