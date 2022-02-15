/*
    lock.h - contains lock wrapper
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

/// @file lock.h

#ifndef _LOCK_H
#define _LOCK_H

#ifdef IN_LIBNEX
#include <libnex_config.h>
#else
#include <libnex/libnex_config.h>
#endif

#ifdef HAVE_PTHREAD
#include <pthread.h>
typedef pthread_mutex_t lock_t;
#elif defined HAVE_WIN32_THREADS
#include <windows.h>
typedef CRITICAL_SECTION lock_t;
#else
#ifndef LIBNEX_BAREMETAL
#error Target platform has no supported supported threading library
#endif
#endif

/**
 * These functions SHALL NOT be used by consumers
 * Their interfaces are privy to change at any time, without notice
 */

/**
 * @brief Initializes a lock
 *
 * Wraps over pthread or NexNix kernel spinlocks, or maybe nothing.
 *
 * @param lock the lock to be initialized
 */
void _libnex_lock_init (lock_t* lock);

/**
 * @brief Locks a lock
 *
 * Wraps over whatever is needed to lock a lock
 * @param lock the lock to lock
 */
void _libnex_lock_lock (lock_t* lock);

/**
 * @brief Unlocks a lock
 *
 * Wraps over whatever is needed to unlock a lock
 * @param lock the lock to unlock
 */
void _libnex_lock_unlock (lock_t* lock);

#endif
