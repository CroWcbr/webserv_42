#include "../include/User.hpp"

User::User(int const &user_fd, \
			ServerInfo *serv, \
			sockaddr_in const &addr, \
			std::map<std::string, std::string> *http_code, \
			std::map<std::string, std::string> *mime_ext_list)
{
	_user_fd = user_fd;
	_addr = addr;
	_server = serv;
	_active_time = time(0);
	_http_code = http_code;
	_mime_ext_list = mime_ext_list;

	_recv_header = false;
	_recv_body = false;
	_is_chunked = false;
	_is_content_length = false;
	_content_length = 0;

	_response_location = NULL;
	_response_send_pos = 0;
	_response_dir = false;
	_response_file = false;
}

User::User(User const &copy) 
{
	*this = copy;
}

User &User::operator=(User const &copy)
{
	_user_fd = copy._user_fd;
	_addr = copy._addr;
	_server = copy._server;
	_active_time = copy._active_time;
	_http_code = copy._http_code;
	_mime_ext_list = copy._mime_ext_list;

	_request = copy._request;
	_request_method = copy._request_method;
	_request_uri = copy._request_uri;
	_request_query = copy._request_query;	
	_request_protocol = copy._request_protocol;
	_requst_header = copy._requst_header;
	_request_body = copy._request_body;
	_recv_header = copy._recv_header;
	_recv_body = copy._recv_body;
	_is_chunked = copy._is_chunked;
	_is_content_length = copy._is_content_length;
	_content_length = copy._content_length;

	_response_location = copy._response_location;
	_response = copy._response;
	_response_send_pos = copy._response_send_pos;
	_response_path = copy._response_path;
	_response_header = copy._response_header;
	_response_body = copy._response_body;
	_response_dir = copy._response_dir;
	_response_file = copy._response_file;
	_response_ext = copy._response_ext;
	_response_header_cookie = copy._response_header_cookie;

	_status_code = copy._status_code;
	_content_type_cgi = copy._content_type_cgi;
	return *this;
}

User::~User() {}

void User::UpdateActiveTime() 
{
	_active_time = time(0);
}

time_t const &User::GetActiveTime() const 
{
	return _active_time;
}

void User::ClearAll()
{
	_request.clear();
	_request_method.clear();
	_request_uri.clear();
	_request_query.clear();	
	_request_protocol.clear();
	_requst_header.clear();
	_request_body.clear();

	_recv_header = false;
	_recv_body = false;
	_is_chunked = false;
	_is_content_length = false;
	_content_length = 0;

	_response_location = NULL;
	_response.clear();
	_response_send_pos = 0;
	_response_path.clear();
	_response_header.clear();
	_response_body.clear();

	_response_dir = false;
	_response_file = false;
	_response_ext.clear();
	_response_header_cookie.clear();

	_status_code.clear();
	_content_type_cgi.clear();
}

void User::RequestPrint() const
{
	std::cout << "\033[90m";
	std::cout << "++++++++++++++++++++" << std::endl;
	std::cout << "\033[92m";
	std::cout << "REQEST from " << _user_fd << " with lenght : " << _request.length() << std::endl;
	std::cout << "\033[96m";

	std::cout << "\tMethod : " << _request_method << std::endl; 
	std::cout << "\tUri : " << _request_uri << std::endl;
	std::cout << "\tQuery : " << _request_query << std::endl;	
	std::cout << "\tProtocol : " << _request_protocol << std::endl;

	std::cout << "\033[93m";
	for (std::map<std::string, std::string>::const_iterator it = _requst_header.begin(), \
			it_end = _requst_header.end(); it != it_end; it++)
		std::cout << "\t" << it->first << " : " << it->second << std::endl;

	std::cout << "\033[94m";
	std::cout << "\tBody : " << _request_body.length() << std::endl;
	if (_request_body.length() > 1000)
		std::cout << _request_body.substr(0, 1000) << std::endl;
	else
		std::cout << _request_body << std::endl;

	std::cout << "\033[90m";
	std::cout << "++++++++++++++++++++" << std::endl;
	std::cout << "\033[0m";
}

