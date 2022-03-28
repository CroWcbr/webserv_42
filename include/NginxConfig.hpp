#ifndef NGINXCONFIG_HPP
# define NGINXCONFIG_HPP

# include <iostream>
# include <string>
# include <fstream>
# include <vector>
# include <sstream>
# include <set>
# include <arpa/inet.h>
# include <unistd.h>

# include "ServerInfo.hpp"
# include "utils.hpp"

# define DEFAULT_CONFIG_PATH					"./config/default.conf"
# define DEFAULT_CONFIG_EXTENTION				".conf"

# define DEFAULT_CONFIG_SERVER					"server {"
# define DEFAULT_CONFIG_LISTEN					"listen"
# define DEFAULT_CONFIG_SERVER_NAME				"server_name"
# define DEFAULT_CONFIG_ERROR_PAGE				"error_page"
# define DEFAULT_CONFIG_CLIENT_MAX_BODY_SIZE	"client_max_body_size"
# define DEFAULT_CONFIG_ROOT					"root"
# define DEFAULT_CONFIG_LOCATION				"location"
# define DEFAULT_CONFIG_ALLOW_METHOD			"allow_methods"
# define DEFAULT_CONFIG_ALLOW_METHOD_GET		"GET"
# define DEFAULT_CONFIG_ALLOW_METHOD_POST		"POST"
# define DEFAULT_CONFIG_ALLOW_METHOD_PUT		"PUT"
# define DEFAULT_CONFIG_ALLOW_METHOD_DELETE		"DELETE"
# define DEFAULT_CONFIG_INDEX					"index"
# define DEFAULT_CONFIG_AUTOINDEX				"autoindex"
# define DEFAULT_CONFIG_UPLOAD_ENABLE			"upload_enable"
# define DEFAULT_CONFIG_UPLOAD_PATH				"upload_path"
# define DEFAULT_CONFIG_CGI_PASS				"cgi_path"
# define DEFAULT_CONFIG_CGI_EXT					"cgi_ext"
# define DEFAULT_CONFIG_RETURN					"return"

class NginxConfig
{
private:
	std::vector<ServerInfo>		_server_info;
	int							_brace;

private:
	NginxConfig();
	NginxConfig(NginxConfig const &copy);
	NginxConfig &operator=(NginxConfig const &copy);

	void 				NginxPrint() const;
	void 				NginxPrintLocation(ServerInfo const &serv) const;	

	std::string			FindConfigPath(int argc, char **argv);
	void				ReadConfigFile(std::string const &configpath);
	void 				ClearBufferSpace(std::string &buffer);
	void 				CheckBuffer(std::string const &buffer);
	void				ClearSemicolonBuffer(std::string &buffer);
	void				ParsingBuffer(ServerInfo &new_serv, \
										Location &new_loc, \
										std::string const &buffer);

	void				AddNewServer(ServerInfo &new_serv);
	void				AddLocationInServer(ServerInfo &new_serv, \
											Location &new_loc);

	void				ParseListen(ServerInfo &new_serv, \
									std::string const &buffer_split);
	void				ParseListenHost(ServerInfo &new_serv, \
										std::string const &host);
	void				ParseListenPort(ServerInfo &new_serv, \
										std::string const &port);
	void				ParseServerName(ServerInfo &new_serv, \
										std::vector<std::string> const &buffer_split);
	void				ParseErrorPage(ServerInfo &new_serv, \
										std::string const &num_error, \
										std::string const &error_path);
	void				ParseRootServ(ServerInfo &new_serv, \
										std::string &root);

	void				ParseLocationMain(Location &new_loc, \
											std::string const &path);
	void				ParseLocationAllowMethod(Location &new_loc, \
													std::string &allow_method);
	void				ParseLocationIndex(Location &new_loc, \
											std::string const &buffer_split);
	void				ParseLocationRoot(Location &new_loc, \
											std::string const &root);
	void				ParseLocationAutoindex(Location &new_loc, \
												std::string const &autoindex);
	void				ParseUploadEnable(Location &new_loc, \
											std::string const &upload_enable);
	void				ParseUploadPath(Location &new_loc, \
										std::string const &upload_path);
	void				ParseCgiPath(Location &new_loc, \
									std::string &cgi_path);
	void				ParseCgiExt(Location &new_loc, \
									std::vector<std::string> const &buffer_split);								
	void				ParseReturn(Location &new_loc, \
									std::string const &ret);
	void				ParseLocationClientMaxBodySize(Location &new_loc, \
														std::string const &client_max_body_size);	

