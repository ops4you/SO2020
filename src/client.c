#include "argus_conf.h"
#include "buf_io/buf_writer.h"
#include "comfy_io.h"
#include "parse_size.h"

#include <fcntl.h>
#include <unistd.h>

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define try_write_cmd_(cmd, argv) \
    if (write_cmd(cmd, (char const**) argv) == EXIT_FAILURE) return EXIT_FAILURE

#define try_write_cmd_i_(cmd, arg_start, line_end) \
    if (write_cmd_i(cmd, arg_start, line_end) == EXIT_FAILURE) \
        return EXIT_FAILURE

#define LINE_BUF_SIZE 8192ul

static char const* const program_name = "argus";
static int commands_fd;
static int running_tasks_fd;
static int finished_tasks_fd;
static BufWriter commands_writter;

static char const arg_strs[][2] = {
    "e ", "t ", "m ", "i ", "l ", "r ", "h ",
};

typedef enum {
    EXEC_TASK,
    END_TASK,
    SET_ACTIVE_TIMEOUT,
    SET_INACTIVE_TIMEOUT,
    LIST_RUNNING_TASKS,
    LIST_FINISHED_TASKS,
    HELP,
} Command;

static int write_cmd(Command const cmd, char const* const argv[const]) {
    switch (cmd) {
        case EXEC_TASK:
        case END_TASK:
        case SET_ACTIVE_TIMEOUT:
        case SET_INACTIVE_TIMEOUT: {
            BwOutcome const bw_write_outcome =
                bw_write(
                    &commands_writter,
                    arg_strs[cmd],
                    sizeof arg_strs[cmd]
                );
            if (bw_write_outcome != BW_OK) {
                program_eprintln(
                    "Failed writing a command to the commands fifo buffered"
                        " writer: %s.",
                    bw_outcome_msg(bw_write_outcome, &errno)
                );
                return EXIT_FAILURE;
            }
            BwOutcome const bw_write_line_outcome =
                bw_write_line(
                    &commands_writter,
                    argv[2],
                    strlen(argv[2])
                );
            if (bw_write_line_outcome != BW_OK) {
                program_eprintln(
                    "Failed writing a command to the commands fifo buffered"
                        " writer: %s.",
                    bw_outcome_msg(bw_write_line_outcome, &errno)
                );
                return EXIT_FAILURE;
            }
            BwOutcome const bw_flush_outcome = bw_flush(&commands_writter);
            if (bw_flush_outcome != BW_OK) {
                program_eprintln(
                    "Failed flushing commands from the commands fifo buffered"
                        " writer: %s.",
                    bw_outcome_msg(bw_flush_outcome, &errno)
                );
                return EXIT_FAILURE;
            }
            break;
        }
        case LIST_RUNNING_TASKS:
        case LIST_FINISHED_TASKS: {
            BwOutcome const bw_write_line_outcome =
                bw_write_line(
                    &commands_writter,
                    arg_strs[cmd],
                    sizeof arg_strs[cmd]
                );
            if (bw_write_line_outcome != BW_OK) {
                program_eprintln(
                    "Failed writing a command to the commands fifo buffered"
                        " writer:  %s.",
                    bw_outcome_msg(bw_write_line_outcome, &errno)
                );
                return EXIT_FAILURE;
            }
            BwOutcome const bw_flush_outcome = bw_flush(&commands_writter);
            if (bw_flush_outcome != BW_OK) {
                program_eprintln(
                    "Failed flushing commands from the commands fifo buffered"
                        " writer: %s.",
                    bw_outcome_msg(bw_flush_outcome, &errno)
                );
                return EXIT_FAILURE;
            }
            break;
        }
        case HELP:
            break;
        default:
            break;
    }
    return EXIT_SUCCESS;
}

