#include "ftp_session.h"

#include <muduo/base/Logging.h>
#include <boost/bind.hpp>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <iterator>
#include <iostream>
#include <fstream>

namespace muduo
{
namespace net
{
/////////////////////////////////////////
PasvServer::PasvServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                        FtpClient* cli)
        : server_(loop, listenAddr, "PasvServer"), cli_(cli)
{
    byOnline = false;
    server_.setConnectionCallback(
            boost::bind(&PasvServer::onConnection, this, _1));
    server_.setMessageCallback(
            boost::bind(&PasvServer::onMessage, this, _1, _2, _3));
    server_.setWriteCompleteCallback(
            boost::bind(&PasvServer::onWriteComplete, this, _1));
}

void PasvServer::start()
{
    server_.start();
}

void PasvServer::send( StringPiece& str )
{
    conn_->send( str );
    LOG_INFO << "conn usecount : " << conn_.use_count();
}

bool PasvServer::isOnline()
{
    TcpConnWeakPtr weak( conn_ );
    if ( !weak.expired() ) return false;

    return true;
}

bool PasvServer::getStatus()
{
    return byOnline;
}

void PasvServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        conn_ = conn;
        LOG_INFO << "connect success : " << conn_.use_count();
        byOnline = true;
    }
    else
    {
        if( cb_ ) cb_();
    }
}

void PasvServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
               muduo::net::Buffer* buf,
               muduo::Timestamp time)
{
    StringPiece str = buf->toStringPiece();

    std::fstream fout( cli_->getFileName().c_str(), std::ios_base::out|std::ios_base::binary );
    fout << str.as_string();
}

void PasvServer::onWriteComplete(const TcpConnectionPtr& conn)
{
    if( cb_ ) cb_();
    conn->shutdown();
}

///////////////////////////////////////////
PortClient::PortClient(muduo::net::EventLoop* loop,
            const muduo::net::InetAddress& serverAddr)
        :client_(loop, serverAddr, "portmode")
{
    client_.setConnectionCallback(
            boost::bind(&PortClient::onConnection, this, _1));
    client_.setMessageCallback(
            boost::bind(&PortClient::onMessage, this, _1, _2, _3));
}

void PortClient::start()
{
    client_.connect();
}

void PortClient::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        LOG_INFO << "connect success";
    }
}

void PortClient::onMessage(const muduo::net::TcpConnectionPtr& conn,
               muduo::net::Buffer* buf,
               muduo::Timestamp time)
{

}

///////////////////////////////////////////
FtpSession::FtpSession( EventLoop* lo, TcpConnectionPtr co ) : loop(lo), conn(co)
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

