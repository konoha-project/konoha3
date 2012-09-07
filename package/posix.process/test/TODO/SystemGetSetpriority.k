//Written by Joseph

K.import("posix.process");

void test() {
	//NOTE: The highest->-20, The lowest->19
	assert(System.setpriority(PRIO_PROCESS, 0, 10) == 0);//TODO Need the root permission to set higher priority
	assert(System.getpriority(PRIO_PROCESS, 0) == 10);
	assert(System.setpriority(PRIO_PROCESS, 0, 15) == 0);
	assert(System.getpriority(PRIO_PROCESS, 0) == 15);
	assert(System.setpriority(PRIO_PROCESS, 0, 20) == -1);
	assert(System.setpriority(PRIO_PROCESS, 0, -21) == -1);

	assert(System.setpriority(PRIO_PGRP, 0, 10) == 0);//TODO Need the root permission to set higher priority
	assert(System.getpriority(PRIO_PGRP, 0) == 10);
	assert(System.setpriority(PRIO_PGRP, 0, 30) == -1);
	assert(System.setpriority(PRIO_PGRP, 0, -30) == -1);

	assert(System.setpriority(PRIO_USER, 0, 5) == 0);//TODO Need the root permission to set higher priority
	assert(System.getpriority(PRIO_USER, 0) == 5);
	assert(System.setpriority(PRIO_USER, 0, 20) == -1);
	assert(System.setpriority(PRIO_USER, 0, -25) == -1);
}

test();
