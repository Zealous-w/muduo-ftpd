#include "ftp_session.h"

#include <muduo/base/Logging.h>

namespace muduo
{
namespace net
{

FtpSession::FtpSession( const TcpConnectionPtr& co ) : conn(co)
{
    InitOpcodeHandler();
    InitReponseCode();
}

FtpSession::~FtpSession()
{

}

FtpSession::cmd_callback FtpSession::FindCallBack( std::string& strCmd )
{
    MapHandlerContainer::iterator it = mapHandlerFunc.find( strCmd );
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

void FtpSession::InitReponseCode()
{
    mapResponse.clear();

    mapResponse[150] = "150 %s\r\n";
    mapResponse[200] = "200 %s\r\n";
    mapResponse[220] = "220 the service is ready(khaki ftpd 1.0.0 free software) \r\n";
    mapResponse[221] = "221 Goodbye dear\r\n";
    mapResponse[226] = "226 %s\r\n";
    mapResponse[227] = "227 Entering passive Mode (%d;%d;%d;%d;%d;%d)\r\n";
    mapResponse[331] = "331 enter password.\r\n";
    mapResponse[230] = "230 login success.....\r\n";
    mapResponse[257] = "257 \"%s\" is a directory.\r\n";
    mapResponse[250] = "250 %s\r\n";
    mapResponse[215] = "215 Linux 2.6.32.\r\n";
    mapResponse[500] = "500 invaild command\r\n";
    mapResponse[213] = "213 size of \"%s\" is %ld.";
    mapResponse[550] = "550 Port is failed\r\n";
}

void FtpSession::HandlerFtpUser(FtpCommand& cmd)
{
    cliRole.setUser( cmd.getParam() );

    StringPiece buf( mapResponse[220] );
    conn->send( buf );
}

void FtpSession::HandlerFtpPass(FtpCommand& cmd)
{

}

void FtpSession::HandlerFtpAuth(FtpCommand& cmd)
{
    LOG_INFO << cmd.getCmd() << "; param : " << cmd.getParam();
    if ( cliRole.getStatus() ) return;

    cliRole.setStatus( true );
}

void FtpSession::HandlerFtpPort(FtpCommand& cmd)
{
    LOG_INFO << cmd.getCmd() << "; param : " << cmd.getParam();
}

void FtpSession::HandlerFtpPasv(FtpCommand& cmd)
{
    LOG_INFO << cmd.getCmd() << "; param : " << cmd.getParam();
}
void FtpSession::HandlerFtpList(FtpCommand& cmd)
{
    LOG_INFO << cmd.getCmd() << "; param : " << cmd.getParam();
}

void FtpSession::HandlerFtpQuit(FtpCommand& cmd)
{
    LOG_INFO << cmd.getCmd() << "; param : " << cmd.getParam();
}

void FtpSession::HandlerFtpPwd(FtpCommand& cmd)
{

}

void FtpSession::HandlerFtpCwd(FtpCommand& cmd)
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