	void				CheckHostPort(ServerInfo const &new_serv) const;
	void				CheckErrorPagePath(ServerInfo &new_serv);

public:
	NginxConfig(int argc, char **argv);
	~NginxConfig() {};

	std::vector<ServerInfo>	const	&getServ() const;
};

# define ERROR_INPUT_WEBSERV						"INPUR ERROR! Too many arguments! Usage : ./webserv [ config_file.conf ]"
# define ERROR_CONFIG_EXTENSION						"CONFIG ERROR! Wrong config extension! Usage : [ *.conf ]"
# define ERROR_CONFIG_OPEN							"CONFIG ERROR! Cannot open config file : "
# define ERROR_CONFIG_BRACE_NUMBER					"CONFIG ERROR! Wrong number of brace during parser"
# define ERROR_CONFIG_SYMBOL_AFTER_OPENNING_BRACE	"CONFIG ERROR! Wrong symbol after brace { : "
# define ERROR_CONFIG_SYMBOL_WITH_OPENNING_BRACE	"CONFIG ERROR! Wrong symbol with brace { : "
# define ERROR_CONFIG_SYMBOL_WITH_CLOSING_BRACE		"CONFIG ERROR! Any symbol with brace } : "
# define ERROR_CONFIG_NO_SEMIKOLON					"CONFIG ERROR! Wrong semikolon : "
# define ERROR_CONFIG_SEMIKOLON_AFTER_CLEAR			"CONFIG ERROR! Wrong line after del semikolon : "

# define ERROR_CONFIG_PARSING						"CONFIG ERROR! Unknown parameter format/type"

# define ERROR_CONFIG_LISTEN_AGAIN					"CONFIG ERROR! Listen again"
# define ERROR_CONFIG_LISTEN_PARAM					"CONFIG ERROR! Wrong listen parameter [ host:port ]. But : "
# define ERROR_CONFIG_LISTEN_HOST_WRONG				"CONFIG ERROR! Wrong format host : "
# define ERROR_CONFIG_LISTEN_NOT_DIGIT_PORT			"CONFIG ERROR! Port is not digit : "
# define ERROR_CONFIG_LISTEN_SPECIAL_PORT			"CONFIG ERROR! Port incorrect (special port 0 - 1024) : "
# define ERROR_CONFIG_LISTEN_TOO_BIG_PORT			"CONFIG ERROR! Port incorrect (too big, max 65535) : "
# define ERROR_CONFIG_SERVER_NAME_AGAIN				"CONFIG ERROR! Server_name again"
# define ERROR_CONFIG_SERVER_NAME_DOUBLE			"CONFIG ERROR! Server_name doubel : "
# define ERROR_CONFIG_ERROR_PAGE_AGAIN				"CONFIG ERROR! Error_page again : "
# define ERROR_CONFIG_ERROR_PAGE_NOT_THREE_SYM		"CONFIG ERROR! Error_page is too big : "
# define ERROR_CONFIG_ERROR_PAGE_NOT_DIGIT_ERROW	"CONFIG ERROR! Error_page is not digit : "
# define ERROR_CONFIG_ERROR_PAGE_WRONG_NUMBER		"CONFIG ERROR! Error_page is wrong int [100...600] : "
# define ERROR_CONFIG_ROOT_SERV_AGAIN				"CONFIG ERROR! Root in serev again"
# define ERROR_CONFIG_ROOT_DOESNT_EXISTS			"CONFIG ERROR! Root in serev doesnt exists : "

