#ifndef ADAPTERTIMESTREAM_H
#define ADAPTERTIMESTREAM_H

#include "pelican/adapters/AbstractStreamAdapter.h"
#include "LofarUdpHeader.h"
#include <complex>

/**
 * @file AdapterTimeStream.h
 */

namespace pelican {

class ConfigNode;
class TimeStreamData;

namespace lofar {


/**
 * @class AdapterTimeStream
 *
 * @brief
 * Adapter to deserialise time stream data chunks from a lofar station.
 *
 * @details
 * Stream adapter to deserialise time stream data chunks from a lofar station.
 *
 * \section Configuration:
 *
 * Example configuration node:
 *
 * \verbatim
 *		<AdapterTimeStream name="">
 *			<packetsPerChunk number=""/>
 *			<subbands number=""/>
 *			<polarisations number=""/>
 *			<samples number=""/>
 *			<sampleSize bits=""/>
 *		<\AdapterTimeStream>
 * \verbatim
 *
 * - packetsPerChunk: Number of UDP packets in each input data chunk.
 * - subbands: Number of sub-bands per packet.
 * - polarisations: Number of polarisations per packet.
 * - samples: Number of (time) samples per packet.
 * - sampleSize: Number of bits per sample. (Samples are assumed to be complex
 *               pairs of the number of bits specified).
 *
 */

class AdapterTimeStream : public AbstractStreamAdapter
{
    private:
        friend class AdapterTimeStreamTest;

    public:
        /// Constructs a new AdapterTimeStream.
        AdapterTimeStream(const ConfigNode& config);

        /// Destroys the AdapterTimeStream.
        ~AdapterTimeStream() {}

    protected:
        /// Method to deserialise a LOFAR time stream data.
        void deserialise(QIODevice* in);

    private:
        /// Updates and checks the size of the time stream data.
        void _checkData();

        /// Read the udp packet header from a buffer read from the IO device.
        void _readHeader(UDPPacket::Header& header, char* buffer);

        /// Reads the udp data data section into the data blob data array.
        void _readData(std::complex<double>* data, char* buffer);

        /// Updates dimensions of the time stream data being deserialised.
        void _updateDimensions();

        /// Prints the header to standard out (for debugging).
        void _printHeader(const UDPPacket::Header& header);

    private:
        TimeStreamData* _timeData;
        unsigned _nUDPPackets;
        unsigned _nSubbands;
        unsigned _nPolarisations;
        unsigned _nSamples;
        unsigned _sampleBits;
};

} // namespace lofar
} // namespace pelican

#endif // ADAPTERTIMESTREAM_H