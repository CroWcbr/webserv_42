#ifndef USER_HPP
# define USER_HPP

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

# include "utils.hpp"
# include "ServerInfo.hpp"

# define HEADER_FIELD_SIZE		100
# define HEADER_KEY_LENGTH		100
# define HEADER_VALUE_LENGTH	2048

class User
{
private:
	int									_user_fd;
	sockaddr_in							_addr;
	const ServerInfo					*_server;
	time_t								_active_time;
	std::map<std::string, std::string>	*_http_code;
	std::map<std::string, std::string>	*_mime_ext_list;

	std::string							_request;
	std::string							_request_method;
	std::string							_request_uri;
	std::string							_request_query;	
	std::string							_request_protocol;
	std::map<std::string, std::string>	_requst_header;
	std::string							_request_body;
	bool								_recv_header;
	bool								_recv_body;
	bool								_is_chunked;
	bool								_is_content_length;
	long								_content_length;

	const Location						*_response_location;
	std::string							_response;
	int									_response_send_pos;
	std::string							_response_path;
	std::string							_response_header;
	std::string							_response_body;
	bool								_response_dir;
	bool								_response_file;
	std::string							_response_ext;
	std::string							_response_header_cookie;

	std::string							_status_code;
	std::string							_content_type_cgi;	
private:
	User();

	bool				_RecvHeader();
	void				_CheckBodyLength();
	bool				_RecvBody();
	void				_ParseRequestMethod(std::string const &method_string);
	void				_ParseRequestHeader(std::string const &header_string);
	void				_RecvBodyParseChunked();
	void				_RecvBodyParseLength();
	void				_ParseRequestFindLocation(); //DO BETTER

	void				_CheckResponsePathUri();
	void				_CreateResponseReturnHeader();
	void				_ParseResponseGetBody();
	void				_ParseResponsePostBody();
	void				_ParseResponseUpload();
	void				_ParseResponsePutBody();
	void				_ParseResponseDeleteBody();

	void				_CreateResponseBodyFromFile(std::string const &body_path_to_file);
	void				_CreateResponseErrorBody(std::string const &msg_error);
	void				_CreateResponseBodyDirectory(std::string const &dir_path);

	void				_ParseResponseCGI();
	void				_CGIsetenv();
	void				_CGIParseBody();

public:
	User(int const &user_fd, \
			ServerInfo *serv, \
			sockaddr_in const &addr, \
			std::map<std::string, std::string> *http_code,
			std::map<std::string, std::string> *mime_ext_list);
	User(User const &copy);
	User &operator=(User const &copy);
	~User();

	void				UpdateActiveTime();
	time_t const		&GetActiveTime() const;
	void				ClearAll();
	void				RequestPrint() const;
	void				ResponsePrint() const;

	std::string	const 	&GetRequest() const;
	bool				RecvRequest(char const *buffer, size_t const &nbytes);
	void				CheckAndParseRequest();

	std::string	const 	&GetResponse() const;
	std::string	const 	&GetResponseHeader() const;
	int	const 			&GetResponseSendPos() const;
	void	 			UpdateResponseSendPos(int const &send);
	void				CreateResponse();
	void				CreateResponseError(std::string const &msg_error);
};

#endif