void FtpSession::DisconnectCallBack()
{
    switch ( cliRole.getCmdStatus() )
    {
        case E_FTP_SESSION_CMD_STOR:
        {
            StringPiece bufCom( util::string_format( mapResponse[226], "Transfer complete" ) );
            conn->send( bufCom );

            cliRole.setCmdStatus( 0 );
            cliRole.setFileName("");
            cliRole.clearFileData();
            break;
        }
        case E_FTP_SESSION_CMD_RETR:
        {
            std::string resTran = util::string_format( mapResponse[226], "Transfer complate." );
            StringPiece bufTran( resTran );
            conn->send( bufTran );

            cliRole.setCmdStatus( 0 );
            break;
        }
    }

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
    mapResponse[227] = "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n";
    mapResponse[331] = "331 enter password.\r\n";
    mapResponse[230] = "230 login success.....\r\n";
    mapResponse[257] = "257 \"%s\" is a directory.\r\n";
    mapResponse[250] = "250 %s\r\n";
    mapResponse[215] = "215 Linux 2.6.32.\r\n";
    mapResponse[500] = "500 invaild command\r\n";
    mapResponse[213] = "213 size of \"%s\" is %ld.\r\n";
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

void FtpSession::HandlerFtpPort(FtpCommand& cmd)
{
    portCli.reset();
    cliRole.setPort( 20 );
    cliRole.setIp( conn->peerAddress().toIp().c_str() );
    InetAddress serverAddr(cliRole.getIp().c_str(), cliRole.getPort());
    portCli = PortClientPtr( new PortClient( loop, serverAddr ) );
    portCli->start();
}

void FtpSession::HandlerFtpPasv(FtpCommand& cmd)
{
    pasvSer.reset();
    PasvWeakPtr pasvWeak(pasvSer);
    if ( !pasvWeak.expired() )
    {
    }

    unsigned short int port = static_cast<uint16_t >( ( rand() % 10000 ) + 1024 );
    InetAddress listenAddr("172.16.1.174", port);
    cliRole.setPort(port);
    pasvSer = PasvServerPtr( new PasvServer( loop, listenAddr, &cliRole ) );

    const struct sockaddr_in* addr = static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(listenAddr.getSockAddr()));

    unsigned long  hostip = ntohl( addr->sin_addr.s_addr );

    std::string resonse = util::string_format( mapResponse[227], (hostip >> 24) & 0xff,
                                               (hostip >> 16) & 0xff, (hostip >> 8) & 0xff,
                                               hostip & 0xff, (port >> 8) & 0xff, port & 0xff );

    pasvSer->start();
    StringPiece buf( resonse );
    conn->send( buf );
}
void FtpSession::HandlerFtpList(FtpCommand& cmd)
{
    DIR *dir_ptr;
    int mode;
    std::string strList;

    struct dirent *direntp;
    struct stat info;
    struct passwd *pw_ptr;
    struct group  *gp_ptr;
    char modestr[11] = {"----------"};

    if( (dir_ptr = opendir(".")) == NULL) return;

    while((direntp = readdir(dir_ptr)) != NULL)
    {
        if(stat(direntp->d_name,&info) == -1)
        {
            return ;
        }
        else
        {
            memset(modestr,'-',sizeof(modestr));
            mode = info.st_mode;
            if(S_ISDIR(mode)) modestr[0] = 'd';
            if(S_ISCHR(mode)) modestr[0] = 'c';
            if(S_ISBLK(mode)) modestr[0] = 'b';

            if(mode & S_IRUSR) modestr[1] = 'r';
            if(mode & S_IWUSR) modestr[2] = 'w';
            if(mode & S_IXUSR) modestr[3] = 'x';


            if(mode & S_IRGRP) modestr[4] = 'r';
            if(mode & S_IWGRP) modestr[5] = 'w';
            if(mode & S_IXGRP) modestr[6] = 'x';


            if(mode & S_IROTH) modestr[7] = 'r';
            if(mode & S_IWOTH) modestr[8] = 'w';
            if(mode & S_IXOTH) modestr[9] = 'x';

            if((pw_ptr = getpwuid(info.st_uid)) == NULL)
            {
                struct passwd temp;
                sprintf(temp.pw_name,"%d",info.st_uid);
                pw_ptr = &temp;
            }
            if((gp_ptr = getgrgid(info.st_gid)) == NULL)
            {
                struct group tempgp;
                sprintf(tempgp.gr_name,"%d",info.st_gid);
                gp_ptr = &tempgp;
            }
            strList += util::string_format( "%s%4d %-8s %-8s %8ld %.12s %s\n",
                                 modestr, static_cast<int>(info.st_nlink),pw_ptr->pw_name,gp_ptr->gr_name,
                                 static_cast<long>(info.st_size),4 + ctime(&info.st_mtime),direntp->d_name );
        }
    }
    closedir(dir_ptr);

    std::string resList = util::string_format( mapResponse[150], "Here comes the directory listing." );
    std::string resOk = util::string_format( mapResponse[226], "Directory send OK." );

    StringPiece bufList( resList );
    conn->send( bufList );

    StringPiece bufCon( strList );
    pasvSer->send( bufCon );

    StringPiece bufOk( resOk );
    conn->send( bufOk );

    pasvSer.reset();
}

void FtpSession::HandlerFtpQuit(FtpCommand& cmd)
{
    StringPiece buf( mapResponse[221] );
    conn->send( buf );
}

void FtpSession::HandlerFtpPwd(FtpCommand& cmd)
{
    char path[2096] = {0};
    std::string response = util::string_format( mapResponse[257], getcwd( path, sizeof path ) );
    StringPiece buf( response );
    conn->send( buf );
}

void FtpSession::HandlerFtpCwd(FtpCommand& cmd)
{
    if ( chdir( cmd.getParam().c_str() ) == -1 ) return;

    std::string response = util::string_format( mapResponse[250], "Directory successfully changed" );

    StringPiece buf( response );

    conn->send( buf );
}

void FtpSession::HandlerFtpSyst(FtpCommand& cmd)
{
    StringPiece buf( mapResponse[215].c_str() );
    conn->send( buf );
}

