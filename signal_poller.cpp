/*

Signal Poller.

Copyright (C) 2015 Sergey Kolevatov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

// $Revision: 2668 $ $Date:: 2015-10-02 #$ $Author: serge $

#include "signal_poller.h"              // self

#include <cstring>                      // memset
#include <csignal>                      // siginfo_t
#include <unistd.h>                     // sysconf
#include <memory>                       // std::unique_ptr
#include <atomic>                       // std::atomic_bool

using namespace daemons;

#define MODULENAME "SignalPoller"

class SignalHandler
{
public:
    static void handle_signal( int sig, siginfo_t* siginfo, void* );
    static void handle_fatal_signal( int sig, siginfo_t* siginfo, void* );

    static bool poll_hup();
    static bool poll_int();
    static bool poll_term();
    static bool poll_abrt();
    static bool poll_segv();

private:
    SignalHandler();
    static bool poll( std::atomic_bool & b );
    static void close_fds();

private:
    static std::unique_ptr<SignalHandler>     inst_;

    static bool should_close_fds_;

    static std::atomic_bool is_hup_;
    static std::atomic_bool is_int_;
    static std::atomic_bool is_usr1_;
    static std::atomic_bool is_usr2_;
    static std::atomic_bool is_term_;
    static std::atomic_bool is_abrt_;
    static std::atomic_bool is_bus_;
    static std::atomic_bool is_fpe_;
    static std::atomic_bool is_segv_;
};

std::unique_ptr<SignalHandler>  SignalHandler::inst_( new SignalHandler() );
bool SignalHandler::should_close_fds_   = true;
std::atomic_bool SignalHandler::is_hup_( false );
std::atomic_bool SignalHandler::is_int_( false );
std::atomic_bool SignalHandler::is_usr1_( false );
std::atomic_bool SignalHandler::is_usr2_( false );
std::atomic_bool SignalHandler::is_term_( false );
std::atomic_bool SignalHandler::is_abrt_( false );
std::atomic_bool SignalHandler::is_bus_( false );
std::atomic_bool SignalHandler::is_fpe_( false );
std::atomic_bool SignalHandler::is_segv_( false );


SignalHandler::SignalHandler()
{
    struct sigaction handler;
    memset( &handler, 0, sizeof( handler ) );

    // non-fatal signals
    handler.sa_sigaction = handle_signal;
    handler.sa_flags = SA_SIGINFO;

    sigaction( SIGHUP,  &handler, NULL );
    sigaction( SIGINT,  &handler, NULL );
    sigaction( SIGUSR1, &handler, NULL );
    sigaction( SIGUSR2, &handler, NULL );
    sigaction( SIGTERM, &handler, NULL );

    // fatal signals
    handler.sa_sigaction = handle_fatal_signal;
    handler.sa_flags = SA_SIGINFO | SA_RESETHAND;

    sigaction( SIGABRT, &handler, NULL );
    sigaction( SIGBUS,  &handler, NULL );
    sigaction( SIGFPE,  &handler, NULL );
    sigaction( SIGSEGV, &handler, NULL );

    // ignored signals
    signal( SIGPIPE, SIG_IGN );

    std::set_terminate( __gnu_cxx::__verbose_terminate_handler );
}

void SignalHandler::handle_signal( int sig, siginfo_t*, void* )
{
    switch( sig )
    {
    case SIGHUP:
        is_hup_     = true;
        break;
    case SIGUSR1:
        is_usr1_    = true;
        break;
    case SIGUSR2:
        is_usr2_    = true;
    default:
    case SIGTERM:
        is_term_    = true;
        break;
    case SIGINT:
        is_int_     = true;
        break;
    }
}

void SignalHandler::handle_fatal_signal( int sig, siginfo_t* siginfo, void* )
{
    siginfo_t s;
    memset( &s, 0, sizeof( s ) );

    if( siginfo == NULL )
        siginfo = &s;

    struct sigaction default_handler;
    memset( &default_handler, 0, sizeof( default_handler ) );
    default_handler.sa_handler = SIG_DFL;

    sigaction( SIGSEGV, &default_handler, NULL );
    sigaction( SIGBUS,  &default_handler, NULL );
    sigaction( SIGABRT, &default_handler, NULL );
    sigaction( SIGFPE,  &default_handler, NULL );

    if( should_close_fds_ )
    {
        close_fds();
    }

    *(int*)0 = 0;
}

void SignalHandler::close_fds()
{
    int n = sysconf( _SC_OPEN_MAX );
    for( int i = 3; i < n; ++i )
        close( i );
}

bool SignalHandler::poll( std::atomic_bool & b )
{
    bool res = false;

    res = b.exchange( res );

    return res;
}

bool SignalHandler::poll_hup()
{
    return poll( is_hup_ );
}

bool SignalHandler::poll_int()
{
    return poll( is_int_ );
}

bool SignalHandler::poll_term()
{
    return poll( is_term_ );
}

bool SignalHandler::poll_abrt()
{
    return poll( is_abrt_ );
}

bool SignalHandler::poll_segv()
{
    return poll( is_segv_ );
}

bool SignalPoller::poll_hup()
{
    return SignalHandler::poll_hup();
}

bool SignalPoller::poll_int()
{
    return SignalHandler::poll_int();
}

bool SignalPoller::poll_term()
{
    return SignalHandler::poll_term();
}

bool SignalPoller::poll_abrt()
{
    return SignalHandler::poll_abrt();
}

bool SignalPoller::poll_segv()
{
    return SignalHandler::poll_segv();
}
