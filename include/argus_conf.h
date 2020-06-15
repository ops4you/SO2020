#ifndef ARGUS_CONF_H
#define ARGUS_CONF_H

#define EXEC_TASK_FLAG 'e'
#define END_TASK_FLAG 't'
#define SET_ACTIVE_TIMEOUT_FLAG 'm'
#define SET_INACTIVE_TIMEOUT_FLAG 'i'
#define LIST_RUNNING_TASKS_FLAG 'l'
#define LIST_FINISHED_TASKS_FLAG 'r'
#define HELP_FLAG 'h'

char const* const server_dirname = "/tmp/argus";
char const* const commands_fifoname = "/tmp/argus/commands";
char const* const running_tasks_fifoname = "/tmp/argus/running_tasks";
char const* const finished_tasks_fifoname = "/tmp/argus/finished_tasks";

char const* const exec_task_cmd = "executar";
char const* const end_task_cmd = "terminar";
char const* const set_active_timeout_cmd = "tempo-execucao";
char const* const set_inactive_timeout_cmd = "tempo-inactividade";
char const* const list_running_tasks_cmd = "listar";
char const* const list_finished_tasks_cmd = "historico";
char const* const help_cmd = "ajuda";

#endif  // ARGUS_CONF_H
