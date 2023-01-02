#pragma once

class NonCopyAble
{
public:
    NonCopyAble(const NonCopyAble&) = delete;
    NonCopyAble(NonCopyAble&&) = delete;
    void operator=(const NonCopyAble&) = delete;
    void operator=(NonCopyAble&&) = delete;

protected:
    NonCopyAble() = default;
    ~NonCopyAble() = default;
};