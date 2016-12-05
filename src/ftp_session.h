#ifndef MUDUO_FTP_SESSION_H
#define MUDUO_FTP_SESSION_H

#include "basic.h"

#include <string>
#include <map>

#include <muduo/net/TcpServer.h>

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

namespace muduo
{
namespace net
{

class FtpSession
{
public:
    FtpSession( const TcpConnectionPtr& co );

    ~FtpSession();

    typedef void (FtpSession::*cmd_callback)( FtpCommand& cmd );
    cmd_callback FindCallBack( std::string& strCmd );

    void ExecuteCmd( FtpCommand& cmd );
private:
    typedef boost::function<void(std::string &)> FtpCallback;

    typedef std::map< std::string, cmd_callback > mapHandlerContainer;
    mapHandlerContainer mapHandlerFunc;

    void InitOpcodeHandler();

    FtpClient cliRole;
    const TcpConnectionPtr& conn;
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