static int write_cmd_i(
    Command const cmd,
    char const* const arg_start,
    char const* const line_end
) {
    switch (cmd) {
        case EXEC_TASK:
        case END_TASK:
        case SET_ACTIVE_TIMEOUT:
        case SET_INACTIVE_TIMEOUT: {
            BwOutcome const bw_write_outcome =
                bw_write(
                    &commands_writter,
                    arg_strs[cmd],
                    sizeof arg_strs[cmd]
                );
            if (bw_write_outcome != BW_OK) {
                eprintln(
                    "Failed writing a command to the commands fifo buffered"
                        " writer: %s.",
                    bw_outcome_msg(bw_write_outcome, &errno)
                );
                return EXIT_FAILURE;
            }
            BwOutcome const bw_write_line_outcome =
                bw_write_line(
                    &commands_writter,
                    arg_start,
                    line_end - arg_start
                );
            if (bw_write_line_outcome != BW_OK) {
                eprintln(
                    "Failed writing a command to the commands fifo buffered"
                        " writer: %s.",
                    bw_outcome_msg(bw_write_line_outcome, &errno)
                );
                return EXIT_FAILURE;
            }
            BwOutcome const bw_flush_outcome = bw_flush(&commands_writter);
            if (bw_flush_outcome != BW_OK) {
                eprintln(
                    "Failed flushing commands from the commands fifo buffered"
                        " writer: %s.",
                    bw_outcome_msg(bw_flush_outcome, &errno)
                );
                return EXIT_FAILURE;
            }
            break;
        }
        case LIST_RUNNING_TASKS:
        case LIST_FINISHED_TASKS: {
            BwOutcome const bw_write_line_outcome =
                bw_write_line(
                    &commands_writter,
                    arg_strs[cmd],
                    sizeof arg_strs[cmd]
                );
            if (bw_write_line_outcome != BW_OK) {
                eprintln(
                    "Failed writing a command to the commands fifo buffered"
                        " writer:  %s.",
                    bw_outcome_msg(bw_write_line_outcome, &errno)
                );
                return EXIT_FAILURE;
            }
            BwOutcome const bw_flush_outcome = bw_flush(&commands_writter);
            if (bw_flush_outcome != BW_OK) {
                eprintln(
                    "Failed flushing commands from the commands fifo buffered"
                        " writer: %s.",
                    bw_outcome_msg(bw_flush_outcome, &errno)
                );
                return EXIT_FAILURE;
            }
            break;
        }
        case HELP:
            break;
        default:
            break;
    }
    return EXIT_SUCCESS;
}

static bool is_empty_str(char const* begin, char const* const end) {
    for ( ; begin != end; ++begin) {
        if (!isspace(*begin)) {
            false;
        }
    }
    return true;
}

static void print_help(void) {
    printf(
        "Usage: %s [options]\n"
        "Options:\n"
        "  -%c [task1 | task2 | ...]\t%s.\n"
        "  -%c n\t\t\t\t%s.\n"
        "  -%c n\t\t\t\t%s.\n"
        "  -%c n\t\t\t\t%s.\n"
        "  -%c\t\t\t\t%s.\n"
        "  -%c\t\t\t\t%s.\n"
        "  -%c\t\t\t\t%s.\n",
        program_name,
        EXEC_TASK_FLAG, "Execute a task",
        END_TASK_FLAG, "End a task with id 'n'",
        SET_ACTIVE_TIMEOUT_FLAG, "Set a timeout of n seconds for task activity",
        SET_INACTIVE_TIMEOUT_FLAG, "Set a timeout of n seconds for task"
            " inactivity",
        LIST_RUNNING_TASKS_FLAG, "List all active tasks",
        LIST_FINISHED_TASKS_FLAG, "List all finished tasks",
        HELP_FLAG, "Display this message"
    );
}

