#ifndef MUDUO_BASIC_H
#define MUDUO_BASIC_H

#include <string>

class FtpCommand
{
public:
    FtpCommand( const char* str )
    {
        // 处理命令
        std::string strAll(str);
//        std::vector<std::string> vToken =  split( strAll, ' ' );
//        strCmd = *(vToken.begin());
//        strParam = *(--vToken.end());
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
    }

    ~FtpClient(){}

    DEFINE_CLIENT_FUNC( User, user, std::string& )
    DEFINE_CLIENT_FUNC( Pass, passwd, std::string& )
    DEFINE_CLIENT_FUNC( Status, byStatus, bool )
private:
    bool byStatus;
    std::string user;
    std::string passwd;
};

#endif //MUDUO_BASIC_H
