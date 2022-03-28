#include "../include/NginxConfig.hpp"

std::vector<ServerInfo> const &NginxConfig::getServ() const
{
	return _server_info;
}

NginxConfig::NginxConfig(int argc, char **argv)
{
	std::string	configpath = FindConfigPath(argc, argv);
	_brace = 0;

	ReadConfigFile(configpath);

	NginxPrint();
}

std::string NginxConfig::FindConfigPath(int argc, char **argv)
{
	std::string configpath;

	if (argc == 1)
		configpath = DEFAULT_CONFIG_PATH;
	else if (argc == 2)
		configpath = argv[1];
	else
		throw std::runtime_error(ERROR_INPUT_WEBSERV);
	
	std::string check = DEFAULT_CONFIG_EXTENTION;
	int check_len = check.length();
	if (configpath.length() <= check_len || \
			configpath.substr(configpath.length() - check_len, check_len) != check)
		throw std::runtime_error(ERROR_CONFIG_EXTENSION);
	return configpath;
}

void NginxConfig::ReadConfigFile(std::string const &configpath)
{
	std::ifstream config_read(configpath);

	if (!config_read.is_open())
		throw std::runtime_error(ERROR_CONFIG_OPEN + configpath);
	
	std::string buffer;
	ServerInfo new_serv;
	Location new_loc;
	while (true)
	{
		std::getline(config_read, buffer);
		ClearBufferSpace(buffer);
		if (!buffer.empty())
		{
			if (_brace == 0)
			{
				new_serv.init();
				new_loc.init();
			}
			CheckBuffer(buffer);
			if (_brace == 1 && !new_loc.GetPath().empty())
				AddLocationInServer(new_serv, new_loc);
			ClearSemicolonBuffer(buffer);
			ParsingBuffer(new_serv, new_loc, buffer);
			if (_brace == 0)
				AddNewServer(new_serv);
		}
		if (config_read.eof())
		{
			if (_brace != 0)
				throw std::runtime_error(ERROR_CONFIG_BRACE_NUMBER);
			break ;
		}
	}
	config_read.close();
}

void NginxConfig::ClearBufferSpace(std::string &buffer)
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
}

void NginxConfig::CheckBuffer(std::string const &buffer)
{
	if ((buffer == DEFAULT_CONFIG_SERVER && _brace != 0) || 
		(buffer != DEFAULT_CONFIG_SERVER && _brace == 0))
		throw std::runtime_error(ERROR_CONFIG_BRACE_NUMBER);
	for (int i = 0; i < buffer.size(); i++)
	{
		if (buffer[i] == '{')
		{
			_brace++;
			if (i != buffer.size() - 1)
				throw std::runtime_error(ERROR_CONFIG_SYMBOL_AFTER_OPENNING_BRACE + buffer);
			if (!((buffer == DEFAULT_CONFIG_SERVER && _brace == 1) || 
				(buffer.substr(0, std::string(DEFAULT_CONFIG_LOCATION).length()) == DEFAULT_CONFIG_LOCATION && _brace == 2)))
				throw std::runtime_error(ERROR_CONFIG_SYMBOL_WITH_OPENNING_BRACE + buffer);
		}
		if (buffer[i] == '}')
			_brace--;
		if (buffer[i] == '}' && buffer.size() != 1)
			throw std::runtime_error(ERROR_CONFIG_SYMBOL_WITH_CLOSING_BRACE + buffer);
	}	
}

void NginxConfig::ClearSemicolonBuffer(std::string &buffer)
{
	if (buffer == DEFAULT_CONFIG_SERVER || buffer == "}" || \
		buffer.substr(0, std::string(DEFAULT_CONFIG_LOCATION).length()) == DEFAULT_CONFIG_LOCATION)
		return ;

	if (buffer.back() != ';')
		throw std::runtime_error(ERROR_CONFIG_NO_SEMIKOLON + buffer);
	buffer.pop_back();
	if (isspace(buffer.back()))
		throw std::runtime_error(ERROR_CONFIG_SEMIKOLON_AFTER_CLEAR + buffer);
}