static void close_commands_fifo(void) {
    if (close(commands_fd) == -1) {
        program_eprintln(
            "Failed closing the commands fifo: %s.",
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

static void drop_commands_writer(void) {
    BwOutcome const drop_outcome = bw_drop(&commands_writter);
    if (drop_outcome != BW_OK) {
        program_eprintln(
            "Failed dropping the commands fifo buffered writer: %s.",
            bw_outcome_msg(drop_outcome, &errno)
        );
    }
}

static Command* find_cmd(
    char const* const restrict word_start,
    size_t const word_len,
    Command* const restrict cmd
) {
    if (strncmp(word_start, exec_task_cmd, word_len) == 0) {
        *cmd = EXEC_TASK;
    } else if (strncmp(word_start, end_task_cmd, word_len) == 0) {
        *cmd = END_TASK;
    } else if (strncmp(word_start, set_active_timeout_cmd, word_len) == 0) {
        *cmd = SET_ACTIVE_TIMEOUT;
    } else if (strncmp(word_start, set_inactive_timeout_cmd, word_len) == 0) {
        *cmd = SET_INACTIVE_TIMEOUT;
    } else if (strncmp(word_start, list_running_tasks_cmd, word_len) == 0) {
        *cmd = LIST_RUNNING_TASKS;
    } else if (strncmp(word_start, list_finished_tasks_cmd, word_len) == 0) {
        *cmd = LIST_FINISHED_TASKS;
    } else if (strncmp(word_start, help_cmd, word_len) == 0) {
        *cmd = HELP;
    } else {
        return NULL;
    }
    return cmd;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {  // no args, interactive mode
        if ((commands_fd = open(commands_fifoname, O_WRONLY)) == -1) {
            program_eprintln(
                "Failed opening the commands fifo: %s.",
                strerror(errno)
            );
            return EXIT_FAILURE;
        }
        atexit(close_commands_fifo);

        if ((running_tasks_fd = open(running_tasks_fifoname, O_RDONLY)) == -1) {
            program_eprintln(
                "Failed opening the running tasks fifo: %s.",
                strerror(errno)
            );
            return EXIT_FAILURE;
        }
        atexit(close_running_tasks_fifo);

        if ((finished_tasks_fd = open(finished_tasks_fifoname, O_RDONLY)) ==
            -1
        ) {
            program_eprintln(
                "Failed opening the finished tasks fifo: %s.",
                strerror(errno)
            );
            return EXIT_FAILURE;
        }
        atexit(close_finished_tasks_fifo);

        BwOutcome const commands_fifo_writer_init_outcome =
            bw_with_default_cap(&commands_writter, commands_fd);
        if (commands_fifo_writer_init_outcome != BW_OK) {
            program_eprintln(
                "Failed initializing the commands fifo buffered writer: %s.",
                bw_outcome_msg(commands_fifo_writer_init_outcome, &errno)
            );
            return EXIT_FAILURE;
        }
        atexit(drop_commands_writer);

        for (;;) {
            char line_buf[LINE_BUF_SIZE];
            ssize_t const read_bytes =
                read(STDIN_FILENO, line_buf, LINE_BUF_SIZE);
            if (read_bytes == -1l) {
                eprintln(
                    "Failed reading a line from stdin: %s.",
                    strerror(errno)
                );
                return EXIT_FAILURE;
            }
            if (read_bytes == 0ul) {
                return EXIT_SUCCESS;
            }
            char const* const line_end = line_buf + read_bytes;
            char const* i = line_buf;
            while (i != line_end && isspace(*i)) ++i;
            char const* const first_word_start = i;
            while (i != line_end && !isspace(*i)) ++i;
            size_t const first_word_len = i - first_word_start;
            if (first_word_len == 0ul) {
                eputs("Expected a command");
                continue;
            }
            Command cmd;
            if (!find_cmd(first_word_start, first_word_len, &cmd)) {
                eprintln(
                    "Unknown command %.*s\n",
                    (int) first_word_len,
                    first_word_start
                );
                continue;
            }

            // i is at the second word start, or at the end of the line
            switch (cmd) {
            case EXEC_TASK: {
                if (is_empty_str(i, line_end)) {
                    eputs("Expected a task to execute.");
                    continue;
                }
                try_write_cmd_i_(EXEC_TASK, i, line_end);
                break;
            }

            case END_TASK: {
                if (is_empty_str(i, line_end)) {
                    eputs("Expected task id.");
                    continue;
                }
                size_t task_id;
                char maybe_inv_char;
                ParseSizeOutcome const parse_outcome =
                    parse_size_slice(i, line_end, &task_id, &maybe_inv_char);
                if (parse_outcome != PARSE_SIZE_OK) {
                    eprintf(
                        "Failed to parse task id: %s",
                        parse_size_outcome_msg(parse_outcome)
                    );
                    if (parse_outcome == PARSE_SIZE_ERR_INV_CHAR) {
                        eprintf(" '%c'", maybe_inv_char);
                    }
                    eputs(".");
                    continue;
                }
                try_write_cmd_i_(END_TASK, i, line_end);
                break;
            }

            case SET_ACTIVE_TIMEOUT: {
                if (is_empty_str(i, line_end)) {
                    eputs("Expected active task timeout value");
                    continue;
                }
                size_t active_task_timeout;
                char maybe_inv_char;
                ParseSizeOutcome const parse_outcome =
                    parse_size_slice(
                        i,
                        line_end,
                        &active_task_timeout,
                        &maybe_inv_char
                    );
                if (parse_outcome != PARSE_SIZE_OK) {
                    eprintf(
                        "Failed to parse active task timeout: %s",
                        parse_size_outcome_msg(parse_outcome)
                    );
                    if (parse_outcome == PARSE_SIZE_ERR_INV_CHAR) {
                        eprintf(" '%c'", maybe_inv_char);
                    }
                    eputs(".");
                    continue;
                }
                try_write_cmd_i_(SET_ACTIVE_TIMEOUT, i, line_end);
                break;
            }

            case SET_INACTIVE_TIMEOUT: {
                if (is_empty_str(i, line_end)) {
                    eputs("Expected inactive task timeout value");
                    continue;
                }
                size_t inactive_task_timeout;
                char maybe_inv_char;
                ParseSizeOutcome const parse_outcome =
                    parse_size_slice(
                        i,
                        line_end,
                        &inactive_task_timeout,
                        &maybe_inv_char
                    );
                if (parse_outcome != PARSE_SIZE_OK) {
                    eprintf(
                        "Failed to parse inactive task timeout: %s",
                        parse_size_outcome_msg(parse_outcome)
                    );
                    if (parse_outcome == PARSE_SIZE_ERR_INV_CHAR) {
                        eprintf(" '%c'", maybe_inv_char);
                    }
                    eputs(".");
                    continue;
                }
                try_write_cmd_i_(SET_INACTIVE_TIMEOUT, i, line_end);
                break;
            }

            case LIST_RUNNING_TASKS: {
                try_write_cmd_i_(LIST_RUNNING_TASKS, i, line_end);
                for (;;) {
                    char line_buf[LINE_BUF_SIZE];
                    ssize_t const read_bytes =
                        read(running_tasks_fd, line_buf, LINE_BUF_SIZE);
                    if (read_bytes == -1l) {
                        eprintln(
                            "Failed reading a line from the running tasks fifo"
                                " : %s",
                            strerror(errno)
                        );
                        return EXIT_FAILURE;
                    }
                    if (read_bytes != 0l) {
                        write(STDOUT_FILENO, line_buf, read_bytes);
                    } else {
                        break;
                    }
                }
                break;
            }

            case LIST_FINISHED_TASKS: {
                try_write_cmd_i_(LIST_FINISHED_TASKS, i, line_end);
                for (;;) {
                    char line_buf[LINE_BUF_SIZE];
                    ssize_t const read_bytes =
                        read(finished_tasks_fd, line_buf, LINE_BUF_SIZE);
                    if (read_bytes == -1l) {
                        eprintln(
                            "Failed reading a line from the finished tasks fifo"
                            " : %s",
                            strerror(errno)
                        );
                    }
                    if (read_bytes != 0l) {
                        write(STDOUT_FILENO, line_buf, read_bytes);
                    } else {
                        break;
                    }
                }
                break;
            }

            case HELP: {
                print_help();
                break;
            }

            default:
                break;
            }
        }
    }

    if (argv[1][0] != '-') {
        program_eprintln("Expected '-', got '%c'.", argv[1][0]);
        return EXIT_FAILURE;
    }

    switch (argv[1][1]) {
    case '\0':
        program_eputs("Expected accompanying flag.");
        return EXIT_FAILURE;

    case EXEC_TASK_FLAG: {
        if (argc < 3) {
            program_eputs("Expected a task to execute.");
            return EXIT_FAILURE;
        }
        if ((commands_fd = open(commands_fifoname, O_WRONLY)) == -1) {
            program_eprintln(
                "Failed opening the commands fifo: %s.",
                strerror(errno)
            );
            return EXIT_FAILURE;
        }
        atexit(close_commands_fifo);

        BwOutcome const commands_fifo_writer_init_outcome =
            bw_with_default_cap(&commands_writter, commands_fd);
        if (commands_fifo_writer_init_outcome != BW_OK) {
            program_eprintln(
                "Failed initializing the commands fifo buffered writer: %s.",
                bw_outcome_msg(commands_fifo_writer_init_outcome, &errno)
            );
            return EXIT_FAILURE;
        }
        atexit(drop_commands_writer);

        try_write_cmd_(EXEC_TASK, argv);
        break;
    }

    case END_TASK_FLAG: {
        if (argc < 3) {
            program_eputs("Expected task id.");
            return EXIT_FAILURE;
        }
        size_t task_id;
        char maybe_inv_char;
        ParseSizeOutcome const parse_outcome =
            parse_size(argv[2], &task_id, &maybe_inv_char);
        if (parse_outcome != PARSE_SIZE_OK) {
            program_eprintf(
                "Failed to parse task id: %s",
                parse_size_outcome_msg(parse_outcome)
            );
            if (parse_outcome == PARSE_SIZE_ERR_INV_CHAR) {
                eprintf(" '%c'", maybe_inv_char);
            }
            eputs(".");
            return EXIT_FAILURE;
        }

        if ((commands_fd = open(commands_fifoname, O_WRONLY)) == -1) {
            program_eprintln(
                "Failed opening the commands fifo: %s.",
                strerror(errno)
            );
            return EXIT_FAILURE;
        }
        atexit(close_commands_fifo);

        BwOutcome const commands_fifo_writer_init_outcome =
            bw_with_default_cap(&commands_writter, commands_fd);
        if (commands_fifo_writer_init_outcome != BW_OK) {
            program_eprintln(
                "Failed initializing the commands fifo buffered writer: %s.",
                bw_outcome_msg(commands_fifo_writer_init_outcome, &errno)
            );
            return EXIT_FAILURE;
        }
        atexit(drop_commands_writer);

        try_write_cmd_(END_TASK, argv);
        break;
    }

    case SET_ACTIVE_TIMEOUT_FLAG: {
        if (argc < 3) {
            program_eputs("Expected active task timeout value");
            return EXIT_FAILURE;
        }
        size_t active_task_timeout;
        char maybe_inv_char;
        ParseSizeOutcome const parse_outcome =
            parse_size(argv[2], &active_task_timeout, &maybe_inv_char);
        if (parse_outcome != PARSE_SIZE_OK) {
            program_eprintf(
                "Failed to parse active task timeout: %s",
                parse_size_outcome_msg(parse_outcome)
            );
            if (parse_outcome == PARSE_SIZE_ERR_INV_CHAR) {
                eprintf(" '%c'", maybe_inv_char);
            }
            eputs(".");
            return EXIT_FAILURE;
        }

        if ((commands_fd = open(commands_fifoname, O_WRONLY)) == -1) {
            program_eprintln(
                "Failed opening the commands fifo: %s.",
                strerror(errno)
            );
            return EXIT_FAILURE;
        }
        atexit(close_commands_fifo);

        BwOutcome const commands_fifo_writer_init_outcome =
            bw_with_default_cap(&commands_writter, commands_fd);
        if (commands_fifo_writer_init_outcome != BW_OK) {
            program_eprintln(
                "Failed initializing the commands fifo buffered writer: %s.",
                bw_outcome_msg(commands_fifo_writer_init_outcome, &errno)
            );
            return EXIT_FAILURE;
        }
        atexit(drop_commands_writer);

        try_write_cmd_(SET_ACTIVE_TIMEOUT, argv);
        break;
    }

    case SET_INACTIVE_TIMEOUT_FLAG: {
        if (argc < 3) {
            program_eputs("Expected inactive task timeout value.");
            return EXIT_FAILURE;
        }
        size_t inactive_task_timeout;
        char maybe_inv_char;
        ParseSizeOutcome const parse_outcome =
            parse_size(argv[2], &inactive_task_timeout, &maybe_inv_char);
        if (parse_outcome != PARSE_SIZE_OK) {
            program_eprintf(
                "Failed to parse inactive task timeout: %s",
                parse_size_outcome_msg(parse_outcome)
            );
            if (parse_outcome == PARSE_SIZE_ERR_INV_CHAR) {
                eprintf(" '%c'", maybe_inv_char);
            }
            eputs(".");
            return EXIT_FAILURE;
        }

        if ((commands_fd = open(commands_fifoname, O_WRONLY)) == -1) {
            program_eprintln(
                "Failed opening the commands fifo: %s.",
                strerror(errno)
            );
            return EXIT_FAILURE;
        }
        atexit(close_commands_fifo);

        BwOutcome const commands_fifo_writer_init_outcome =
            bw_with_default_cap(&commands_writter, commands_fd);
        if (commands_fifo_writer_init_outcome != BW_OK) {
            program_eprintln(
                "Failed initializing the commands fifo buffered writer: %s.",
                bw_outcome_msg(commands_fifo_writer_init_outcome, &errno)
            );
            return EXIT_FAILURE;
        }
        atexit(drop_commands_writer);

        try_write_cmd_(SET_INACTIVE_TIMEOUT, argv);
        break;
    }

    case LIST_RUNNING_TASKS_FLAG: {
        if ((commands_fd = open(commands_fifoname, O_WRONLY)) == -1) {
            program_eprintln(
                "Failed opening the commands fifo: %s.",
                strerror(errno)
            );
            return EXIT_FAILURE;
        }
        atexit(close_commands_fifo);

        BwOutcome const commands_fifo_writer_init_outcome =
            bw_with_default_cap(&commands_writter, commands_fd);
        if (commands_fifo_writer_init_outcome != BW_OK) {
            program_eprintln(
                "Failed initializing the commands fifo buffered writer: %s.",
                bw_outcome_msg(commands_fifo_writer_init_outcome, &errno)
            );
            return EXIT_FAILURE;
        }
        atexit(drop_commands_writer);

        try_write_cmd_(LIST_RUNNING_TASKS, argv);

        if ((running_tasks_fd = open(running_tasks_fifoname, O_RDONLY)) == -1) {
            program_eprintln(
                "Failed opening the running tasks fifo: %s.",
                strerror(errno)
            );
            return EXIT_FAILURE;
        }
        atexit(close_running_tasks_fifo);

        for (;;) {
            char line_buf[LINE_BUF_SIZE];
            ssize_t const read_bytes =
                read(running_tasks_fd, line_buf, LINE_BUF_SIZE);
            if (read_bytes == -1l) {
                program_eprintln(
                    "Failed reading a line from the running tasks fifo: %s.",
                    strerror(errno)
                );
                return EXIT_FAILURE;
            }
            if (read_bytes != 0l) {
                write(STDOUT_FILENO, line_buf, read_bytes);
            } else {
                break;
            }
        }
        break;
    }

    case LIST_FINISHED_TASKS_FLAG: {
        if ((commands_fd = open(commands_fifoname, O_WRONLY)) == -1) {
            program_eprintln(
                "Failed opening the commands fifo: %s.",
                strerror(errno)
            );
            return EXIT_FAILURE;
        }
        atexit(close_commands_fifo);

        BwOutcome const commands_fifo_writer_init_outcome =
            bw_with_default_cap(&commands_writter, commands_fd);
        if (commands_fifo_writer_init_outcome != BW_OK) {
            program_eprintln(
                "Failed initializing the commands fifo buffered writer: %s.",
                bw_outcome_msg(commands_fifo_writer_init_outcome, &errno)
            );
            return EXIT_FAILURE;
        }
        atexit(drop_commands_writer);

        try_write_cmd_(LIST_FINISHED_TASKS, argv);

        if ((finished_tasks_fd = open(finished_tasks_fifoname, O_RDONLY)) ==
            -1
        ) {
            program_eprintln(
                "Failed opening the finished tasks fifo: %s.",
                strerror(errno)
            );
            return EXIT_FAILURE;
        }
        atexit(close_finished_tasks_fifo);

        for (;;) {
            char line_buf[LINE_BUF_SIZE];
            ssize_t const read_bytes =
                read(finished_tasks_fd, line_buf, LINE_BUF_SIZE);
            if (read_bytes == -1l) {
                program_eprintln(
                    "Failed reading a line from the finished tasks fifo: %s.",
                    strerror(errno)
                );
            }
            if (read_bytes == -1l) {
                write(STDOUT_FILENO, line_buf, read_bytes);
            } else {
                break;
            }
        }
        break;
    }

    case HELP_FLAG: {
        print_help();
        break;
    }

    default: {
        program_eprintln(
            "Unrecognized command line option -%c.",
            argv[1][1]
        );
        return EXIT_FAILURE;
    }
    }
}
