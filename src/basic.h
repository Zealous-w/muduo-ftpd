#ifndef MUDUO_BASIC_H
#define MUDUO_BASIC_H

#include <string>

class FtpCommand
{
public:
    FtpCommand( const char* str )
    {
        // 处理命令
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

class FtpClient
{
public:
    FtpClient()
    {
        byStatus = false;
    }
    ~FtpClient(){}

    void setStatus( bool bySta )
    {
        this->byStatus = bySta;
    }
    bool getStatus()
    {
        return this->byStatus;
    }
private:
    bool byStatus;
};

#endif //MUDUO_BASIC_H
