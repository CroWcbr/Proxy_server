#pragma once

# include <string>
# include <fstream>
# include <vector>
# include <map>
# include <iostream>
# include <netinet/ip.h>
# include <dirent.h>
# include <sys/stat.h>
# include <unistd.h>
# include <arpa/inet.h>

class User
{
private:
	int									_user_fd;
	sockaddr_in							_user_addr;

	int									_remote_fd;

	std::string							_request_user;
	std::string							_request_server;

private:
	User();

public:
	User(int const &user_fd,
			sockaddr_in const &addr,
			sockaddr_in &remote_addr);
	~User();

	int	const			&GetUserFd() const;
	int const			&GetRemoteFd() const;

	void				RecvRequestUser(char const *buffer, size_t const &nbytes);
	void				RecvRequestServer(char const *buffer, size_t const &nbytes);

	std::string const	&GetRequestUser() const;
	std::string const	&GetRequestServer() const;

	void				UpdateRequestUser(size_t const &result);
	void 				UpdateRequestServer(size_t const &result);
};
