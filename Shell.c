//Jennifer Shammas
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <pwd.h>

// size of buff for directory
#define PATH_MAX 512
#define true 1

int checkForPipe(char ** str2, int* pipeCounter , int * pipeCounterForDone, int* firstPipeIndex, int* secondPipeIndex);
int dealWithPipes(char ** str2, int* pipeCounter, int firstPipeIndex, int lastPipeIndex , int wordsNum);
int dealWithOnePipe(char ** str2, int pipeIndex, int wordsNum);
int dealWithTwoPipes(char ** str2, int fisrtPipeIndex, int secondPipeIndex, int wordsNum);
char* editInput(char* str);

int main( int argc, const char * argv[]) {

    // the array size
    int size;
    //size of commands
    int LEN=PATH_MAX;
    char buff[LEN];
    char* str1 = (char*)malloc(sizeof(char) * PATH_MAX);

    //the new array
    char **str2;
    int numwords = 0;
    int wordsCounter = 0;
    int numchars = 0;
    int numcmd = 0;
    int PipeCounter = 0;
    int pipeCounterForDone = 0;
    int firstPipeIndex = 0;
    int secondPipeIndex = 0;
    char* paw;
    pid_t pid;
    struct passwd * pw;
    if( ( pw = getpwuid( getuid()) ) == NULL ) {
        perror( "getpwnam: unknown %s\n");
        paw="null";
    }
    else
        paw = pw->pw_name;  // get the name of the user

    if (getcwd(buff, LEN) == NULL) { // get the directory

        perror( "getcwdnam: unknown %s\n");
        exit(1);
    }

    while (true) {

        // full command array
        str2 = (char **) malloc(1 * sizeof(char **));

        if (str2 == NULL) {

            fprintf(stderr,"malloc falied");

            exit(1);

        }

        size = 0;

        printf("%s@%s>",paw,buff);
        str1 = (char*)malloc(sizeof(char) * PATH_MAX);
        fgets(str1,LEN, stdin);
        //
        numcmd++;

        str1[strlen(str1) - 1] = '\0';
        str1 = editInput(str1);
        numchars += (int)strlen(str1);

        char *tmp;
        tmp = strtok(str1, " ");
        while (tmp != NULL) {
            if (size > 0) {
                str2 = (char **) realloc(str2, (size + 1) * sizeof(char **));
                if (str2 == NULL) {
                    fprintf(stderr,"malloc falied");
                    exit(1);
                }
            }
            size++;
            //for words in command
            str2[size - 1] = (char *) malloc(strlen(tmp) * sizeof(char));
            if (str2[size-1] == NULL) {
                fprintf(stderr,"malloc falied");
                exit(1);
            }
            strcpy(str2[size - 1], tmp);
            numwords ++;
            wordsCounter ++;
            tmp = strtok(NULL, " ");

        }
        if (size==1&& strcmp(str2[0], "done") == 0) { //// cd | adasd
            free(str2[0]);
            free(str2);
            break;
        }

        if (strcmp(str2[0], "cd") == 0) { // when command is cd
            numcmd++;
            for (int i = 0; i < size; i++) {
                free(str2[i]);
            }
            free(str2);
            printf("command not supported (Yet)\n");
            continue;
        }

        str2 = (char **) realloc(str2, (size + 1) * sizeof(char **));
        if (str2 == NULL) {
            fprintf(stderr,"malloc falied");
            exit(1);
        }
        str2[size] = NULL; // add null for the last

        if(checkForPipe(str2, &PipeCounter ,&pipeCounterForDone ,&firstPipeIndex, &secondPipeIndex) == 1){
            dealWithPipes( str2, &PipeCounter, firstPipeIndex, secondPipeIndex, wordsCounter);
            wordsCounter = 0;
            PipeCounter = 0;
            firstPipeIndex = 0;
            secondPipeIndex = 0;
        }
        else{
            pid= fork();

            if (pid == 0) {

                if (execvp(str2[0], str2) == -1)
                    printf("This command not found!\n"); // send the command to the son after fork

                exit(0);
            }

            else if(pid >0) {
                numwords++;
                wait(NULL);
            }
            else{
                perror( "fail to fork %s\n");
                for (int i = 0; i < size; i++) {

                    free(str2[i]);

                }
                free(str2);
                exit(1);
            }

            // free all data in the array
             wordsCounter = 0;
            for (int i = 0; i < size; i++) {

                free(str2[i]);

            }
            free(str1);
            free(str2);

        }


    }

    printf("Number of commands: %d \n", numcmd -1);

    printf("Number of pipes: %d \n", pipeCounterForDone);

    printf("See You Next Time !");

    return 0;

}


