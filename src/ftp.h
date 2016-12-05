#ifndef MUDUO_FTP_H
#define MUDUO_FTP_H

#include <set>
#include <muduo/net/TcpServer.h>
#include "ftp_session.h"

// RFC
class FtpServer
{
 public:
	FtpServer(muduo::net::EventLoop* loop,
                const muduo::net::InetAddress& listenAddr);

  void start();

 private:
  void onConnection(const muduo::net::TcpConnectionPtr& conn);

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp time);

  typedef boost::shared_ptr<muduo::net::FtpSession> FtpSessionPtr;
  typedef std::map<muduo::net::TcpConnectionPtr, FtpSessionPtr> MapLinkSession;

  muduo::net::TcpServer server_;
  MapLinkSession mapSession_;
};

#endif  // MUDUO_FTP_H
