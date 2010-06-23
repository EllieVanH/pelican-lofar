// Pelican stuff
#include "pelican/comms/PelicanClientProtocol.h"
#include "pelican/comms/StreamDataRequest.h"
#include "pelican/data/DataRequirements.h"
#include "pelican/comms/DataBlobResponse.h"
#include "pelican/comms/ServerResponse.h"

// Pelican-Lofar stuff
#include "PelicanBlobClient.h"

// QT stuff
#include "QtNetwork/QHostAddress"
#include "QtCore/QByteArray"

// Boost stuff
#include <boost/shared_ptr.hpp>

// C++ stuff
#include <iostream>

namespace pelican {
namespace lofar {

// class PelicanBlobClient
PelicanBlobClient::PelicanBlobClient(QString blobType, QString server, quint16 port)
    : _server(server), _blobType(blobType), _port(port)
{
    _protocol = new PelicanClientProtocol;
    _tcpSocket = new QTcpSocket();
    connectToLofarPelican();
}

// Destructor
PelicanBlobClient::~PelicanBlobClient()
{
    delete _tcpSocket;
    delete _protocol;
}

// Read data from the Pelican Lofar pipeline
void PelicanBlobClient::getData()
{
    // Check that we are still connected to server, if not reconnect
    if (_tcpSocket -> state() == QAbstractSocket::UnconnectedState) {
        std::cout << "Disconnected from Lofar, reconnecting" << std::endl;
        connectToLofarPelican();
    }

    // Wait for data to be available to socket, and read
    _tcpSocket -> waitForReadyRead();
    boost::shared_ptr<ServerResponse> r = _protocol -> receive(*_tcpSocket);

    // Check what type of response we have
    switch( r -> type() ) {
        case ServerResponse::Error:  // Error occured!!
            std::cerr << "PelicanBlobClient: Server Error: " << r -> message().toStdString() << std::endl;
            break;
        case ServerResponse::Blob:   // We have received  Blob
            {
                DataBlobResponse* res = static_cast<DataBlobResponse*>(r.get());
                std::cout << res -> dataName().toStdString();
            }
        case ServerResponse::StreamData:   // We have stream data
            break;
        case ServerResponse::ServiceData:  // We have service data
            break;
        default:
            std::cerr << "PelicanBlobClient: Unknown Response" << std::endl;
            break;
    }
}

// Connect to Pelican Lofar and register requested data type
void PelicanBlobClient::connectToLofarPelican()
{
    while (_tcpSocket -> state() == QAbstractSocket::UnconnectedState) {
        QHostAddress serverAddress(_server);
        _tcpSocket -> connectToHost( serverAddress, _port );
        if (!_tcpSocket -> waitForConnected(5000) || _tcpSocket -> state() == QAbstractSocket::UnconnectedState) {
           std::cerr << "Client could not connect to server" << std::endl;
           sleep(2);
           continue;
        }

        // Define the data type which the client will except and send request
        StreamDataRequest req;
        DataRequirements require;
        require.setStreamData(_blobType);
        req.addDataOption(require);

        QByteArray data = _protocol -> serialise(req);
        _tcpSocket -> write(data);
        _tcpSocket -> waitForBytesWritten(data.size());
        _tcpSocket -> flush();
    }
}

} //namespace lofar
} // namespace pelican
