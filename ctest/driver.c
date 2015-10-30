#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SLEEP_TIME 10

int main(int argc, char **argv)
{
    char cmd[50];

    printf("My PID is: %d\n", getpid());
    printf("My network namespace is: ");
    fflush(stdout);
    snprintf(cmd, sizeof(cmd), "%s %d", "ip netns identify", getpid());
    system(cmd);
    printf("\nSleeping for %d seconds...", SLEEP_TIME);
    fflush(stdout);
    sleep(SLEEP_TIME);
    printf("FINISH\n");
	return 0;
}
