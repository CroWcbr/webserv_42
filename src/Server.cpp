#include "../include/Server.hpp"

Server::Server(int argc, char **argv)
{
	try
	{
		signal(SIGPIPE, SIG_IGN);
		signal(SIGINT, signal_handler);
		signal(SIGQUIT, signal_handler);
		signal(SIGTSTP, signal_handler);

		NginxConfig	nginx(argc, argv);

		_CreateHttpCodeList();
		_CreateMimeExt();

		_ServerStart(nginx);
	}
	catch (std::exception& e) 
	{
		std::cerr << "\033[91m" << e.what() << "\033[0m" << std::endl;
		exit(EXIT_FAILURE);
	}
}

void Server::_CreateHttpCodeList()
{
	_http_code_list["200"] = "OK";
	_http_code_list["201"] = "Created";
	_http_code_list["204"] = "No Content";

	_http_code_list["301"] = "Moved Permanently";	

	_http_code_list["400"] = "Bad Request";
	_http_code_list["403"] = "Forbidden";
	_http_code_list["404"] = "Not Found";
	_http_code_list["405"] = "Method Not Allowed";
	_http_code_list["411"] = "Length Required";
	_http_code_list["413"] = "Payload Too Large";
	_http_code_list["414"] = "URI Too Long";

	_http_code_list["500"] = "Internal Server Error";
	_http_code_list["502"] = "Bad Gateway";
	_http_code_list["504"] = "Gateway Timeout";
	_http_code_list["505"] = "HTTP Version Not Supported";
	_http_code_list["508"] = "Loop Detected";
}

void Server::_CreateMimeExt()
{
	std::ifstream mime_read(MIME_PATH);
	if (!mime_read.is_open())
		throw std::runtime_error(ERROR_MIME_OPEN + std::string(MIME_PATH));

	std::string buffer;
	while (true)
	{
		std::getline(mime_read, buffer);
		_ClearBuffer(buffer);
		if (!buffer.empty())
		{
			std::vector<std::string> tmp_mime = _split(buffer, ' ');
			for (int i = 1; i < tmp_mime.size(); i++)
				_mime_ext_list.insert(std::pair<std::string, std::string>("." + tmp_mime[i], tmp_mime[0]));
		}
		if (mime_read.eof())
			break ;
	}
	mime_read.close();
}

void Server::_ClearBuffer(std::string &buffer)
{
	int n = buffer.find("#");
	if (n != std::string::npos)
		buffer = buffer.substr(0, n);
	for (int i = buffer.size() - 1; i >= 0; i--)
	{
		if (isspace(buffer[i]))
			buffer[i] = ' ';
		if (isspace(buffer[i]) && (i == 0 || isspace(buffer[i - 1]) || i == buffer.size() - 1))
			buffer.erase(i, 1);
	}
	if (buffer.back() != ';')
		throw std::runtime_error(ERROR_MIME_SEMIKOLON + buffer);
	buffer.pop_back();
}

Server::~Server() {}

void Server::_ServerStart(NginxConfig const &nginx)
{
	std::cout << "\033[90m" << "++++++++++++++++++++" << "\033[0m" << std::endl;	

	for (std::vector<ServerInfo>::const_iterator it = nginx.getServ().begin(), 
			it_end = nginx.getServ().end(); it != it_end; it++)
	{
		try
		{
			ServerInfo tmp_serv = *it;
			_SocketStart(it->GetPort(), it->GetHost(), tmp_serv);

			struct pollfd tmp;
			tmp.fd = tmp_serv.GetListenFd();
			tmp.events = POLLIN;
			tmp.revents = 0;
			_fds.push_back(tmp);

			_server_info.insert(std::pair<int, ServerInfo>(tmp_serv.GetListenFd(), tmp_serv));

			std::cout << "\033[92m";
			std::cout << _currentDateTime() << SERVER_START << " on " << \
						it->GetHost() << ":" << it->GetPort() << std::endl;
			std::cout << "\033[0m";
		}
		catch (std::exception& e)
		{
			std::cout << "\033[91m";
			std::cout << _currentDateTime() << SERVER_WARNING << " on " << \
					it->GetHost() << ":" << it->GetPort() << std::endl;
			std::cout << "\t" << e.what() << std::endl;
			std::cout << "\033[0m";
		}
	}

	std::cout << "\033[90m" << "++++++++++++++++++++" << "\033[0m" << std::endl;
	if (_server_info.size() == 0)
		throw std::runtime_error(ERROR_SERVER_NOSERVER);
}

