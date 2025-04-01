#include <omp.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Custom types and constants
enum constants { NS_PER_SECOND = 1000000000, NS_PER_MILLISECOND = 1000000, MS_PER_SECOND = 1000 };
enum state {A=0, B=1, C=2};

// Helper function definitions
char *generateString(int n);
enum state dsa(enum state state, char *s);
enum state getNextState(enum state prevState, char c);
void getTimeDiff(struct timespec before, struct timespec after, struct timespec *diff);

int main(int argc, char *argv[]) {
    
    // Parse parameters and set up multi-threading
    if (argc < 3) {
        printf("ERROR: 2 parameters expected\n");
        return -1;
    }
    int t = atoi(argv[1]);
    int n = atoi(argv[2]);
    omp_set_dynamic(0);
    omp_set_num_threads(t);

    // Generate and print our test string
    char *s = generateString(n);
    printf("%s\n", s);
    
    // Allocate memory for substrings
    int subStringLength = t ? n/(t+1) : 0;
    int lastSubStringLength = t ? subStringLength + (n%subStringLength) : 0;
    char **subStrings = malloc(sizeof(*subStrings)*(t+1));
    
    // Divide string into substrings
    for(int i=0; i<t; i++) {
        subStrings[i] = malloc(sizeof(char) * subStringLength+1);
        strncpy(subStrings[i], s+(i*subStringLength), subStringLength);
        subStrings[i][subStringLength] = '\0';
    }
    subStrings[t] = s + (t*subStringLength);

    // Allocate space for speculative state map
    enum state **map = malloc(sizeof(*map) * (t+1));
    for(int i=1; i<=t; i++)
        map[i] = malloc(sizeof(enum state)*3);

    // Start timer
    struct timespec before, after, diff;
    clock_gettime(CLOCK_REALTIME, &before);
    
    // Populate the map for each substring concurrently
#pragma omp parallel for
    for(int i=1; i<=t; i++) {
        map[i][A] = dsa(A, subStrings[i]);
        map[i][B] = dsa(B, subStrings[i]);
        map[i][C] = dsa(C, subStrings[i]);
    }
    
    // Iterate through the map to determine the final state
    enum state finalState = dsa(A, subStrings[0]);
    for(int i=1; i<=t; i++)
        finalState = map[i][finalState];
    
    // Stop timer
    clock_gettime(CLOCK_REALTIME, &after);
    getTimeDiff(before, after, &diff);

    // Print results
    printf("%s\n", finalState == 0 ? "true" : "false");
    printf("%.2fms elapsed\n", (diff.tv_sec * MS_PER_SECOND) + (diff.tv_nsec/(double)NS_PER_MILLISECOND));

    return 0;
}

// Function to randomly generate a string of ascii base-10 digits
char *generateString(int length) {

    char *string = malloc(sizeof(char)*(length+1));
    for(int i=0; i<length; i++) {
        string[i] = (rand() % 10) + 48;
    }
    string[length] = '\0';
    return string;
}

// Apply DSA encoding to each character sequentially
enum state dsa(enum state state, char *s) {
    
    char *c = s;
    while(*c != '\0') {
        state = getNextState(state, *c);
        c += sizeof(char);
    }
    return state;
}

// Encoding of all DSA edges for each state and character pair
enum state getNextState(enum state prevState, char c) {

    switch(prevState) {
        case A:
            switch(c) {
                case '0':
                case '9':
                    return A;
                case '2':
                case '5':
                case '6':
                case '8':
                    return B;
                default:
                    return C;
            }
        case B:
            switch(c) {
                case '1':
                case '3':
                case '7':
                    return A;
                case '4':
                case '8':
                    return B;
                default:
                    return C;
            }
        case C:
            switch(c) {
                case '2':
                case '5':
                case '6':
                case '7':
                    return A;
                case '1':
                case '4':
                case '8':
                    return B;
                default:
                    return C;
            }
    }
}

// Calculate the difference in time between two timepsec structs
void getTimeDiff(struct timespec before, struct timespec after, struct timespec *diff) {

    diff->tv_nsec = after.tv_nsec - before.tv_nsec;
    diff->tv_sec  = after.tv_sec - before.tv_sec;
    if(diff->tv_sec > 0 && diff->tv_nsec < 0) {
        diff->tv_nsec += NS_PER_SECOND;
        diff->tv_sec--;
    } else if (diff->tv_sec < 0 && diff->tv_nsec > 0) {
        diff->tv_nsec -= NS_PER_SECOND;
        diff->tv_sec++;
    }
}