void NginxConfig::ParsingBuffer(ServerInfo &new_serv, Location &new_loc, std::string const &buffer)
{
	if (buffer == DEFAULT_CONFIG_SERVER || buffer == "}")
		return ;

	std::vector<std::string> buffer_split = _split(buffer, ' ');
	if (_brace == 1)
	{
		if (buffer_split[0] == DEFAULT_CONFIG_LISTEN && buffer_split.size() == 2)
			ParseListen(new_serv, buffer_split[1]);
		else if (buffer_split[0] == DEFAULT_CONFIG_SERVER_NAME && buffer_split.size() >= 2)
			ParseServerName(new_serv, buffer_split);
		else if (buffer_split[0] == DEFAULT_CONFIG_ERROR_PAGE && buffer_split.size() == 3)
			ParseErrorPage(new_serv, buffer_split[1], buffer_split[2]);
		else if (buffer_split[0] == DEFAULT_CONFIG_ROOT && buffer_split.size() == 2)
			ParseRootServ(new_serv, buffer_split[1]);
		else
			throw std::runtime_error(ERROR_CONFIG_PARSING + std::string(" : ") + buffer_split[0]);
	}
	else
	{
		if (buffer_split[0] == DEFAULT_CONFIG_LOCATION && buffer_split.size() == 3)
			ParseLocationMain(new_loc, buffer_split[1]);
		else if (buffer_split[0] == DEFAULT_CONFIG_ALLOW_METHOD && buffer_split.size() == 2)
			ParseLocationAllowMethod(new_loc, buffer_split[1]);
		else if (buffer_split[0] == DEFAULT_CONFIG_INDEX && buffer_split.size() == 2)
			ParseLocationIndex(new_loc, buffer_split[1]);
		else if (buffer_split[0] == DEFAULT_CONFIG_ROOT && buffer_split.size() == 2)
			ParseLocationRoot(new_loc, buffer_split[1]);		
		else if (buffer_split[0] == DEFAULT_CONFIG_AUTOINDEX && buffer_split.size() == 2)
			ParseLocationAutoindex(new_loc, buffer_split[1]);
		else if (buffer_split[0] == DEFAULT_CONFIG_UPLOAD_ENABLE && buffer_split.size() == 2)
			ParseUploadEnable(new_loc, buffer_split[1]);
		else if (buffer_split[0] == DEFAULT_CONFIG_UPLOAD_PATH && buffer_split.size() == 2)
			ParseUploadPath(new_loc, buffer_split[1]);
		else if (buffer_split[0] == DEFAULT_CONFIG_CGI_PASS && buffer_split.size() == 2)
			ParseCgiPath(new_loc, buffer_split[1]);
		else if (buffer_split[0] == DEFAULT_CONFIG_CGI_EXT && buffer_split.size() >= 2)
			ParseCgiExt(new_loc, buffer_split);
		else if (buffer_split[0] == DEFAULT_CONFIG_RETURN && buffer_split.size() == 2)
			ParseReturn(new_loc, buffer_split[1]);
		else if (buffer_split[0] == DEFAULT_CONFIG_CLIENT_MAX_BODY_SIZE && buffer_split.size() == 2)
			ParseLocationClientMaxBodySize(new_loc, buffer_split[1]);			
		else
			throw std::runtime_error(ERROR_CONFIG_PARSING + std::string(" : ") + buffer_split[0]);
	}
}

void NginxConfig::ParseListen(ServerInfo &new_serv, std::string const &buffer_split)
{
	if (new_serv.GetPort() != -1 || !new_serv.GetHost().empty())
		throw std::runtime_error(ERROR_CONFIG_LISTEN_AGAIN);

	std::vector<std::string> listen_split = _split(buffer_split, ':');

	if (listen_split.size() != 2)
		throw std::runtime_error(ERROR_CONFIG_LISTEN_PARAM + buffer_split);

	ParseListenHost(new_serv, listen_split[0]);
	ParseListenPort(new_serv, listen_split[1]);
}

