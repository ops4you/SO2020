#define _POSIX_C_SOURCE 200809L

#include "argus_conf.h"
#include "buf_io/buf_writer.h"
#include "comfy_io.h"
#include "parse_size.h"
#include "task/task.h"
#include "task/task_vec.h"

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUF_SIZE 8192ul

static char const* const program_name = "argus_server";
static int commands_fd;
static int running_tasks_fd;
static int finished_tasks_fd;
static TaskVec running_tasks;
static TaskVec finished_tasks;
static BufWriter running_tasks_writer;
static BufWriter finished_tasks_writer;
static size_t total_tasks;

static size_t count_char(char const* s, char const c) {
    size_t count = 0ul;
    for ( ; *s; ++s) {
        if (*s == c) {
            ++count;
        }
    }
    return count;
}

static size_t count_words(char const* s) {
    size_t count = 0ul;
    while (*s) {
        while (isspace(*s)) {
            ++s;
        }
        if (*s) {
            ++count;
            while (*s && !isspace(*s)) {
                ++s;
            }
        }
    }
    return count;
}

static void close_commands_fifo(void) {
    if (close(commands_fd) == 1) {
        program_eprintln(
            "Failed closing commands fifo: %s.",
            strerror(errno)
        );
    }
}

static void close_running_tasks_fifo(void) {
    if (close(running_tasks_fd) == -1) {
        program_eprintln(
            "Failed closing the running tasks fifo: %s.",
            strerror(errno)
        );
    }
}

static void close_finished_tasks_fifo(void) {
    if (close(finished_tasks_fd) == -1) {
        program_eprintln(
            "Failed closing the finished tasks fifo: %s.",
            strerror(errno)
        );
    }
}

static void drop_running_tasks_writer(void) {
    BwOutcome const drop_outcome = bw_drop(&running_tasks_writer);
    if (drop_outcome != BW_OK) {
        program_eprintln(
            "Failed dropping the running tasks fifo buffered writer: %s.",
            bw_outcome_msg(drop_outcome, &errno)
        );
    }
}

static void drop_finished_tasks_writer(void) {
    BwOutcome const drop_outcome = bw_drop(&finished_tasks_writer);
    if (drop_outcome != BW_OK) {
        program_eprintln(
            "Failed dropping the finished tasks fifo buffered writer: %s.",
            bw_outcome_msg(drop_outcome, &errno)
        );
    }
}

static void drop_task_vecs(void) {
    tvec_drop(&running_tasks);
    tvec_drop(&finished_tasks);
}

static void server_sighandler(int const signum) {
    Task const* const end = tvec_end(&running_tasks);
    switch (signum) {
    case SIGINT:
    case SIGTERM:
        for (Task* i = tvec_begin_mut(&running_tasks); i != end; ++i) {
            kill(-(i->process_group), signum);
        }
        exit(0);
        break;
    default:
        break;
    }
}

