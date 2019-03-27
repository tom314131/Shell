#include<sys/types.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>

#define MAX_LINE 512

/* struct of job that contains the pid of the job and
 * the input command of the job
 */
struct JobStruct {
    pid_t pid;
    char inputCommand[512];
} typedef JobStruct;

void InitializeJobs(JobStruct jobs[MAX_LINE]);

int ToToken(char *buffer, char *argv[MAX_LINE]);

void PromptAndCleanArgv(char *argv[MAX_LINE]);

void InsertJobs(char *buffer, JobStruct jobs[MAX_LINE], int pid);

void ForkProcess(char *argv[MAX_LINE], char *buffer, int backgroundPS, int status,
                 JobStruct jobs[MAX_LINE], int count);

void CleanJobs(JobStruct jobs[MAX_LINE],int *status);

void PrintJobs(JobStruct jobs[MAX_LINE]);

int main() {
    char dummy;
    int status;
    char *argv[MAX_LINE];
    JobStruct jobs[MAX_LINE];
    InitializeJobs(jobs);
    while (1) {
        //default process will be foreground
        int backgroundPS = 0;

        //print >  and clean argument array
        PromptAndCleanArgv(argv);

        // get input command
        char buffer[512] = "";
        scanf("%99[^\n]", buffer);
        scanf("%c", &dummy);

        //if the input is not null
        if (strcmp(buffer, "") != 0) {
            int count = ToToken(buffer, argv);

            /* exit command */
            if (strcmp(argv[0], "exit") == 0) {
                printf("%d\n", getppid());
                int i=0;
                for(i=0; i <MAX_LINE; i++){
                    if(jobs[i].pid!=0){
                        kill(jobs[i].pid,SIGKILL);
                    }
                }
                exit(0);
            }

            /*if the command is jobs we will clean all the processes that finished
             and then print them */
            if (strcmp(buffer, "jobs") == 0) {
                CleanJobs(jobs, &status);
                PrintJobs(jobs);
                // if the command is cd
            } else if (strcmp(argv[0], "cd") == 0) {
                if (argv[1]== NULL || strcmp(argv[1], "~") == 0) {
                    chdir(getenv("HOME"));
                } else {
                    chdir(argv[1]);
                }
                printf("%d\n", getppid());
            } else {
                //if the command is regular command we will fork it
                ForkProcess(argv, buffer, backgroundPS, status, jobs, count);
            }
        }
    }
}

//print shell > and clean argument
void PromptAndCleanArgv(char *argv[MAX_LINE]) {
    printf(">");
    int i =0;
    for (i = 0; i < MAX_LINE; i++) {
        argv[i] = NULL;
    }
}

/*
 * Function Name: ToToken
 * buffer : the input
 * argv : array of argument of the input after splitting
 * Function Operation: split the input to tokens and return th number of tokens
 */
int ToToken(char *buffer, char *argv[MAX_LINE]) {
    int count = 0;
    char *pch;
    pch = strtok(buffer, " ");
    while (pch != NULL) {
        argv[count] = pch;
        pch = strtok(NULL, " ");
        ++count;
    }
    argv[count] = NULL;
    return count;
}


/*
 * Function Name: InsertJobs
 * Function Operation: insert the background jobs to the array jobs
 */
void InsertJobs(char *buffer, JobStruct jobs[MAX_LINE], int pid) {
    int i =0;
    for (i = 0; i < MAX_LINE; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].pid = pid;
            int j = 0;
            for (j = 0; j < MAX_LINE; j++) {
                if (buffer[j] == '&') {
                    jobs[i].inputCommand[j - 1] = '\0';
                    jobs[i].inputCommand[j] = '\0';
                    break;
                }
                if (buffer[j] != '\0') {
                    jobs[i].inputCommand[j] = buffer[j];
                }
                if (buffer[j] == '\0') {
                    jobs[i].inputCommand[j] = ' ';
                }

            }
            break;
        }
    }
}


void ForkProcess(char *argv[MAX_LINE], char *buffer, int backgroundPS, int status,
                 JobStruct jobs[MAX_LINE], int count) {
    if (strcmp(argv[count - 1], "&") == 0) {
        argv[count - 1] = NULL;
        backgroundPS = 1;
    }

    pid_t pid = fork();
    if (pid < 0) {  //if fork failed due to some reason
        fprintf(stderr,"Error in system call\n");
    }

    //if it the child's process
    if (pid == 0) {
        //f execvp failed
        if(execvp(argv[0], argv)==-1) {
            fprintf(stderr, "Error in system call\n");
        }
        exit(0);
    }
    //if it is the father's process
    if (pid > 0) {
        printf("%d\n", pid);
        //if it is foreground command we will wait to the child to finish
        if (!backgroundPS) {
            if (WIFEXITED(status)) {
                waitpid(pid,NULL,0);
            }

            /* if it is background we won't wait the child
             and insert it to the array of jobs */
        } else {
            InsertJobs(buffer, jobs, pid);
        }

    }
}

//cleaning all the jobs that finished
void CleanJobs(JobStruct jobs[MAX_LINE],int *status) {
    int i =0;
    for (i = 0; i < MAX_LINE; i++) {
        if (jobs[i].pid != 0) {
            //get the pid that his child finish the process
            pid_t pidRet = waitpid(jobs[i].pid, status, WNOHANG);
            if (pidRet == jobs[i].pid || pidRet == -1) {
                jobs[i].pid = 0;
                strcpy(jobs[i].inputCommand, "");
            }
        }
    }
}


//print the jobs that are still running
void PrintJobs(JobStruct jobs[MAX_LINE]) {
    int i =0;
    for (i = 0; i < MAX_LINE; i++) {
        if (jobs[i].pid != 0) {
            printf("%d ", jobs[i].pid);
            int j=0;
            for (j = 0; j < jobs[i].inputCommand[j] != '\0'; j++) {
                char c = jobs[i].inputCommand[j];
                printf("%c", c);
            }
            printf("\n");
        }
    }
}

// Initialize the array of jobs by set all pid to null
void InitializeJobs(JobStruct jobs[MAX_LINE]) {
    int i = 0;
    for (i = 0; i < MAX_LINE; i++) {
        jobs[i].pid = 0;
    }
}