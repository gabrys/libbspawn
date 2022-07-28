# libbspawn

A small library wrapping Boost::Process and exposing an API for C programs

# Motivation

I needed a simple way to spawn programs on Windows from C. I found surprisingly few
C libraries doing this, notably `reproc` and `subprocess_h`, but they have thier issues.

An alternative solution could be pulling a big library like GLib or APR (from Apache)
but it doesn't look like a lot of fun.

Then I found Boost::Process and the only issue was it didn't expose a C API, so I made one:

# API

The library exposes one function:

```
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
);
```

# TODO

- [ ] More docs
- [ ] Support timeout and exit_message
- [ ] Find the executable in PATH
- [ ] Disable running the command via shell automatically
- [ ] Better handle errors
- [ ] Add tests
