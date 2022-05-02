#include "../include/User.hpp"

User::User(int const &user_fd, sockaddr_in const &addr, sockaddr_in &remote_addr):
	_user_fd(user_fd),
	_user_addr(addr)
{
	_remote_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_remote_fd > 0 && connect(_remote_fd, reinterpret_cast<struct sockaddr*>(&remote_addr), sizeof(remote_addr)) < 0)
		_remote_fd = -1;
}

User::~User()
{}

int const	&User::GetUserFd() const 
{
	return _user_fd;
}

int const	&User::GetRemoteFd() const 
{
	return _remote_fd;
}

void User::RecvRequestUser(char const *buffer, size_t const &nbytes)
{
	_request_user.append(buffer, nbytes);
}

void User::RecvRequestServer(char const *buffer, size_t const &nbytes)
{
	_request_server.append(buffer, nbytes);
}

std::string const &User::GetRequestUser() const
{
	return _request_user;
}

std::string const &User::GetRequestServer() const
{
	return _request_server;
}

void User::UpdateRequestUser(size_t const &result)
{
	_request_user = _request_user.substr(result, _request_user.size());
}

void User::UpdateRequestServer(size_t const &result)
{
	_request_server = _request_server.substr(result, _request_server.size());
}
