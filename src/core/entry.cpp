/*
 * entry.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include <core/logging.hpp>
#include <pthread.h>

#include "hack.hpp"

pthread_mutex_t mutex_quit;
pthread_t thread_main;

bool IsStopping(pthread_mutex_t *mutex_quit_l)
{
    if (!pthread_mutex_trylock(mutex_quit_l))
    {
        logging::Info(XORSTR("Shutting down, unlocking mutex"));
        pthread_mutex_unlock(mutex_quit_l);
        return true;
    }
    else
    {
        return false;
    }
    return true;
}

void *MainThread(void *arg)
{
    pthread_mutex_t *mutex_quit_l = (pthread_mutex_t *) arg;
    hack::Initialize();
    logging::Info(XORSTR("Init done..."));
    while (!IsStopping(mutex_quit_l))
    {
        hack::Think();
    }
    logging::Info(XORSTR("Shutting down..."));
    hack::Shutdown();
    logging::Shutdown();
    return 0;
}

void __attribute__((constructor)) attach()
{
    // std::string test_str = XORSTR("test");
    pthread_mutex_init(&mutex_quit, 0);
    pthread_mutex_lock(&mutex_quit);
    pthread_create(&thread_main, 0, MainThread, &mutex_quit);
}

void __attribute__((destructor)) detach()
{
    logging::Info(XORSTR("Detaching"));
    pthread_mutex_unlock(&mutex_quit);
    pthread_join(thread_main, 0);
}