void Server::_SocketStart(int const &port, std::string const &host, ServerInfo &tmp_serv)
{
	int listen_sd;
	int tmp;

	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sd < 0)
		throw std::runtime_error(ERROR_SERVER_SOCKET);
 
 	int on = 1;
	tmp = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (tmp < 0)
	{
		close(listen_sd);
		throw std::runtime_error(ERROR_SERVER_SETSOCKOPT);
	}

	tmp = fcntl(listen_sd, F_SETFL, O_NONBLOCK);
	if (tmp < 0)
		throw std::runtime_error(ERROR_SERVER_FCNTL);

	struct	sockaddr_in serv_addr;
	std::memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(host.c_str());
	serv_addr.sin_port = htons(port);

	tmp = bind(listen_sd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr));
	if (tmp < 0)
	{	
		close(listen_sd);
		throw std::runtime_error(ERROR_SERVER_BIND);
	}

	tmp = listen(listen_sd, MAX_LISTEN);
	if (tmp < 0)
	{
		close(listen_sd);
		throw std::runtime_error(ERROR_SERVER_LISTEN);
	}
	tmp_serv.SetListenFd(listen_sd);
	tmp_serv.SetSockAddr(serv_addr);
}

void Server::Loop()
{
	while (true)
	{
		if (DEBUG == 1)
			std::cout << "Number of listenning fd : " << _fds.size() << std::endl;
		_PollWait();

		for (pollfdType::iterator it = _fds.begin(), itend = _fds.end(); it != itend; ++it)
		{
			if (it->revents == 0)
				continue;

			if (it->revents & POLLIN && _server_info.find(it->fd) != _server_info.end())
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
		}
		_CloseConnection();
		_CheckUserTimeOut();
	}
}

void Server::_PollWait()
{
	static std::string dot[10] = {"     ", ".    ", "..   ", "...  ", ".... ", ".....", ".... ", "...  ", "..   ", ".    "};
	int poll_count = 0;
	int n = 0;

	while (poll_count == 0)
	{
		std::cout << "\033[94m" << SERVER_POOL_WAIT << dot[n++] << "\033[0m" << std::flush;
		poll_count = poll(&(_fds.front()), _fds.size(), TIMEOUT);
		if (n == 10)
		{
			_CheckUserTimeOut();
			n = 0;
		}
		if (poll_count < 0)
		{
			std::cout << SERVER_POOL_WAIT_MINUS_ONE << std::endl;
			poll_count = 0;
		}
	}
	std::cout << "\033[92m" << "\r" << _currentDateTime() << "\t" << SERVER_POOL_WAIT_CONNECTION << std::endl << "\033[0m";
}

void Server::_PollInServ(pollfdType::iterator &it)
{
	if (DEBUG == 1)
		std::cout << SERVER_POOLIN_RECIEVED_SERV << it->fd << std::endl;

	it->revents = 0;
	sockaddr_in	addr;
	socklen_t addr_len = sizeof(addr);
	int user_fd = accept(it->fd, reinterpret_cast<struct sockaddr*>(&addr), &addr_len);
	if (user_fd < 0)
		std::cout << ERROR_SERVER_POOLIN_NEW_CONN << std::endl;
	else
	{
		char buffer[16];
		inet_ntop( AF_INET, &addr.sin_addr, buffer, sizeof(buffer));
		std::cout << SERVER_POOLIN_NEW_CONN << user_fd << " from : " << buffer << std::endl;
		
		User new_user(user_fd, &_server_info[it->fd], addr, &_http_code_list, &_mime_ext_list);

		struct pollfd tmp;
		tmp.fd = user_fd;
		tmp.events = POLLIN;
		tmp.revents = 0;

		if (fcntl(user_fd, F_SETFL, O_NONBLOCK) < 0)
		{
			std::cout << ERROR_SERVER_POOLIN_FCNTL << std::endl;
			close(user_fd);
			return;
		}
		_fds.push_back(tmp);
		_user.insert(std::pair<int, User>(user_fd, new_user));
	}
}

