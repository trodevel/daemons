/*

Daemon.

Copyright (C) 2014 Sergey Kolevatov

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

// $Revision: 1813 $ $Date:: 2015-06-03 #$ $Author: serge $

#include "daemon.h"                     // self

#include <unistd.h>                     // close
#include <sys/stat.h>                   // umask

#include "../utils/dummy_logger.h"      // dummy_log_error

using namespace daemons;

bool Deamon::is_demonized_  = false;

#define MODULENAME "Deamon"

bool Deamon::daemonize()
{
    if( is_demonized_ )
    {
        dummy_log_error( MODULENAME, "already daemonized" );
        return false;
    }

    // Create child process
    pid_t process_id = fork();

    // Indication of fork() failure
    if( process_id < 0 )
    {
        dummy_log_fatal( MODULENAME, "fork failed" );
        // Return failure in exit status
        ::exit( 1 );
    }

    // PARENT PROCESS. Need to kill it.
    if( process_id > 0 )
    {
        dummy_log_info( MODULENAME, "process_id of child process %d", process_id );
        // return success in exit status
        ::exit( 0 );
    }

    // unmask the file mode
    ::umask( 0 );
    // set new session
    pid_t sid = setsid();
    if( sid < 0 )
    {
        dummy_log_fatal( MODULENAME, "cannot create a session" );
        // Return failure
        ::exit( 1 );
    }
    // Change the current working directory to root.
    // chdir( "/" );
    // Close stdin. stdout and stderr
    close( STDIN_FILENO );
    close( STDOUT_FILENO );
    close( STDERR_FILENO );

    is_demonized_   = true;

    dummy_log_info( MODULENAME, "demonized" );

    return true;
}
