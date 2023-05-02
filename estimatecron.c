//  CITS2002 Project 1 2022
//  Student:   23152009   Barker   Alex

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>

#define MAX_LINES 20
#define MAX_PROCESSES 20
#define MAX_LEN 100
#define NAME_LEN 100

#define DAILY_MINS 1440
#define HOURLY_MINS 60
#define DAILY_HOURS 24

#define ERROR_VALUE -999

// structure representing the crontab entry and its estimated running time
struct crontab {
    int minute;
    int hour;
    int date;
    int month;
    int day;
    int estimate;
    char name[NAME_LEN + 1];
};

// structure representing a running process and how much time it has left to run
struct process {
    int pid;
    char name[NAME_LEN + 1];
    int timer;
};

// check if a given string contains only numbers
bool is_string_number(char *input){
    for (int i = 0; input[i] != '\0'; i++)
        if (!isdigit(input[i])) return false;
    return true;
}

// converts a string to lowercase 
char *lowerString(char *input) {
    for(int i = 0; input[i]; i++)
        input[i] = tolower(input[i]);
    return input;
}

//takes input string and converts it to a month 0-11
//returns -1 to represent any month
//returns -999 if there is an error to report later in context
int find_month(char *input) {
    int month = -1;
    if (is_string_number(input)) {
        month = atoi(input);
        if (month > 11 || month < 0) return ERROR_VALUE;
    }
    else {
        if (!strcmp(input, "*")) return -1;
        if (strlen(input) != 3) return ERROR_VALUE;
        
        lowerString(input);

        const char *abbrvs = "janfebmaraprmayjunjulaugsepoctnovdec";
        char *pointer = strstr(abbrvs, input);
        if (pointer == NULL) return ERROR_VALUE;
        month = (pointer - abbrvs) / 3;
    }
    return month;
}

// takes input string and converts it to a day 0-6
// returns -1 to represent any day
// returns -999 if there is an error to report later in context
int find_day(char *input){
    int day = -1;
    if (is_string_number(input)) {
        day = atoi(input);
        if (day > 6 || day < 0)     return ERROR_VALUE;            // incorrect number
    }
    else {
        if (!strcmp(input, "*"))    return -1;
        if (strlen(input) != 3)     return ERROR_VALUE;            // length other than 3

        lowerString(input);

        const char *abbrvs = "sunmontuewedthufrisat";
        char *pointer = strstr(abbrvs, input);
        if (pointer == NULL)        return ERROR_VALUE;            // day not found
        day = (pointer - abbrvs) / 3;
    }
    return day;
}

// takes in a filename to load 
FILE *loadFile(char *filename) {
    FILE *loadedFile = fopen(filename, "r");
    if (loadedFile == NULL) {
        printf("unable to open file %s", filename);
        exit(EXIT_FAILURE);
    }
    return loadedFile;
}

// converts a string to an integer
// returns -1 to represent any value 
int string_to_int(char* str) {
    int num;
    if (!strcmp(str, "*")) num = -1;
    else if (!is_string_number(str)) return ERROR_VALUE;
    else num = atoi(str);
    return num;
}

