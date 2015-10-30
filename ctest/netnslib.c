#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

// man page says that _GNU_SOURCE has to be defined in order to be
// able to use setns() system call. But, by inspecting the source
// it turns out __USE_GNU has to be defined instead.
#define __USE_GNU
#include <sched.h>

#define STACK_SIZE   1024*1024*10

#define MOUNT_DIR   "/proc/self/ns/net"
#define NETNS_DIR   "/var/run/netns"
#define NETNS_NAME  "PVDNS"


int create_netns_proc(void *netns_name_proc) {
    char *netns_name = (char*)netns_name_proc;
    char netns_path[100];
    int netns_fd;
    char msg[500];

    printf("Proc %d is creating namespace %s\n", getpid(), netns_name);
    sleep(30);

    snprintf(netns_path, sizeof(netns_path), "%s/%s", NETNS_DIR, netns_name);
    /*
     * Create the base NETNS directory if it doesn't exist yet
     */
    if (mkdir(NETNS_DIR, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) < 0 && errno != EEXIST) {
        snprintf(msg, sizeof(msg), "Cannot create base NETNS directory %s\n", NETNS_DIR);
        perror(msg);
        return -1;
	}
    /*
     * Create the file for a given NETNS
     */
    netns_fd = open(netns_path, O_RDONLY|O_CREAT|O_EXCL, 0);
    if (netns_fd < 0) {
        snprintf(msg, sizeof(msg), "Cannot create/open NETNS file %s\n", netns_path);
        perror(msg);
        return -1;
	}
    else {
        /*
         * Create isolated NETNS for the process
         */
        //unshare(CLONE_NEWNET);
        /*
         * Attach the NETNS file to the process
         */
        if (mount(MOUNT_DIR, netns_path, "none", MS_BIND, NULL) < 0) {
            snprintf(msg, sizeof(msg), "Cannot mount NETNS file %s to the %s\n", netns_path, MOUNT_DIR);
            perror(msg);
            return -1;
        }
    }
    close(netns_fd);
    return 0;
}


/*
 * Create new NETNS in a separate process.
 * This will leave the parent process in its original NETNS.
 */
int create_netns(char *netns_name) {
    char *proc_stack;          // Start of stack buffer
    char *proc_stack_top;      // End of stack buffer
    char msg[500];

    proc_stack = malloc(STACK_SIZE);
    if (proc_stack == NULL) {
        snprintf(msg, sizeof(msg), "Cannot allocate stack memory for a NETNS creator process");
        perror(msg);
        return -1;
    }
    proc_stack_top = proc_stack + STACK_SIZE;  // Stack grows downward on Linux
    clone(create_netns_proc, proc_stack_top, CLONE_NEWNET, netns_name);

    /*
    switch(fork()) {
        case 0:
            if (create_netns_proc(netns_name) < 0) {
                snprintf(msg, sizeof(msg), "Cannot create NETNS %s\n", netns_name);
                perror(msg);
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
            break;
        case -1:
            return -1;
            break;
        default:
            return 0;
            break;            
    }
    */
}


int delete_netns(char *netns_name) {
    char netns_path[100];
    char msg[500];

    snprintf(netns_path, sizeof(netns_path), "%s/%s", NETNS_DIR, netns_name);
    /*
     * Dettach the NETNS file from the process
     */
    if (umount(netns_path) < 0) {
        snprintf(msg, sizeof(msg), "Cannot unmount NETNS file %s\n", netns_path);
        perror(msg);
        return -1;
    }
    /*
     * Delete the NETNS file from the filesystem
     */
    if (remove(netns_path) < 0 ) {
        snprintf(msg, sizeof(msg), "Cannot delete NETNS file %s\n", netns_path);
        perror(msg);
        return -1;
	}
    return 0;
}


