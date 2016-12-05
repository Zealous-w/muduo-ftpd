#include "ftp_session.h"

#include <muduo/base/Logging.h>

namespace muduo
{
namespace net
{

FtpSession::FtpSession( const TcpConnectionPtr& co ) : conn(co)
{
    InitOpcodeHandler();
}

FtpSession::~FtpSession()
{

}

FtpSession::cmd_callback FtpSession::FindCallBack( std::string& strCmd )
{
    mapHandlerContainer::iterator it = mapHandlerFunc.find( strCmd );
    if ( it != mapHandlerFunc.end() ) return it->second;

    return NULL;
}

void FtpSession::ExecuteCmd( FtpCommand& cmd )
{
    FtpSession::cmd_callback _cb = FindCallBack( cmd.getCmd() );
    if ( _cb )
        (this->*_cb)( cmd );
}

void FtpSession::InitOpcodeHandler()
{
    mapHandlerFunc.clear();

    ///////////////////////////
    mapHandlerFunc["USER"] = &FtpSession::HandlerFtpUser;
    mapHandlerFunc["PASS"] = &FtpSession::HandlerFtpPass;
    mapHandlerFunc["PWD"]  = &FtpSession::HandlerFtpPwd;
    mapHandlerFunc["PWD"]  = &FtpSession::HandlerFtpAuth;
    mapHandlerFunc["PORT"] = &FtpSession::HandlerFtpPort;
    mapHandlerFunc["PASV"] = &FtpSession::HandlerFtpPasv;
    mapHandlerFunc["LIST"] = &FtpSession::HandlerFtpList;
    mapHandlerFunc["QUIT"] = &FtpSession::HandlerFtpQuit;
    mapHandlerFunc["PWD"]  = &FtpSession::HandlerFtpPwd;
    mapHandlerFunc["CWD"]  = &FtpSession::HandlerFtpCwd;
    mapHandlerFunc["USER"] = &FtpSession::HandlerFtpUser;
    mapHandlerFunc["PASS"] = &FtpSession::HandlerFtpPass;
    mapHandlerFunc["SYST"] = &FtpSession::HandlerFtpSyst;
    mapHandlerFunc["DELE"] = &FtpSession::HandlerFtpDele;
}

void FtpSession::HandlerFtpAuth(FtpCommand& cmd)
{
    LOG_INFO << cmd.getCmd() << ", param : " << cmd.getParam();
    if ( cliRole.getStatus() ) return;

    cliRole.setStatus( true );
}

void FtpSession::HandlerFtpPort(FtpCommand& cmd)
{
    LOG_INFO << cmd.getCmd() << ", param : " << cmd.getParam();
}

void FtpSession::HandlerFtpPasv(FtpCommand& cmd)
{
    LOG_INFO << cmd.getCmd() << ", param : " << cmd.getParam();
}
void FtpSession::HandlerFtpList(FtpCommand& cmd)
{
    LOG_INFO << cmd.getCmd() << ", param : " << cmd.getParam();
}
void FtpSession::HandlerFtpQuit(FtpCommand& cmd)
{
    LOG_INFO << cmd.getCmd() << ", param : " << cmd.getParam();
}
void FtpSession::HandlerFtpPwd(FtpCommand& cmd)
{

}
void FtpSession::HandlerFtpCwd(FtpCommand& cmd)
{

}
void FtpSession::HandlerFtpUser(FtpCommand& cmd)
{

}
void FtpSession::HandlerFtpPass(FtpCommand& cmd)
{

}
void FtpSession::HandlerFtpSyst(FtpCommand& cmd)
{

}
void FtpSession::HandlerFtpDele(FtpCommand& cmd)
{

}
void FtpSession::HandlerFtpRetr(FtpCommand& cmd)
{

}
void FtpSession::HandlerFtpSize(FtpCommand& cmd)
{

}
void FtpSession::HandlerFtpRmd(FtpCommand& cmd)
{

}
void FtpSession::HandlerFtpStor(FtpCommand& cmd)
{

}
void FtpSession::HandlerFtpNlst(FtpCommand& cmd)
{

}
void FtpSession::HandlerFtpMkd(FtpCommand& cmd)
{

}
void FtpSession::HandlerFtpType(FtpCommand& cmd)
{

}
void FtpSession::HandlerFtpCdup(FtpCommand& cmd)
{

}

}
}