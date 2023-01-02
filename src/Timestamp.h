#pragma once
#include <stdlib.h>
#include <iostream>
class Timestamp
{
public:
    Timestamp() : microSecondsSinceEpoch_(0)
    {
        
    }
    explicit Timestamp(int64_t microSecondsSinceEpochArg) : microSecondsSinceEpoch_(microSecondsSinceEpochArg)
    {

    }
    void swap(Timestamp& that)
    {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }
    std::string toString() const;
    std::string toFormattedString(bool showMicroseconds = true)const;

    bool valid() const {return microSecondsSinceEpoch_ > 0;}

    int64_t microSecondsSinceEpoch() const {return microSecondsSinceEpoch_;}
    time_t secondsSinceEpoch() const
    {
        return static_cast<time_t> (microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }

    static Timestamp now();
    static Timestamp invalid()
    {
        return Timestamp();
    }
    static Timestamp fromUnixTime(const time_t& t)
    {
        return fromUnixTime(t, 0);
    }
    static Timestamp fromUnixTime(const time_t& t,const int microseconds)
    {
        return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
    }
    static const int kMicroSecondsPerSecond = 1000 * 1000;
private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(const Timestamp& lhs, const Timestamp& rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}
inline bool operator==(const Timestamp& lhs, const Timestamp& rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}
inline double timeDifference(const Timestamp& hig, const Timestamp& low)
{
    int64_t diff = 0;
    if(low < hig)
    {
        diff = hig.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    }
    else
    {
        diff = low.microSecondsSinceEpoch() - hig.microSecondsSinceEpoch();
    }
    return static_cast<double> (diff) / Timestamp::kMicroSecondsPerSecond;
}
inline Timestamp addTime(const Timestamp& timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}