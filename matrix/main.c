#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char print_separator[] = {"==========================="};

const char *stat_print_template[] =
{
    "pid", "ID процесса",
    "comm", "Имя файла",
    "state", "Состояние процесса",
    "ppid", "ID родительского процесса",
    "pgrp", "ID группы процесса",
    "session", "ID сессии процесса",
    "tty_nr", "управляющий терминал процесса",
    "tpgid", "ID внешней группы процессов управляющего терминала",
    "flags", "Флаги ядра процесса",
    "minflt", "Количество минорных ошибок процесса (Минорные ошибки не включают ошибки загрузки страниц памяти c "
    "диска)",
    "cminflt", "Количество минорных ошибок дочерних процессов (Минорные ошибки не включают ошибки загрузки страниц "
    "памяти c диска)",
    "majflt", "Количество Мажоных ошибок процесса",
    "cmajflt", "Количество Мажоных ошибок дочерних процессов процесса",
    "utime", "Количество времени, в течение которого этот процесс исполнялся в пользовательском режиме",
    "stime", "Количество времени, в течение которого этот процесс исполнялся в режиме ядра",
    "cutime", "Количество времени, в течение которого дети этого процесса исполнялись в "
    "пользовательском режиме",
    "cstime", "Количество времени, в течение которого дети этого процесса были запланированы в режиме "
    "ядра",
    "priority", "Приоритет процесса",
    "nice", "уровень nice процесса, то есть пользовательский приоритет процесса (-20 - высокий, 19 - низкий)",
    "num_threads", "Количество потоков",
    "itrealvalue", "Время в тиках до следующего SIGALRM отправленного в процесс из-за интервального таймера",
    "starttime", "Время, прошедшее c момента загрузки системы, когда процесс был запущен",
    "vsize", "Объем виртуальной памяти в байтах",
    "rss", "Resident Set (Memory) Size: Количество страниц процесса в физической памяти",
    "rsslim", "Текущий предел в байтах для RSS процесса",
    "startcode", "Адрес, над которым может работать текст программы",
    "endcode", "Адрес, до которого может работать текст программы",
    "startstack", "Адрес начала стека",
    "kstkesp", "Текущее значение ESP (Stack pointer)",
    "kstkeip", "Текущее значение EIP (instruction pointer)",
    "pending", "Битовая карта отложенных сигналов, отображаемое в виде десятичного числа",
    "blocked", "Битовая карта заблокированных сигналов, отображаемое в виде десятичного числа",
    "sigign", "Битовая карта игнорированных сигналов, отображаемое в виде десятичного числа",
    "sigcatch", "Битовая карта пойманных сигналов, отображаемое в виде десятичного числа",
    "wchan", "Адрес ядрёной функции, где процесс находится в состоянии сна",
    "nswap", "Количество страниц, поменявшихся местами",
    "cnswap", "Накопительный своп для дочерних процессов",
    "exit_signal", "Сигнал, который будет послан родителю, когда процесс будет завершен",
    "processor", "Номер процессора, на котором происходило последнее выполнение",
    "rt_priority", "Приоритет планирования в реальном времени - число в диапазоне от 1 до 99 для процессов, "
    "запланированных в соответствии c политикой реального времени",
    "policy", "Политика планирования",
    "blkio_ticks", "Общие блочные задержки ввода/вывода",
    "gtime", "Гостевое время процесса",
    "cgtime", "Гостевое время дочерних процессов",
    "start_data", "Адрес, над которым размещаются инициализированные и неинициализированные данные программы (BSS)",
    "end_data", "Адрес, под которым размещаются инициализированные и неинициализированные данные программы (BSS)",
    "start_brk", "Адрес, выше которого куча программ может быть расширена c помощью brk (станавливает конец "
    "сегмента данных в значение, указанное в аргументе end_data_segment, когда это значение является приемлимым, "
    "система симулирует нехватку памяти и процесс не достигает своего максимально возможного размера сегмента "
    "данных)",
    "arg_start", "Адрес, над которым размещаются аргументы командной строки программы (argv)",
    "arg_end", "Адрес, под которым размещаются аргументы командной строки программы (argv)",
    "env_start", "Адрес, над которым размещена программная среда",
    "env_end", "Адрес, под которым размещена программная среда",
    "exit_code", "Состояние выхода потока в форме, сообщаемой waitpid"};

