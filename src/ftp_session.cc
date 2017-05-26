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

TransferData::TransferData(EventLoop* loop, FtpClient* cli)
        : loop_(loop), cli_(cli)
{
}

void TransferData::start()
{
    switch ( static_cast<int>(mode_) )
    {
        case E_TRANSFER_TYPE_PASV:
            server_->start();
            break;
        case E_TRANSFER_TYPE_PORT:
            client_->connect();
            break;
    }
}

void TransferData::send( StringPiece& str )
{
    conn_->send( str );
}

void TransferData::createDataChannel( const muduo::net::InetAddress& addr, int mode )
{
    setMode( mode );

    client_.reset();
    server_.reset();
    if ( mode == E_TRANSFER_TYPE_PASV )
    {
        server_ = TcpServerPtr( new TcpServer( loop_, addr, "pasv" ) );
        server_->setConnectionCallback(
                boost::bind(&TransferData::onConnection, this, _1));
        server_->setMessageCallback(
                boost::bind(&TransferData::onMessage, this, _1, _2, _3));
        server_->setWriteCompleteCallback(
                boost::bind(&TransferData::onWriteComplete, this, _1));
    }
    else
    {
        client_ = TcpClientPtr( new TcpClient( loop_, addr, "port" ) );
        client_->setConnectionCallback(
                boost::bind(&TransferData::onConnection, this, _1));
        client_->setMessageCallback(
                boost::bind(&TransferData::onMessage, this, _1, _2, _3));
        client_->setWriteCompleteCallback(
                boost::bind(&TransferData::onWriteComplete, this, _1));
    }

}

void TransferData::close()
{
    switch ( static_cast<int>(mode_) )
    {
        case E_TRANSFER_TYPE_PASV:
            conn_.reset();
            server_.reset();

            break;
        case E_TRANSFER_TYPE_PORT:
            conn_.reset();
            client_.reset();
            break;
    }
}


void TransferData::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        conn_ = conn;
    }
    else
    {
        if( cb_ ) cb_();
    }
}

void TransferData::onMessage(const muduo::net::TcpConnectionPtr& conn,
               muduo::net::Buffer* buf,
               muduo::Timestamp time)
{
    StringPiece str = buf->toStringPiece();

    int mode = static_cast<int>(mode_);

    switch (mode)
    {
        case E_TRANSFER_TYPE_PASV:
            break;
        case E_TRANSFER_TYPE_PORT:
            break;
    }

    std::fstream fout( cli_->getFileName().c_str(), std::ios_base::out|std::ios_base::binary );
    fout << str.as_string();
    fout.close();
}

void TransferData::onWriteComplete(const TcpConnectionPtr& conn)
{
    if( cb_ ) cb_();
    conn->shutdown();
}