void User::ResponsePrint() const
{
	std::cout << "\033[90m";
	std::cout << "++++++++++++++++++++" << std::endl;
	std::cout << "\033[92m";
	std::cout << "RESPONSE from " << _user_fd << " with lenght : " << _response.length() << std::endl;
	std::cout << "\033[96m";
	std::cout << _response_header;

	std::cout << "\033[92m";
	std::cout << "BODY : " << _response_body.length() << std::endl;
	std::cout << "\033[96m";
	if (_response_body.length() > 1000)
		std::cout << _response_body.substr(0, 1000) << std::endl;
	else
		std::cout << _response_body << std::endl;

	std::cout << "\033[90m";
	std::cout << "++++++++++++++++++++" << std::endl;
	std::cout << "\033[0m";	
}

std::string	const &User::GetRequest() const { return _request; }

bool User::RecvRequest(char const *buffer, size_t const &nbytes)
{

	_request.append(buffer, nbytes);
	
	if (_recv_header == false && _RecvHeader() == false)
		return false;

	if (_recv_body == false && _RecvBody() == false)
		return false;

	return true;
}

bool User::_RecvHeader()
{
	if(_request.find("\r\n\r\n", 0) != std::string::npos)
	{
		_recv_header = true;
		_CheckBodyLength();
		return true;
	}
	return false;
}

void User::_CheckBodyLength()
{
	if (_request.find("Transfer-Encoding: chunked", 0) != std::string::npos)
		_is_chunked = true;
	else if (_request.find("Content-Length: ", 0) != std::string::npos)
	{
		_is_content_length = true;
		std::string len_type = "Content-Length: ";
		int len_start = _request.find(len_type) + len_type.length();
		int len_end = _request.find("\r\n", len_start);
		_content_length = std::atoi(_request.substr(len_start, len_end - len_start).c_str());
	}
}

bool User::_RecvBody()
{
	if (_is_chunked && _request.find("0\r\n\r\n", 0) == std::string::npos)
		return false;
	else if (_is_content_length && _request.size() < _request.find("\r\n\r\n", 0) + 4 + _content_length)
		return false;
	_recv_body = true;
	return true;
}

void User::CheckAndParseRequest()
{
	std::string new_row = "\r\n";
	int len_new_row = new_row.length();
	int pars_position = 0;
	int header_len = _request.find("\r\n\r\n");

	while (pars_position < header_len)
	{
		int find_pos = _request.find(new_row, pars_position);
		std::string msg = _request.substr(pars_position, find_pos - pars_position);
		if (_request_method.empty())
			_ParseRequestMethod(msg);
		else if (!msg.empty())
			_ParseRequestHeader(msg);
		pars_position = find_pos + len_new_row;
	}
	if (_is_chunked)
		_RecvBodyParseChunked();
	else if  (_is_content_length)
		_RecvBodyParseLength();

	_ParseRequestFindLocation();
	if (_response_location->GetClientMaxBodySize() != 0 && \
		_response_location->GetClientMaxBodySize() < _request_body.size())
	throw "413 REQUEST ERROR! Size BODY too big";
}

void User::_ParseRequestMethod(std::string const &method_string)
{
	std::vector<std::string> msg = _split(method_string, ' ');

	if (msg.size() != 3)
	{
		_request_protocol = "HTTP/1.1";
		throw "400 REQUEST ERROR! Wrong first line in reqest";
	}

	_request_method = msg[0];
	int find_path_delim = msg[1].find("?");
	if (find_path_delim == std::string::npos)
	{
		_request_uri = msg[1];
		_request_query = "";		
	}
	else
	{
		_request_uri = msg[1].substr(0, find_path_delim);
		_request_query = msg[1].substr(find_path_delim + 1);
	}
	_request_protocol = msg[2];

	if (msg[0] != "GET" && msg[0] != "POST" && msg[0] != "PUT" && msg[0] != "DELETE")
		throw "405 REQUEST ERROR! Wrong method in reqest";
	if (msg[1].length() > 2048)
		throw "414 REQEST ERROR! Wrong URI length";
	if (msg[2] != "HTTP/1.1" && msg[2] != "HTTP/1.0")
		throw "505 REQUEST ERROR! Wrong protocol in reqest";
}

