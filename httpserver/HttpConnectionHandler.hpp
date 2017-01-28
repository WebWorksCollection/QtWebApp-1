#ifndef HTTPCONNECTIONHANDLER_HPP
#define HTTPCONNECTIONHANDLER_HPP

#ifndef QT_NO_OPENSSL
   #include <QSslConfiguration>
#endif

#include <QTcpSocket>
#include <QTimer>
#include <QThread>
#include "HttpGlobal.hpp"
#include "HttpRequest.hpp"
#include "httprequesthandler.h"
#include "HttpServerSettings.hpp"

namespace stefanfrings {

/** Alias type definition, for compatibility to different Qt versions */
#if QT_VERSION >= 0x050000
    using tSocketDescriptor = qintptr;
#else
    using tSocketDescriptor = int;
#endif

/** Alias for QSslConfiguration if OpenSSL is not supported */
#ifdef QT_NO_OPENSSL
  #define QSslConfiguration QObject
#endif

/**
  The connection handler accepts incoming connections and dispatches incoming requests to to a
  request mapper. Since HTTP clients can send multiple requests before waiting for the response,
  the incoming requests are queued and processed one after the other.
  <p>
  Example for the required configuration settings:
  <code><pre>
  readTimeout=60000
  maxRequestSize=16000
  maxMultiPartSize=1000000
  </pre></code>
  <p>
  The readTimeout value defines the maximum time to wait for a complete HTTP request.
  @see HttpRequest for description of config settings maxRequestSize and maxMultiPartSize.
*/
class DECLSPEC HttpConnectionHandler : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(HttpConnectionHandler)

public:

    /**
      Constructor.
      @param settings Configuration settings of the HTTP webserver
      @param requestHandler Handler that will process each incoming HTTP request
      @param sslConfiguration SSL (HTTPS) will be used if not `nullptr`
    */
    HttpConnectionHandler(HttpServerSettings *settings, HttpRequestHandler *requestHandler, QSslConfiguration *sslConfiguration = nullptr);

    /** Destructor */
    virtual ~HttpConnectionHandler();

    /** Returns true, if this handler is in use. */
    bool isBusy();

    /** Mark this handler as busy */
    void setBusy();

private:

    /** Configuration settings */
    HttpServerSettings *settings = nullptr;

    /** TCP socket of the current connection  */
    QTcpSocket *socket = nullptr;

    /** Time for read timeout detection */
    QTimer readTimer;

    /** Storage for the current incoming HTTP request */
    HttpRequest *currentRequest = nullptr;

    /** Dispatches received requests to services */
    HttpRequestHandler *requestHandler = nullptr;

    /** This shows the busy-state from a very early time */
    bool busy = false;

    /** Configuration for SSL */
    QSslConfiguration *sslConfiguration = nullptr;

    /** Executes the threads own event loop */
    void run();

    /**  Create SSL or TCP socket */
    void createSocket();

public slots:

    /**
      Received from from the listener, when the handler shall start processing a new connection.
      @param socketDescriptor references the accepted connection.
    */
    void handleConnection(tSocketDescriptor socketDescriptor);

private slots:

    /** Received from the socket when a read-timeout occured */
    void readTimeout();

    /** Received from the socket when incoming data can be read */
    void read();

    /** Received from the socket when a connection has been closed */
    void disconnected();

};

} // end of namespace

#endif // HTTPCONNECTIONHANDLER_HPP