static char *statm_print_template[] = 
    {"size", "общее число страниц выделенное процессу в виртуальной памяти",
    "resident", "количество страниц, загруженных в физическую память"
    "shared", "количество общих резедентных страниц",
    "trs", "количество страниц кода",
    "lrs", "количество страниц библиотеки",
    "drs", "количество страниц данных/стека",
    "dt", "dirty pages - данные для записи находятся в кэше страниц, но требуется к первоначальной записи на носитель"
    };

#define BUF_SIZE 2048
#define PATH_LENGTH 256

void print_cmdline(FILE *fout, const int pid)
{
    char path[PATH_MAX];
    char buf[BUF_SIZE];
    snprintf(path, PATH_MAX, "/proc/%d/cmdline", pid);

    FILE *file = fopen(path, "r");
    int len = fread(buf, 1, BUF_SIZE, file);
    buf[len - 1] = 0;

    fprintf(fout, "\n/proc/%d/cmdline:\n%s\n%s\n", pid, print_separator, buf);

    fclose(file);
}

void print_environ(FILE *fout, const int pid)
{
    char path_to_open[PATH_MAX];
    snprintf(path_to_open, PATH_MAX, "/proc/%d/environ", pid);
    int len;
    char buf[BUF_SIZE];

    FILE *file = fopen(path_to_open, "r");

    fprintf(fout, "\n/proc/%d/environ:\n%s\n", pid, print_separator);

    while ((len = fread(buf, 1, BUF_SIZE, file)) > 0)
    {
        for (int i = 0; i < len; i++)
        {
            if (buf[i] == 0)
            {
                buf[i] = '\n';
            }
        }
        buf[len - 1] = '\n';

        fprintf(fout, "%s", buf);
    }

    fclose(file);
}

void print_fd(FILE *fout, const int pid)
{
    char path_to_open[PATH_MAX];
    snprintf(path_to_open, PATH_MAX, "/proc/%d/fd/", pid);
    DIR *dir = opendir(path_to_open);
    struct dirent *readDir;
    char string[PATH_MAX];
    char path[10000] = {'\0'};

    fprintf(fout, "\n/proc/%d/fd/:\n%s\n", pid, print_separator);

    while ((readDir = readdir(dir)) != NULL)
    {
        if ((strcmp(readDir->d_name, ".") != 0) && (strcmp(readDir->d_name, "..") != 0))
        {
            sprintf(path, "%s%s", path_to_open, readDir->d_name);
            int _read_len = readlink(path, string, PATH_MAX);

            fprintf(fout, "%s -> %s\n", readDir->d_name, string);
        }
    }

    closedir(dir);
}

void print_stat(FILE *fout, const int pid)
{
    char path_to_open[PATH_MAX];
    snprintf(path_to_open, PATH_MAX, "/proc/%d/stat", pid);
    char buf[BUF_SIZE];
    FILE *file = fopen(path_to_open, "r");

    fread(buf, 1, BUF_SIZE, file);
    char *token = strtok(buf, " ");

    fprintf(fout, "\n/proc/%d/stat:\n%s\n", pid, print_separator);

    for (int i = 0; token != NULL; i+=2)
    {
        fprintf(fout, "%s = %s\n%s\n", stat_print_template[i], token, stat_print_template[i+1]);
        token = strtok(NULL, " ");
    }

    fclose(file);
}

void print_statm(FILE *fout, const int pid)
{
    char path_to_open[PATH_MAX];
    snprintf(path_to_open, PATH_MAX, "/proc/%d/statm", pid);
    FILE *file = fopen(path_to_open, "r");
    char buf[BUF_SIZE];

    fread(buf, 1, BUF_SIZE, file);
    char *token = strtok(buf, " ");

    fprintf(fout, "\n/proc/%d/statm:\n%s\n", pid, print_separator);
    for (int i = 0; token != NULL; i+=2)
    {
        fprintf(fout, "%s = %s\n%s\n", statm_print_template[i], token, statm_print_template[i+1]);
        token = strtok(NULL, " ");
    }

    fclose(file);
}

void print_cwd(FILE *fout, const int pid)
{
    char path_to_open[PATH_MAX];
    snprintf(path_to_open, PATH_MAX, "/proc/%d/cwd", pid);
    char buf[BUF_SIZE] = {'\0'};
    int _read_len = readlink(path_to_open, buf, BUF_SIZE);

    fprintf(fout, "\n/proc/%d/cwd:\n%s\n%s\n", pid, print_separator, buf);
}

