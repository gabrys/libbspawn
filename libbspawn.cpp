#include <iostream>
#include <boost/process.hpp>

#include "libbspawn.h"

namespace bp = boost::process;

bp::environment prepare_environment(const char **envp)
{
    bp::environment env;
    if (!envp)
        return env;

    for (int i = 0; envp[i] != NULL; i++)
    {
        char *name = strdup(envp[i]);
        char *eq = strchr(name, '=');
        if (eq)
        {
            char *val = eq + 1;
            *eq = '\0';
            env[name] = val;
        }
    }
    return env;
}

char *read_all_from_stream(std::istream &in)
{
    std::string ret;
    char buffer[4096];
    while (in.read(buffer, sizeof(buffer)))
        ret.append(buffer, sizeof(buffer));
    ret.append(buffer, in.gcount());
    int size = ret.size() + 1;

    char *output = (char *)malloc(size);
    strncpy(output, ret.c_str(), size);
    return output;
}

#define BSPAWN_STREAM_IGNORE 1
#define BSPAWN_STREAM_INHERIT 2
#define BSPAWN_STREAM_PIPE 3
#define BSPAWN_STREAM_STDOUT 4

#define _setup_stdin(stdin_policy, stdin_file, ...) (                                                                      \
    (stdin_policy == BSPAWN_STREAM_IGNORE) ? /*                              */ bp::child(__VA_ARGS__, bp::std_in < bp::null)   \
                                      : ((stdin_policy == BSPAWN_STREAM_PIPE) ? bp::child(__VA_ARGS__, bp::std_in < stdin_file) \
                                                                         : bp::child(__VA_ARGS__, bp::std_in < stdin)))

#define _setup_stdin_and_stdout(stdin_policy, stdin_file, stdout_policy, stdout_stream, ...) (                                                                 \
    (stdout_policy == BSPAWN_STREAM_IGNORE) ? /*                               */ _setup_stdin(stdin_policy, stdin_file, __VA_ARGS__, bp::std_out > bp::null)       \
                                       : ((stdout_policy == BSPAWN_STREAM_PIPE) ? _setup_stdin(stdin_policy, stdin_file, __VA_ARGS__, bp::std_out > *stdout_stream) \
                                                                           : _setup_stdin(stdin_policy, stdin_file, __VA_ARGS__, bp::std_out > stdout)))

template <typename... Types>
bp::child _start_process(
    int stdin_policy,
    std::FILE *stdin_file,
    int stdout_policy,
    bp::ipstream *stdout_stream,
    int stderr_policy,
    bp::ipstream *stderr_stream,
    Types... rest)
{
    // handle 2>&1 redirection:
    if (stderr_policy == BSPAWN_STREAM_STDOUT && stdout_policy == BSPAWN_STREAM_PIPE)
        return _setup_stdin(stdin_policy, stdin_file, rest..., bp::std_out > *stdout_stream, bp::std_err > *stdout_stream);

    int policy = stderr_policy;
    if (stderr_policy == BSPAWN_STREAM_STDOUT)
        policy = stdout_policy;

    if (policy == BSPAWN_STREAM_IGNORE)
        return _setup_stdin_and_stdout(stdin_policy, stdin_file, stdout_policy, stdout_stream, rest..., bp::std_err > bp::null);
    if (policy == BSPAWN_STREAM_PIPE)
        return _setup_stdin_and_stdout(stdin_policy, stdin_file, stdout_policy, stdout_stream, rest..., bp::std_err > *stderr_stream);
    return _setup_stdin_and_stdout(stdin_policy, stdin_file, stdout_policy, stdout_stream, rest..., bp::std_err > stderr);
}

int spawn_child(
    const char **argv, // args (first is the program to run)
    const char *cwd,   // requested working directory
    const char **envp, // environment to set in the standard form
    int timeout,       // timeout in milliseconds (0 means no limit)
    int stdin_policy,  // one of: BSPAWN_STREAM_IGNORE, BSPAWN_STREAM_INHERIT, BSPAWN_STREAM_PIPE (read from stdin_data)
    int stdout_policy, // one of: BSPAWN_STREAM_IGNORE, BSPAWN_STREAM_INHERIT, BSPAWN_STREAM_PIPE (save to stdout_data)
    int stderr_policy, // one of: BSPAWN_STREAM_IGNORE, BSPAWN_STREAM_INHERIT, BSPAWN_STREAM_PIPE (save to stderr_data)
    // * BSPAWN_STREAM_IGNORE   equivalent to 0</dev/null, 1>/dev/null, 2>/dev/null respectively
    // * BSPAWN_STREAM_INHERIT  attach the stdin/stdout/stderr to the parent's stdin/stdout/stderr. This is useful for interactive processes
    // * BSPAWN_STREAM_PIPE     feed the process data from stdin_data and save output to stdout_data and stderr_data respectively
    // * stderr_policy=BSPAWN_STREAM_STDOUT  equivalent to 2>&1 (put stderr wherever stdout goes)
    const char *stdin_data,   // used if stdin_policy=BSPAWN_STREAM_PIPE
    const char **stdout_data, // out: set if stdout_policy=BSPAWN_STREAM_PIPE
    const char **stderr_data, // out: set if stderr_policy=BSPAWN_STREAM_PIPE
    int *exit_code,           // out: exit code of the process
    const char **exit_message // out: error message
)
{
    bp::ipstream stdout_stream;
    bp::ipstream stderr_stream;
    std::error_code child_error;

    std::FILE *stdin_file = std::tmpfile();

    std::vector<std::string> child_args;

    for (int i = 0; argv[i] != NULL; i++)
        child_args.push_back(argv[i]);

    std::string child_cwd(cwd);

    // Search PATH: requires linking to boost::filesystem
    // boost::filesystem::path p = bp::search_path("bash");

    bp::child child = _start_process(
        stdin_policy,
        stdin_file,
        stdout_policy,
        &stdout_stream,
        stderr_policy,
        &stderr_stream,
        child_args,
        bp::start_dir = cwd,
        prepare_environment(envp));

    if (stdin_policy == BSPAWN_STREAM_PIPE)
    {
        if (stdin_data != NULL)
        {
            std::fputs(stdin_data, stdin_file);
            std::rewind(stdin_file);
        }
        std::fclose(stdin_file);
    }

    if (stdout_policy == BSPAWN_STREAM_PIPE)
        *stdout_data = read_all_from_stream(stdout_stream);

    if (stderr_policy == BSPAWN_STREAM_PIPE)
        *stderr_data = read_all_from_stream(stderr_stream);

    // boost::process::windows::hide — Hides the window and activates another window.

    // Timeout:
    // template<typename Rep, typename Period>
    //   bool wait_for(const std::chrono::duration< Rep, Period > & rel_time);
    // Wait for the child process to exit for a period of time.
    // Returns:
    // True if child exited while waiting.

    child.wait();

    int child_exit_code = child.exit_code();

    *exit_code = child_exit_code;

    // For now
    return 0;
}
