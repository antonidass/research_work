#include <iostream>
#include <stdio.h>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <vector>
#include <cmath>
#if defined(_WIN32)
#include <Windows.h>

struct matrix_s
{
    int rows;
    int columns;
    int **matrix;
};

#define ITERATIONS 20
using namespace std;

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <time.h>

#else
#error "Unable to define getCPUTime( ) for an unknown OS."
#endif

double getCPUTime( )
{
#if defined(_WIN32)
    /* Windows -------------------------------------------------- */
    FILETIME createTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    if ( GetProcessTimes( GetCurrentProcess( ),
                          &createTime, &exitTime, &kernelTime, &userTime ) != -1 )
    {
        SYSTEMTIME userSystemTime;
        if ( FileTimeToSystemTime( &userTime, &userSystemTime ) != -1 )
            return (double)userSystemTime.wHour * 3600.0 +
                   (double)userSystemTime.wMinute * 60.0 +
                   (double)userSystemTime.wSecond +
                   (double)userSystemTime.wMilliseconds / 1000.0;
    }

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
    /* AIX, BSD, Cygwin, HP-UX, Linux, OSX, and Solaris --------- */

#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
    /* Prefer high-res POSIX timers, when available. */
    {
        clockid_t id;
        struct timespec ts;
#if _POSIX_CPUTIME > 0
        /* Clock ids vary by OS.  Query the id, if possible. */
        if ( clock_getcpuclockid( 0, &id ) == -1 )
#endif
#if defined(CLOCK_PROCESS_CPUTIME_ID)
            /* Use known clock id for AIX, Linux, or Solaris. */
            id = CLOCK_PROCESS_CPUTIME_ID;
#elif defined(CLOCK_VIRTUAL)
            /* Use known clock id for BSD or HP-UX. */
            id = CLOCK_VIRTUAL;
#else
            id = (clockid_t)-1;
#endif
        if ( id != (clockid_t)-1 && clock_gettime( id, &ts ) != -1 )
            return (double)ts.tv_sec +
                (double)ts.tv_nsec / 1000000000.0;
    }
#endif

#if defined(RUSAGE_SELF)
    {
        struct rusage rusage;
        if ( getrusage( RUSAGE_SELF, &rusage ) != -1 )
            return (double)rusage.ru_utime.tv_sec +
                (double)rusage.ru_utime.tv_usec / 1000000.0;
    }
#endif

#if defined(_SC_CLK_TCK)
    {
        const double ticks = (double)sysconf( _SC_CLK_TCK );
        struct tms tms;
        if ( times( &tms ) != (clock_t)-1 )
            return (double)tms.tms_utime / ticks;
    }
#endif

#if defined(CLOCKS_PER_SEC)
    {
        clock_t cl = clock( );
        if ( cl != (clock_t)-1 )
            return (double)cl / (double)CLOCKS_PER_SEC;
    }
#endif

#endif

    return -1;      /* Failed. */
}

struct matrix_s
{
    int rows;
    int columns;
    int **matrix;
};

#define ITERATIONS 20
using namespace std;

void fill_with_rand(matrix_s &matrix)
{
    for (int i = 0; i < matrix.rows; i++)
        for (int j = 0; j < matrix.columns; j++)
            matrix.matrix[i][j] = rand() % 10 + 1;
}

void input_matrix(matrix_s &matrix)
{
    cout << "Input matrix:" << endl;
    for (int i = 0; i < matrix.rows; i++)
        for (int j = 0; j < matrix.columns; j++)
            cin >> matrix.matrix[i][j];
}

void generate_matrix(matrix_s &matrix)
{
    cout << "Input matrix rows: ";
    cin >> matrix.rows;
    cout << "\n";

    cout << "Input matrix columns: ";
    cin >> matrix.columns;
    cout << "\n";

   // matrix.columns = 10;
   // matrix.rows = 10;

    matrix.matrix = (int**) malloc(matrix.rows * sizeof(int*));

    for (int i = 0; i <= matrix.rows; i++)
    {
        matrix.matrix[i] = (int*) malloc(matrix.columns * sizeof(int));
    }

    for (int i = 0; i < matrix.rows; i++)
    {
        for (int j = 0; j < matrix.columns; j++)
        {
            matrix.matrix[i][j] = 0;
        }
    }
}

void output_matrix(matrix_s matrix)
{
    cout << "Matrix:" << endl;
    for (int i = 0; i < matrix.rows; i++) {
        for (int j = 0; j < matrix.columns; j++)
            cout << matrix.matrix[i][j] << " ";

        cout << endl;
    }
}

void output_geometric_mean(double* result, int size)
{
    cout << endl << "Geometric mean of each row:" << endl;
    for (int i = 0; i < size; i++)
    {
        cout << result[i] << " ";
    }
    cout << endl;
}

double *find_geometric_mean(matrix_s matrix)
{
    double *result = (double*) calloc(matrix.rows, sizeof(double));

    long long int temp = 1;

    for (int i = 0; i < matrix.rows; i++)
    {
        for (int j = 0; j < matrix.columns; j++)
        {
            temp *= matrix.matrix[i][j];
        }

        result[i] = pow(temp, 1.0/matrix.rows);
        temp = 1;
    }

    return result;
}

