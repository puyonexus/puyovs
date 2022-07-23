#include "pcmreader.h"

namespace alib
{

struct PCMReader::Priv
{
	BinaryStream* dataStream;
	int channels, freq;
};

PCMReader::PCMReader(BinaryStream* dataStream, int channels, int freq)
	: p(new Priv)
{
	p->dataStream = dataStream;
	p->channels = channels;
	p->freq = freq;
}

PCMReader::~PCMReader()
{
	delete p->dataStream;
	delete p;
}

void PCMReader::read(float* buffer, int& bufferFrames)
{
	const int frameSize = (p->channels * sizeof(float));

	const int bytesRead = p->dataStream->read(buffer, bufferFrames * frameSize);

	bufferFrames = bytesRead / frameSize;
}

void PCMReader::reset()
{
	p->dataStream->rewind();
}

bool PCMReader::atEnd() const
{
	return p->dataStream->eof();
}

bool PCMReader::haveEnd() const
{
	return p->dataStream->hasEof();
}

bool PCMReader::error() const
{
	return p->dataStream->error();
}

int PCMReader::numChannels() const
{
	return p->channels;
}

int PCMReader::sampleRate() const
{
	return p->freq;
}

bool PCMReader::hasLooped()
{
	return false;
}

}