///////////////////////////////////////////
FtpSession::FtpSession( EventLoop* lo, TcpConnectionPtr co )
        : loop_(lo), conn_(co), data_( loop_, &cli_ )
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
    switch ( cli_.getCmdStatus() )
    {
        case E_FTP_SESSION_CMD_STOR:
        {
            StringPiece bufCom( util::string_format( mapResponse[226], "Transfer complete" ) );
            conn_->send( bufCom );

            cli_.setCmdStatus( 0 );
            cli_.setFileName("");
            cli_.clearFileData();
            break;
        }
        case E_FTP_SESSION_CMD_RETR:
        {
            std::string resTran = util::string_format( mapResponse[226], "Transfer complate." );
            StringPiece bufTran( resTran );
            conn_->send( bufTran );

            cli_.setCmdStatus( 0 );
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
    mapHandlerFunc["FEAT"] = &FtpSession::HandlerFtpFeat;
}

void FtpSession::InitReponseCode()
{
    mapResponse.clear();

    mapResponse[150] = "150 %s\r\n";
    mapResponse[200] = "200 %s\r\n";
    mapResponse[211] = "211 Extension supported\r\n";
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
    cli_.setUser( cmd.getParam() );

    StringPiece buf( mapResponse[331] );
    conn_->send( buf );
}

void FtpSession::HandlerFtpPass(FtpCommand& cmd)
{
    cli_.setPass( cmd.getParam() );

    StringPiece buf( mapResponse[230] );
    conn_->send( buf );
}

void FtpSession::HandlerFtpPort(FtpCommand& cmd)
{
    std::vector<std::string> vToken;
    util::strSplit( cmd.getParam(), ",", vToken );
    std::string strIp;
    int port;

    if ( vToken.size() != 6 ) return;
    strIp = vToken[0] + "." + vToken[1] + "." + vToken[2] + "." + vToken[3];
    port = atoi( vToken[4].c_str() ) * 256 + atoi( vToken[5].c_str() );

    cli_.setPort( static_cast<uint16_t>(port) );
    cli_.setIp( strIp );
    InetAddress serverAddr(cli_.getIp().c_str(), cli_.getPort());

    data_.createDataChannel( serverAddr, 1 );
    data_.setMode( 1 );
    data_.start();

    std::string resonse = util::string_format( mapResponse[200], "PORT command successful." );
    StringPiece buf( resonse );
    conn_->send( buf );
}

void FtpSession::HandlerFtpPasv(FtpCommand& cmd)
{
    unsigned short int port = static_cast<uint16_t >( ( rand() % 10000 ) + 1024 );
    InetAddress listenAddr("192.168.65.128", port);
    cli_.setPort(port);

    data_.createDataChannel( listenAddr, 0 );
    data_.setMode( 0 );
    data_.start();

    const struct sockaddr_in* addr = static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(listenAddr.getSockAddr()));

    unsigned long  hostip = ntohl( addr->sin_addr.s_addr );

    std::string resonse = util::string_format( mapResponse[227], (hostip >> 24) & 0xff,
                                               (hostip >> 16) & 0xff, (hostip >> 8) & 0xff,
                                               hostip & 0xff, (port >> 8) & 0xff, port & 0xff );

    StringPiece buf( resonse );
    conn_->send( buf );
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

    while( (direntp = readdir(dir_ptr)) != NULL )
    {
        if( stat(direntp->d_name,&info) == -1 ) continue;

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

        if( ( pw_ptr = getpwuid( info.st_uid ) ) == NULL)
        {
            struct passwd temp;
            sprintf(temp.pw_name,"%d",info.st_uid);
            pw_ptr = &temp;
        }
        if( ( gp_ptr = getgrgid( info.st_gid ) ) == NULL)
        {
            struct group tempgp;
            sprintf(tempgp.gr_name,"%d",info.st_gid);
            gp_ptr = &tempgp;
        }

        strList += util::string_format( "%s%4d %-8s %-8s %8ld %.12s %s\n",
                             modestr, static_cast<int>(info.st_nlink),pw_ptr->pw_name,gp_ptr->gr_name,
                             static_cast<long>(info.st_size),4 + ctime(&info.st_mtime),direntp->d_name );
    }

    closedir(dir_ptr);

    std::string resList = util::string_format( mapResponse[150], "Here comes the directory listing." );
    std::string resOk = util::string_format( mapResponse[226], "Directory send OK." );

    StringPiece bufList( resList );
    conn_->send( bufList );

    StringPiece bufCon( strList );
    data_.send( bufCon );

    StringPiece bufOk( resOk );
    conn_->send( bufOk );

    data_.close();
}

void FtpSession::HandlerFtpQuit(FtpCommand& cmd)
{
    StringPiece buf( mapResponse[221] );
    conn_->send( buf );
    conn_->shutdown();
}

void FtpSession::HandlerFtpPwd(FtpCommand& cmd)
{
    char path[2096] = {0};
    std::string response = util::string_format( mapResponse[257], getcwd( path, sizeof path ) );
    StringPiece buf( response );
    conn_->send( buf );
}

