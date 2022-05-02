#include "../include/Proxy.hpp"

int main()
{
	Proxy proxy("127.0.0.1", 5555);

	proxy.Loop();

	return 0;
}