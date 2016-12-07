#ifndef MUDUO_FTP_SESSION_H
#define MUDUO_FTP_SESSION_H

#include "basic.h"
#include "util.h"

#include <string>
#include <map>

#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpClient.h>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

namespace muduo
{
namespace net
{

///////////////////////////////////
class PasvServer
{
public:
    PasvServer(muduo::net::EventLoop* loop,
               const muduo::net::InetAddress& listenAddr);

    void start();
    void send( StringPiece& str );
private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn);

    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp time);

    TcpServer server_;
    TcpConnectionPtr conn_;
};

////////////////////////////////////
class PortClient
{
public:
    PortClient(muduo::net::EventLoop* loop,
                const muduo::net::InetAddress& serverAddr);

    void start();
private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn);

    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp time);
private:
    TcpClient client_;
};

////////////////////////////////////
class FtpSession
{
public:
    FtpSession( EventLoop* loop, TcpConnectionPtr co );

    ~FtpSession();

    typedef void (FtpSession::*cmd_callback)( FtpCommand& cmd );
    cmd_callback FindCallBack( std::string& strCmd );

    void ExecuteCmd( FtpCommand& cmd );
private:
    typedef boost::shared_ptr<PasvServer> PasvServerPtr;
    typedef boost::weak_ptr<PasvServer>   PasvWeakPtr;
    typedef boost::shared_ptr<PortClient> PortClientPtr;

    typedef std::map< std::string, cmd_callback > MapHandlerContainer;
    MapHandlerContainer mapHandlerFunc;

    typedef std::map< int, std::string > MapResponseContainer;
    MapResponseContainer mapResponse;

    void InitOpcodeHandler();
    void InitReponseCode();

    FtpClient        cliRole;
    EventLoop*       loop;
    TcpConnectionPtr conn;
    PasvServerPtr    pasvSer;
    PortClientPtr    portCli;

private:
    void HandlerFtpAuth(FtpCommand& cmd);
    void HandlerFtpPort(FtpCommand& cmd);
    void HandlerFtpPasv(FtpCommand& cmd);
    void HandlerFtpList(FtpCommand& cmd);
    void HandlerFtpQuit(FtpCommand& cmd);
    void HandlerFtpPwd(FtpCommand&cmd);
    void HandlerFtpCwd(FtpCommand&cmd);
    void HandlerFtpUser(FtpCommand& cmd);
    void HandlerFtpPass(FtpCommand& cmd);
    void HandlerFtpSyst(FtpCommand& cmd);
    void HandlerFtpDele(FtpCommand& cmd);
    void HandlerFtpRetr(FtpCommand& cmd);
    void HandlerFtpSize(FtpCommand& cmd);
    void HandlerFtpRmd(FtpCommand& cmd);
    void HandlerFtpStor(FtpCommand& cmd);
    void HandlerFtpNlst(FtpCommand& cmd);
    void HandlerFtpMkd(FtpCommand& cmd);
    void HandlerFtpType(FtpCommand& cmd);
    void HandlerFtpCdup(FtpCommand& cmd);
};
}
}
#endif //MUDUO_FTP_SESSION_H
