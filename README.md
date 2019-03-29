Embedded & CP linux tool to show SHEDULE and priority of proces.

	Help:

		chss -l - list settings for all processes
		chss pid - list settings for given pid
		chss pid policy prio [nice] - set params for given pid
		Policy:
        		SCHED_OTHER = 0
        		SCHED_FIFO = 1
        		SCHED_RR = 2 SCHED_BATCH = 3
