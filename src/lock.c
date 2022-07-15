/*
    lock.c - contains lock wrapper
    Copyright 2022 The NexNix Project

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    There should be a copy of the License distributed in a file named
    LICENSE, if not, you may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/// @file lock.c

#include <libnex/libnex_config.h>
#include <libnex/lock.h>

/**
 * @brief Initializes a lock
 *
 * Wraps over pthread or NexNix kernel spinlocks, or maybe nothing.
 *
 * @param lock the lock to be initialized
 */
void __Libnex_lock_init (lock_t* lock)
{
#ifdef HAVE_PTHREAD
    pthread_mutexattr_t attr;
    // FIXME: attr is leaked. I currently can't think of an easy way to structure this so that
    // _lock_destroy can destroy the mutex and mutexattr. TBD.
    pthread_mutexattr_init (&attr);
    pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init (lock, &attr);
#elif defined HAVE_WIN32_THREADS
    InitializeCriticalSection (lock);
#else
    UNUSED (lock);
#endif
}

/**
 * @brief Locks a lock
 *
 * Wraps over whatever is needed to lock a lock
 * @param lock the lock to lock
 */
void __Libnex_lock_lock (lock_t* lock)
{
#ifdef HAVE_PTHREAD
    pthread_mutex_lock (lock);
#elif defined HAVE_WIN32_THREADS
    EnterCriticalSection (lock);
#else
    UNUSED (lock);
#endif
}

/**
 * @brief Unlocks a lock
 *
 * Wraps over whatever is needed to unlock a lock
 * @param lock the lock to unlock
 */
void __Libnex_lock_unlock (lock_t* lock)
{
#ifdef HAVE_PTHREAD
    pthread_mutex_unlock (lock);
#elif defined HAVE_WIN32_THREADS
    LeaveCriticalSection (lock);
#else
    UNUSED (lock);
#endif
}

/**
 * @brief Destroys a lock
 *
 * Wraps over whatever is needed to destroy a lock
 * @param lock the lock to destroy
 */
void __Libnex_lock_destroy (lock_t* lock)
{
#ifdef HAVE_PTHREAD
    pthread_mutex_destroy (lock);
#elif defined HAVE_WIN32_THREADS
    DeleteCriticalSection (lock);
#else
    UNUSED (lock);
#endif
}