void NginxConfig::ParseListenHost(ServerInfo &new_serv, std::string const &host)
{
	if (host == "localhost")
		new_serv.SetHost("127.0.0.1");
	else
	{
		if (inet_addr(host.c_str()) == INADDR_NONE)
			throw std::runtime_error(ERROR_CONFIG_LISTEN_HOST_WRONG + host);
		new_serv.SetHost(host);
	}
}

void NginxConfig::ParseListenPort(ServerInfo &new_serv, std::string const &port)
{
	for (int i = 0; i < port.length(); ++i)
		if (!isdigit(port[i]))
			throw std::runtime_error(ERROR_CONFIG_LISTEN_NOT_DIGIT_PORT + port);

	int tmp_port = std::atoi(port.c_str());
	if (tmp_port < 1024)
		throw std::runtime_error(ERROR_CONFIG_LISTEN_SPECIAL_PORT + port);
	if (tmp_port > 65535)
		throw std::runtime_error(ERROR_CONFIG_LISTEN_TOO_BIG_PORT + port);

	new_serv.SetPort(tmp_port);
}

void NginxConfig::ParseServerName(ServerInfo &new_serv, std::vector<std::string> const &buffer_split)
{
	if (!new_serv.GetServerName().empty())
		throw std::runtime_error(ERROR_CONFIG_SERVER_NAME_AGAIN);

	std::vector<std::string> server_name;
	for (int i = 1; i < buffer_split.size(); i++)
	{
		for (std::vector<std::string>::const_iterator it = server_name.begin(); it != server_name.end(); it++)
			if (*it == buffer_split[i])
				throw std::runtime_error(ERROR_CONFIG_SERVER_NAME_DOUBLE + buffer_split[i]);
		server_name.push_back(buffer_split[i]);
	}
	new_serv.SetServerName(server_name);
}

void NginxConfig::ParseErrorPage(ServerInfo &new_serv, std::string const &num_error, std::string const &error_path)
{
	if (num_error.length() != 3 )
		throw std::runtime_error(ERROR_CONFIG_ERROR_PAGE_NOT_THREE_SYM + num_error);
	for (int i = 0; i < num_error.length(); ++i)
		if (!isdigit(num_error[i]))
			throw std::runtime_error(ERROR_CONFIG_ERROR_PAGE_NOT_DIGIT_ERROW + num_error);
	int int_error = std::atoi(num_error.c_str());

	if (int_error < 100 || int_error > 599)
		throw std::runtime_error(ERROR_CONFIG_ERROR_PAGE_WRONG_NUMBER + num_error);
	std::map<int, std::string> error_page = new_serv.GetErrorPage();
	if (error_page.find(int_error) != error_page.end())
		throw std::runtime_error(ERROR_CONFIG_ERROR_PAGE_AGAIN + num_error + " " + error_path);

	error_page.insert(std::pair<int, std::string>(int_error, error_path));

	new_serv.SetErrorPage(error_page);
}

void NginxConfig::ParseRootServ(ServerInfo &new_serv, std::string &root)
{
	if (!new_serv.GetRoot().empty())
		throw std::runtime_error(ERROR_CONFIG_ROOT_SERV_AGAIN);

	if (!isDirectory(root))
		throw std::runtime_error(ERROR_CONFIG_ROOT_DOESNT_EXISTS + root);

	clearDoubleSplash(root);
	new_serv.SetRoot(root);
}

void NginxConfig::ParseLocationMain(Location &new_loc, std::string const &path)
{
	new_loc.init();

	if (path[0] != '/')
		throw std::runtime_error(ERROR_CONFIG_LOCATION_MAIN_PATH);
		
	new_loc.SetPath(path);
}

