#include <iostream>
#include <iomanip>
#include <string>
#include "MulticastHandler.hpp"
#include "IOHandler.hpp"

using namespace Lecista;

int main()
{
	IOHandler io;
	MulticastHandler mcast(io);
	std::cin.get();

	return 0;
}
