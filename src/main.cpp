#include "../include/Proxy.hpp"

int main(int argc, char **argv)
{
	Proxy proxy(argc, argv);

	proxy.Loop();

	return 0;
}
