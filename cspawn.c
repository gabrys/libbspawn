#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libbspawn.h"

int main(int argc, char **argv)
{
    const char *cwd = "/";
    const char *exec_argv[1000];
    const char *envp[1000];
    const char *stdin_data = "(none)";
    const char *stdout_data = "(none)";
    const char *stderr_data = "(none)";

    int timeout = 0;
    int envp_len = 0;
    int exec_argv_len = 0;

    int stdin_policy = BSPAWN_STREAM_INHERIT;
    int stdout_policy = BSPAWN_STREAM_INHERIT;
    int stderr_policy = BSPAWN_STREAM_INHERIT;

    const char *policy_names[5];
    policy_names[1] = "ignore";
    policy_names[2] = "inherit";
    policy_names[3] = "pipe";
    policy_names[4] = "stdout";

    int exit_code;
    const char *error_message = "(none)";

    int i;
    for (i = 1; i < argc; i++)
    {
        const char *arg = argv[i];
        if (strcmp(arg, "--") == 0)
        {
            i++;
            break;
        }
        if (strncmp(arg, "cwd=", 4) == 0)
            cwd = arg + 4;
        else if (strcmp(arg, "in=ignore") == 0)
            stdin_policy = BSPAWN_STREAM_IGNORE;
        else if (strncmp(arg, "in=", 3) == 0)
        {
            stdin_policy = BSPAWN_STREAM_PIPE;
            stdin_data = arg + 3;
        }
        else if (strcmp(arg, "out=ignore") == 0)
            stdout_policy = BSPAWN_STREAM_IGNORE;
        else if (strcmp(arg, "out=pipe") == 0)
            stdout_policy = BSPAWN_STREAM_PIPE;
        else if (strcmp(arg, "err=ignore") == 0)
            stderr_policy = BSPAWN_STREAM_IGNORE;
        else if (strcmp(arg, "err=pipe") == 0)
            stderr_policy = BSPAWN_STREAM_PIPE;
        else if (strcmp(arg, "err=out") == 0)
            stderr_policy = BSPAWN_STREAM_STDOUT;
        else if (strncmp(arg, "timeout=", 8) == 0)
            timeout = atol(arg + 8);
        // set envs like this: @var=name
        else if (arg[0] == '@' && strchr(arg, '='))
            envp[envp_len++] = arg + 1;
        else
            exec_argv[exec_argv_len++] = arg;
    }

    // Anything after -- is command and args
    for (; i < argc; i++)
        exec_argv[exec_argv_len++] = argv[i];

    envp[envp_len] = NULL;
    exec_argv[exec_argv_len] = NULL;

    if (exec_argv_len == 0)
    {
        printf("No command specified\n");
        return 1;
    }

    printf("About to spawn\n");
    printf("\n");
    printf("  cwd:     %s\n", cwd);
    if (stdin_policy == BSPAWN_STREAM_PIPE)
        printf("  stdin:  << %s\n", stdin_data);
    else
        printf("  stdin:   %s\n", policy_names[stdin_policy]);
    printf("  stdout:  %s\n", policy_names[stdout_policy]);
    printf("  stderr:  %s\n", policy_names[stderr_policy]);
    printf("  timeout: ");
    if (timeout)
        printf("%d ms\n", timeout);
    else
        printf("not set\n");
    printf("  cmd:    ");
    for (i = 0; i < exec_argv_len; i++)
        printf(" \"%s\"", exec_argv[i]);
    printf("\n");
    printf("  envp:\n");
    for (i = 0; i < envp_len; i++)
        printf("    %s\n", envp[i]);
    printf("\n");

    printf("=================\n");

    spawn_child(
        exec_argv,
        cwd,
        envp_len == 0 ? NULL : envp,
        timeout,
        stdin_policy,
        stdout_policy,
        stderr_policy,
        stdin_data,
        &stdout_data,
        &stderr_data,
        &exit_code,
        &error_message);

    printf("\n");
    printf("=================\n");
    printf("\n");
    // printf("Error message: ", error_message);
    printf("exit code:    %d\n", exit_code);
    printf("stdout data:  %s\n", stdout_data);
    printf("stderr data:  %s\n", stderr_data);
}