void User::_ParseRequestHeader(std::string const &header_string)
{
	std::string delimeter= ": ";
	int find_delim = header_string.find(delimeter, 0);
	if (find_delim == std::string::npos)
		throw "400 REQUEST ERROR! Header mistake, no delimeter ':'";

	int value_start = find_delim + delimeter.length();
	std::string key = header_string.substr(0, find_delim);
	if (key.size() > HEADER_KEY_LENGTH)
		throw "400 REQUEST ERROR! Header key too big";
	std::string value = header_string.substr(value_start, header_string.size() - value_start);
	if (value.size() > HEADER_VALUE_LENGTH)
		throw "400 REQUEST ERROR! Header value too big";
	_requst_header.insert(std::pair<std::string, std::string>(key, value));
	if (_requst_header.size() > HEADER_FIELD_SIZE)
		throw "400 REQUEST ERROR! Header fields too much";
}

void User::_RecvBodyParseChunked()
{
	std::string new_row = "\r\n";
	int len_new_row = new_row.length();
	int pars_position = _request.find("\r\n\r\n") + 4;
	int header_len = _request.find("\r\n0\r\n\r\n");

	while (pars_position < header_len)
	{
		int find_pos = _request.find(new_row, pars_position);
		int msg_len= std::atoi(_request.substr(pars_position, find_pos - pars_position).c_str());
		pars_position = find_pos + len_new_row;
		find_pos = _request.find(new_row, pars_position);
		std::string msg  = _request.substr(pars_position, find_pos - pars_position).c_str();
		_request_body += msg;
		pars_position = find_pos + len_new_row;
	}
	_content_length = _request_body.size();
}

void User::_RecvBodyParseLength()
{
	int _body_start = _request.find("\r\n\r\n", 0) + 4;
	if (_request.size() > _body_start + _content_length + 2)
		throw "400 REQUEST ERROR! RECV size > then MUST BE";
	_request_body = _request.substr(_body_start, _content_length);
}

void User::_ParseRequestFindLocation()
{
	std::map<std::string, Location>::const_iterator it_loc;
	std::string tmp_path = _request_uri;
	int n = 0;

	while (true)
	{
		it_loc = _server->GetLocation().find(tmp_path);
		if (it_loc != _server->GetLocation().end())
		{
			if (!it_loc->second.GetReturn().empty())
			{
				if (_server->GetLocation().find(it_loc->second.GetReturn()) != _server->GetLocation().end())
				{
					it_loc = _server->GetLocation().find(it_loc->second.GetReturn());
					_request_uri = _request_uri.substr(tmp_path.length());
					_request_uri = it_loc->second.GetPath() + "/" + _request_uri;
					clearDoubleSplash(_request_uri);
					tmp_path = _request_uri;
					n++;
					if (n == 5)
						throw "508 loop return in config";
					continue ;
				}
			}
			break;
		}

		if (tmp_path == "/")
			throw "404 ..... path not found _ParseRequestFindLocation : ";

		tmp_path = tmp_path.substr(0, tmp_path.rfind('/'));
		if (tmp_path.empty())
			tmp_path = "/";
	}
	_response_location = &(it_loc->second);
}

std::string	const &User::GetResponse() const { return _response; }

std::string	const &User::GetResponseHeader() const { return _response_header; }

int	const &User::GetResponseSendPos() const { return _response_send_pos; }

void User::UpdateResponseSendPos(int const &send) { _response_send_pos += send; }

