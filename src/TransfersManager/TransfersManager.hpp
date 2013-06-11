#pragma once

#include <deque>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/chrono.hpp>
#include "compat.hpp"

namespace Lecista {

class Transfer
{
public:
	explicit Transfer(File::SharedPtr const& file, IOHandler& io);
	Transfer(Transfer const& other) : Transfer(other.m_file, other.m_io) {}

	File::SharedPtr const& file() const { return m_file; }
	bool operator==(Transfer const& other) const { return *m_file == *other.m_file; }
	bool operator<(Transfer const& other) const { return *m_file < *other.m_file; }

	void restartTimer();
	float elapsedTime() const;
	void resetRates() { m_downloadedBytes = 0; m_uploadedBytes = 0; restartTimer(); }
	float uploadSpeed() const;
	float downloadSpeed() const;
	void maxUploadSpeed(float rate) { m_maxUploadSpeed = rate; }
	void maxDownloadSpeed(float rate) { m_maxDownloadSpeed = rate; }

	void read(size_t bytes);
	void write(char const* data, size_t bytes, boost::function<void()> callback);

private:
	static float const AVERAGE_DELAY;
	typedef boost::chrono::system_clock Clock;

	File::SharedPtr const& m_file;
	IOHandler& m_io;
	unsigned int m_downloadedBytes;
	unsigned int m_uploadedBytes;
	float m_maxDownloadSpeed;
	float m_maxUploadSpeed;
	Clock::time_point m_bandwidthTime;
};

class TransfersManager
{
public:
	class NotFoundException {};
	TransfersManager(IOHandler& io);

	bool add(Transfer const& transfer);
	bool add(File::SharedPtr const& file);
	TransfersManager& operator<<(Transfer const& transfer);
	TransfersManager& operator<<(File::SharedPtr const& file);

	Transfer& operator[](Hash const& hash);

	unsigned int downloadSpeed() const { return m_downloadSpeed; }
	unsigned int uploadSpeed() const { return m_uploadSpeed; }
	unsigned int maxDownloadSpeed() const { return m_maxDownloadSpeed; }
	unsigned int maxUploadSpeed() const { return m_maxUploadSpeed; }
	void maxDownloadSpeed(unsigned int rate);
	void maxUploadSpeed(unsigned int rate);
	void applyRates();

	void updateRates();
	void run();

private:
	IOHandler& m_io;
	std::deque<Transfer> m_transfers;

	float m_maxDownloadSpeed;
	float m_maxUploadSpeed;
	float m_downloadSpeed;
	float m_uploadSpeed;
};

}
