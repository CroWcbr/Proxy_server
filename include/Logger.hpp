#pragma once

# include <iostream>
# include <string>
#include <fstream>

# define LOG_TYPE 2
// 0 - terminal
// 1 - log_file
// 2 - terminal + log_file

class Logger
{
private:
	std::string	_file_name;

private:
	std::string const currentDate();

public:
	Logger();
	~Logger();

	void logging(std::string const &log, 
				int log_type = LOG_TYPE);
};
