#ifndef MUDUO_BASIC_H
#define MUDUO_BASIC_H

#include <string>
#include <vector>
#include <stdio.h>

class FtpCommand
{
public:
    FtpCommand( const char* str )
    {
        std::string strAll(str) ;
        strAll.erase( strAll.size() - 2, 2 );
        //strAll.replace( 0,
        std::vector<std::string> vToken;
        strSplit( strAll, " ", vToken );
        strCmd = *(vToken.begin());
        strParam = *(--vToken.end());
    }
    ~FtpCommand()
    {

    }

    void strSplit(const std::string & src, const std::string & sep,
                  std::vector<std::string>& tokens )
    {
        tokens.clear();
        std::string s;
        for(std::string::const_iterator i = src.begin(); i != src.end(); i++)
        {
            if (sep.find(*i) != std::string::npos)
            {
                tokens.push_back(s);
                s = "";
            }
            else
            {
                s += *i;
            }
        }
        if ( s.length() || tokens.size() ) tokens.push_back(s);
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

    DEFINE_CLIENT_FUNC( User, user, std::string& )
    DEFINE_CLIENT_FUNC( Pass, passwd, std::string& )
    DEFINE_CLIENT_FUNC( Status, byStatus, bool )
    DEFINE_CLIENT_FUNC( Ascii, byAscii, bool )
private:
    std::string user;
    std::string passwd;
    bool byAscii;
    bool byStatus;
};

#endif //MUDUO_BASIC_H