void FtpSession::HandlerFtpCwd(FtpCommand& cmd)
{
    if ( chdir( cmd.getParam().c_str() ) == -1 ) return;

    std::string response = util::string_format( mapResponse[250], "Directory successfully changed" );

    StringPiece buf( response );

    conn_->send( buf );
}

void FtpSession::HandlerFtpSyst(FtpCommand& cmd)
{
    StringPiece buf( mapResponse[215].c_str() );
    conn_->send( buf );
}

void FtpSession::HandlerFtpDele(FtpCommand& cmd)
{
    std::string filename = cmd.getParam();
    if ( unlink( filename.c_str() ) == -1 ) return;

    std::string response = util::string_format( mapResponse[200], "Delete file successfully." );
    StringPiece buf( response );
    conn_->send( buf );
}

void FtpSession::HandlerFtpRetr(FtpCommand& cmd)
{
    std::string filename = cmd.getParam();
    std::fstream file;
    std::string strMode;

    struct stat info;
    stat( filename.c_str(), &info );

    if ( cli_.getAscii() )
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
    conn_->send( buf );

    StringPiece File( strFile );
    cli_.setCmdStatus( E_FTP_SESSION_CMD_RETR );

    data_.setDisconnectCallback( boost::bind( &FtpSession::DisconnectCallBack, this ) );
    data_.send( File );
}

void FtpSession::HandlerFtpSize(FtpCommand& cmd)
{
    struct stat info;
    if ( stat( cmd.getParam().c_str(), &info ) == -1 )
    {
        return;
    }

    StringPiece response( util::string_format( mapResponse[213], cmd.getParam().c_str(), info.st_size ) );
    conn_->send( response );
}

void FtpSession::HandlerFtpRmd(FtpCommand& cmd)
{
    DIR *dir_ptr;
    StringPiece buf( mapResponse[215] );

    if((dir_ptr = opendir(cmd.getParam().c_str())) != NULL)
    {
        conn_->send( buf );
        return;
    }
    /*loop delete this dir file ,then call rmdir*/
    if(-1 == rmdir(cmd.getParam().c_str()))
    {
        conn_->send( buf );
        return;
    }

    StringPiece bufRes( util::string_format( mapResponse[257], cmd.getParam().c_str() ) );
    conn_->send( bufRes );
}

void FtpSession::HandlerFtpStor(FtpCommand& cmd)
{
    StringPiece bufRes( util::string_format( mapResponse[150], "Ok to send data" ) );
    conn_->send( bufRes );

    data_.setDisconnectCallback( boost::bind(&FtpSession::DisconnectCallBack, this ) );
    cli_.setFileName( cmd.getParam() );
    cli_.setCmdStatus( E_FTP_SESSION_CMD_STOR );
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
        conn_->send( bufRes );
        return;
    }

    if ( mkdir( cmd.getParam().c_str(), 0777 ) == -1 )
    {
        conn_->send( bufRes );
        return;
    }

    StringPiece buf( util::string_format( mapResponse[257], cmd.getParam().c_str() ) );
    conn_->send( buf );

}
void FtpSession::HandlerFtpType(FtpCommand& cmd)
{
    std::string response;
    if ( cmd.getParam() == "A" )
    {
        cli_.setAscii( true );
        response = util::string_format( mapResponse[200], "Switching to ASCII mode." );
    }
    else
    {
        cli_.setAscii( false );
        response = util::string_format( mapResponse[200], "Switching to BINARY mode." );
    }

    StringPiece buf( response );
    conn_->send( buf );
}
void FtpSession::HandlerFtpCdup(FtpCommand& cmd)
{
    if ( chdir( ".." ) == -1 ) return;

    std::string response = util::string_format( mapResponse[250], "Directory successfully changed" );

    StringPiece buf( response );
    conn_->send( buf );
}

void FtpSession::HandlerFtpFeat(FtpCommand& cmd)
{
    StringPiece buf( mapResponse[211] );
    conn_->send( buf );
}


}
}