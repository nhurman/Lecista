#include "CompleteFile.hpp"

namespace Lecista {

// throws boost::filesystem::filesystem_error
CompleteFile::CompleteFile(std::string const& filename)
 : File(boost::filesystem::file_size(filename), Hash::SharedPtr())
{
	hash(filename);
	for (int i = 0; i < m_blocks.size(); ++i) {
		m_blocks[i] = true;
	}
}

void CompleteFile::hash(std::string const& filename)
{
	std::ifstream fh(filename, std::ios::binary);
	char buffer[1000 * 1000]; // 1 MB
	std::streamsize toRead, read;
	Hasher hasher;

	fh.seekg(0, fh.beg);

	for (unsigned int i = 0; i < m_blocks.size() && !fh.eof(); ++i) {
		toRead = BLOCK_SIZE;
		read = 0;
		hasher.reset();

		while (toRead > 0 && !fh.eof()) {
			fh.read(buffer, std::min((unsigned int)toRead, (unsigned int)sizeof buffer));
			read = fh.gcount();

			if (read > 0) {
				toRead -= read;
				hasher.update(buffer, static_cast<unsigned long>(read));
			}
		}

		setHash(i, hasher.getHash());
	}
}

}
