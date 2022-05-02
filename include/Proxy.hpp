#pragma once

# include "User.hpp"
# include "../include/Logger.hpp"

# include <fcntl.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <cstring>
# include <sstream>
# include <fstream>
# include <stdlib.h>
# include <signal.h>

# include <poll.h>
# include <vector>
# include <set>

# define MAX_LISTEN 1024
# define MAX_BUFFER_RECV 1024

void _signal_handler(int signal);

class Proxy
{
private:
	typedef std::vector<struct pollfd>	pollfdType;
	pollfdType							_fds;
	Logger								_log;

	std::string							_remote_ip;
	int									_remote_port;
	sockaddr_in 						_remote_addr;

	std::string							_local_ip;
	int									_local_port;
	sockaddr_in 						_local_addr;
	int									_local_fd;

	std::map<int, User*>				_user;
	std::set<int>						_user_close;

	std::string							_request;
	std::string							_request2;

private:	
	Proxy();

	struct pollfd  				_createPollFd(int const &fd);
	void						_CheckInputInfo(int argc, char **argv);
	std::vector<std::string>	_split(std::string const &str, const char &delim);
	void						_MakeLocalAndRemoteAddr();
	void						_SocketStart();


	void	_PollInServ(pollfdType::iterator &it);
	void	_PollInUser(pollfdType::iterator &it);
	void	_PollOut(pollfdType::iterator &it);
	void	_PollElse(pollfdType::iterator &it);
	void 	_CloseConnection();

public:
	Proxy(int argc, char **argv);
	~Proxy();

	void Loop();
};