int main(void) {
    if (mkdir(server_dirname, 0777) != 0 && errno != EEXIST) {
        program_eprintln(
            "Failed creating server directory: %s.",
            strerror(errno)
        );
        return EXIT_FAILURE;
    }

    if (mkfifo(commands_fifoname, 0666) != 0 && errno != EEXIST) {
        program_eprintln(
            "Failed creating commands fifo: %s.",
            strerror(errno)
        );
        return EXIT_FAILURE;
    }

    if (mkfifo(running_tasks_fifoname, 0666) != 0 && errno != EEXIST) {
        program_eprintln(
            "Failed creating running tasks fifo: %s.",
            strerror(errno)
        );
        return EXIT_FAILURE;
    }

    if (mkfifo(finished_tasks_fifoname, 0666) != 0 && errno != EEXIST) {
        program_eprintln(
            "Failed creating finished tasks fifo: %s.",
            strerror(errno)
        );
        return EXIT_FAILURE;
    }

    if ((commands_fd = open(commands_fifoname, O_RDONLY | O_CLOEXEC)) == -1) {
        program_eprintln(
            "Failed opening the commands fifo: %s.",
            strerror(errno)
        );
        return EXIT_FAILURE;
    }
    atexit(close_commands_fifo);

    if ((running_tasks_fd =
        open(running_tasks_fifoname, O_WRONLY | O_CREAT | O_CLOEXEC, 0666) == -1
    )) {
        program_eprintln(
            "Failed opening the running tasks fifo: %s.",
            strerror(errno)
        );
        return EXIT_FAILURE;
    }
    atexit(close_running_tasks_fifo);

    if ((finished_tasks_fd =
        open(finished_tasks_fifoname, O_WRONLY | O_CREAT | O_CLOEXEC , 0666) ==
        -1
    )) {
        program_eprintln(
            "Failed opening the finished tasks fifo: %s.",
            strerror(errno)
        );
        return EXIT_FAILURE;
    }
    atexit(close_finished_tasks_fifo);

    BwOutcome const running_tasks_writer_init_outcome =
        bw_with_default_cap(&running_tasks_writer, running_tasks_fd);
    if (running_tasks_writer_init_outcome != BW_OK) {
        program_eprintln(
            "Failed initializing the running tasks fifo buffered writer:"
                " %s.",
            bw_outcome_msg(running_tasks_writer_init_outcome, &errno)
        );
        return EXIT_FAILURE;
    }
    atexit(drop_running_tasks_writer);

    BwOutcome const finished_tasks_writer_init_outcome =
        bw_with_default_cap(&finished_tasks_writer, finished_tasks_fd);
    if (finished_tasks_writer_init_outcome != BW_OK) {
        program_eprintln(
            "Failed initializing the finished tasks fifo buffered writer:"
                " %s.",
            bw_outcome_msg(finished_tasks_writer_init_outcome, &errno)
        );
        return EXIT_FAILURE;
    }
    atexit(drop_finished_tasks_writer);

    TaskVec running_tasks;
    tvec_new(&running_tasks);
    TaskVec finished_tasks;
    tvec_new(&finished_tasks);
    atexit(drop_task_vecs);

    struct sigaction action = { 0 };
	action.sa_handler = server_sighandler;
	if (sigaction(SIGTERM, &action, NULL) == -1 ||
        sigaction(SIGINT, &action, NULL) == -1
    ) {
        program_eprintln(
            "Failed setting server signal handler: %s.",
            strerror(errno)
        );
        return EXIT_FAILURE;
    }

    for (;;) {
        char line_buf[LINE_BUF_SIZE];
        ssize_t const read_bytes =
            read(commands_fd, line_buf, LINE_BUF_SIZE);
        if (read_bytes == -1l) {
            program_eprintln(
                "Failed reading a line from the commands fifo: %s.",
                strerror(errno)
            );
            return EXIT_FAILURE;
        }
        if (read_bytes == 0ul) {
            continue;
        }
        line_buf[read_bytes] = '\0';

        switch (line_buf[0]) {
        case EXEC_TASK_FLAG: {
            pid_t const pid = fork();
            switch (pid) {
            case -1:
                program_eprintln(
                    "Failed creating a new process: %s.",
                    strerror(errno)
                );
                return EXIT_FAILURE;
            case 0:
                break;
            default:
                tvec_push(&running_tasks, &(Task) {
                    .task_id = total_tasks++,
                    .task_name = line_buf + 2ul,
                    .process_group = pid
                });

                continue;
                break;
            }
            setsid();
            // line_buf[] = "e p1 arg1 arg2 | p2 | p3\0"
            // cmd_start = "p1 arg1 arg2 | p2 | p3\0"
            char* const cmd_start = strndup(line_buf + 2ul, read_bytes);
            size_t const proc_count = count_char(cmd_start, '|') + 1ul;
            size_t const procs_size = proc_count * sizeof(char*);
            char** const procs = malloc(procs_size);
            char* const* const procs_end = procs + procs_size;

            procs[0] = strtok(cmd_start, "|");
            for (char** i = procs + 1; i != procs_end; ++i) {
                *i = strtok(NULL, "|");
            }
            // procs[] = { "p1 arg1 arg2 \0, p2\0, p3\0"}

            for (char* const* proc_i = procs; proc_i != procs_end; ++proc_i) {
                // argc = 3
                size_t const argc = count_words(*proc_i);
                char** const argv = malloc((argc + 1) * sizeof(char*));
                char** argv_i = argv;
                argv[argc] = NULL;
                // argv[] = { _ _ _ NULL }

                // *proc_i = "p1 arg1 arg2\0"
                char* char_i = *proc_i;
                while (*char_i) {
                    *argv_i++ = char_i;
                    while (*char_i && !isspace(*char_i)) {
                        ++char_i;
                    }
                    *char_i++ = '\0';
                }
                // proc_i = "{ p1\0 arg1\0 arg2\0, p2\0, p3\0 }"
                // argv[] = { p1\0, arg1\0, arg2\0, NULL }

                int pipe_fd[2];
                pipe(pipe_fd);

                pid_t const pid = fork();
                switch (pid) {
                case -1:
                    program_eprintln(
                        "Failed creating a new process: %s.",
                        strerror(errno)
                    );
                    return EXIT_FAILURE;
                case 0: {
                    setsid();
                    dup2(pipe_fd[0], STDIN_FILENO);
                    execv(cmd_start, argv);
                    break;
                }
                default:
                    dup2(pipe_fd[1], STDOUT_FILENO);
                    break;
                }
            }
            free(procs);
            free(cmd_start);
        }
        break;

        case END_TASK_FLAG: {
            size_t task_id;
            if (parse_size_slice(
                line_buf + 2,
                line_buf + read_bytes,
                &task_id, NULL) != PARSE_SIZE_OK
            ) {
                break;
            }
            size_t task_idx;
            Task* const scheduled_for_deletion =
                tvec_search_by_tid_mut(&running_tasks, task_id, &task_idx);
            if (!scheduled_for_deletion) {
                break;
            }
            if (kill(-(scheduled_for_deletion->process_group), SIGTERM) == -1) {
                program_eprintln(
                    "Failed killing task '%s' with group process id %d: %s.",
                    scheduled_for_deletion->task_name,
                    scheduled_for_deletion->process_group,
                    strerror(errno)
                );
                return EXIT_FAILURE;
            }
            tvec_rm_ord_at(&running_tasks, task_idx);
            break;
        }

        case SET_ACTIVE_TIMEOUT_FLAG: { break; }
        case SET_INACTIVE_TIMEOUT_FLAG: { break; }

        case LIST_RUNNING_TASKS_FLAG: {
            Task const* const end = tvec_end(&running_tasks);
            for (Task const* i = tvec_begin(&running_tasks); i != end; ++i) {
                BwOutcome const write_line_outcome =
                    bw_write_line(
                        &running_tasks_writer,
                        i->task_name,
                        strlen(i->task_name)
                    );
                if (write_line_outcome != BW_OK) {
                    program_eprintln(
                        "Failed writing a line from the running tasks fifo"
                            " buffered writer: %s.",
                        bw_outcome_msg(write_line_outcome, &errno)
                    );
                    return EXIT_FAILURE;
                }
            }
            BwOutcome const bw_flush_outcome = bw_flush(&running_tasks_writer);
            if (bw_flush_outcome != BW_OK) {
                program_eprintln(
                    "Failed flushing the running tasks from the running fifo"
                        " buffered writer: %s.",
                    bw_outcome_msg(bw_flush_outcome, &errno)
                );
                return EXIT_FAILURE;
            }
            break;

            break;
        }

        case LIST_FINISHED_TASKS_FLAG: {
            Task const* const end = tvec_end(&finished_tasks);
            for (Task const* i = tvec_begin(&finished_tasks); i != end; ++i) {
                BwOutcome const write_line_outcome =
                    bw_write_line(
                        &finished_tasks_writer,
                        i->task_name,
                        strlen(i->task_name)
                    );
                if (write_line_outcome != BW_OK) {
                    program_eprintln(
                        "Failed writing a line from the finished tasks fifo"
                            " buffered writer: %s.",
                        bw_outcome_msg(write_line_outcome, &errno)
                    );
                    return EXIT_FAILURE;
                }
            }
            BwOutcome const bw_flush_outcome = bw_flush(&finished_tasks_writer);
            if (bw_flush_outcome != BW_OK) {
                program_eprintln(
                    "Failed flushing the finished tasks from the finished fifo"
                        " buffered writer: %s.",
                    bw_outcome_msg(bw_flush_outcome, &errno)
                );
                return EXIT_FAILURE;
            }
        }

        default:
            break;
        }
    }
}
