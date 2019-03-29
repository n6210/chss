/*
    (C) 2014 Taddy G. fotonix@protonmail.com
*/

#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/resource.h>


int mode, policy, prio, a_nice;
pid_t pid;
struct timespec tp;
char process_name[2000];

void ERR(char *str)
{
    printf("Error in %s() %d %s\n", str, errno, strerror(errno));
    exit(2);
}

void how_to_use()
{
    printf(
    "chss -l - list settings for all processes\n"
    "chss pid - list settings for given pid\n"
    "chss pid policy prio [nice] - set params for given pid\n"
    "Policy:\n \tSCHED_OTHER = %d\n \tSCHED_FIFO = %d\n \tSCHED_RR = %d SCHED_BATCH = %d\n\n", 
    SCHED_OTHER, SCHED_FIFO, SCHED_RR, SCHED_BATCH);
    exit(1);
}

int is_user_process(pid_t pid)
{
    int up, fd;
    char filename[64];
    
    sprintf(filename,"/proc/%d/cmdline", pid);
    fd = open(filename, O_RDONLY);
    up = read(fd, filename, 1);
    close(fd);
    
    return (up);
}

int read_process_name(pid_t pid, char *name, int len)
{
    int fd;
    char filename[64], buffer[2000], *pos;
    
    sprintf(filename,"/proc/%d/status", pid);
    
    if ((fd = open(filename, O_RDONLY)) < 0)
		return -1;
	
    if (read(fd, buffer, 2000) <= 0)
		return -1;
	
    close(fd);
    
    if ((pos = strchr(buffer, 0x09) + 1))
		if ((pos = memccpy(name, pos, 0x0a, len)))
			pos[-1] = 0;
    		
    if (pos == NULL)
	    return -1;
	    
    return 0;
}


void set_prio(int int_policy, int priority, pid_t pid, int n)
{
    struct  sched_param p;
    
    if (int_policy)
    {
		if (setpriority(PRIO_PROCESS, pid, n))
			ERR("setpriority");

		p.sched_priority = priority;
		if (sched_setscheduler(pid, int_policy, &p) < 0)
			ERR("sched_setscheduler");
    }
    else
    {
		p.sched_priority = 0;
		if (sched_setscheduler(pid, int_policy, &p) < 0)
			ERR("sched_setscheduler");
		
		if (setpriority(PRIO_PROCESS, pid, priority))
			ERR("setpriority");
    }

}


int get_prio(int int_policy, pid_t pid)
{
    int int_prio = 0;
    struct  sched_param int_p;
    
    if (int_policy)
    {
		if (sched_getparam(pid, &int_p) < 0)
			ERR("sched_getparam");
		
		int_prio = int_p.sched_priority;
    }
    else
    {
		errno = 0;
		int_prio = getpriority(PRIO_PROCESS, pid);
		if (errno)
			ERR("getpriority");
    }
    
    return int_prio;
}


int main(int argc, char *argv[])
{
    if (argc == 2)
    {
		if ((argv[1][0] == 'l') || ((argv[1][0] == '-') && (argv[1][1] == 'l')))
		{
		    // Listing mode by PIDs
	    	// PID == 0 means own parameters
	    	for (pid = 1; pid <= 0x8000 ; pid++)
	    	{
	    		policy = sched_getscheduler(pid);
				if (policy < 0)
					continue;
				else
				{
		    		if (sched_rr_get_interval(pid, &tp) < 0) 
						ERR("sched_rr_get_interval");
        	    
		    		if (read_process_name(pid, process_name, sizeof(process_name)) < 0)
						strcpy(process_name, "?!!!");
		    
            	    printf("PID: %5d  ", pid);
            	    
		    		if (is_user_process(pid))
						printf("%s  ",process_name);
		    		else
						printf("[%s]",process_name);
			
		    		{ //Small ajusting of process name
						char buf[32];
						int x = strlen(process_name);
			
						x = (x < 32) ? 32 - x : 0;
						if (x > 0) 
						{
			    			memset(buf, ' ', x);
			    			buf[x] = 0;
			    			printf("%s", buf);
						}
						printf(" ");
		    		}
			
            	    switch (policy & 0xFFFF)
            	    {
                	case 0:
                    	    printf("SCHED_OTHER ");
                    	    break;
                	case 1:
                    	    printf("SCHED_FIFO  ");
                    	    break;
                	case 2:
                    	    printf("SCHED_RR    ");
                    	    break;
                    case 3:
        					printf("SCHED_BATCH ");
        					break;
                    default:
                    		printf("UNKNOWN (%d)", policy);
            	    }

            	    printf("   prio: %3d", get_prio(policy, pid));

            	    if (policy & SCHED_RESET_ON_FORK)
            	    		printf(" SCHED_RESET_ON_FORK");
		    		
		    		if ((policy == 0) || (policy == 2))
						printf("   timeslice: %4d msec", (int)tp.tv_nsec/1000000);
		    		
		    		printf("\n");
				} 
	    }
	    printf("\n");
	    fflush(0);
	    exit(0);
	}
	else
	{
	    // We wait for a digit in other case it's wrong program parameter
	    if (!isdigit(argv[1][0]))
			    how_to_use();
		
	    pid = atoi(argv[1]);
            read_process_name(pid, process_name, sizeof(process_name));
	    
	    printf("PID: %d  ", pid);
	    if (is_user_process(pid))
		    printf("%s   ", process_name);
	    else
		    printf("[%s]   ", process_name);
	
	    if ((policy = sched_getscheduler(pid)) < 0)
		    ERR("sched_getscheduler");
	    
	    if (sched_rr_get_interval(pid, &tp) < 0)
		    ERR("sched_rr_get_interval");
	    
	    switch (policy)
	    {
    		case 0:
        	    printf("SCHED_OTHER");
		    break;
    		case 1:
        	    printf("SCHED_FIFO");
        	    break;
    		case 2:
        	    printf("SCHED_RR");
        	    break;
        	case 3:
        		printf("SCHED_BATCH");
	    }
            printf("   prio: %2d   ", get_prio(policy, pid));
        
	    if ((policy == 0) || (policy == 2))
			printf("timeslice: %4d msec", (int)tp.tv_nsec/1000000);
	    
	    printf("\n");
	    fflush(0);
	    exit(0);
	}
    }
    else
    {
	// program was called with longer arguments list
	// we accept 4 and max 5 (pid, policy, priority [nice])
	// in other case we show how to use it
	if (argc >= 4) 
	{
	    pid = atoi(argv[1]);
	    policy = atoi(argv[2]);
	    prio = atoi(argv[3]);
	    a_nice = 0;

	    if (argc >= 5)
			a_nice = atoi(argv[4]);
	}
	else
	    how_to_use();
    }
    
    read_process_name(pid, process_name, sizeof(process_name));
	    
    printf("PID: %d   ", pid);
    if (is_user_process(pid))
	    printf("%s   ", process_name);
    else
	    printf("[%s]   ", process_name);

    set_prio(policy, prio, pid, a_nice);

    switch (policy)
    {
        case 0:
            printf("SCHED_OTHER");
	    	break;
        case 1:
            printf("SCHED_FIFO");
            break;
        case 2:
            printf("SCHED_RR");
            break;
        case 3:
        	printf("SCHED_BATCH");
    }
    printf("   prio: %d", prio);
    
    if ((policy == 0) || (policy == 2)) 
    {
		if (sched_rr_get_interval(pid, &tp) < 0)
        	ERR("sched_rr_get_interval");
        
        printf("   timeslice: %4d msec", (int)tp.tv_nsec/1000000);
    }
    printf("\n");
    fflush(0);
    
    return 0;
}