void User::CreateResponse()
{
	if (!_response_location->GetReturn().empty())
		_CreateResponseReturnHeader();
	else
	{
		_CheckResponsePathUri();
		if (_request_method == "GET")
			_ParseResponseGetBody();
		else if (_request_method == "POST")
			_ParseResponsePostBody();
		else if (_request_method == "DELETE")
			_ParseResponseDeleteBody();
		else if (_request_method == "PUT")
			_ParseResponsePutBody();
		else
			throw "405 Not Allow11111";

		_response_header += _request_protocol + " " + _status_code + " " + _http_code->find(_status_code)->second + "\r\n";
	}

	if (_content_type_cgi != "")
		_response_header += _content_type_cgi + "\r\n";
	else if (_mime_ext_list->find(_response_ext) != _mime_ext_list->end())
		_response_header += "Content-Type: " + _mime_ext_list->find(_response_ext)->second + "\r\n";
	else 
		_response_header += "Content-Type: text/html; charset=utf-8\r\n";

	if (!_response_header_cookie.empty())
		_response_header += _response_header_cookie + "\r\n";

	_response_header += "Version: " + _request_protocol + "\r\n";
	_response_header += "Connection: keep-alive\r\n";
	_response_header += "Keep-Alive: timeout=10\r\n";
	_response_header += "Content-Length: " + std::to_string(_response_body.length());
	_response_header += "\r\n\r\n";
	_response = _response_header + _response_body;	
}

void User::_CheckResponsePathUri()
{
	_response_path = _server->GetRoot() + "/" + _response_location->GetRoot() + "/" + _request_uri;
	clearDoubleSplash(_response_path);

	if (isDirectory(_response_path))
	{
		_response_path += "/";
		_response_dir = true;
	}
	if (isFile(_response_path))
		_response_file = true;
}

void User::_CreateResponseReturnHeader()
{
	_status_code = "301";
	_response_header += _request_protocol + " " + _status_code + " " + _http_code->find(_status_code)->second + "\r\n";
	_response_header += "Location:" + _response_location->GetReturn() + "\r\n";
}

void User::_ParseResponseGetBody()
{
	if (_response_dir == true && _response_location->GetIndex() != "" && 
			isFile(_response_path + _response_location->GetIndex()))
	{
		_response_path += _response_location->GetIndex();
		_response_dir = false;
		_response_file = true;
	}
	if (_response_file == true && \
			_response_path.rfind('.') != std::string::npos && \
			_response_path.rfind('.') > _response_path.rfind('/'))
		_response_ext = _response_path.substr(_response_path.rfind('.'));	

	if (_response_location->GetAllowMethod().find(_request_method) == _response_location->GetAllowMethod().end())
		throw "405 Not Allow _ParseResponseGetBody";
	else if (_response_dir == true && _response_location->GetAutoindex() == "on")
		_CreateResponseBodyDirectory(_response_path);
	else if (_response_file == true)
		_CreateResponseBodyFromFile(_response_path);
	else
		throw "404 ..... path not found _ParseResponseGetBody : ";
}

void User::_ParseResponsePostBody()
{
	std::string tmp_ext = "";
	if (_request_uri.rfind('.') != std::string::npos && \
			_request_uri.rfind('.') > _request_uri.rfind('/'))
		tmp_ext = _request_uri.substr(_request_uri.rfind('.'));	

	if (tmp_ext != "" && _response_location->GetCgiExt().find(tmp_ext) != _response_location->GetCgiExt().end())
	{
		_response_path = _server->GetRoot() + _response_location->GetRoot() + _request_uri;
		_response_ext = tmp_ext;
		_ParseResponseCGI();
	}
	else
	{		
		if (_response_file == true && _response_path.rfind('.') != std::string::npos && \
				_response_path.rfind('.') > _response_path.rfind('/'))
			_response_ext = _response_path.substr(_response_path.rfind('.'));

		if (_response_file == true && _response_ext != "" && _response_location && \
				 _response_location->GetCgiExt().find(_response_ext) != _response_location->GetCgiExt().end())
			_ParseResponseCGI();
		else if (_response_location->GetAllowMethod().find(_request_method) == _response_location->GetAllowMethod().end())
			throw "405 Not Allow _ParseResponsePostBody";
		else if (_response_location->GetUploadEnable() == "on")
			_ParseResponseUpload();
		else if (_response_file == true)
			_CreateResponseBodyFromFile(_response_path);
		else
			_ParseResponseCGI();
	}
}

