#include "basic.h"
#include "ftp.h"
#include "ftp_session.h"
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

FtpServer::FtpServer(EventLoop* lo,
                             const InetAddress& listenAddr)
  : server_(lo, listenAddr, "FtpServer")
{
  loop = lo;
  server_.setConnectionCallback(
      boost::bind(&FtpServer::onConnection, this, _1));
  server_.setMessageCallback(
      boost::bind(&FtpServer::onMessage, this, _1, _2, _3));
}

void FtpServer::start()
{
  server_.start();
}

void FtpServer::onConnection(const TcpConnectionPtr& conn)
{
  LOG_INFO << "FtpServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");

  mapSession_.insert( std::make_pair(conn, FtpSessionPtr(new FtpSession( loop, const_cast<TcpConnectionPtr&>(conn) )) ));

  StringPiece buf("220 the serviceis ready(khaki ftpd1.0.0 free software) \r\n");
  conn->send( buf );
}

void FtpServer::onMessage(const TcpConnectionPtr& conn,
                              Buffer* buf,
                              Timestamp time)
{
  string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " discards " << msg.size()
           << " bytes received at " << time.toString();
  LOG_INFO << "msg : " << msg ;

  FtpCommand stcCmd( msg.c_str() );

  MapLinkSession::iterator it = mapSession_.find( const_cast<TcpConnectionPtr&>(conn) );
  if ( it != mapSession_.end() )
  {
    it->second->ExecuteCmd( stcCmd );
  }
}
