#include "Timer.h"
#include <unistd.h>
#include <queue>

using namespace std;
std::atomic<int64_t> Timer::sNumCreated_(0);

void Timer::restart(Timestamp now)
{
    if(repeat_)
    {
        expiration_ = addTime(now,interval_);
    }
    else
    {
        expiration_ = Timestamp::invalid();
    }
}