void NginxConfig::ParseLocationAllowMethod(Location &new_loc, std::string &allow_method)
{
	if (!new_loc.GetAllowMethod().empty())
		throw std::runtime_error(ERROR_CONFIG_ALLOW_METHOD_AGAIN);

	if (!(allow_method[0] == '[' && allow_method[allow_method.size() - 1] == ']') || 
		allow_method.length() <= 2)
		throw std::runtime_error(ERROR_CONFIG_ALLOW_METHOD_FORMAT + allow_method);
	allow_method.pop_back();
	allow_method.erase(0, 1);

	std::vector<std::string> allow_method_split = _split(allow_method, ',');
	std::set<std::string> set_allow_method;
	for (int i = 0; i < allow_method_split.size(); i++)
	{
		if (new_loc.GetAllowMethod().find(allow_method_split[i]) != new_loc.GetAllowMethod().end())
			throw std::runtime_error(ERROR_CONFIG_ALLOW_METHOD_DOUBLE + allow_method_split[i]);
	
		if (allow_method_split[i] == DEFAULT_CONFIG_ALLOW_METHOD_GET || \
				allow_method_split[i] == DEFAULT_CONFIG_ALLOW_METHOD_POST || \
				allow_method_split[i] == DEFAULT_CONFIG_ALLOW_METHOD_PUT || \
				allow_method_split[i] == DEFAULT_CONFIG_ALLOW_METHOD_DELETE )
			set_allow_method.insert(allow_method_split[i]);
		else
			throw std::runtime_error(ERROR_CONFIG_ALLOW_METHOD_UNKNOWN + allow_method_split[i]);
	}
	new_loc.SetAllowMethod(set_allow_method);
}

void NginxConfig::ParseLocationIndex(Location &new_loc, std::string const &index)
{
	if (!new_loc.GetIndex().empty())
		throw std::runtime_error(ERROR_CONFIG_INDEX_AGAIN);

	new_loc.SetIndex(index);
}

void NginxConfig::ParseLocationRoot(Location &new_loc, std::string const &root)
{
	if (!new_loc.GetRoot().empty())
		throw std::runtime_error(ERROR_CONFIG_LOC_ROOT_AGAIN);

	new_loc.SetRoot(root);
}

void NginxConfig::ParseLocationAutoindex(Location &new_loc, std::string const &autoindex)
{
	if (!new_loc.GetAutoindex().empty())
		throw std::runtime_error(ERROR_CONFIG_LOC_AUTOINDEX_AGAIN);

	if (autoindex != "on" && autoindex != "off")
		throw std::runtime_error(ERROR_CONFIG_LOC_AUTOINDEX_WRONG + autoindex);
	
	new_loc.SetAutoindex(autoindex);
}

void NginxConfig::ParseUploadEnable(Location &new_loc, std::string const &upload_enable)
{
	if (!new_loc.GetUploadEnable().empty())
		throw std::runtime_error(ERROR_CONFIG_LOC_UPLOAD_ENABLE_AGAIN);

	if (upload_enable != "on" && upload_enable != "off")
		throw std::runtime_error(ERROR_CONFIG_LOC_UPLOAD_ENABLE_WRONG);

	new_loc.SetUploadEnable(upload_enable);
}

void NginxConfig::ParseUploadPath(Location &new_loc, std::string const &upload_path)
{
	if (!new_loc.GetUploadPath().empty())
		throw std::runtime_error(ERROR_CONFIG_LOC_UPLOAD_PATH_AGAIN);

	new_loc.SetUploadPath(upload_path);
}

void NginxConfig::ParseCgiPath(Location &new_loc, std::string &cgi_path)
{
	if (!new_loc.GetCgiPath().empty())
		throw std::runtime_error(ERROR_CONFIG_LOC_CGI_PATH_AGAIN);

	if (!isFile(cgi_path))
		throw std::runtime_error(ERROR_CONFIG_CGI_DOESNT_EXISTS + cgi_path);

	clearDoubleSplash(cgi_path);
	new_loc.SetCgiPath(cgi_path);
}

void NginxConfig::ParseCgiExt(Location &new_loc, std::vector<std::string> const &buffer_split)
{
	if (!new_loc.GetCgiExt().empty())
		throw std::runtime_error(ERROR_CONFIG_LOC_CGI_EXT_AGAIN);

	std::set<std::string> cgi_ext;
	for (int i = 1; i < buffer_split.size(); i++)
	{
		if (buffer_split[i][0] != '.')
			throw std::runtime_error(ERROR_CONFIG_LOC_CGI_EXT_NO_POINT + buffer_split[i]);
		cgi_ext.insert(buffer_split[i]);
	}
	new_loc.SetCgiExt(cgi_ext);
}

