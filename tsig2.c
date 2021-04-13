#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define WITH_SIGNALS 0

const unsigned int NUM_CHILD = 2;
pid_t pids[NUM_CHILD];
int currPid = 0;
const unsigned int noChildProcessesStatus = -1;
unsigned int interruptReceived = 0;

void sigintHandler(int sig)
{
    interruptReceived = 1;
    printf("parent[%d] : Keyboard interrupt was received\n", getpid());
}

void ignoreAllSignals()
{
    for (int i = 1; i <= 31; i++)
    {
        signal(i, SIG_IGN);
    }
}

void restoreSignals()
{
    for (int i = 1; i <= 31; i++)
    {
        signal(i, SIG_DFL);
    }
}

void sigtermHandler(int sig)
{
    printf("child[%d] : termination of the process\n", getpid());
}

void childAlgorithm()
{
    if (WITH_SIGNALS == 1)
    {
        signal(SIGINT, SIG_IGN);         // ignore keyboard interrupt signal
        signal(SIGTERM, sigtermHandler); // set custom sigterm signal handler
    }

    int childpid = getpid();
    int parentpid = getppid();

    printf("child[%d]: parent process identifier is %d\n", childpid, parentpid);
    sleep(10);
    printf("child [%d]: Execution completed\n", childpid);
}

void parentAlgorithm(pid_t child)
{
    if (WITH_SIGNALS == 1)
    {
        ignoreAllSignals();            // ignore all signals
        signal(SIGCHLD, SIG_DFL);      // restore default behaviour for SIGCHLD
        signal(SIGINT, sigintHandler); // setting custom handler for sigint

        pids[currPid] = child;
        currPid++;
        if (interruptReceived != 0)
        {
            printf("parent[%d] : Interrupt of the creation process\n", getpid());

            for (int i = 0; i < currPid; i++)
            {
                kill(pids[i], SIGTERM);
            }
        }
        else
        {
            printf("parent[%d] : Child process has been created with id %d.\n", getpid(), child);
        }
    }
    else
    {
        printf("parent[%d] : Child process has been created with id %d.\n", getpid(), child);
    }

    int stat;
    unsigned int numberOfTerminations = 0;
    while (wait(&stat) != noChildProcessesStatus)
    {
        numberOfTerminations += 1;
    }
    printf("parent[%d] : number of received child processes exit codes is %d\n",
           getpid(), numberOfTerminations);

    if (WITH_SIGNALS == 1)
    {
        restoreSignals(); // old service handlers are restored
    }
}

int main()
{
    int stat;
    pid_t pid[NUM_CHILD];

    for (int i = 0; i < NUM_CHILD; i++)
    {
        pid[i] = fork();

        if (pid[i] == 0)
        {
            childAlgorithm();
        }
        else if (pid[i] < 0)
        {
            printf("Child was created unsuccessfully.\n");
            for (int j = i - 1; j >= 0; j--)
            {
                kill(pid[j], SIGTERM);
            }
            exit(1);
        }
        else
        {
            parentAlgorithm(pid[i]);
        }

        sleep(1);
        if (WITH_SIGNALS == 1)
        {
            sigintHandler(stat);
        }
    }

    return 0;
}