void User::_ParseResponseUpload()
{
	std::string upload_dir = _server->GetRoot() + _response_location->GetUploadPath();
	if (!isDirectory(upload_dir))
		throw "500 no dir for upload in POST";

	std::string boundary;
	std::string boundary_end;
	std::map<std::string, std::string>::iterator it_content = _requst_header.find("Content-Type");
	if (it_content != _requst_header.end() && 
		it_content->second.find("multipart/form-data") != std::string::npos)
	{
		if (it_content->second.find("boundary") == std::string::npos)
			throw "400 REQUEST ERROR! multipart/form-data have no boundery";
		std::string name_boundary = "boundary=";
		int name_boundary_len = name_boundary.length();
		int position_bounery_start = it_content->second.find(name_boundary) + name_boundary_len;
		int position_bounery_end = it_content->second.find("\r\n", position_bounery_start);
		boundary = "--" + it_content->second.substr(position_bounery_start, position_bounery_end - position_bounery_start);
		boundary_end = boundary + "--";
	}
	else 
		throw "500 REQUEST ERROR! wrong form for upload";

	std::string file_name;
	std::string path_file;
	if (_request_body.find("filename=", 0) == std::string::npos)
		throw "400 REQUEST ERROR! no File name in request";
	else
	{
		std::string tmp_name = "filename=\"";
		int tmp_name_len = tmp_name.length();
		int position_filename_start = _request_body.find(tmp_name) + tmp_name_len;
		int position_filename_end = _request_body.find("\"", position_filename_start);
		file_name = _request_body.substr(position_filename_start, position_filename_end - position_filename_start);
		path_file = upload_dir + "/" + file_name;
 	}

	_request_body = _request_body.substr(_request_body.find("\r\n\r\n") + 4);
	_request_body = _request_body.substr(0, _request_body.find(boundary));

	std::ofstream tmp_file(path_file);
	if (!tmp_file.is_open())
		throw "500 cannot creat file in POST";
	tmp_file << _request_body;
	tmp_file.close();
	_CreateResponseBodyDirectory(upload_dir);
	_status_code = "200";
}

void User::_ParseResponsePutBody()
{
	if (_response_location->GetAllowMethod().find(_request_method) == _response_location->GetAllowMethod().end())
		throw "405 Not Allow _ParseResponsePutBody";
	if (_response_location->GetUploadEnable() == "off")
		throw "400 not allow to upload";

	if (_request_uri == "/")
		throw "400 no file in PUT"; 
	std::string file_name = _request_uri.substr(_request_uri.rfind("/"));

	std::string upload_dir = _server->GetRoot() + _response_location->GetUploadPath();
	if (!isDirectory(upload_dir))
		throw "500 no dir for upload in PUT";

	std::string path_file = upload_dir + file_name;
	if (isFile(path_file))
		_status_code = "204";
	else
		_status_code = "201";
	std::ofstream tmp_file(path_file);
	if (!tmp_file.is_open())
		throw "500 cannot creat file in PUT"; 
	tmp_file << _request_body;
	tmp_file.close();
}

void User::_ParseResponseDeleteBody()
{
	if (_response_location->GetAllowMethod().find(_request_method) == _response_location->GetAllowMethod().end())
		throw "405 Not Allow11111";
	else if (_response_dir == false && _response_file == false)
		throw "404 ..... path not found _ParseResponseDeleteBody : ";
	else if (access(_response_path.c_str(), W_OK) != 0)
		throw "403 no access";
	else if (std::remove(_response_path.c_str()) != 0)
		throw "500 .... Delete Error";		

	_response_body += "<html>\r\n<body>\r\n<h1>File deleted.</h1>\r\n</body>\r\n</html>\r\n";

	_status_code = "200";
}

