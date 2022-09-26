#include "Common.h"

std::string getTime()
{
    char buffer[20];
#if defined( WIN32 )
    SYSTEMTIME wlocaltime;
    GetLocalTime(&wlocaltime);
    ::snprintf(buffer, sizeof buffer, "%02d:%02d:%02d.%03d ", wlocaltime.wHour, wlocaltime.wMinute, wlocaltime.wSecond, wlocaltime.wMilliseconds);
#else
    std::chrono::time_point< std::chrono::system_clock > now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto hours = std::chrono::duration_cast< std::chrono::hours >( duration );
    duration -= hours;
    auto minutes = std::chrono::duration_cast< std::chrono::minutes >( duration );
    duration -= minutes;
    auto seconds = std::chrono::duration_cast< std::chrono::seconds >( duration );
    duration -= seconds;
    auto milliseconds = std::chrono::duration_cast< std::chrono::milliseconds >( duration );
    duration -= milliseconds;
    time_t theTime = time( NULL );
    struct tm* aTime = localtime( &theTime );
    ::snprintf(buffer, sizeof buffer, "%02d:%02d:%02d.%03ld ", aTime->tm_hour, aTime->tm_min, aTime->tm_sec, milliseconds.count());
#endif
    return buffer;
}