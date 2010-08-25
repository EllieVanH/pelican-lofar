#include "test/PPFChanneliserTest.h"

#include "PPFChanneliser.h"
#include "SubbandSpectra.h"
#include "SubbandTimeSeries.h"

#include "pelican/utility/ConfigNode.h"

#include <QtCore/QTime>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <iostream>
#include <complex>
#include <vector>
#include <cmath>

using std::cout;
using std::endl;

namespace pelican {
namespace lofar {

CPPUNIT_TEST_SUITE_REGISTRATION(PPFChanneliserTest);


void PPFChanneliserTest::setUp()
{
    _verbose = false;

    _nChannels = 16;
    _nSubbands = 62;
    _nPols = 2;
    _nTaps = 8;

    unsigned timesPerChunk =  512 * 1000;

    if (timesPerChunk%_nChannels) CPPUNIT_FAIL("Setup error");

    _nBlocks = timesPerChunk / _nChannels;

//    cout << "---------- PPF Channeliser test ---------- " << endl;
//    cout << "nChunks = " << _nChunks << endl;
//    cout << "timesPerChunk = " << timesPerChunk << endl;
//    cout << "- nBlocks = " << _nBlocks << endl;
//    cout << "- _nSubbands = " << _nSubbands << endl;
//    cout << "- nPols = " << _nPols << endl;
//    cout << "- nChannels = " << _nChannels << endl;
}


/**
 * @details
 * Method to test the module construction and configuration.
 */
void PPFChanneliserTest::test_configuration()
{
    unsigned nThreads = 1;
    try {
        ConfigNode config(_configXml(_nChannels, nThreads, _nTaps));
        PPFChanneliser channeliser(config);
        CPPUNIT_ASSERT_EQUAL(_nChannels, channeliser._nChannels);
        CPPUNIT_ASSERT_EQUAL(nThreads, channeliser._nThreads);
    }
    catch (QString err) {
        CPPUNIT_FAIL(err.toLatin1().data());
    }
}


/**
 * @details
 */
void PPFChanneliserTest::test_threadAssign()
{
    unsigned nThreads = 2;
    ConfigNode config(_configXml(_nChannels, nThreads, _nTaps));
    PPFChanneliser channeliser(config);
    try {
        unsigned start = 0, end = 0;
        unsigned nSubbands = 62;
        unsigned threadId = 0;

        channeliser._threadProcessingIndices(start, end, nSubbands, nThreads, threadId);
        CPPUNIT_ASSERT_EQUAL(unsigned(0), start);
        CPPUNIT_ASSERT_EQUAL(unsigned(31), end);

        threadId = 1;
        channeliser._threadProcessingIndices(start, end, nSubbands, nThreads, threadId);
        CPPUNIT_ASSERT_EQUAL(unsigned(31), start);
        CPPUNIT_ASSERT_EQUAL(unsigned(62), end);

        nSubbands = 1;
        threadId = 0;
        channeliser._threadProcessingIndices(start, end, nSubbands, nThreads, threadId);
        CPPUNIT_ASSERT_EQUAL(unsigned(0), start);
        CPPUNIT_ASSERT_EQUAL(unsigned(1), end);

        threadId = 1;
        channeliser._threadProcessingIndices(start, end, nSubbands, nThreads, threadId);
        CPPUNIT_ASSERT_EQUAL(unsigned(0), start);
        CPPUNIT_ASSERT_EQUAL(unsigned(0), end);

        nSubbands = 3;
        threadId = 0;
        channeliser._threadProcessingIndices(start, end, nSubbands, nThreads, threadId);
        CPPUNIT_ASSERT_EQUAL(unsigned(0), start);
        CPPUNIT_ASSERT_EQUAL(unsigned(2), end);

        threadId = 1;
        channeliser._threadProcessingIndices(start, end, nSubbands, nThreads, threadId);
        CPPUNIT_ASSERT_EQUAL(unsigned(2), start);
        CPPUNIT_ASSERT_EQUAL(unsigned(3), end);

        nSubbands = 4;
        threadId = 0;
        channeliser._threadProcessingIndices(start, end, nSubbands, nThreads, threadId);
        CPPUNIT_ASSERT_EQUAL(unsigned(0), start);
        CPPUNIT_ASSERT_EQUAL(unsigned(2), end);

        threadId = 1;
        channeliser._threadProcessingIndices(start, end, nSubbands, nThreads, threadId);
        CPPUNIT_ASSERT_EQUAL(unsigned(2), start);
        CPPUNIT_ASSERT_EQUAL(unsigned(4), end);

        nSubbands = 4;
        nThreads = 3;
        threadId = 0;
        channeliser._threadProcessingIndices(start, end, nSubbands, nThreads, threadId);
        CPPUNIT_ASSERT_EQUAL(unsigned(0), start);
        CPPUNIT_ASSERT_EQUAL(unsigned(2), end);

        threadId = 1;
        channeliser._threadProcessingIndices(start, end, nSubbands, nThreads, threadId);
        CPPUNIT_ASSERT_EQUAL(unsigned(2), start);
        CPPUNIT_ASSERT_EQUAL(unsigned(3), end);

        threadId = 2;
        channeliser._threadProcessingIndices(start, end, nSubbands, nThreads, threadId);
        CPPUNIT_ASSERT_EQUAL(unsigned(3), start);
        CPPUNIT_ASSERT_EQUAL(unsigned(4), end);
    }
    catch (QString const& err) {
        CPPUNIT_FAIL(err.toLatin1().data());
    }

}


/**
 * @details
 * Test updating the delay buffering.
 */
void PPFChanneliserTest::test_updateBuffer()
{
    try {
        // Setup the channeliser.
        unsigned nThreads = 1;
        ConfigNode config(_configXml(_nChannels, nThreads, _nTaps));
        PPFChanneliser channeliser(config);

        // Setup the work buffers.
        channeliser._setupWorkBuffers(_nSubbands, _nPols, _nChannels, _nTaps);

        // Create a vector of input samples to test with.
        std::vector<PPFChanneliser::Complex> sampleBuffer(_nChannels * _nSubbands
                * _nPols * _nBlocks);

        // Local pointers to buffers.
        PPFChanneliser::Complex* subbandBuffer;
        PPFChanneliser::Complex* newSamples;

        // Iterate over the update buffer method to time it.
        QTime timer;
        timer.start();
        for (unsigned b = 0; b < _nBlocks; ++b)
        {
            for (unsigned s = 0; s < _nSubbands; ++s)
            {
                for (unsigned p = 0; p < _nPols; ++p)
                {
                    unsigned i = _nChannels * (p +  _nPols * (s + _nSubbands * b));
                    newSamples = &sampleBuffer[i];;
                    subbandBuffer = &(channeliser._workBuffer[s * _nPols + p])[0];
                    channeliser._updateBuffer(newSamples, _nChannels, _nTaps, subbandBuffer);
                }
            }
        }
        int elapsed = timer.elapsed();

        cout << endl;
        cout << "-------------------------------------------------" << endl;
        cout << "[PPFChanneliser]: _updateBuffer() " << endl;
        cout << "- nChan = " << _nChannels << endl << endl;
        if (_verbose) {
            cout << "- nTaps = " << _nTaps << endl;
            cout << "- nBlocks = " << _nBlocks << endl;
            cout << "- nSubbands = " << _nSubbands << endl;
            cout << "- nPols = " << _nPols << endl;
        }
        cout << "* Elapsed = " << elapsed << " ms. [" << nThreads << " threads]";
        cout << " (data time = " << _nBlocks * _nChannels * 5e-3 << " ms.)" << endl;
        cout << "-------------------------------------------------" << endl;
    }
    catch (QString const& err)
    {
        CPPUNIT_FAIL(err.toLatin1().data());
    }
}


/**
 * @details
 * Test the FIR filter stage.
 */
void PPFChanneliserTest::test_filter()
{
    // Setup the channeliser.
    unsigned nThreads = 1;
    ConfigNode config(_configXml(_nChannels, nThreads, _nTaps));
    PPFChanneliser channeliser(config);

    // Setup work buffers.
    channeliser._setupWorkBuffers(_nSubbands, _nPols, _nChannels, _nTaps);

    std::vector<PPFChanneliser::Complex> filteredData(_nChannels);
    PPFChanneliser::Complex* filteredSamples = &filteredData[0];
    PPFChanneliser::Complex* workBuffer;

    double const* coeff = channeliser._ppfCoeffs.ptr();
    unsigned nCoeffs =  channeliser._ppfCoeffs.size();
    float* fCoeffs = new float[nCoeffs];
    for (unsigned i = 0; i < nCoeffs; ++i) fCoeffs[i] = float(coeff[i]);

    QTime timer;
    timer.start();
    for (unsigned b = 0; b < _nBlocks; ++b) {
        for (unsigned s = 0; s < _nSubbands; ++s) {
            for (unsigned p = 0; p < _nPols; ++p) {
                workBuffer = &(channeliser._workBuffer[s * _nPols + p])[0];
                channeliser._filter(workBuffer, _nTaps, _nChannels, fCoeffs,
                        filteredSamples);
            }
        }
    }
    int elapsed = timer.elapsed();

    cout << endl;
    cout << "-------------------------------------------------" << endl;
    cout << "[PPFChanneliser]: _filter() " << endl;
    cout << "- nChan = " << _nChannels << endl << endl;
    if (_verbose) {
        cout << "- nTaps = " << _nTaps << endl;
        cout << "- nBlocks = " << _nBlocks << endl;
        cout << "- nSubbands = " << _nSubbands << endl;
        cout << "- nPols = " << _nPols << endl;
    }
    cout << "* Elapsed = " << elapsed << " ms. [" << nThreads << " threads]";
    cout << " (data time = " << _nBlocks * _nChannels * 5e-3 << " ms.)" << endl;
    cout << "-------------------------------------------------" << endl;
}


/**
 * @details
 * Test the fft stage.
 */
void PPFChanneliserTest::test_fft()
{
    // Setup the channeliser.
    unsigned nThreads = 1;
    ConfigNode config(_configXml(_nChannels, nThreads, _nTaps));
    PPFChanneliser channeliser(config);

    SubbandSpectraC32 spectra;
    spectra.resize(_nBlocks, _nSubbands, _nPols);
    std::vector<PPFChanneliser::Complex> filteredData(_nChannels);
    const PPFChanneliser::Complex* filteredSamples = &filteredData[0];

    QTime timer;
    timer.start();
    for (unsigned b = 0; b < _nBlocks; ++b) {
        for (unsigned s = 0; s < _nSubbands; ++s) {
            for (unsigned p = 0; p < _nPols; ++p) {

                Spectrum<PPFChanneliser::Complex>* spectrum = spectra.ptr(b, s, p);
                spectrum->resize(_nChannels);
                PPFChanneliser::Complex* spectrumData = spectrum->ptr();
                channeliser._fft(filteredSamples, spectrumData);

            }
        }
    }
    int elapsed = timer.elapsed();

    cout << endl;
    cout << "-------------------------------------------------" << endl;
    cout << "[PPFChanneliser]: _fft() " << endl;
    cout << "- nChan = " << _nChannels << endl << endl;
    if (_verbose) {
        cout << "- nTaps = " << _nTaps << endl;
        cout << "- nBlocks = " << _nBlocks << endl;
        cout << "- nSubbands = " << _nSubbands << endl;
        cout << "- nPols = " << _nPols << endl;
    }
    cout << "* Elapsed = " << elapsed << " ms. [" << nThreads << " threads]";
    cout << " (data time = " << _nBlocks * _nChannels * 5e-3 << " ms.)" << endl;
    cout << "-------------------------------------------------" << endl;
}


/**
 * @details
 * Test the run method.
 */
void PPFChanneliserTest::test_run()
{
    // Setup the channeliser.
  unsigned nThreads = 4;
    ConfigNode config(_configXml(_nChannels, nThreads, _nTaps));
    PPFChanneliser channeliser(config);

    SubbandSpectraC32 spectra;
    spectra.resize(_nBlocks, _nSubbands, _nPols);
    SubbandTimeSeriesC32 timeSeries;
    timeSeries.resize(_nBlocks, _nSubbands, _nPols, _nChannels);

    unsigned iter = 1;

    QTime timer;
    timer.start();
    for (unsigned i = 0; i < iter; ++i)
      channeliser.run(&timeSeries, &spectra);
    int elapsed = timer.elapsed();

    cout << endl;
    cout << "-------------------------------------------------" << endl;
    cout << "[PPFChanneliser]: run() " << endl;
    cout << "- nChan = " << _nChannels << endl << endl;
    if (_verbose) {
        cout << "- nTaps = " << _nTaps << endl;
        cout << "- nBlocks = " << _nBlocks << endl;
        cout << "- nSubbands = " << _nSubbands << endl;
        cout << "- nPols = " << _nPols << endl;
    }
    cout << "* Elapsed = " << elapsed << " ms. [" << nThreads << " threads]";
    cout << " (data time = " << _nBlocks * _nChannels * 5e-3 << " ms.)" << endl;
    cout << "-------------------------------------------------" << endl;
}



/**
 * @details
 * Contruct a spectrum and save it to file.
 */
void PPFChanneliserTest::test_makeSpectrum()
{
    // Setup the channeliser.
    unsigned nThreads = 1;
    _nChannels = 64;

    ConfigNode config(_configXml(_nChannels, nThreads, _nTaps));
    PPFChanneliser channeliser(config);

    double freq = 10.12; // Hz
    double sampleRate = 50.0; // Hz

    SubbandTimeSeriesC32 data;
    _nSubbands = 1;
    _nPols = 1;
    data.resize(_nBlocks, _nSubbands, _nPols, _nChannels);

    // Generate signal.
    for (unsigned i = 0, t = 0; t < _nBlocks; ++t) {

        PPFChanneliser::Complex* timeData = data.ptr(t, 0, 0)->ptr();

        for (unsigned c = 0; c < _nChannels; ++c) {
            double time = double(i) / sampleRate;
            double re = std::cos(2 * math::pi * freq * time);
            double im = std::sin(2 * math::pi * freq * time);
            timeData[c] = PPFChanneliser::Complex(re, im);
            i++;
        }
    }

    SubbandSpectraC32 spectra;
    spectra.resize(_nBlocks, _nSubbands, _nPols, _nChannels);

    // PPF - have to run enough times for buffer to fill with new signal.
    channeliser.run(&data, &spectra);

    // Write the last spectrum.
    QFile file("spectrum.dat");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    PPFChanneliser::Complex* spectrum = spectra.ptr(_nBlocks-1, 0, 0)->ptr();
    QTextStream out(&file);
    double maxFreq = sampleRate / 2.0;
    double freqInc = sampleRate / _nChannels;
    for (unsigned i = 0; i < _nChannels; ++i) {
        out << i * freqInc - maxFreq << " "
            <<	20 * std::log10(std::abs(spectrum[i])) << " "
            << std::abs(spectrum[i]) << " "
            << spectrum[i].real() << " "
            << spectrum[i].imag() << endl;
    }
    file.close();
}


/**
 * @details
 * Test to generate a channel profile.
 */
void PPFChanneliserTest::test_channelProfile()
{
    // Options.
    unsigned nThreads = 1;
    QString coeffFile = "";

    unsigned nProfiles = 2;
    double sampleRate = 50.0e6; // Hz
    double startFreq = 8.0e6; // Hz
    unsigned nSteps = 1000;
    double freqInc = 0.01e6;
    std::vector<double> freqs(nSteps);

    double endFreq = startFreq + freqInc * nSteps;
    double midTestFreq = startFreq + (endFreq - startFreq) / 2.0;
    std::cout << "scanning freqs " << startFreq << " -> " << endFreq << " ("
              << midTestFreq << ")" << std::endl;

    //	unsigned testChannelIndex = nChannels / 2 + std::floor(midTestFreq / channelDelta);
    std::vector<unsigned> testIndices(nProfiles);
    testIndices[0] = 45;
    testIndices[1] = 46;

    try {
        ConfigNode config(_configXml(_nChannels, nThreads, _nTaps));
        PPFChanneliser channeliser(config);

        unsigned _nBlocks = _nTaps;

        SubbandTimeSeriesC32 data;
        data.resize(_nBlocks, _nSubbands, _nPols, _nChannels);

        SubbandSpectraC32 spectra;
        spectra.resize(_nBlocks, _nSubbands, _nPols, _nChannels);

        std::vector<std::vector<PPFChanneliser::Complex> > channelProfile;
        channelProfile.resize(nProfiles);
        for (unsigned i = 0; i < nProfiles; ++i) channelProfile[i].resize(nSteps);

        // Scan frequencies to generate channel profile.
        for (unsigned k = 0; k < nSteps; ++k) {

            // Generate signal.
            double freq = startFreq + k * freqInc;
            freqs[k] = freq;
            for (unsigned i = 0, t = 0; t < _nBlocks; ++t) {
                PPFChanneliser::Complex* timeData = data.ptr(t, 0, 0)->ptr();
                for (unsigned c = 0; c < _nChannels; ++c) {
                    double time = double(i) / sampleRate;
                    double re = std::cos(2 * math::pi * freq * time);
                    double im = std::sin(2 * math::pi * freq * time);
                    timeData[c] = PPFChanneliser::Complex(re, im);
                    i++;
                }
            }

            channeliser.run(&data, &spectra);

            // Save the amplitude of the specified channel.
            PPFChanneliser::Complex* spectrum =
                    spectra.ptr(_nBlocks-1, 0, 0)->ptr();
            for (unsigned p = 0; p < nProfiles; ++p) {
                channelProfile[p][k] = spectrum[testIndices[p]];
            }
        }

        // Write the channel profile to file.
        QFile file("channelProfileGeneratedCoeffs.dat");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return;
        }
        QTextStream out(&file);
        for (unsigned i = 0; i < nSteps; ++i) {
            out << freqs[i] << " ";
            for (unsigned p = 0; p < nProfiles; ++p) {
                out << 20 * std::log10(std::abs(channelProfile[p][i])) << " ";
            }
            out << endl;
        }
        file.close();
    }
    catch (QString err) {
        std::cout << err.toStdString() << std::endl;
    }
}



/**
 * @details
 *
 * @param nChannels
 * @return
 */
QString PPFChanneliserTest::_configXml(unsigned nChannels,
        unsigned nThreads, unsigned nTaps, const QString& windowType)
{
    QString xml =
            "<PPFChanneliser>"
            "	<outputChannelsPerSubband value=\"" + QString::number(nChannels) + "\"/>"
            "	<processingThreads value=\"" + QString::number(nThreads) + "\"/>"
            "	<filter nTaps=\"" + QString::number(nTaps) + "\" filterWindow=\"" + windowType + "\"/>"
            "</PPFChanneliser>";
    return xml;
}


} // namespace lofar
} // namespace pelican