void User::CreateResponseError(std::string const &msg_error)
{
	std::string num_error = msg_error.substr(0, msg_error.find(" "));		
	_CreateResponseErrorBody(msg_error);

	_response_header += _request_protocol + " " + num_error + " " + _http_code->find(num_error)->second + "\r\n";
	_response_header += "Version: " + _request_protocol + "\r\n";
	_response_header += "Content-Type: text/html; charset=utf-8\r\n";
	_response_header += "Connection: keep-alive\r\n";
	_response_header += "Keep-Alive: timeout=5\r\n";
	_response_header += "Content-Length: " + std::to_string(_response_body.length());
	_response_header += "\r\n\r\n";
	_response =  _response_header + _response_body;
}

void User::_CreateResponseErrorBody(std::string const &msg_error)
{
	int error = std::atoi(msg_error.substr(0, msg_error.find(" ")).c_str());
	std::map<int, std::string>::const_iterator it_error = _server->GetErrorPage().find(error);

	if (it_error != _server->GetErrorPage().end())
		_CreateResponseBodyFromFile(it_error->second);
	else
		_response_body = msg_error;
}

void User::_CreateResponseBodyFromFile(std::string const &body_path_to_file)
{
	if (_response_location && _response_ext != "" && \
			_response_location->GetCgiExt().find(_response_ext) != _response_location->GetCgiExt().end())
		_ParseResponseCGI();
	else
	{
		std::string buffer;
		std::ifstream open_file(body_path_to_file);
		if (!open_file.is_open())
			throw "500 ...... File Body Cannot Open";	
		while(std::getline(open_file, buffer))
			_response_body += buffer + "\n";
		open_file.close();
		_status_code = "200";
	}
}

void User::_CreateResponseBodyDirectory(std::string const &path)
{
	DIR				*dp;
	struct dirent	*di_struct;
	struct stat		file_stats;
	std::string		tmp_uri = _request_uri;
	
	if (tmp_uri.back() != '/')
		tmp_uri += "/";


	_response_body += "<h1> PATH : " + tmp_uri + "</h1>";
	_response_body += "<table>";
	_response_body += "<tr> <th>File name</th> <th>File size</th> </tr>";
	if ((dp = opendir(path.data())) != NULL) 
	{
		while ((di_struct = readdir(dp)) != nullptr) 
		{
			std::string tmp_path = path + "/" + di_struct->d_name;
			stat(tmp_path.data(), &file_stats);

			_response_body += "<tr>";

			_response_body += "<td><a href=\"" + tmp_uri;
			_response_body += di_struct->d_name;
			if (S_ISDIR(file_stats.st_mode))
				_response_body += "/";
			_response_body += "\">" + std::string(di_struct->d_name) + "</a></td>";
			
			_response_body += "<td>";
			if (S_ISDIR(file_stats.st_mode))
				_response_body += " dir ";
			else
			{
				std::string size = std::to_string(static_cast<float>(file_stats.st_size) / 1000);
				size = size.substr(0, size.size() - 3);
				_response_body +=  size + "Kb";
			}
			_response_body += "</td>";

            _response_body += "</tr>";
        }
        closedir(dp);
    }
    _response_body += "</table>";
}


