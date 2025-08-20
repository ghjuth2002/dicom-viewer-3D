#pragma once

template<typename T>
class Singleton
{
public:
    Singleton() = default;
    ~Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    inline static T& instance()
    {
        static T instance;
        return instance;
    }
};
