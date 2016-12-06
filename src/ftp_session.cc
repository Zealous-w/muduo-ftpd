#include "ftp_session.h"

#include <muduo/base/Logging.h>

#include <stdio.h>
#include <unistd.h>

namespace muduo
{
namespace net
{

FtpSession::FtpSession( TcpConnectionPtr co ) : conn(co)
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
    mapHandlerFunc["CWD"]  = &FtpSession::HandlerFtpCwd;
    mapHandlerFunc["QUIT"] = &FtpSession::HandlerFtpQuit;
    mapHandlerFunc["SYST"] = &FtpSession::HandlerFtpSyst;
    mapHandlerFunc["DELE"] = &FtpSession::HandlerFtpDele;
    mapHandlerFunc["RETR"] = &FtpSession::HandlerFtpRetr;
    mapHandlerFunc["SIZE"] = &FtpSession::HandlerFtpSize;
    mapHandlerFunc["RMD"]  = &FtpSession::HandlerFtpRmd;
    mapHandlerFunc["STOR"] = &FtpSession::HandlerFtpStor;
    mapHandlerFunc["NLST"] = &FtpSession::HandlerFtpNlst;
    mapHandlerFunc["MKD"]  = &FtpSession::HandlerFtpMkd;
    mapHandlerFunc["TYPE"] = &FtpSession::HandlerFtpType;
    mapHandlerFunc["CDUP"] = &FtpSession::HandlerFtpCdup;
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

    StringPiece buf( mapResponse[331] );
    conn->send( buf );
}

void FtpSession::HandlerFtpPass(FtpCommand& cmd)
{
    cliRole.setPass( cmd.getParam() );

    StringPiece buf( mapResponse[230] );
    conn->send( buf );
}

void FtpSession::HandlerFtpAuth(FtpCommand& cmd)
{
}

void FtpSession::HandlerFtpPort(FtpCommand& cmd)
{
    LOG_INFO << cmd.getCmd() << "; param : " << cmd.getParam();
}

void FtpSession::HandlerFtpPasv(FtpCommand& cmd)
{
    
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
    char pwd[4096] = {0};
    char path[4096] = {0};
    snprintf( pwd, sizeof pwd,  mapResponse[257].c_str(), getcwd( path, sizeof path ) );

    LOG_INFO << pwd;
    StringPiece buf( pwd );
    conn->send( buf );
}

void FtpSession::HandlerFtpCwd(FtpCommand& cmd)
{
    char pwd[4096] = {0};
    if ( chdir( cmd.getParam().c_str() ) == -1 ) return;
    snprintf( pwd, sizeof pwd,  mapResponse[250].c_str(), "Directory successfully changed" );

    LOG_INFO << pwd;
    StringPiece buf( pwd );
    conn->send( buf );
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
    char pwd[4096] = {0};
    if ( cmd.getParam() == "A" )
    {
        cliRole.setAscii( true );
        snprintf( pwd, sizeof pwd,  mapResponse[200].c_str(), "Switching to ASCII mode." );
    }
    else
    {
        cliRole.setAscii( false );
        snprintf( pwd, sizeof pwd,  mapResponse[200].c_str(), "Switching to BINARY mode." );
    }

    LOG_INFO << pwd;
    StringPiece buf( pwd );
    conn->send( buf );
}
void FtpSession::HandlerFtpCdup(FtpCommand& cmd)
{

}

}
}