// fills a crontab struct array with values from two files, crontab_file & estimates_file
int construct_crontabs(struct crontab crontabs[MAX_LEN], FILE *crontab_file, FILE *estimates_file){
    char line[MAX_LEN];
    int ctIndex = 0;  

    int lineNum = 0;
    while (fgets(line, sizeof line, crontab_file) != NULL) {
        lineNum++;
        // find first non-whitespace & skip comments
        int i = 0;
        while (isspace(line[i])) i++;  
        if (line[i] == '#') continue;

        char *token = strtok(line, " \t");
        int field = 0;
        while (token != NULL) {
            token[strcspn(token, "\r\n")] = 0; // remove newlines
            // if line isnt empty
            if (strcmp(token,"\0")) {
                switch (field){
                    case 0:
                        crontabs[ctIndex].minute    = string_to_int(token);
                        break;
                    case 1:
                        crontabs[ctIndex].hour      = string_to_int(token);
                        break;
                    case 2:
                        crontabs[ctIndex].date      = string_to_int(token);
                        break;
                    case 3:
                        crontabs[ctIndex].month     = find_month(token);
                        break;
                    case 4:
                        crontabs[ctIndex].day       = find_day(token);
                        break;
                    case 5:
                        strcpy(crontabs[ctIndex].name, token);
                        break;
                }
                field++;    
            }
            if (field == 6) break; // skip anything written after
            token = strtok (NULL, " \t");
        }
        // incorrect number of fields, or bad value
        if (field != 6 ||
            crontabs[ctIndex].minute    == ERROR_VALUE ||
            crontabs[ctIndex].hour      == ERROR_VALUE ||
            crontabs[ctIndex].date      == ERROR_VALUE ||
            crontabs[ctIndex].month     == ERROR_VALUE ||
            crontabs[ctIndex].day       == ERROR_VALUE) {
            fprintf(stderr, "invalid crontab entry at line %d\n", lineNum);
            exit(EXIT_FAILURE);
        }
        //printf("%d\t%d\t%d\t%d\t%d\t%s\n",crontabs[ctIndex].minute, crontabs[ctIndex].hour, crontabs[ctIndex].date, crontabs[ctIndex].month, crontabs[ctIndex].day,crontabs[ctIndex].name);
        crontabs[ctIndex].estimate = -1;
        ctIndex++;
    }
    
    // add estimates
    lineNum = 0;
    while (fgets(line, sizeof line, estimates_file) != NULL) {
        lineNum++;
        // find first non-whitespace & skip comments
        int i = 0;
        while (isspace(line[i])) i++;  
        if (line[i] == '#') continue;

        char *token = strtok(line, " \t");
        int field = 0;
        int estimate;
        char name[NAME_LEN];
        while (token != NULL) {
            token[strcspn(token, "\r\n")] = 0; // remove newlines
            // if line isnt empty
            if (strcmp(token,"\0")) {
                switch (field){
                    case 0:
                        strcpy(name, token);
                        break;
                    case 1:
                        if (!is_string_number(token)) estimate = ERROR_VALUE;
                        else estimate = atoi(token);
                        break;
                }
                field++;
            }
            if (field == 2) break; // skip anything written after
            token = strtok (NULL, " \t");
        }

        // incorrect number of fields, or bad value
        if (field != 2 || estimate == ERROR_VALUE) {
            fprintf(stderr, "invalid estimates entry at line %d\n", lineNum);
            exit(EXIT_FAILURE);
        }

        // find crontab with the same name 
        for (int j = 0; j < MAX_LEN; j++) {
            if (strcmp(crontabs[j].name, name)) continue;
            crontabs[j].estimate = estimate;
        }
    }
    // loop through all crontabs and check if their estimate is blank
    for (int j = 0; j < MAX_LEN; j++) {
        if (crontabs[j].estimate != -1) continue;
        fprintf(stderr, "estimates not found for command %s\n", crontabs[j].name);
        exit(EXIT_FAILURE);
    }
}

// code from workshop 1 by Chris McDonald
int first_day_of_month(int month, int year) {
    struct tm tm;

    memset(&tm, 0, sizeof(tm));

    tm.tm_mday = 1;
    tm.tm_mon = month-1; 
    tm.tm_year = year-1900;

    mktime(&tm);

    return tm.tm_wday;
}