int checkForPipe(char ** str2, int*pipCounter, int * pipeCounterForDone , int * fisrtPipeIndex, int* secondPipeIndex){
    int i = 0;
    int found = 0;
    while(str2[i] != NULL){

        if(strcmp(str2[i], "|") == 0){
            if(found == 1){
                *secondPipeIndex = i;
            }
            else{
                *fisrtPipeIndex = i;
            }
            found = 1;
            *pipCounter = *pipCounter + 1;
            *pipeCounterForDone = *pipeCounterForDone + 1;
        }
        i++;
    }
    return found;

}

int dealWithPipes(char ** str2, int* pipeCounter, int firstPipeIndex , int secondPipeIndex, int wordsNum){

    *pipeCounter == 1 ? dealWithOnePipe(str2, firstPipeIndex, wordsNum) : dealWithTwoPipes(str2, firstPipeIndex, secondPipeIndex, wordsNum);
    return 0;

}

int dealWithOnePipe(char ** str2 , int pipeIndex ,int wordsNum){
    char ** leftArr = NULL;
    char ** rightArr = NULL;     //// echo hello | wc -c Null
    int i;
    int leftArrLen = pipeIndex + 1;
    int rightArrLen = (wordsNum + 1) - (pipeIndex + 1);

    leftArr = (char**) malloc( leftArrLen  * sizeof(char *));
    rightArr = (char **) malloc(rightArrLen * sizeof(char *));
    if (leftArr == NULL) {
        fprintf(stderr,"malloc falied");
        exit(1);
    }
    if (rightArr == NULL) {
        fprintf(stderr,"malloc falied");
        exit(1);
    }
    /// buliding the left array:
    for(i = 0; i < pipeIndex ; i++){

        leftArr[i] = (char*) malloc(sizeof(char) * (strlen(str2[i]) + 1));
        strcpy(leftArr[i], str2[i]);

    }

    leftArr[pipeIndex] = NULL;


    /// buliding the right array:

    for(i = 0;  i < rightArrLen - 1; i++){
        rightArr[i] = (char*) malloc(sizeof(char) * (strlen(str2[pipeIndex + i + 1]) + 1));
        strcpy(rightArr[i], str2[pipeIndex + 1 + i]);
    }

    rightArr[rightArrLen - 1] = NULL;

    /// creating the pipes
    int fd[2];
    int pid1;
    int pid2;
    pipe(fd);
    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(1);
    }
    pid1 = fork();
    if (pid1 == -1)
    {
        perror("fork");
        exit(2);
    }
    if( pid1 == 0){
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execvp(leftArr[0], leftArr);
    }
    else{

        pid2 = fork();

        if(pid2 == 0){

            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            execvp(rightArr[0], rightArr);


        }
        else{
            close(fd[0]);
            close(fd[1]);
            wait(NULL);
            wait(NULL);
        }

    }
    return 0;
}

