# include "../include/Server.hpp"

int main(int argc, char **argv)
{
	Server serve(argc, argv);
	
	serve.Loop();

	return 0;
}
