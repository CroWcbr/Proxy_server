#include "../include/Proxy.hpp"

Proxy::Proxy(int argc, char **argv)
{
	try
	{
		signal(SIGPIPE, SIG_IGN);
		signal(SIGINT, _signal_handler);
		signal(SIGQUIT, _signal_handler);
		signal(SIGTSTP, _signal_handler);

		_CheckInputInfo(argc, argv); // input origin

		// _local_ip = "127.0.0.1";	//	for test
		// _remote_ip = "127.0.0.1";	//	for test
		// _local_port = 2121;			//	for test
		// _remote_port = 5555;		//	for test

		_MakeLocalAndRemoteAddr();
		_SocketStart();

		_fds.push_back(_createPollFd(_local_fd));
		_log.logging("Server start");
	}
	catch (std::exception& e)
	{
		_log.logging("Server start ERROR : " + (std::string)(e.what()));
		_exit(1);
	}
}

void _signal_handler(int signal) 
{
	std::cout << "\033[93m";
	std::cout << "\rSTOP SIGNAL " << signal << " from server" << std::endl;
	std::cout << "\033[0m";
	exit(signal);
}

struct pollfd  Proxy::_createPollFd(int const &fd)
{
	struct pollfd tmp;
	tmp.fd = fd;
	tmp.events = POLLIN;
	tmp.revents = 0;
	return tmp;
}

Proxy::~Proxy()
{
	while(_user.size() > 0)
	{
		User *it = _user.begin()->second;
		close(it->GetUserFd());
		close(it->GetRemoteFd());
		_user.erase(it->GetUserFd());		
		_user.erase(it->GetRemoteFd());
		delete(it);
	}
	_log.logging("Proxy close");
}

void Proxy::_CheckInputInfo(int argc, char **argv)
{
	if (argc != 3)
		throw std::runtime_error("Error input. Need ./proxy <proxy_port> <remote_host>:<remote_port>");

	_local_ip = "127.0.0.1";
	std::string _local_port_str = argv[1];
	for (int i = 0; _local_port_str[i]; ++i)
		if (!isdigit(_local_port_str[i]))
			throw std::runtime_error("Local port is not digital");
	_local_port = std::atoi(_local_port_str.c_str());	

	std::vector<std::string> _remote_info = _split(argv[2], ':');
	if (_remote_info.size() != 2)
		throw std::runtime_error("Error input _remote_info. Need ./proxy <proxy_port> <remote_host>:<remote_port>");

	_remote_ip = _remote_info[0];
	if (_remote_ip == "localhost")
		_remote_ip = "127.0.0.1";
	else if (inet_addr(_remote_ip.c_str()) == INADDR_NONE)
		throw std::runtime_error("ERROR wrong remote ip");

	std::string _remote_port_str = _remote_info[1];
	for (int i = 0; _remote_port_str[i]; ++i)
		if (!isdigit(_remote_port_str[i]))
			throw std::runtime_error("Remote port is not digital");
	_remote_port = std::atoi(_remote_port_str.c_str());	
}

std::vector<std::string> Proxy::_split(std::string const &str, const char &delim)
{
	std::vector<std::string> tmp_split;

	std::stringstream ss(str);
	std::string item;
	while(std::getline(ss, item, delim))
		tmp_split.push_back(item);
	return tmp_split;
}

void Proxy::_MakeLocalAndRemoteAddr()
{
	memset(&_local_addr, 0, sizeof(_local_addr));
	_local_addr.sin_family = AF_INET;
	_local_addr.sin_addr.s_addr = inet_addr(_local_ip.c_str());
	_local_addr.sin_port = htons(_local_port);

	memset(&_remote_addr, 0, sizeof(_remote_addr));
	_remote_addr.sin_family = AF_INET;
	_remote_addr.sin_addr.s_addr = inet_addr(_remote_ip.c_str());
	_remote_addr.sin_port = htons(_remote_port);
}