void NginxConfig::ParseReturn(Location &new_loc, std::string const &ret)
{
	if (!new_loc.GetReturn().empty())
		throw std::runtime_error(ERROR_CONFIG_LOC_RETURN_AGAIN);

	new_loc.SetReturn(ret);
}

void NginxConfig::ParseLocationClientMaxBodySize(Location &new_loc, std::string const &client_max_body_size)
{
	if (new_loc.GetClientMaxBodySize() != -1)
		throw std::runtime_error(ERROR_CONFIG_CLIENT_MAX_BODY_AGAIN);
	if (client_max_body_size.back() != 'm' && client_max_body_size.back() != 'k' && client_max_body_size.back() != 'b')
		throw std::runtime_error(ERROR_CONFIG_CLIENT_MAX_BODY_UNKNOWN_TYPE + client_max_body_size);
	for (int i = 0; i < client_max_body_size.length() - 1; ++i)
		if (!isdigit(client_max_body_size[i]))
			throw std::runtime_error(ERROR_CONFIG_CLIENT_MAX_BODY_UNKNOWN_TYPE + client_max_body_size);

	int tmp_cmbs = std::atoi(client_max_body_size.c_str());
	if (client_max_body_size.back() == 'm')
		new_loc.SetClientMaxBodySize(tmp_cmbs * 1024 * 1024);
	else if (client_max_body_size.back() == 'k')
		new_loc.SetClientMaxBodySize(tmp_cmbs * 1024);
	else
		new_loc.SetClientMaxBodySize(tmp_cmbs);
}

void NginxConfig::NginxPrint() const
{
	for (int i = 0; i < _server_info.size(); i++)
	{
		std::cout << "\033[90m";
		std::cout << "++++++++++++++++++++" << std::endl;
		std::cout << "\033[94m";
		std::cout << "SERVER " << i + 1 << " info :" << std::endl;
		std::cout << "\033[96m";
		std::cout << "\tport : " << _server_info[i].GetPort()<< std::endl;
		std::cout << "\thost : " << _server_info[i].GetHost()<< std::endl;

		if (!_server_info[i].GetServerName().empty())
		{
			std::cout << "\tserver_name : ";
			for (std::vector<std::string>::const_iterator it = _server_info[i].GetServerName().begin(), \
				itend = _server_info[i].GetServerName().end(); it != itend; it++)
				std::cout << *it << " ";
			std:: cout << std:: endl;
		}

		if (!_server_info[i].GetErrorPage().empty())
		{
			std::cout << "\terror_page : " << std::endl;
			for (std::map<int, std::string>::const_iterator it = _server_info[i].GetErrorPage().begin(), \
				itend = _server_info[i].GetErrorPage().end(); it != itend; it++)
				std::cout << "\t\t" << it->first << "\t" << it->second << std::endl;
		}

		if (!_server_info[i].GetRoot().empty())
			std::cout << "\troot : " << _server_info[i].GetRoot()<< std::endl;
		
		if (!_server_info[i].GetLocation().empty())
			NginxPrintLocation(_server_info[i]);

		std::cout << "\033[90m";		
		std::cout << "++++++++++++++++++++" << std::endl;
		std::cout << "\033[0m";
	}
}

