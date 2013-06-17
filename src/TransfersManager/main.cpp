#include "TransfersManager.hpp"
#include <boost/thread/thread.hpp>

using namespace Lecista;

int main(int argc, char* argv[])
{
	// ____________________________
	Hash h1("Hello______________");
	Hash h2("Zello______________");
	
	IOHandler io;
	TransfersManager manager(io);
	
	manager.maxDownloadSpeed(100);
	manager.maxUploadSpeed(10);
	
	File::SharedPtr f1(new File(1, h1));
	File::SharedPtr f2(new File(1, h2));

	manager << f1 << f2;
	std::cout << manager[h2].file()->rootHash().hash() << std::endl;

	/*
	manager[h1].read(1000);
	std::cout << "Down: " << manager[h1].downloadSpeed() << " ; Up: "
		<< manager[h1].uploadSpeed() << std::endl;

	for (int i = 0; i < 5; ++i) {
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		std::cout << "Down: " << manager[h1].downloadSpeed() << " ; Up: "
			<< manager[h1].uploadSpeed() << std::endl;
	}*/

	char data[50] = {'a'};
	//manager[h1].write(data, sizeof data, [] { std::cout << "Done :)" << std::endl; });
	manager[h1].read(100, [](size_t read, char* data) { std::cout << "Callback: read " << read << std::endl; });
	
	std::cin.get();
	io.start();
	std::cin.get();
	return 0;
}
