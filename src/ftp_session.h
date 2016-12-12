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

/////////////////TransferData//////////////////
class TransferData
{
public:
    enum Mode{
        E_TRANSFER_TYPE_PASV = 0,
        E_TRANSFER_TYPE_PORT = 1,
    };

    typedef boost::function<void ()> DisconnectCallback;

    TransferData(muduo::net::EventLoop* loop, FtpClient* cli);

    void start();
    void createDataChannel( const muduo::net::InetAddress& addr, int mode );
    void setDisconnectCallback( const DisconnectCallback& cb )
    { cb_ = cb; }

    void send( StringPiece& str );
    void close();

    void setMode( bool mode )
    { mode_ = mode; }

    bool getMode()
    { return mode_; }
private:

    typedef boost::weak_ptr<TcpConnection> TcpConnWeakPtr;
    typedef boost::shared_ptr<TcpServer>   TcpServerPtr;
    typedef boost::shared_ptr<TcpClient>   TcpClientPtr;

    void onConnection(const muduo::net::TcpConnectionPtr& conn);

    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp time);
    void onWriteComplete(const TcpConnectionPtr& conn);

    bool      mode_;
    TcpServerPtr server_;
    TcpClientPtr client_;
    TcpConnectionPtr conn_;
    muduo::net::EventLoop* loop_;
    FtpClient* cli_;
    DisconnectCallback cb_;
};

///////////////FtpSession////////////////
class FtpSession
{
public:
    enum {
        E_FTP_SESSION_CMD_STOR = 1,
        E_FTP_SESSION_CMD_RETR = 2,
    };
    FtpSession( EventLoop* loop, TcpConnectionPtr co );

    ~FtpSession();

    typedef void (FtpSession::*cmd_callback)( FtpCommand& cmd );
    cmd_callback FindCallBack( std::string& strCmd );

    void ExecuteCmd( FtpCommand& cmd );

private:
    typedef std::map< std::string, cmd_callback > MapHandlerContainer;
    MapHandlerContainer mapHandlerFunc;

    typedef std::map< int, std::string > MapResponseContainer;
    MapResponseContainer mapResponse;

    void InitOpcodeHandler();
    void InitReponseCode();
    void DisconnectCallBack();

    FtpClient        cli_;
    EventLoop*       loop_;
    TcpConnectionPtr conn_;
    TransferData     data_;

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