void NginxConfig::NginxPrintLocation(ServerInfo const &serv) const
{
	for (std::map<std::string, Location>::const_iterator it = serv.GetLocation().begin(), \
		itend = serv.GetLocation().end(); it != itend; it++)
	{
		std::cout << "\033[94m";
		std::cout << "\tLocation : ";
		std::cout << "\033[95m";
		std::cout << it->first << std::endl;

		if (!it->second.GetAllowMethod().empty())
		{
			std::cout << "\t\tallow_method : " ;
			for (std::set<std::string>::const_iterator itt = it->second.GetAllowMethod().begin(), \
				ittend = it->second.GetAllowMethod().end(); itt != ittend; itt++)
				std::cout << *itt << " ";
			std:: cout << std:: endl;
		}

		if (!it->second.GetIndex().empty())
			std:: cout << "\t\tindex : " << it->second.GetIndex()<< std::endl;

		if (!it->second.GetRoot().empty())
			std::cout << "\t\troot : " << it->second.GetRoot()<< std::endl;

		if (it->second.GetAutoindex() == "on")
			std::cout << "\t\tautoindex : " << it->second.GetAutoindex()<< std::endl;

		if (it->second.GetUploadEnable() == "on")
		{
			std::cout << "\t\tupload_enable : " << it->second.GetUploadEnable()<< std::endl;
			std::cout << "\t\tupload_path : " << it->second.GetUploadPath()<< std::endl;
		}

		if (!it->second.GetCgiPath().empty())
		{
			std::cout << "\t\tcgi_ext : " ;
			for (std::set<std::string>::const_iterator itt = it->second.GetCgiExt().begin(), \
				ittend = it->second.GetCgiExt().end(); itt != ittend; itt++)
				std::cout << *itt << " ";
			std::cout << std::endl;
			std::cout << "\t\tcgi_path : " << it->second.GetCgiPath()<< std::endl;
		}

		if (!it->second.GetReturn().empty())
			std::cout << "\t\treturn : " << it->second.GetReturn()<< std::endl;
	}
}

void NginxConfig::AddLocationInServer(ServerInfo &new_serv, Location &new_loc)
{
	if (new_serv.GetLocation().find(new_loc.GetPath()) != new_serv.GetLocation().end())
		throw std::runtime_error(ERROR_CONFIG_LOCATION_AGAIN);

	if (new_loc.GetCgiPath().empty() != new_loc.GetCgiExt().empty())
		throw std::runtime_error(ERROR_CONFIG_LOC_CGI_PATH_EXT);

	if (new_loc.GetAutoindex() == "")
		new_loc.SetAutoindex("off");

	if (new_loc.GetUploadEnable() == "")
		new_loc.SetUploadEnable("off");		

	if (new_loc.GetUploadEnable() == "on" && new_loc.GetUploadPath().empty())
		throw std::runtime_error(ERROR_CONFIG_LOCATION_UPLOAD_ON_NO_PATH);

	if (new_loc.GetClientMaxBodySize() == -1)
		new_loc.SetClientMaxBodySize(0);

	new_serv.AddLocation(new_loc);
}

void NginxConfig::AddNewServer(ServerInfo &new_serv)
{
	CheckHostPort(new_serv);
	CheckErrorPagePath(new_serv);

	_server_info.push_back(new_serv);
}

void NginxConfig::CheckHostPort(ServerInfo const &new_serv) const
{
	if (new_serv.GetHost().empty() || new_serv.GetPort() == -1)
		throw std::runtime_error(ERROR_CONFIG_CHECK_NECCESSARY);

	for (std::vector<ServerInfo>::const_iterator it = _server_info.begin(), ite = _server_info.end(); it != ite; it++)
		if (it->GetHost() == new_serv.GetHost() && it->GetPort() == new_serv.GetPort())
			throw std::runtime_error(ERROR_CONFIG_CHECK_HOST_PORT_AGAIN);
}

void NginxConfig::CheckErrorPagePath(ServerInfo &new_serv)
{
	for (std::map<int, std::string>::const_iterator it = new_serv.GetErrorPage().begin(),
		it_end = new_serv.GetErrorPage().end(); it != it_end; it++)
	{	
		std::string test_path;
		if (new_serv.GetRoot().empty())
			test_path = std::string(".") + it->second;
		else
			test_path = new_serv.GetRoot() + it->second;

		if (!isFile(test_path))
			throw std::runtime_error(ERROR_CONFIG_ERROR_PAGE_DOESNT_EXISTS + test_path);

		clearDoubleSplash(test_path);
		if (it->second != test_path)
			new_serv.UpdateErrorPagePath(it->first, test_path);
	}
}
