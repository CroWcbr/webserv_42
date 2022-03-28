#ifndef SERVERINFO_HPP
# define SERVERINFO_HPP

# include <string>
# include <vector>
# include <map>
# include <set>
# include <netinet/ip.h>

class Location
{
private:
	std::string					_path;
	std::string					_index;
	std::set<std::string>		_allow_method;
	std::string					_root;
	std::string					_autoindex;
	std::string					_upload_enable;
	std::string					_upload_path;
	std::string					_cgi_path;
	std::set<std::string>		_cgi_ext;
	std::string					_return;
	int							_client_max_body_size;

public:
	Location() {}
	~Location() {}
	Location(Location const &copy) { *this = copy; };
	Location &operator=(Location const &copy)
	{
		_path = copy._path;
		_index = copy._index;				
		_allow_method = copy._allow_method;	
		_root = copy._root;		
		_autoindex = copy._autoindex;
		_upload_enable = copy._upload_enable;
		_upload_path = copy._upload_path;
		_cgi_path = copy._cgi_path;
		_cgi_ext = copy._cgi_ext;
		_return = copy._return;
		_client_max_body_size = copy._client_max_body_size;
		return *this;
	}

	void	init()
	{
		_path.clear();
		_index.clear();
		_allow_method.clear();
		_root.clear();
		_autoindex.clear();
		_upload_enable.clear();
		_upload_path.clear();
		_cgi_path.clear();
		_cgi_ext.clear();
		_return.clear();
		_client_max_body_size = -1;
	}

	void	SetPath(std::string const &path) { _path = path; }
	void	SetIndex(std::string const &index) { _index = index; }
	void	SetAllowMethod(std::set<std::string> const &allow_method) { _allow_method = allow_method; }	
	void	SetRoot(std::string const &root) { _root = root; }
	void	SetAutoindex(std::string const &autoindex) { _autoindex = autoindex; }	
	void	SetUploadEnable(std::string const &upload_enable) { _upload_enable = upload_enable; }		
	void	SetUploadPath(std::string const &upload_path) { _upload_path = upload_path; }	
	void	SetCgiPath(std::string const &cgi_path) { _cgi_path = cgi_path; }
	void	SetCgiExt(std::set<std::string> const &cgi_ext) { _cgi_ext = cgi_ext; }	
	void	SetReturn(std::string const &ret) { _return = ret; }	
	void	SetClientMaxBodySize(int const &client_max_body_size) { _client_max_body_size = client_max_body_size; }

	std::string const			&GetPath() const { return _path; }
	std::string const			&GetIndex() const { return _index; }
	std::set<std::string> const	&GetAllowMethod() const { return _allow_method; }
	std::string	const			&GetRoot() const { return _root; }
	std::string	const			&GetAutoindex() const { return _autoindex; }
	std::string	const			&GetUploadEnable() const { return _upload_enable; }
	std::string	const			&GetUploadPath() const { return _upload_path; }
	std::string	const			&GetCgiPath() const { return _cgi_path; }
	std::set<std::string> const	&GetCgiExt() const { return _cgi_ext; }
	std::string	const			&GetReturn() const { return _return; }
	int	const					&GetClientMaxBodySize() const { return _client_max_body_size; }		
};

class ServerInfo
{
private:
	int									_listen_fd;
	sockaddr_in							_addr;
	int									_port;
	std::string							_host;
	std::vector<std::string>			_server_name;
	std::map<int, std::string>			_error_page;
	std::string							_root;
	std::map<std::string, Location>		_location;

public:
	ServerInfo() {}
	~ServerInfo() {}
	ServerInfo(ServerInfo const &copy) { *this = copy; }
	ServerInfo &operator=(ServerInfo const &copy) {
		_listen_fd = copy._listen_fd;
		_addr = copy._addr;
		_port = copy._port;
		_host = copy._host;
		_server_name = copy._server_name;
		_error_page = copy._error_page;
		_root = copy._root;
		_location = copy._location;		
		return *this;
	}

	void	init()
	{
		_port = -1;
		_host.clear();
		_server_name.clear();
		_error_page.clear();
		_root.clear();
		_location.clear();
	}

	void	SetListenFd(int const &listen_fd) { _listen_fd = listen_fd; }
	void	SetSockAddr(sockaddr_in addr) { _addr = addr; }
	void	SetPort(int const &port) { _port = port; }
	void	SetHost(std::string const &host) { _host = host; }
	void	SetServerName(std::vector<std::string> const &server_name) { _server_name = server_name; }
	void	SetErrorPage(std::map<int, std::string> const &error_page) { _error_page = error_page; }
	void	SetRoot(std::string const &root) { _root = root; }

	int	const								&GetListenFd() const { return _listen_fd; }
	sockaddr_in const						&GetSockAddr() const { return _addr; }
	int	const								&GetPort() const { return _port; }
	std::string	const						&GetHost() const { return _host; }
	std::vector<std::string> const			&GetServerName() const { return _server_name; }
	std::map<int, std::string> const		&GetErrorPage() const { return _error_page; }
	std::string	const						&GetRoot() const { return _root; }
	std::map<std::string, Location> const	&GetLocation() const { return _location; }

	void	AddLocation(Location const &new_loc) { 
		_location.insert(std::pair<std::string, Location>(new_loc.GetPath(), new_loc)); }

	void	UpdateErrorPagePath(int const &err_int, std::string const &new_path) {
		_error_page.find(err_int)->second = new_path; }
};

#endif
