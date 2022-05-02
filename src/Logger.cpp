#include "../include/Logger.hpp"

Logger::Logger()
{
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];

	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d_%X", &tstruct);
	for(int i = 0; buf[i]; i++)
		if (buf[i] == ':')
			buf[i] = '-';

	_file_name = "proxy_log[";
	_file_name += buf;
	_file_name += "].txt";
	std::cout << _file_name << std::endl;
}

Logger::~Logger()
{
}

void Logger::logging(std::string const &log, int log_type)
{
	std::string tmp = currentDate() + log;
	if (log_type == 0 || log_type == 2)
		std::cout << tmp << std::endl;
	
	if (log_type == 1 || log_type == 2)	
	{
		std::ofstream out(_file_name.c_str(), std::ios::app);
		if (!out) {
			std::cerr << "Error open file : " << _file_name << std::endl;
			return;
		}
		out << tmp << std::endl;
		out.close();
	}
}

std::string const Logger::currentDate()
{
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];

	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "[ %Y-%m-%d %X ]  :  ", &tstruct);
	return buf;
}