void parallel_additional(double *result, matrix_s matrix, int thread, int threads_count)
{
    long long int temp = 1;

    for (int i = thread; i < matrix.rows; i += threads_count)
    {
        for (int j = 0; j < matrix.columns; j++)
        {
            temp *= matrix.matrix[i][j];
        }

        result[i] = pow(temp, 1.0/matrix.rows);
        temp = 1;
    }
}

double* parallel_geometric_mean(matrix_s matrix, int threads_count)
{
    double *result = (double*) calloc(matrix.rows, sizeof(double));

    vector<thread> threads(threads_count);

    for (int thread = 0; thread < threads_count; thread++)
    {
        threads[thread] = std::thread(parallel_additional, ref(result), matrix, thread, threads_count);
    }

    for (int i = 0; i < threads_count; i++)
    {
        threads[i].join();
    }

    return result;
}

void time_analysis()
{
    matrix_s matrix;
    double *result = NULL;

    generate_matrix(matrix);
    fill_with_rand(matrix);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    double result_time;

    start = std::chrono::system_clock::now();

    for (int i = 0; i < ITERATIONS; i++)
    {
        result = find_geometric_mean(matrix);
    }

    end = std::chrono::system_clock::now();

    result_time = (std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
    result_time /= ITERATIONS;

    printf("Time sequential on random array: %lf seconds\n\n", result_time / 1e9);

    start = std::chrono::system_clock::now();

    for (int i = 0; i < ITERATIONS; i++)
    {
        result = parallel_geometric_mean(matrix, 1);
    }

    end = std::chrono::system_clock::now();

    result_time = (std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
    result_time /= ITERATIONS;

    printf("Time parallel 1 thread on random array: %lf seconds\n\n", result_time / 1e9);

    start = std::chrono::system_clock::now();

    for (int i = 0; i < ITERATIONS; i++)
    {
        result = parallel_geometric_mean(matrix, 2);
    }

    end = std::chrono::system_clock::now();

    result_time = (std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
    result_time /= ITERATIONS;

    printf("Time parallel 2 threads on random array: %lf seconds\n\n", result_time / 1e9);

    start = std::chrono::system_clock::now();

    for (int i = 0; i < ITERATIONS; i++)
    {
        result = parallel_geometric_mean(matrix, 4);
    }

    end = std::chrono::system_clock::now();

    result_time = (std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
    result_time /= ITERATIONS;

    printf("Time parallel 4 threads on random array: %lf seconds\n\n", result_time / 1e9);

    start = std::chrono::system_clock::now();

    for (int i = 0; i < ITERATIONS; i++)
    {
        result = parallel_geometric_mean(matrix, 8);
    }

    end = std::chrono::system_clock::now();

    result_time = (std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
    result_time /= ITERATIONS;

    printf("Time parallel 8 threads on random array: %lf seconds\n\n", result_time / 1e9);

    start = std::chrono::system_clock::now();

    for (int i = 0; i < ITERATIONS; i++)
    {
        result = parallel_geometric_mean(matrix, 16);
    }

    end = std::chrono::system_clock::now();

    result_time = (std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
    result_time /= ITERATIONS;

    printf("Time parallel 16 threads on random array: %lf seconds\n\n", result_time / 1e9);

    start = std::chrono::system_clock::now();

    for (int i = 0; i < ITERATIONS; i++)
    {
        result = parallel_geometric_mean(matrix, 32);
    }

    end = std::chrono::system_clock::now();

    result_time = (std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
    result_time /= ITERATIONS;

    printf("Time parallel 32 threads on random array: %lf seconds\n\n", result_time / 1e9);

    start = std::chrono::system_clock::now();

    for (int i = 0; i < ITERATIONS; i++)
    {
        result = parallel_geometric_mean(matrix, 64);
    }

    end = std::chrono::system_clock::now();

    result_time = (std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
    result_time /= ITERATIONS;

    printf("Time parallel 64 threads on random array: %lf seconds\n\n", result_time / 1e9);
}

void show_info()
{
    cout << endl << "1. Run Sequential algorithm" << endl;
    cout << "2. Run Parallel algorithm" << endl;
    cout << "3. Run Time analysis" << endl;
    cout << "0. Exit" << endl;
}


int main()
{
    int command = -1, threads = 10;
    matrix_s matrix;
    double *result = NULL;
/*
    generate_matrix(matrix);
    fill_with_rand(matrix);
    output_matrix(matrix);

    result = parallel_geometric_mean(matrix, threads);
    output_geometric_mean(result, matrix.rows);
*/

    while (command != 0)
    {
        show_info();
        cin >> command;
        cout << endl;

        if (command == 3)
        {
            time_analysis();
        }

        else if (command == 1 || command == 2)
        {
            generate_matrix(matrix);
            input_matrix(matrix);

            if (command == 1)
                result = find_geometric_mean(matrix);

            else if (command == 2)
            {
                cout << "Input number of threads: ";
                cin >> threads;
                cout << endl;

                result = parallel_geometric_mean(matrix, threads);
            }

            output_geometric_mean(result, matrix.rows);
        }
    }

    return 0;
}
