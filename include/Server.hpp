#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <vector>
# include <string>
# include <poll.h>
# include <fcntl.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <cstring>
# include <signal.h>

# include "NginxConfig.hpp"
# include "ServerInfo.hpp"
# include "User.hpp"
# include "utils.hpp"

# define DEBUG 1

# define MIME_PATH			"./config/mime.conf"
# define MAX_BUFFER_RECV	65535	// max possible read
# define MAX_LISTEN			1024	// number of listen
# define TIMEOUT 			100		// milisecond
# define TIMEOUT_USER 		30		// second
# define RETRY_TO_SEND 		5		// try to send times

# define ERROR_MIME_OPEN					"MIME ERROR: No file in : "
# define ERROR_MIME_SEMIKOLON				"MIME ERROR: No semikolon at the end : "

# define ERROR_SERVER_SOCKET				"SERVER ERROR: socket() failed"
# define ERROR_SERVER_SETSOCKOPT			"SERVER ERROR: setsockopt() failed"
# define ERROR_SERVER_FCNTL					"SERVER ERROR: fcntl() failed"
# define ERROR_SERVER_BIND					"SERVER ERROR: binding connection. Socket has already been establishng"
# define ERROR_SERVER_LISTEN				"SERVER ERROR: Listen on web port through socket file descriptor failed"
# define ERROR_SERVER_POLL					"SERVER ERROR: POLL error"
# define ERROR_SERVER_NOSERVER				"SERVER ERROR: NO SERVER"
# define SERVER_START						"!!!SERVER START!!!"
# define SERVER_WARNING						"!!!SERVER DONT START!!!"

# define SERVER_POOL_WAIT					"\rWaiting connection"
# define SERVER_POOL_WAIT_MINUS_ONE			"\rPOLL ERROR: poll = -1  "
# define SERVER_POOL_WAIT_CONNECTION		"Recieved connection"
# define SERVER_POOLIN_RECIEVED_SERV		"_PollInServ : "
# define SERVER_POOLIN_RECIEVED_USER		"_PollInUser : "
# define SERVER_POOLOUT_RECIEVED			"_PollOut"
# define SERVER_POOLERR_RECIEVED			"_PollElse : "

# define ERROR_SERVER_POOLIN_NEW_CONN		"\tACCEPT ERROR: could not accept new connection"
# define ERROR_SERVER_POOLIN_FCNTL			"\tFCNTL ERROR: could not do fcntl()"
# define SERVER_POOLIN_NEW_CONN				"\tNew incoming connection on fd : "

# define ERROR_SERVER_POOLIN_USER_READ		"\tError read from : "
# define SERVER_POOLIN_USER_SESS_END		"\tClient ended session from : "
# define SERVER_POOLIN_USER_READ_END		"\tOK Reading ended from : "

# define ERROR_SERVER_POOLIN_USER_SEND		"\tError send from : "
# define SERVER_POOLOUT_USER_SEND_END		"\tOK Sending ended from : "
# define SERVER_POOLOUT_CLOSE				"\tConnection CLOSE."
# define SERVER_POOLOUT_NOT_CLOSE			"\tConnection NOT close."

# define SERVER_POLLNVAL					"Socket was not created"
# define SERVER_POLLHUP						"Socket broken connection"
# define SERVER_POLLERR						"Error occurred with socket"

class Server
{
private:
	typedef std::vector<struct pollfd>	pollfdType;
	pollfdType							_fds;

	std::map<int, ServerInfo>			_server_info;
	std::map<int, User>					_user;
	std::map<std::string, std::string>	_http_code_list;
	std::map<std::string, std::string>	_mime_ext_list;

	time_t								_last_check;
	std::set<int>						user_close;

private:
	Server();
	Server(Server const &copy);
	Server &operator=(Server const &copy);
	
	void			_ServerStart(NginxConfig const &nginx);
	void			_SocketStart(int const &port, \
								std::string const &host, \
								ServerInfo &tmp_serv);
	void			_CreateHttpCodeList();
	void			_CreateMimeExt();
	void			_ClearBuffer(std::string &buffer);

	void			_PollWait();
	void			_PollInServ(pollfdType::iterator &it);
	void			_PollInUser(pollfdType::iterator &it);
	void			_PollOut(pollfdType::iterator &it);
	void			_PollElse(pollfdType::iterator &it);

	void			_CloseConnection();
	void			_CheckUserTimeOut();

public:
	Server(int argc, char **argv);
	~Server();

	void			Loop();
};

#endif