void User::_ParseResponseCGI()
{
    int input = dup(STDIN_FILENO);
    int output = dup(STDOUT_FILENO);

    FILE *fsInput = tmpfile();
    FILE *fsOutput = tmpfile();

    int fdInput = fileno(fsInput);
    int fdOutput = fileno(fsOutput);

    write(fdInput, _request_body.c_str(), _request_body.size());
    lseek(fdInput, 0, SEEK_SET);

	pid_t pid = fork();
	if (pid == -1)
		throw "502 Could not create process in CgiHandler";	
	if (pid == 0)
	{
		extern char **environ;
		_CGIsetenv();

		char const	*cgi_info[3];
		cgi_info[0] = _response_location->GetCgiPath().c_str();
		cgi_info[1] = _response_path.c_str();
		cgi_info[2] = NULL;

		dup2(fdInput, STDIN_FILENO);
		dup2(fdOutput, STDOUT_FILENO);
		if (execve(cgi_info[0], (char *const *)cgi_info, environ) == -1)
			exit(1);
	}
	int status;
	if (waitpid(pid, &status, 0) == -1)
		throw "500 ... waitpid error";
	if (WIFEXITED(status) && WEXITSTATUS(status))
		throw "502 ... cgi status error";

	lseek(fdOutput, 0, SEEK_SET);
	int size_buf = 65535;
	char buffer[size_buf];
	int ret;
	while ((ret = read(fdOutput, buffer, size_buf - 1)) != 0) 
	{
		if (ret == -1) 
			throw "500 .... Error read from tmpfile in CGI";
		_response_body.append(buffer, ret);
	}

	dup2(STDIN_FILENO, input);
	dup2(STDOUT_FILENO, output);
	close(input);
	close(output);
	close(fdInput);
	close(fdOutput);
	fclose(fsInput);
	fclose(fsOutput);
	_CGIParseBody();
}

void User::_CGIsetenv()
{
	setenv("CONTENT_LENGTH", std::to_string(_content_length).c_str(), 1);
	setenv("CONTENT_TYPE", "text/html", 1);
	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	setenv("PATH_INFO", _request_uri.c_str(), 1);
	setenv("REQUEST_URI", _request_uri.c_str(), 1);
	setenv("QUERY_STRING", _request_query.c_str(), 1);
	setenv("REQUEST_METHOD", _request_method.c_str(), 1);
	char buffer[16];
	inet_ntop( AF_INET, &_addr.sin_addr, buffer, sizeof(buffer));
	setenv("REMOTE_ADDR", buffer, 1);
	setenv("SCRIPT_NAME", _response_location->GetCgiPath().c_str(), 1);
	setenv("SERVER_NAME", "webserv", 1);
	setenv("SERVER_PORT", std::to_string(_server->GetPort()).c_str(), 1);
	setenv("SERVER_PROTOCOL", _request_protocol.c_str(), 1);
	setenv("SERVER_SOFTWARE", "WebServ/21.0", 1);
	setenv("AUTH_TYPE", "", 1);
	setenv("PATH_TRANSLATED", _response_path.c_str(), 1);
	setenv("REMOTE_IDENT", "", 1);
	setenv("REMOTE_USER", "", 1);
	setenv("REDIRECT_STATUS", "200", 1);

	for (std::map<std::string, std::string>::iterator it = _requst_header.begin(), \
		it_end = _requst_header.end(); it != it_end; it++)
	{
		std::string tmp = it->first;
		for (int i = 0; i < tmp.size(); i++)
			tmp[i] = toupper(tmp[i]);
		setenv(("HTTP_" + tmp).c_str(), it->second.c_str(), 1);
	}
}

void User::_CGIParseBody()
{
	size_t end;
	while ((end = _response_body.find("\r\n")) != std::string::npos)
	{
		std::vector<std::string> cgi_body_split= _split(_response_body.substr(0, end), ' ');	
		if (!cgi_body_split.empty() && cgi_body_split[0] == "Status:")
		{
			if (cgi_body_split[1] != "200")
			{
				if (_http_code->find(cgi_body_split[1]) == _http_code->end())
					throw "500 .... No error code find in CGI";	
				std::string error = cgi_body_split[1] + " Error from CGI";
				throw error.c_str();
			}
		}
		else if (!cgi_body_split.empty() && cgi_body_split[0] == "Content-Type:")
			_content_type_cgi = _response_body.substr(0, end);
		else if (!cgi_body_split.empty() && cgi_body_split[0] == "Set-Cookie:")
		{
			_response_header_cookie = _response_body.substr(0, end);
		}
		_response_body.erase(0, end + 2);
	}
	_status_code = "200";
}
