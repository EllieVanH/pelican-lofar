#include "DedispersionSpectra.h"


namespace pelican {

namespace lofar {


/**
 *@details DedispersionSpectra 
 */
DedispersionSpectra::DedispersionSpectra()
    : DataBlob("DedispersionSpectra"), _timeBins(0)
{
}

/**
 *@details
 */
DedispersionSpectra::~DedispersionSpectra()
{
}

void DedispersionSpectra::resize( unsigned timebins, unsigned dedispersionBins,
                                  float dedispersionBinStart, float dedispersionBinWidth ) { 
    _timeBins = timebins;
    _dmBin.reset( dedispersionBins );
    _dmBin.setStart( dedispersionBinStart );
    _dmBin.setBinWidth( dedispersionBinWidth );
    _data.resize( timebins*dedispersionBins ); 
}

float DedispersionSpectra::dmAmplitude( unsigned timeSlice, float dm ) const {
    unsigned dm_index = dmIndex(dm);
    return _data[timeSlice + _timeBins * dm_index];
}

unsigned DedispersionSpectra::dmIndex( float dm ) const {
    return _dmBin.binIndex(dm);
}

float DedispersionSpectra::dm( unsigned dm ) const {
    return _dmBin.binAssignmentNumber( dm );
}

void DedispersionSpectra::setInputDataBlobs( const QList< WeightedSpectrumDataSet* >& blobs ) {
    _inputBlobs = blobs;
}

} // namespace lofar
} // namespace pelican
