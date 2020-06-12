#include "comfy_io.h"
#include "parse_size.h"
#include "task.h"
#include "task_vec.h"

#include <unistd.h>

#include <stdlib.h>

#define ACTIVE_TIMEOUT_FLAG 'm'
#define END_TASK_FLAG 't'
#define EXEC_TASK_FLAG 'e'
#define HELP_FLAG 'h'
#define INACTIVE_TIMEOUT_FLAG 'i'
#define LIST_ACTIVE_TASKS_FLAG 'l'
#define LIST_FINISHED_TASKS_FLAG 'r'

static char const* const s_program_name = "argus";

void print_help(void) {
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
        s_program_name,
        EXEC_TASK_FLAG, "Execute a task",
        END_TASK_FLAG, "End a task with id 'n'",
        ACTIVE_TIMEOUT_FLAG, "Set a timeout of n seconds for task activity",
        INACTIVE_TIMEOUT_FLAG, "Set a timeout of n seconds for task inactivity",
        LIST_ACTIVE_TASKS_FLAG, "List all active tasks",
        LIST_FINISHED_TASKS_FLAG, "List all finished tasks",
        HELP_FLAG, "Display this message"
    );
}

int main(int argc, char* argv[]) {
    TaskVec ongoing_tasks;
    tvec_new(&ongoing_tasks);

    TaskVec finished_tasks;
    tvec_new(&finished_tasks);

    if (argc == 1) {  // no args, interactive mode
        char c;
        while (read(STDIN_FILENO, &c, 1)) {
            continue;
        }
        return EXIT_SUCCESS;
    }

    if (argv[1][0] != '-') {
        eprintln("%s: Expected '-', got '%c'.", s_program_name, argv[1][0]);
        return EXIT_FAILURE;
    }

    switch (argv[1][1]) {
    case ACTIVE_TIMEOUT_FLAG: {
        if (argc < 3) {
            eprintln(
                "%s: Expected active task timeout value.",
                s_program_name
            );
            return EXIT_FAILURE;
        }
        size_t active_task_timeout;
        char maybe_inv_char;
        ParseSizeOutcome const parse_outcome =
            parse_size(argv[2], &active_task_timeout, &maybe_inv_char);
        if (parse_outcome != PARSE_SIZE_OK) {
            eprintf(
                "%s: Failed to parse active task timeout: %s",
                s_program_name,
                parse_size_outcome_to_msg(parse_outcome)
            );
            if (parse_outcome == PARSE_SIZE_ERR_INV_CHAR) {
                eprintf(" '%c'", maybe_inv_char);
            }
            eputs(".");
            return EXIT_FAILURE;
        }
        // do something with active_task_timeout here
        break;
    }

    case END_TASK_FLAG: {
        if (argc < 3) {
            eprintln("%s: Expected task id.", s_program_name);
            return EXIT_FAILURE;
        }
        size_t task_id;
        char maybe_inv_char;
        ParseSizeOutcome const parse_outcome =
            parse_size(argv[2], &task_id, &maybe_inv_char);
        if (parse_outcome != PARSE_SIZE_OK) {
            eprintf(
                "%s: Failed to parse task id: %s",
                s_program_name,
                parse_size_outcome_to_msg(parse_outcome)
            );
            if (parse_outcome == PARSE_SIZE_ERR_INV_CHAR) {
                eprintf(" '%c'", maybe_inv_char);
            }
            eputs(".");
            return EXIT_FAILURE;
        }
        Task* const marked_for_deletion =
            tvec_at_mut(&ongoing_tasks, task_id);
        tvec_rm_ord_at(&ongoing_tasks, task_id);
        break;
    }

    case EXEC_TASK_FLAG: {
        if (argc < 3) {
            eprintln("%s: Expected task.", s_program_name);
            return EXIT_FAILURE;
        }
        Task task;
        new_task(&task, argv[2]);
        tvec_push(&ongoing_tasks, &task);
        break;
    }

    case HELP_FLAG:
        print_help();
        break;

    case INACTIVE_TIMEOUT_FLAG: {
        if (argc < 3) {
            eprintln(
                "%s: Expected inactive task timeout value.",
                s_program_name
            );
            return EXIT_FAILURE;
        }
        size_t inactive_task_timeout;
        char maybe_inv_char;
        ParseSizeOutcome const parse_outcome =
            parse_size(argv[2], &inactive_task_timeout, &maybe_inv_char);
        if (parse_outcome != PARSE_SIZE_OK) {
            eprintf(
                "%s: Failed to parse inactive task timeout: %s",
                s_program_name,
                parse_size_outcome_to_msg(parse_outcome)
            );
            if (parse_outcome == PARSE_SIZE_ERR_INV_CHAR) {
                eprintf(" '%c'", maybe_inv_char);
            }
            eputs(".");
            return EXIT_FAILURE;
        }
        // do something with active_task_timeout here
        break;
    }

    case LIST_ACTIVE_TASKS_FLAG: {
        size_t task_id = 0;
        Task const* const end = tvec_end(&ongoing_tasks);
        for (Task const* i = tvec_begin(&ongoing_tasks); i != end; ++i) {
            printf("#%zu: ", task_id);
            print_task(i);
            ++task_id;
        }
        break;
    }

    case LIST_FINISHED_TASKS_FLAG: {
        size_t task_id = 0;
        Task const* const end = tvec_end(&finished_tasks);
        for (Task const* i = tvec_begin(&finished_tasks); i != end; ++i) {
            printf("#%zu: ", task_id);
            print_task(i);
            ++task_id;
        }
        break;
    }

    default:
        eprintln(
            "%s: Unrecognized command line option -%c.",
            s_program_name,
            argv[1][1]
        );
        return EXIT_FAILURE;
    }

    tvec_drop(&ongoing_tasks);
    tvec_drop(&finished_tasks);
}
