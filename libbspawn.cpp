#include <iostream>
#include <boost/process.hpp>

#include "libbspawn.h"

namespace bp = boost::process;
namespace fs = boost::filesystem;

extern "C" char **environ;

bp::environment prepare_environment(const char **envp)
{
    bp::environment env;

    if (envp == NULL)
        envp = (const char **) environ;

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

#define _setup_stdin(stdin_policy, stdin_path, ...) (                                                                                \
    (stdin_policy == BSPAWN_STREAM_IGNORE) ? /*                              */ bp::child(__VA_ARGS__, bp::std_in < bp::null)        \
                                           : ((stdin_policy == BSPAWN_STREAM_PIPE) ? bp::child(__VA_ARGS__, bp::std_in < stdin_path) \
                                                                                   : bp::child(__VA_ARGS__, bp::std_in < stdin)))

#define _setup_stdin_and_stdout(stdin_policy, stdin_path, stdout_policy, stdout_stream, ...) (                                                                           \
    (stdout_policy == BSPAWN_STREAM_IGNORE) ? /*                               */ _setup_stdin(stdin_policy, stdin_path, __VA_ARGS__, bp::std_out > bp::null)            \
                                            : ((stdout_policy == BSPAWN_STREAM_PIPE) ? _setup_stdin(stdin_policy, stdin_path, __VA_ARGS__, bp::std_out > *stdout_stream) \
                                                                                     : _setup_stdin(stdin_policy, stdin_path, __VA_ARGS__, bp::std_out > stdout)))

template <typename... Types>
bp::child _start_process(
    int stdin_policy,
    std::string stdin_path,
    int stdout_policy,
    bp::ipstream *stdout_stream,
    int stderr_policy,
    bp::ipstream *stderr_stream,
    Types... rest)
{
    // handle 2>&1 redirection:
    if (stderr_policy == BSPAWN_STREAM_STDOUT && stdout_policy == BSPAWN_STREAM_PIPE)
        return _setup_stdin(stdin_policy, stdin_path, rest..., bp::std_out > *stdout_stream, bp::std_err > *stdout_stream);

    int policy = stderr_policy;
    if (stderr_policy == BSPAWN_STREAM_STDOUT)
        policy = stdout_policy;

    if (policy == BSPAWN_STREAM_IGNORE)
        return _setup_stdin_and_stdout(stdin_policy, stdin_path, stdout_policy, stdout_stream, rest..., bp::std_err > bp::null);
    if (policy == BSPAWN_STREAM_PIPE)
        return _setup_stdin_and_stdout(stdin_policy, stdin_path, stdout_policy, stdout_stream, rest..., bp::std_err > *stderr_stream);
    return _setup_stdin_and_stdout(stdin_policy, stdin_path, stdout_policy, stdout_stream, rest..., bp::std_err > stderr);
}

std::string build_temporary_path() {
    std::string tmp_name = "pow-" + boost::filesystem::unique_path().string() + ".txt";
    fs::path tmp_path = boost::filesystem::temp_directory_path() / tmp_name;
    return tmp_path.string();
}

int spawn_child(
    const char **argv,
    const char *cwd,
    const char **envp,
    int timeout,
    int stdin_policy,
    int stdout_policy,
    int stderr_policy,
    const char *stdin_data,
    const char **stdout_data,
    const char **stderr_data,
    int *exit_code,
    const char **exit_message)
{
    std::string stdin_path = build_temporary_path();

    bp::ipstream stdout_stream;
    bp::ipstream stderr_stream;

    std::error_code child_error;

    if (argv == NULL || argv[0] == NULL)
    {
        std::cerr << "Incorrect argv passed" << std::endl;
        return 6;

    }
    fs::path cmd_path(argv[0]);
    // TODO: use fs::path - leaf == full
    if (strchr(argv[0], '/') == NULL && strchr(argv[0], '\\') == NULL)
    {
        cmd_path = bp::search_path(argv[0]);
        if (cmd_path.empty())
        {
            std::cerr << "Didn't find cmd in PATH " << argv[0] << std::endl;
            return 7;
        }
    }
    // if (!fs::exists(cmd_path))
    // {
    //     std::cerr << "cmd is not a valid command " << argv[0] << << std::endl;
    //     return 9;
    // }

    std::vector<std::string> cmd_args;
    for (int i = 1; argv[i] != NULL; i++)
        cmd_args.push_back(argv[i]);

    if (stdin_policy == BSPAWN_STREAM_PIPE)
    {
        FILE *stdin_file = std::fopen(stdin_path.c_str(), "w");
        if (stdin_data != NULL)
            std::fputs(stdin_data, stdin_file);
        std::fclose(stdin_file);
    }

    bp::child child = _start_process(
        stdin_policy,
        stdin_path,
        stdout_policy,
        &stdout_stream,
        stderr_policy,
        &stderr_stream,
        cmd_path,
        cmd_args,
        bp::start_dir = cwd,
        prepare_environment(envp));

    if (stdout_policy == BSPAWN_STREAM_PIPE)
        *stdout_data = read_all_from_stream(stdout_stream);

    if (stderr_policy == BSPAWN_STREAM_PIPE)
        *stderr_data = read_all_from_stream(stderr_stream);

    // boost::process::windows::hide ??? Hides the window and activates another window.

    // Timeout:
    // template<typename Rep, typename Period>
    //   bool wait_for(const std::chrono::duration< Rep, Period > & rel_time);
    // Wait for the child process to exit for a period of time.
    // Returns:
    // True if child exited while waiting.

    child.wait();

    int child_exit_code = child.exit_code();

    if (exit_code)
        *exit_code = child_exit_code;

    if (stdin_policy == BSPAWN_STREAM_PIPE)
        fs::remove(stdin_path);

    // For now
    return 0;
}
