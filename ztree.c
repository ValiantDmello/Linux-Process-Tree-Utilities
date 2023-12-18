#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<time.h>
#include<sys/stat.h>

int getDescCount(pid_t p){	//same function from prctree
	char cmd[256], line[256];
	int i=0, descprc_count=0;
	sprintf(cmd, "pstree -p %d| grep -oP '\\(\\K\\d+(?=\\))'", p);
	FILE *fp = popen(cmd, "r");
	while(fgets(line, 256, fp) != NULL){
		descprc_count++;
	}
	return descprc_count;
}

pid_t *getDescPrc(pid_t p, int count){ //same function from prctree
	char cmd[256], line[256];
	int i=0;
	pid_t *desc_prc = malloc(sizeof(pid_t)*count);
	sprintf(cmd, "pstree -p %d| grep -oP '\\(\\K\\d+(?=\\))'", p);
	FILE *fp = popen(cmd, "r");
	while(fgets(line, 256, fp) != NULL){
		//printf("%s\n", line);
		desc_prc[i] = atoi(line);
		i++;
	}
	pclose(fp);
	return desc_prc;
}

int checkzombie(pid_t pid){ //same function from prctree
	char path[256];
	sprintf(path, "/proc/%d/status", pid);
	FILE *file = fopen(path, "r");
	if(file==NULL){
		printf("error\n");
		exit(EXIT_FAILURE);
	}
	
	char line[1024];
	char *pid_status;
	while(fgets(line, 1024, file)!=NULL){
		if(strncmp(line, "State:", 6) == 0){	//line which has the status of the process
			pid_status = strchr(line, ':') + 2; //+2 to remove : and blankspace
			break;
		}
	}
	//printf("status: %s\n", pid_status);
	if(pid_status == NULL){
		printf("Error: unable to detremine if process %d is zombie or not\n", pid);
		exit(EXIT_FAILURE);
	}else if(strncmp(pid_status, "Z (zombie)", 10) == 0){	
		return 1;
	}else{
		return 0;
		//printf("not a zombie process\n");
	}
	return 0;
}

pid_t getParentID(pid_t pid){ //same function from prctree
	pid_t ppid;
	char cmd[256], output[256]; //ps -o ppid= 3049901
	sprintf(cmd, "ps -o ppid= %d", pid);
	FILE *fp = popen(cmd, "r");
	if(fp == NULL){
		printf("ERROR : vant get parent process id of %d\n", pid);
		exit(EXIT_FAILURE);
	}
	if(fgets(output, 256, fp) != NULL){
		ppid = atoi(output);
	}
	
	return ppid;
}

int killzparent(pid_t root_pid){	//function to kill parent of zombie process
	int count = getDescCount(root_pid);	
	int n = 0;
	printf("count : %d\n", count);
	pid_t *desc_prc = getDescPrc(root_pid, count);	//get descendant processes
	printf("size : %d\n", sizeof(desc_prc));
	int i;
	for(i=0; i<count; i++){
		if(checkzombie(desc_prc[i])==1){	//check if process is zombie
			int ppid = getParentID(desc_prc[i]);	//get ppid
			printf("Terminating %d which if parent of defunct process %d\n", ppid, desc_prc[i]);
			kill(ppid , SIGTERM);	//kill zombie parent
			n++;
		}
	}
	return n;	
}

double getelaspedtime(pid_t pid){	//function to return elasped time of a process 
	char path[256];
	sprintf(path, "/proc/%d/stat", pid);	//file which contains time of process start
	
	struct stat stat_info;
	if(stat(path, &stat_info) == -1){	//error
		printf("Error: cant determine elspsed time of process %d\n", pid);
		exit(EXIT_FAILURE);
	} 
	
	time_t pid_stime = stat_info.st_ctime;	//start time of process
	time_t curr_time = time(NULL);		//current time
	double pid_eltime;			
	pid_eltime = difftime(curr_time, pid_stime)/60;	//elasped time of the process
	
	return pid_eltime; 
}

int killtzparent(pid_t root_pid, double t){	//kill parent with elasped time parameter
	int count = getDescCount(root_pid);
	int n = 0;
	pid_t *desc_prc = getDescPrc(root_pid, count);//get descendant processes
	int i;
	for(i=0; i<count; i++){
		if(checkzombie(desc_prc[i])==1){	//check if process is zombie
			int ppid = getParentID(desc_prc[i]);	//get ppid
			double elasped_time = getelaspedtime(ppid);	//get elasped time of pid
			if(elasped_time>t){
				printf("Terminating %d which if parent of defunct process %d\n", ppid, desc_prc[i]);
				kill(ppid , SIGTERM);	//kill ppid if condition true
				n++;
			}
		}
	}
	return n;	
}

int killbzparent(pid_t root_pid, int ndfc){
	int count = getDescCount(root_pid);
	pid_t defunc[count];
	int n = 0;
	pid_t *desc_prc = getDescPrc(root_pid, count); //get descendant processes
	int i;
	for(i=0; i<count; i++){
		if(checkzombie(desc_prc[i])==1){	//check if process is zombie
			defunc[n] = desc_prc[i];
			n++;	//counter for number of defunt processes
		}
	}
	if (n == 0)
		return 0;
	i = 0;
	if(n>=ndfc){	//if total no of defunct process >= given no
		for(i=0; i<n; i++){
			int ppid = getParentID(defunc[i]);	//get ppid
			printf("Terminating %d which if parent of defunct process %d\n", ppid, defunc[i]);
			kill(ppid , SIGTERM);	//kill ppid if condition true
		}
	}
	return i;
		
}

int main(int argc, char *argv[]){
	int temp;
	pid_t ppid;
	
	if(argc<2 || argc>4){
		perror("ERROR: incorrect parameter\n");
		exit(EXIT_FAILURE);
	}
	pid_t root_pid = atoi(argv[1]);	//root process
	
	if(argc==2){
		int defunct = killzparent(root_pid);	//kill parent process of zombies
		if(defunct == 0){
			printf("No defunct process in processes rooted at %d\n", root_pid);
			exit(EXIT_SUCCESS);
		}
		exit(EXIT_SUCCESS);
	}
		
	if(argc == 3){
		printf("Error: missing parameter\n");
		exit(EXIT_FAILURE);
	}
	
	if(strcmp(argv[2], "-t")==0){
		double t = atof(argv[3]);
		if(t<1){
			printf("ERROR: PROC_ELTIME < 1\n");
			exit(EXIT_FAILURE);
		}
		int defunct = killtzparent(root_pid, t);	//kill parent of zombie based on input PROC_ELTIME
		if(defunct == 0){
			printf("No defunct process in processes rooted at %d with parent process elasped time > %0.1f minutes\n", root_pid, t);
		}
		exit(EXIT_SUCCESS);	
	}else if(strcmp(argv[2], "-b")==0){
		int ndfc = atoi(argv[3]);
		if(ndfc<1){
			printf("ERROR; NO_OF_DFCS < 1\n");
			exit(EXIT_FAILURE);
		}
		int defunct = killbzparent(root_pid, ndfc);	//kill parent of zombie based on input NO_OF_DFCS
		if(defunct == 0){
			printf("No defunct process in processes rooted at %d with parent process who has defunct child >= %d\n", root_pid, ndfc);
		}
		exit(EXIT_SUCCESS);
	}else{
		printf("Error: Incorrect options\n");
		exit(EXIT_FAILURE);
	}
	
}