# define ERROR_CONFIG_LOCATION_MAIN_PATH			"CONFIG ERROR! Location format wrong [ /.... ]"
# define ERROR_CONFIG_ALLOW_METHOD_AGAIN			"CONFIG ERROR! Allow_method again"
# define ERROR_CONFIG_ALLOW_METHOD_FORMAT			"CONFIG ERROR! Allow_method format [...] : "
# define ERROR_CONFIG_ALLOW_METHOD_DOUBLE			"CONFIG ERROR! Double method : "
# define ERROR_CONFIG_ALLOW_METHOD_UNKNOWN			"CONFIG ERROR! Allow_method format unknown : "
# define ERROR_CONFIG_INDEX_AGAIN					"CONFIG ERROR! Index again"
# define ERROR_CONFIG_LOC_ROOT_AGAIN				"CONFIG ERROR! Root in location again"
# define ERROR_CONFIG_LOC_AUTOINDEX_AGAIN			"CONFIG ERROR! Autoindex in location again"
# define ERROR_CONFIG_LOC_AUTOINDEX_WRONG			"CONFIG ERROR! Autoindex in location wrong parameter [on/off] : "
# define ERROR_CONFIG_LOC_UPLOAD_ENABLE_AGAIN		"CONFIG ERROR! Upload_enable in location again"
# define ERROR_CONFIG_LOC_UPLOAD_ENABLE_WRONG		"CONFIG ERROR! Upload_enable in location wrong parameter [on/off] : "
# define ERROR_CONFIG_LOC_UPLOAD_PATH_AGAIN			"CONFIG ERROR! Upload_path in location again"
# define ERROR_CONFIG_LOC_CGI_PATH_AGAIN			"CONFIG ERROR! Cgi_path in location again"
# define ERROR_CONFIG_CGI_DOESNT_EXISTS				"CONFIG ERROR! Cgi_path doesn't exists : "
# define ERROR_CONFIG_LOC_CGI_EXT_AGAIN				"CONFIG ERROR! Cgi_ext in location again"
# define ERROR_CONFIG_LOC_CGI_EXT_NO_POINT			"CONFIG ERROR! Cgi_ext no point in it : "
# define ERROR_CONFIG_LOC_RETURN_AGAIN				"CONFIG ERROR! Return in location again"
# define ERROR_CONFIG_CLIENT_MAX_BODY_AGAIN			"CONFIG ERROR! Client_max_body_size again"
# define ERROR_CONFIG_CLIENT_MAX_BODY_UNKNOWN_TYPE	"CONFIG ERROR! Client_max_body_size must be 'm' or 'k' or 'b': "
# define ERROR_CONFIG_CLIENT_MAX_BODY_IS_NOT_DIGIT	"CONFIG ERROR! Client_max_body_size must be digit : "

# define ERROR_CONFIG_LOCATION_AGAIN				"CONFIG ERROR! Location again"
# define ERROR_CONFIG_LOC_CGI_PATH_EXT				"CONFIG ERROR! Cgi_ext || Cgi_path is wrong"
# define ERROR_CONFIG_LOCATION_UPLOAD_ON_NO_PATH	"CONFIG ERROR! Upload ON, but there is no path"
# define ERROR_CONFIG_CHECK_NECCESSARY				"CONFIG ERROR! No neccessary parameters host:port"
# define ERROR_CONFIG_CHECK_HOST_PORT_AGAIN			"CONFIG ERROR! Host:Port again"
# define ERROR_CONFIG_ERROR_PAGE_DOESNT_EXISTS		"CONFIG ERROR! Error_page is wrong. The file doesnt exist : "
# define ERROR_CONFIG_LOCATION_ROOT_DOESNT_EXISTS	"CONFIG ERROR! Loc Root is wrong. The root doesnt exist : "
# define ERROR_CONFIG_LOC_UPLOAD_PATH_DOESNT_EXISTS	"CONFIG ERROR! Loc Upload_Path is wrong. The upload_path doesnt exist : "
# define ERROR_CONFIG_LOC_INDEX_PATH_DOESNT_EXISTS	"CONFIG ERROR! Loc Index is wrong. The index doesnt exist : "

#endif
