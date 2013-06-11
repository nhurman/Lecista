#include "TransfersManager.hpp"

namespace Lecista {

float const Transfer::AVERAGE_DELAY = 0.500; // Reset rate every 500ms

Transfer::Transfer(File::SharedPtr const& file, IOHandler& io)
 : m_file(file), m_io(io),
   m_uploadedBytes(0), m_downloadedBytes(0),
   m_maxUploadSpeed(0), m_maxDownloadSpeed(0),
   m_bandwidthTime(Clock::now())
{

}

void Transfer::restartTimer()
{
	m_bandwidthTime = Clock::now();
}

float Transfer::elapsedTime() const
{
	Clock::duration d(Clock::now() - m_bandwidthTime);
	return d.count() * (float)(Clock::period::num) / Clock::period::den;
}

float Transfer::uploadSpeed() const
{
	float elapsed = elapsedTime();
	if (elapsed < AVERAGE_DELAY / 3 || elapsed > AVERAGE_DELAY * 3) {
		return 0;
	}

	return m_uploadedBytes / elapsed;
}

float Transfer::downloadSpeed() const
{
	float elapsed = elapsedTime();
	if (elapsed < AVERAGE_DELAY / 3 || elapsed > AVERAGE_DELAY * 3) {
		return 0;
	}

	return m_downloadedBytes / elapsed;
}

void Transfer::read(size_t bytes)
{
	float elapsed = elapsedTime();
	if (elapsed > AVERAGE_DELAY) {
		resetRates();
	}

	m_downloadedBytes += bytes;
	if (m_maxDownloadSpeed > 0) {
		float targetTime = ceil(1000 * m_downloadedBytes / m_maxDownloadSpeed);
		float sleepTime = targetTime - elapsed;

		if (sleepTime > 0) { // Going too fast
			// Delay next call
		}
		else {
			// Next call asap
		}
	}
}

void Transfer::write(char const* data, size_t bytes, boost::function<void()> callback)
{

}

/*******************************************************************************************************/

TransfersManager::TransfersManager(IOHandler& io)
 : m_io(io),
   m_maxDownloadSpeed(0), m_maxUploadSpeed(0),
   m_downloadSpeed(0), m_uploadSpeed(0)
{

}

bool TransfersManager::add(Transfer const& transfer)
{
	if (m_transfers.end() != std::find(m_transfers.begin(), m_transfers.end(), transfer)) {
		return false;
	}

	m_transfers.push_back(transfer);
	return true;
}

bool TransfersManager::add(File::SharedPtr const& file)
{
	return add(Transfer(file, m_io));
}


TransfersManager& TransfersManager::operator<<(Transfer const& transfer)
{
	add(transfer);
	return *this;
}

TransfersManager& TransfersManager::operator<<(File::SharedPtr const& file)
{
	add(file);
	return *this;
}

Transfer& TransfersManager::operator[](Hash const& hash)
{
	for (Transfer& t: m_transfers) {
		if (hash == t.file()->rootHash()) {
			return t;
		}
	}

	throw NotFoundException();
}

void TransfersManager::maxDownloadSpeed(unsigned int rate)
{
	m_maxDownloadSpeed = rate;
	applyRates();
}

void TransfersManager::maxUploadSpeed(unsigned int rate)
{
	m_maxUploadSpeed = rate;
	applyRates();
}

void TransfersManager::applyRates()
{
	unsigned int uploadRate = m_maxDownloadSpeed / m_transfers.size();
	unsigned int downloadRate = m_maxUploadSpeed / m_transfers.size();

	for (Transfer& t: m_transfers) {
		t.maxUploadSpeed(uploadRate);
		t.maxDownloadSpeed(downloadRate);
	}
}

void TransfersManager::updateRates()
{
	m_downloadSpeed = 0;
	m_uploadSpeed = 0;

	for (Transfer& t: m_transfers) {
		m_downloadSpeed += t.downloadSpeed();
		m_uploadSpeed += t.uploadSpeed();
		t.resetRates();
	}
}

void TransfersManager::run()
{
	for (Transfer& t: m_transfers) {
		//
	}
}

}
