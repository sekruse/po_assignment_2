#include <time.h>
#include <stdio.h>

clock_t counter;

void start_timer()
{
    counter = clock();
}

void stop_timer(char* message)
{
    counter = clock() - counter;
    printf("%s\n", message);
    printf("\tExecution time: %.3f secs\n", ((float)counter) / CLOCKS_PER_SEC);
}
