#ifndef UTILS_HPP
# define UTILS_HPP

# include <vector>
# include <string>
# include <sstream>
# include <iostream>
# include <sys/types.h>
# include <sys/stat.h>

std::vector<std::string>	_split(std::string const &str, const char &delim);
void						signal_handler(int signal);
bool						isFile(std::string const &path);
bool						isDirectory(std::string const &path);
std::string const 			_currentDateTime();
void						clearDoubleSplash(std::string &test_path);

#endif
