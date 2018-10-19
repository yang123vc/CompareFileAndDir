#pragma once


#include <memory>

#define DECLARE_PRIVATE_PTR(T) typedef std::shared_ptr<class T::Private> T##Private_Ptr;
#define DECLARE_PTR(T) typedef std::shared_ptr<class T> T##_Ptr;


#define DISABLE_COPY(Class) \
    Class(const Class &) = delete;\
    Class &operator=(const Class &) = delete;