void print_exe(FILE *fout, const int pid)
{
    char path_to_open[PATH_MAX];
    snprintf(path_to_open, PATH_MAX, "/proc/%d/exe", pid);
    char buf[BUF_SIZE] = {'\0'};
    int _read_len = readlink(path_to_open, buf, BUF_SIZE);

    fprintf(fout, "\n/proc/%d/exe:\n%s\n%s\n", pid, print_separator, buf);
}

void print_maps(FILE *fout, const int pid)
{
    char path_to_open[PATH_MAX];
    snprintf(path_to_open, PATH_MAX, "/proc/%d/maps", pid);
    char buf[BUF_SIZE] = {'\0'};
    FILE *file = fopen(path_to_open, "r");
    int lengthOfRead;

    fprintf(fout, "\n/proc/%d/maps:\n%s\n", pid, print_separator);

    while ((lengthOfRead = fread(buf, 1, BUF_SIZE, file)))
    {
        buf[lengthOfRead] = '\0';
        fprintf(fout, "%s\n", buf);
    }

    fclose(file);
}

void print_root(FILE *fout, const int pid)
{
    char path_to_open[PATH_MAX];
    snprintf(path_to_open, PATH_MAX, "/proc/%d/root", pid);
    char buf[BUF_SIZE] = {'\0'};
    int _read_len = readlink(path_to_open, buf, BUF_SIZE);

    fprintf(fout, "\n/proc/%d/root:\n%s\n%s\n", pid, print_separator, buf);
}

void print_comm(FILE *fout, const int pid)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%d/comm", pid);
    FILE *file = fopen(path, "r");

    char buf[BUF_SIZE];
    int len = fread(buf, 1, BUF_SIZE, file);
    buf[len - 1] = 0;
    fprintf(fout, "\n/proc/%d/comm:\n%s\n%s\n", pid, print_separator, buf);

    fclose(file);
}

void print_io(FILE *fout, const int pid)
{
    char path_to_open[PATH_MAX];
    snprintf(path_to_open, PATH_MAX, "/proc/%d/io", pid);
    FILE *file = fopen(path_to_open, "r");
    int len, i;
    char buf[BUF_SIZE] = {"\0"};

    fprintf(fout, "\n/proc/%d/io:\n%s\n", pid, print_separator);

    while ((len = fread(buf, 1, BUF_SIZE, file)) > 0)
    {
        for (i = 0; i < len; i++)
        {
            if (buf[i] == 0)
                buf[i] = 10;
        }

        fprintf(fout, "%s", buf);
    }

    fclose(file);
}

void print_task(FILE *fout, const int pid)
{
    char path_to_open[PATH_MAX];
    snprintf(path_to_open, PATH_MAX, "/proc/%d/task", pid);
    DIR *dir = opendir(path_to_open);
    struct dirent *readDir;
    char buf[BUF_SIZE] = {"\0"};
    char string[PATH_MAX];

    fprintf(fout, "\n/proc/%d/task:\n%s\n", pid, print_separator);

    while ((readDir = readdir(dir)) != NULL)
    {
        if ((strcmp(readDir->d_name, ".") != 0) && (strcmp(readDir->d_name, "..") != 0))
        {
            sprintf(buf, "%s%s", path_to_open, readDir->d_name);
            fprintf(fout, "{%4s}\n", readDir->d_name);
        }
    }
}

int get_pid(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Введите pid процесса.");
        exit(1);
    }

    int pid = atoi(argv[1]);
    char check_dir[PATH_MAX];
    snprintf(check_dir, PATH_MAX, "/proc/%d", pid);

    if (!pid || access(check_dir, F_OK))
    {
        printf("Директория для введенного pid не доступна");
        exit(1);
    }

    return pid;
}

int main(int argc, char *argv[])
{
    int pid = get_pid(argc, argv);
    FILE *fout = fopen("info.txt", "w");

    print_cmdline(fout, pid);
    print_environ(fout, pid);
    print_fd(fout, pid);
    print_stat(fout, pid);
    print_statm(fout, pid);
    print_cwd(fout, pid);
    print_exe(fout, pid);
    print_maps(fout, pid);
    print_root(fout, pid);
    print_comm(fout, pid);
    print_io(fout, pid);
    print_task(fout, pid);

    fclose(fout);
    printf("Saved in info.txt\n");

    return 0;
}