void Proxy::_SocketStart()
{
	if ((_local_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Error socket");
	
 	int on = 1;
	if (setsockopt(_local_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		throw std::runtime_error("Error setsockopt");
	
	if (fcntl(_local_fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Error fcntl");	

	if (bind(_local_fd, reinterpret_cast<struct sockaddr*>(&_local_addr), sizeof(_local_addr)) < 0)
		throw std::runtime_error("Error bind");

	if (listen(_local_fd, MAX_LISTEN) < 0)
		throw std::runtime_error("Error listen");
}

void Proxy::Loop()
{
	while (true)
	{
		int poll_count = poll(&(_fds.front()), _fds.size(), 1000);
		if (poll_count < 0)
		{
			_log.logging("poll_count < 0");
			continue;
		}

		if (poll_count == 0)
			continue;

		for (pollfdType::iterator it = _fds.begin(), itend = _fds.end(); it != itend; ++it)
		{
			if (it->revents == 0 || _user_close.find(it->fd) != _user_close.end())
				continue;

			if (it->revents & POLLIN && it->fd == _local_fd)
			{
				_PollInServ(it);
				break;
			}
			else if (it->revents & POLLIN)
				_PollInUser(it);
			else if (it->revents & POLLOUT)
				_PollOut(it);
			else
				_PollElse(it);
			it->revents = 0;
		}
		_CloseConnection();	
	}	
}

void Proxy::_PollInServ(pollfdType::iterator &it)
{
	it->revents = 0;
	_log.logging("Poll in serv: " +  std::to_string(it->fd));

	sockaddr_in	addr;
	socklen_t addr_len = sizeof(addr);
	int user_fd = accept(it->fd, reinterpret_cast<struct sockaddr*>(&addr), &addr_len);
	if (user_fd < 0)
		_log.logging("NEW USER ERROR : ACCEPT ERROR, could not accept new connection");
	else
	{
		if (fcntl(user_fd, F_SETFL, O_NONBLOCK) < 0)
		{
			_log.logging("NEW USER ERROR : FCNTL ERROR, could not do fcntl()");
			close(user_fd);		
			return;
		}

		char buffer[16];
		inet_ntop( AF_INET, &addr.sin_addr, buffer, sizeof(buffer));
		_log.logging("NEW USER CONNECT : " + std::to_string(user_fd) + " from : " + buffer);

		User *new_user;
		if (( new_user = new User(user_fd, addr, _remote_addr)) == NULL)
		{
			close(user_fd);
			_log.logging("NEW USER ERROR : new_user malloc error");
			return ;
		}

		int remote_fd = new_user->GetRemoteFd();
		if (remote_fd < 0)
		{
			close(user_fd);
			_log.logging("NEW USER ERROR : remote_fd connect error");
			delete (new_user);
		}
		else
		{
			_fds.push_back(_createPollFd(user_fd));
			_fds.push_back(_createPollFd(remote_fd));
			_user.insert(std::pair<int, User *>(user_fd, new_user));
			_user.insert(std::pair<int, User *>(remote_fd, new_user));
			_log.logging("NEW USER fd: " + std::to_string(user_fd) + " NEW remote fd: " + std::to_string(remote_fd));		
		}
	}
}

void Proxy::_PollInUser(pollfdType::iterator &it)
{
	_log.logging("_PollInUser : " +  std::to_string(it->fd));

	User *itu = _user.find(it->fd)->second;

	char buffer[MAX_BUFFER_RECV];
	int nbytes = recv(it->fd, buffer, MAX_BUFFER_RECV - 1, 0);

	_log.logging("recv : " +  std::to_string(nbytes));	
	if (nbytes < 0)
	{
		_log.logging("Error read from : " +  std::to_string(it->fd));		
		_user_close.insert(itu->GetUserFd());
		_user_close.insert(itu->GetRemoteFd());
	}
	else if (nbytes == 0)
	{
		_log.logging("Client ended session from : " +  std::to_string(it->fd));	
		_user_close.insert(itu->GetUserFd());
		_user_close.insert(itu->GetRemoteFd());
	}
	else
	{	
		if (it->fd == itu->GetUserFd())
		{
			itu->RecvRequestUser(buffer, nbytes);
			_log.logging("RecvRequestUser : " + itu->GetRequestUser());	
		}
		else
		{
			itu->RecvRequestServer(buffer, nbytes);
			_log.logging("RecvRequestServer : " +  itu->GetRequestServer());			
		}

		it->events = POLLIN | POLLOUT;
	}
}

void Proxy::_PollOut(pollfdType::iterator &it)
{
	_log.logging("_PollOut : " +  std::to_string(it->fd));
	
	User *itu = _user.find(it->fd)->second;
	
	int result;
	if (it->fd == itu->GetUserFd())
		result = send(itu->GetRemoteFd(), itu->GetRequestUser().c_str(), itu->GetRequestUser().length(), 0);
	else
		result = send(itu->GetUserFd(), itu->GetRequestServer().c_str(), itu->GetRequestServer().length(), 0);

	if (result < 0)
	{
		_log.logging("Error send from : " +  std::to_string(it->fd));		
		_user_close.insert(itu->GetUserFd());
		_user_close.insert(itu->GetRemoteFd());
		return ;
	}

	if (it->fd == itu->GetUserFd())
	{
		itu->UpdateRequestUser(result);
		if (itu->GetRequestUser().size() == 0)
			it->events = POLLIN;
	}
	else
	{
		itu->UpdateRequestServer(result);		
		if (itu->GetRequestServer().size() == 0)
			it->events = POLLIN;
	}
}

void Proxy::_PollElse(pollfdType::iterator &it)
{
	if (it->revents & POLLNVAL)
		_log.logging("_PollElse SERVER_POLLNVAL :" +  std::to_string(it->fd));
	else if (it->revents & POLLHUP)
		_log.logging("_PollElse SERVER_POLLHUP :" +  std::to_string(it->fd));
	else if (it->revents & POLLERR)
		_log.logging("_PollElse SERVER_POLLERR :" +  std::to_string(it->fd));

	User *itu = _user.find(it->fd)->second;
	_user_close.insert(itu->GetUserFd());
	_user_close.insert(itu->GetRemoteFd());
}

void Proxy::_CloseConnection()
{
	while(_user_close.size())
	{			
		User *itu = _user.find(*_user_close.begin())->second;
		int fd_user = itu->GetUserFd();
		int fd_remote = itu->GetRemoteFd();				
		for (pollfdType::iterator ii = _fds.begin(), iie = _fds.end(); ii != iie; ii++)
			if (ii->fd == fd_user)
			{
				_fds.erase(ii);
				_log.logging("Close connect : " +  std::to_string(fd_user));
				break;
			}
		for (pollfdType::iterator ii = _fds.begin(), iie = _fds.end(); ii != iie; ii++)
			if (ii->fd == fd_remote)
			{
				_fds.erase(ii);
				_log.logging("Close connect : " +  std::to_string(fd_remote));
				break;
			}
		_user.erase(fd_user);
		_user.erase(fd_remote);
		_user_close.erase(fd_user);
		_user_close.erase(fd_remote);		
		free(itu);
		close(fd_user);
		close(fd_remote);
	}	
}
