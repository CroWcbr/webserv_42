#include "../include/utils.hpp"

std::vector<std::string> _split(std::string const &str, const char &delim)
{
	std::vector<std::string> tmp_split;

	std::stringstream ss(str);
	std::string item;
	while(std::getline(ss, item, delim))
		tmp_split.push_back(item);
	return tmp_split;
}

void signal_handler(int signal) 
{
	std::cout << "\033[93m";
	std::cout << "\rSTOP SIGNAL " << signal << " from server" << std::endl;
	std::cout << "\033[0m";
	exit(signal);
}

bool isDirectory(std::string const &path) 
{
    struct stat s;
    if (stat(path.data(), &s) == 0)
        return S_ISDIR(s.st_mode);
    return false;
}

bool isFile(std::string const &path) 
{
	struct stat s;
	if (stat(path.data(), &s) == 0)
		return S_ISREG(s.st_mode);
	return false;
}

std::string const _currentDateTime()
{
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];

	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "[ %Y-%m-%d %X ]  :  ", &tstruct);
	return buf;
}

void clearDoubleSplash(std::string &test_path)
{
	for (int i = test_path.size() - 1; i > 0; i--)
		if ((test_path[i] == '/' && test_path[i - 1] == '/') || test_path.back() == '/')
			test_path.erase(i, 1);
}