void FtpSession::HandlerFtpDele(FtpCommand& cmd)
{
    std::string filename = cmd.getParam();
    if ( unlink( filename.c_str() ) == -1 ) return;

    std::string response = util::string_format( mapResponse[200], "Delete file successfully." );
    StringPiece buf( response );
    conn->send( buf );
}

void FtpSession::HandlerFtpRetr(FtpCommand& cmd)
{
    std::string filename = cmd.getParam();
    std::fstream file;
    std::string strMode;

    struct stat info;
    stat( filename.c_str(), &info );

    if ( cliRole.getAscii() )
    {
        file.open( filename.c_str(), std::ios::in );
        strMode = "ASCII";
    }
    else
    {
        file.open( filename.c_str(), std::ios_base::in | std::ios_base::binary );
        strMode = "BINARY";
    }

    std::istreambuf_iterator<char> begin(file), end;
    std::string strFile( begin, end );

    std::string resSize = util::string_format( "150 Opening %s mode data connection for %s (%ld bytes).\r\n",
                                               strMode.c_str(), filename.c_str(), info.st_size );
    std::string resTran = util::string_format( mapResponse[226], "Transfer complate." );

    StringPiece buf( resSize );
    conn->send( buf );

    StringPiece File( strFile );
    PasvWeakPtr pasvWeak(pasvSer);
    if ( !pasvWeak.expired() )
    {
        cliRole.setCmdStatus( E_FTP_SESSION_CMD_RETR );
        pasvSer->setDisconnectCallback( boost::bind( &FtpSession::DisconnectCallBack, this ) );
        pasvSer->send( File );
    }
}

void FtpSession::HandlerFtpSize(FtpCommand& cmd)
{
    struct stat info;
    if ( stat( cmd.getParam().c_str(), &info ) == -1 )
    {
        return;
    }

    StringPiece response( util::string_format( mapResponse[213], cmd.getParam().c_str(), info.st_size ) );
    conn->send( response );
}

void FtpSession::HandlerFtpRmd(FtpCommand& cmd)
{
    DIR *dir_ptr;
    StringPiece buf( mapResponse[215] );

    if((dir_ptr = opendir(cmd.getParam().c_str())) != NULL)
    {
        conn->send( buf );
        return;
    }
    /*loop delete this dir file ,then call rmdir*/
    if(-1 == rmdir(cmd.getParam().c_str()))
    {
        conn->send( buf );
        return;
    }

    StringPiece bufRes( util::string_format( mapResponse[257], cmd.getParam().c_str() ) );
    conn->send( bufRes );
}

void FtpSession::HandlerFtpStor(FtpCommand& cmd)
{
    PasvWeakPtr pasvWeak(pasvSer);
    if ( pasvWeak.expired() ) return ;

    StringPiece bufRes( util::string_format( mapResponse[150], "Ok to send data" ) );
    conn->send( bufRes );

    pasvSer->setDisconnectCallback( boost::bind(&FtpSession::DisconnectCallBack, this ) );
    cliRole.setFileName( cmd.getParam() );
    cliRole.setCmdStatus( E_FTP_SESSION_CMD_STOR );
}

void FtpSession::HandlerFtpNlst(FtpCommand& cmd)
{

}

void FtpSession::HandlerFtpMkd(FtpCommand& cmd)
{
    DIR* dir_ptr;
    StringPiece bufRes( mapResponse[215] );

    if ( ( dir_ptr = opendir( cmd.getParam().c_str() ) ) != NULL )
    {
        conn->send( bufRes );
        return;
    }

    if ( mkdir( cmd.getParam().c_str(), 0777 ) == -1 )
    {
        conn->send( bufRes );
        return;
    }

    StringPiece buf( util::string_format( mapResponse[257], cmd.getParam().c_str() ) );
    conn->send( buf );

}
void FtpSession::HandlerFtpType(FtpCommand& cmd)
{
    std::string response;
    if ( cmd.getParam() == "A" )
    {
        cliRole.setAscii( true );
        response = util::string_format( mapResponse[200], "Switching to ASCII mode." );
    }
    else
    {
        cliRole.setAscii( false );
        response = util::string_format( mapResponse[200], "Switching to BINARY mode." );
    }

    StringPiece buf( response );
    conn->send( buf );
}
void FtpSession::HandlerFtpCdup(FtpCommand& cmd)
{
    if ( chdir( ".." ) == -1 ) return;

    std::string response = util::string_format( mapResponse[250], "Directory successfully changed" );

    StringPiece buf( response );
    conn->send( buf );
}

}
}