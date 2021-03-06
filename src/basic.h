#ifndef MUDUO_BASIC_H
#define MUDUO_BASIC_H

#include <string>
#include <vector>
#include <stdio.h>
#include <stdint.h>

#include <muduo/base/StringPiece.h>
#include <muduo/net/Buffer.h>
#include "util.h"

class FtpCommand
{
public:
    FtpCommand( const char* str )
    {
        std::string strAll(str) ;
        strAll.erase( strAll.size() - 2, 2 );

        std::vector<std::string> vToken;
        util::strSplit( strAll, " ", vToken );
        strCmd = *(vToken.begin());
        strParam = *(--vToken.end());
    }
    ~FtpCommand()
    {

    }

    std::string& getCmd()
    {
        return strCmd;
    }

    std::string& getParam()
    {
        return strParam;
    }

private:
    std::string     strCmd;
    std::string     strParam;
};

#define DEFINE_CLIENT_FUNC( FUNC, PARAM, TYPE ) \
        void set##FUNC( TYPE param ) { \
            PARAM = param; \
        }\
        TYPE get##FUNC() { \
            return PARAM; \
        }

class FtpClient
{
public:
    FtpClient()
    {
        byStatus = false;
        byAscii = true;
    }

    ~FtpClient(){}

    typedef std::vector< muduo::net::Buffer > DataContainer;

    DEFINE_CLIENT_FUNC( User, user, std::string& )
    DEFINE_CLIENT_FUNC( Pass, passwd, std::string& )
    DEFINE_CLIENT_FUNC( Ip, strIp, std::string )
    DEFINE_CLIENT_FUNC( FileName, filename, std::string )
    DEFINE_CLIENT_FUNC( Port, port, uint16_t )
    DEFINE_CLIENT_FUNC( Status, byStatus, bool )
    DEFINE_CLIENT_FUNC( Ascii, byAscii, bool )
    DEFINE_CLIENT_FUNC( CmdStatus, cmdStatus, int )

    void addFileData( muduo::StringPiece& str ) { filedata.append( str ); }
    muduo::net::Buffer& getFileData() { return filedata; }
    void clearFileData() { filedata.retrieveAll(); }
private:

    std::string user;
    std::string passwd;
    std::string filename;
    muduo::net::Buffer filedata;
    std::string strIp;
    uint16_t    port;     //port mode || pasv
    bool        byAscii;
    bool        byStatus;
    int         cmdStatus;
};

#endif //MUDUO_BASIC_H
