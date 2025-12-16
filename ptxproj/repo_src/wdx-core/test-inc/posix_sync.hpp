//------------------------------------------------------------------------------
// Copyright (c) 2021 WAGO GmbH & Co. KG
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    POSIX thread sync utilities.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_POSIX_SYNC_HPP_
#define TEST_INC_POSIX_SYNC_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <mutex>
#include <cstring>
#include <pthread.h>
#include <cerrno>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
class posix_mutex
{
private:
    pthread_mutex_t mutex_m;

public:
    posix_mutex()
    {
        errno = pthread_mutex_init(&mutex_m, NULL);
        if(0 != errno)
        {
            throw std::runtime_error(strerror(errno));
        }
    }
    ~posix_mutex()
    {
        errno = pthread_mutex_destroy(&mutex_m);
    }
    void lock()
    {
        errno = pthread_mutex_lock(&mutex_m);
        if(0 != errno)
        {
            throw std::runtime_error(strerror(errno));
        }
    }
    void unlock()
    {
        errno = pthread_mutex_unlock(&mutex_m);
        if(0 != errno)
        {
            throw std::runtime_error(strerror(errno));
        }
    }
    pthread_mutex_t & get_native_handle()
    {
        return mutex_m;
    }
};

class posix_convar
{
private:
    posix_mutex    mutex_m;
    pthread_cond_t condition_m;
    bool           condition_match_m;

public:
    posix_convar()
    :condition_match_m(false)
    {
        errno = pthread_cond_init(&condition_m, NULL);
        if(0 != errno)
        {
            throw std::runtime_error(strerror(errno));
        }
    }
    ~posix_convar()
    {
        errno = pthread_cond_destroy(&condition_m);
    }
    void wait()
    {
        std::lock_guard<posix_mutex> lock(mutex_m);
        while(!condition_match_m)
        {
            pthread_cond_wait(&condition_m, &(mutex_m.get_native_handle()));
        }
    }
    void notify()
    {
        std::lock_guard<posix_mutex> lock(mutex_m);
        condition_match_m = true;
        pthread_cond_signal(&condition_m);
    }
};


#endif // TEST_INC_POSIX_SYNC_HPP_

//---- End of source file ------------------------------------------------------