void Server::_PollInUser(pollfdType::iterator &it)
{
	if (DEBUG == 1)	
		std::cout << SERVER_POOLIN_RECIEVED_USER << it->fd << std::endl;

	it->revents = 0;
	User *itu = &_user.find(it->fd)->second;
	itu->UpdateActiveTime();

	char buffer[MAX_BUFFER_RECV];
	int nbytes = recv(it->fd, buffer, MAX_BUFFER_RECV - 1, 0);
	if (nbytes < 0)
	{
		std::cout << ERROR_SERVER_POOLIN_USER_READ << it->fd << std::endl;
		user_close.insert(it->fd);
	}
	else if (nbytes == 0)
	{
		std::cout << SERVER_POOLIN_USER_SESS_END << it->fd << std::endl;
		user_close.insert(it->fd);
	}
	else
	{		
		if (itu->RecvRequest(buffer, nbytes) == false)
			return ;
		std::cout << SERVER_POOLIN_USER_READ_END << it->fd << std::endl;
		try
		{
			itu->CheckAndParseRequest();
			itu->CreateResponse();
		}
		catch (const char* s) 
		{
			std::cout << s << " " << it->fd << std::endl;	
			itu->CreateResponseError(s);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			itu->CreateResponseError("500 Unexpected Error....");
		}
		itu->RequestPrint();
		itu->ResponsePrint();
		it->events = POLLOUT;
	}
}

void Server::_PollOut(pollfdType::iterator &it)
{
	if (DEBUG == 1)	
		std::cout << SERVER_POOLOUT_RECIEVED << it->fd << std::endl;

	it->revents = 0;
	User *itu = &_user.find(it->fd)->second;
	itu->UpdateActiveTime();
	int n = 0;
	while (true)
	{
		int result = send(it->fd, itu->GetResponse().c_str() + itu->GetResponseSendPos(), \
							itu->GetResponse().length() - itu->GetResponseSendPos(), 0);
		if (result < 0 || (result == 0 && itu->GetResponse().length() - itu->GetResponseSendPos() > 0))
			n++;
		else
		{
			itu->UpdateResponseSendPos(result);
			break;
		}
		if (n < RETRY_TO_SEND)
		{
			std::cout << ERROR_SERVER_POOLIN_USER_SEND << it->fd << std::endl;
			user_close.insert(it->fd);
			return ;
		}
	}

	if (itu->GetResponse().length() - itu->GetResponseSendPos() > 0)
		return ;

	std::cout << SERVER_POOLOUT_USER_SEND_END << it->fd << ". ";
	if (itu->GetResponseHeader().find("Connection: close") != std::string::npos )
	{
		std::cout << SERVER_POOLOUT_CLOSE << std::endl;
		user_close.insert(it->fd);
	}
	else
	{
		std::cout << SERVER_POOLOUT_NOT_CLOSE << std::endl;
		itu->ClearAll();
		it->events = POLLIN;
	}
}

void Server::_PollElse(pollfdType::iterator &it)
{
	if (DEBUG == 1)		
		std::cout << SERVER_POOLERR_RECIEVED << it->fd << " : ";

	if ((*it).revents & POLLNVAL)
		std::cout << SERVER_POLLNVAL << std::endl;
	else if ((*it).revents & POLLHUP)
		std::cout << SERVER_POLLHUP << std::endl;
	else if ((*it).revents & POLLERR)
		std::cout << SERVER_POLLERR	<< std::endl;

	user_close.insert(it->fd);
}

void Server::_CheckUserTimeOut()
{
	if (time(0) - _last_check < TIMEOUT_USER / 2)
		return ;

	std::map<int, User>::iterator it, it_end, it_tmp;
	for (it = _user.begin(), it_end = _user.end(); it != it_end; )
	{
		if (time(0) - it->second.GetActiveTime() >= TIMEOUT_USER)
		{
			it_tmp = it++;
			int del_fd = it_tmp->first;
			for (pollfdType::iterator itfd = _fds.begin(), 
				itfd_end = _fds.end(); itfd != itfd_end; itfd++)
			{
				if (itfd->fd == del_fd)
				{
					if (it_tmp->second.GetRequest().empty())
					{
						close(itfd->fd);
						_fds.erase(itfd);
						_user.erase(it_tmp);
						std::cout << "\033[91m";
						std::cout << "\ttimeout: user disconected " << del_fd << std::endl;
						std::cout << "\033[0m";
					}
					else
					{
						itfd->events = POLLOUT;
						it_tmp->second.CreateResponseError("504 ...... send to close TIMEOUT");
					}
					break;
				}
			}
		}
		else
			it++;
	}
	_last_check = time(0);
}

void Server::_CloseConnection()
{
	for (std::set<int>::iterator itfd = user_close.begin(), 
			itfd_end = user_close.end(); itfd != itfd_end; itfd++)
	{
		for (pollfdType::iterator ii = _fds.begin(), iie = _fds.end(); ii != iie; ii++)
			if (*itfd == ii->fd)
			{
				close(ii->fd); 
				_user.erase(ii->fd);
				_fds.erase(ii);
				break;
			}
	}
	user_close.clear();
}