// check if the given crontab can run in the given minute of the month
bool can_run(struct crontab crontab, int minute, int month, int firstDay){
    int curDay = (minute / DAILY_MINS + firstDay) % 7;
    int curDate = (minute / DAILY_MINS) + 1;
    int curHour = ((minute / HOURLY_MINS) % DAILY_HOURS);
    int curMin = ((minute % DAILY_MINS) % HOURLY_MINS);

    // returns false if any part of the time is not correct 
    if (crontab.minute  != curMin   && crontab.minute   != -1 ||
        crontab.hour    != curHour  && crontab.hour     != -1 ||
        crontab.day     != curDay   && crontab.day      != -1 ||
        crontab.date    != curDate  && crontab.date     != -1 || 
        crontab.month   != month    && crontab.month    != -1
        ) return false;
    return true;
}

// add the process to the array of active processes
void invoke(struct process processes[MAX_PROCESSES], struct crontab crontab, int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid != 0) continue; // skip full process slots

        strcpy(processes[i].name, crontab.name);
        processes[i].timer = crontab.estimate;
        processes[i].pid = pid;
        break;
    }
}

// simulate each minute of a given month 
void simulate(int month, struct crontab crontabs[MAX_LEN]){

    // count number of crontabs there are 
    int numCrontabs;
    for (int j = 0; j < MAX_LEN; j++) {
        if (strcmp(crontabs[j].name,"\0")) continue;
        numCrontabs = j + 1;
        break;
    }
   
    // find first day of current year with given month
    time_t t = time(NULL);
    int firstDay = first_day_of_month(month, (*localtime(&t)).tm_year + 1900);

    // simulate every minute of a month
    const int DAYS[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int minutes = DAYS[month] * DAILY_MINS;
    int nRunning;
    int maxRunning;
    int pid;

    struct process processes[MAX_PROCESSES] = {0};
    int counter[MAX_LEN];

    for (int minute = 0; minute < minutes; minute++){

        // manage timers & terminate all programs first
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (processes[i].pid == 0) continue; // skip empty process slots
            processes[i].timer--;
            if (processes[i].timer <= 0){
                printf("%d:  terminated pid: %d %s: now %d running\n",minute, processes[i].pid, processes[i].name, (nRunning - 1));
                processes[i].pid = 0;
                processes[i].timer = 0;
                strcpy(processes[i].name, "\0");

                nRunning--;
            }
        }
        // loop through all crontabs and check if they can run
        for (int j = 0; j < numCrontabs; j++){
            if (!can_run(crontabs[j],minute,month,firstDay)) continue;
            
            // run new programs
            pid++;
            invoke(processes, crontabs[j], pid);
            nRunning++;
            counter[j]++;
            printf("invoked %s: now %d running\n", crontabs[j].name, nRunning);
        }

        if (nRunning > maxRunning) 
            maxRunning = nRunning;
    }
    // find most run crontab by index
    int mostRunsIndex = -1;
    for (int j = 0; j < numCrontabs; j++){
        if (mostRunsIndex == -1 || counter[j] > counter[mostRunsIndex]) 
            mostRunsIndex = j;
    }

    printf("%s\t%d\t%d\n",crontabs[mostRunsIndex].name, pid, maxRunning);
}

int main(int argc, char *argv[]) {
    char *usage =   "Usage:\n"
                    "./estimatecron month crontab-file estimates-file\n"
                    "\tmonth: 0-6 or jan-dec\n"
                    "\tcrontab-file: name of file or path\n"
                    "\testimates-file: name of file or path";
    if (argc != 4) {
        fprintf(stderr, "Incorrect number of arguments\n%s\n", usage);
        exit(EXIT_FAILURE);
    }

    // get the month specified
    int month = find_month(argv[1]);
    if (month == ERROR_VALUE || month == -1) {
        fprintf(stderr, "Error in month argument\n%s\n", usage);
        exit(EXIT_FAILURE);
    }

    // load files into crontabs struct array
    FILE *crontab = loadFile(argv[2]);
    FILE *estimates = loadFile(argv[3]);
    struct crontab crontabs[MAX_LEN] = {0}; 
    construct_crontabs(crontabs, crontab, estimates);
    fclose(crontab);
    fclose(estimates);

    simulate(month, crontabs);

    return 0;
}