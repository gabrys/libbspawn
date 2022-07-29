#ifndef LIBBSPAWN_HPP_INCLUDED
#define LIBBSPAWN_HPP_INCLUDED

#define BSPAWN_STREAM_IGNORE 1
#define BSPAWN_STREAM_INHERIT 2
#define BSPAWN_STREAM_PIPE 3
#define BSPAWN_STREAM_STDOUT 4

#ifdef __cplusplus
extern "C"
#endif

int spawn_child(
    const char **argv,        // args (first is the program to run)
    const char *cwd,          // requested working directory
    const char **envp,        // environment to set in the standard form
    int timeout,              // timeout in milliseconds (0 means no limit)
    int stdin_config,         // one of: BSPAWN_STREAM_IGNORE, BSPAWN_STREAM_INHERIT, BSPAWN_STREAM_PIPE (read from stdin_data)
    int stdout_config,        // one of: BSPAWN_STREAM_IGNORE, BSPAWN_STREAM_INHERIT, BSPAWN_STREAM_PIPE (save to stdout_data)
    int stderr_config,        // one of: BSPAWN_STREAM_IGNORE, BSPAWN_STREAM_INHERIT, BSPAWN_STREAM_PIPE (save to stderr_data), BSPAWN_STREAM_STDOUT
                              // BSPAWN_STREAM_IGNORE   equivalent to 0 < /dev/null, 1 > /dev/null, 2 > /dev/null respectively
                              // BSPAWN_STREAM_INHERIT  attach the stdin/stdout/stderr to the parent's stdin/stdout/stderr. This is useful for interactive processes
                              // BSPAWN_STREAM_PIPE     feed the process data from stdin_data and save output to stdout_data and stderr_data respectively
                              // BSPAWN_STREAM_STDOUT   equivalent to 2>&1 (put stderr wherever stdout goes)
    const char *stdin_data,   // used if stdin_config=BSPAWN_STREAM_PIPE
    const char **stdout_data, // out: set if stdout_config=BSPAWN_STREAM_PIPE
    const char **stderr_data, // out: set if stderr_config=BSPAWN_STREAM_PIPE
    int *exit_code,           // out: exit code of the process
    const char **exit_message // out: error message
);

#endif /* LIBBSPAWN_HPP_INCLUDED */