int set_netns(char *netns_name) {
    char netns_path[100];
    int netns_fd;
    char msg[500];

    snprintf(netns_path, sizeof(netns_path), "%s/%s", NETNS_DIR, netns_name);
    netns_fd = open(netns_path, O_RDONLY);
    if (netns_fd < 0) {
        snprintf(msg, sizeof(msg), "Cannot open NETNS file %s\n", netns_path);
        perror(msg);
        return -1;
	}
    setns(netns_fd, CLONE_NEWNET);
    close(netns_fd);
    return 0;
}


int main(int argc, char **argv) {
    char msg[500];
    char NAME[50];

    printf("My PID: %d\n\n", getpid());

    printf("Sleeping for 30 seconds\n");
    sleep(30);


    if (create_netns(NETNS_NAME) < 0) {
        snprintf(msg, sizeof(msg), "Cannot create network namespace %s\n", NETNS_NAME);
        perror(msg);
        exit(EXIT_FAILURE);  
    }
    else {
        printf("Network namespace %s created\n", NETNS_NAME);
    }
    printf("Sleeping for 30 seconds\n");
    sleep(30);
    snprintf(NAME, sizeof(NAME), "bezveze");
    if (create_netns(NAME) < 0) {
        snprintf(msg, sizeof(msg), "Cannot create network namespace %s\n", NAME);
        perror(msg);
        exit(EXIT_FAILURE);  
    }
    else {
        printf("Network namespace %s created\n", NAME);
    }
    printf("Sleeping for 30 seconds\n");
    sleep(30);
    snprintf(NAME, sizeof(NAME), "imamvezu");
    if (create_netns(NAME) < 0) {
        snprintf(msg, sizeof(msg), "Cannot create network namespace %s\n", NAME);
        perror(msg);
        exit(EXIT_FAILURE);  
    }
    else {
        printf("Network namespace %s created\n", NAME);
    }


    printf("Sleeping for 30 seconds\n");
    sleep(30);




    if (set_netns(NETNS_NAME) < 0) {
        snprintf(msg, sizeof(msg), "Cannot enter network namespace %s\n", NETNS_NAME);
        perror(msg);
        exit(EXIT_FAILURE);  
    }
    else {
        printf("Now running in network namespace %s\n", NETNS_NAME);
    }

    printf("Sleeping for 30 seconds\n");
    sleep(30);

    snprintf(NAME, sizeof(NAME), "bezveze");
    if (set_netns(NAME) < 0) {
        snprintf(msg, sizeof(msg), "Cannot enter network namespace %s\n", NAME);
        perror(msg);
        exit(EXIT_FAILURE);  
    }
    else {
        printf("Now running in network namespace %s\n", NAME);
    }

    printf("Sleeping for 30 seconds\n");
    sleep(30);

    snprintf(NAME, sizeof(NAME), "imamvezu");
    if (set_netns(NAME) < 0) {
        snprintf(msg, sizeof(msg), "Cannot enter network namespace %s\n", NAME);
        perror(msg);
        exit(EXIT_FAILURE);  
    }
    else {
        printf("Now running in network namespace %s\n", NAME);
    }




    printf("Sleeping for 30 seconds\n");
    sleep(30);



    if (delete_netns(NETNS_NAME) < 0) {
        snprintf(msg, sizeof(msg), "Cannot delete network namespace %s\n", NETNS_NAME);
        perror(msg);
        exit(EXIT_FAILURE);  
    }
    else {
        printf("Network namespace %s deleted\n", NETNS_NAME);
    }
    snprintf(NAME, sizeof(NAME), "bezveze");
    if (delete_netns(NAME) < 0) {
        snprintf(msg, sizeof(msg), "Cannot delete network namespace %s\n", NAME);
        perror(msg);
        exit(EXIT_FAILURE);  
    }
    else {
        printf("Network namespace %s deleted\n", NAME);
    }
    snprintf(NAME, sizeof(NAME), "imamvezu");
    if (delete_netns(NAME) < 0) {
        snprintf(msg, sizeof(msg), "Cannot delete network namespace %s\n", NAME);
        perror(msg);
        exit(EXIT_FAILURE);  
    }
    else {
        printf("Network namespace %s deleted\n", NAME);
    }
}