int dealWithTwoPipes(char ** str2, int firstPipeIndex, int secondPipeIndex, int wordsNum){
    char ** leftArr = NULL;
    char ** midArr = NULL;    //// echo hello | wc -c | sort (null)
    char ** rightArr =NULL;
    int i;
    int leftArrLen = firstPipeIndex + 1;
    int midArrLen = secondPipeIndex - firstPipeIndex;
    int rightArrLen = (wordsNum + 1)  - (secondPipeIndex + 1);

    leftArr = (char**) malloc( leftArrLen  * sizeof(char *));
    midArr = (char **) malloc(midArrLen * sizeof(char *));
    rightArr= (char **) malloc(rightArrLen * sizeof(char *));

    if (leftArr == NULL) {
        fprintf(stderr,"malloc falied");
        exit(1);
    }
    if (midArr == NULL) {
        fprintf(stderr,"malloc falied");
        exit(1);
    }
    if (rightArr == NULL) {
        fprintf(stderr,"malloc falied");
        exit(1);
    }
    /// buliding the left array:
    for(i = 0; i < firstPipeIndex ; i++){

        leftArr[i] = (char*) malloc(sizeof(char) * (strlen(str2[i]) + 1));
        strcpy(leftArr[i], str2[i]);

    }
    leftArr[firstPipeIndex] = NULL;

    /// buliding the mid array:
    for(i = 0;  i < midArrLen - 1; i++){
        midArr[i] = (char*) malloc(sizeof(char) * (strlen(str2[firstPipeIndex + i + 1]) + 1));
        strcpy(midArr[i], str2[firstPipeIndex + 1 + i]);
    }
    midArr[midArrLen - 1] = NULL;

    /// buliding the right array:
    for(i = 0;  i < rightArrLen - 1; i++){
        rightArr[i] = (char*) malloc(sizeof(char) * (strlen(str2[secondPipeIndex + i + 1]) + 1));
        strcpy(rightArr[i], str2[secondPipeIndex + 1 + i]);
    }
    rightArr[rightArrLen - 1] = NULL;

    
    

/// creating the pipes
    int fd[2];
    int fd2[2];
    int pid1;
    int pid2;
    int pid3;
    pipe(fd);
    pipe(fd2);
    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(1);
    }
    pid1 = fork();
    if (pid1 == -1)
    {
        perror("fork");
        exit(2);
    }
    if( pid1 == 0){
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        close(fd2[0]);
        close(fd2[1]);
        execvp(leftArr[0], leftArr);
    }
    else{

        pid2 = fork();

        if(pid2 == 0){

            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            close(fd2[0]);
            dup2(fd2[1], STDOUT_FILENO);
            close(fd2[1]);
            execvp(midArr[0], midArr);
            
        }
        else{
            
            pid3 =fork();

            if(pid3 == 0){
                close(fd2[1]);
                close(fd[0]);
                close(fd[1]);
                dup2(fd2[0], STDIN_FILENO);
                close(fd2[0]);
                execvp(rightArr[0], rightArr);
            }

            else{

                close(fd[0]);
                close(fd[1]);
                close(fd2[0]);
                close(fd2[1]);
                wait(NULL);
                wait(NULL);
                wait(NULL);
            }


        }

    }

    return 0;

}






char* editInput(char* str){
    int len = strlen(str);
    char* newStr = (char*)malloc(sizeof(char) * (len + 5));
    int i = 0;
    int j = 0;

    while(i < len){
        if(str[i] == '|' && str[i -1] != ' ' && str[i + 1] != ' '){
            newStr[j] = ' ';
            newStr[j + 1] = str[i];
            newStr[j + 2] = ' ';
            i += 1;
            j += 3;
        }
       else  if(str[i] == '|' && str[i -1 ] != ' '){
            newStr[j] = ' ';
            newStr[j + 1] = str[i];
            i += 1;
            j += 2;
        }
        else if(str[i] == '|' && str[i + 1 ] != ' '){
            newStr[j] = str[i];
            newStr[j + 1] = ' ';
            i += 1;
            j += 2;
        }

        else{
            newStr[j] = str[i];
            j++;
            i++;
        }

    }

    newStr[j] = '\0';
    free(str);
    return newStr;
}