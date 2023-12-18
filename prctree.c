#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>

pid_t getParentID(pid_t pid){	//function to return ppid of pid
	pid_t ppid;
	char cmd[256], output[256]; 
	sprintf(cmd, "ps -o ppid= %d", pid);	//command to get ppid of pid
	FILE *fp = popen(cmd, "r");
	if(fp == NULL){
		printf("ERROR : vant get parent process id of %d\n", pid);
		exit(EXIT_FAILURE);
	}
	if(fgets(output, 256, fp) != NULL){
		ppid = atoi(output);	
	}
	
	return ppid;	//return ppid
}

int getDescCount(pid_t p){	//function to get count of descendant processes of given process
	char cmd[256], line[256];
	int i=0, descprc_count=0;
	sprintf(cmd, "pstree -p %d| grep -oP '\\(\\K\\d+(?=\\))'", p); //pstree command to get all processed rooted at p
	FILE *fp = popen(cmd, "r");
	while(fgets(line, 256, fp) != NULL){
		descprc_count++;
	}
	return descprc_count;	//return count
}

pid_t *getDescPrc(pid_t p, int count){ //function to get all descendant processes of given process
	char cmd[256], line[256];
	int i=0;
	pid_t *desc_prc = malloc(sizeof(pid_t)*count);	//array to store pids of descendant process
	sprintf(cmd, "pstree -p %d| grep -oP '\\(\\K\\d+(?=\\))'", p);	//pstree command to get all processed rooted at p
	FILE *fp = popen(cmd, "r");
	while(fgets(line, 256, fp) != NULL){
		//printf("%s\n", line);
		desc_prc[i] = atoi(line);	//add process id to array
		i++;
	}
	pclose(fp);
	return desc_prc;	//return array of descendant process ids
}

void checkifdescofroot(pid_t root_pid, pid_t pid){
	int count = getDescCount(root_pid);
	pid_t *desc_prc = getDescPrc(root_pid, count);
	int i;
	int is_desc = 0;
	for(i=0; i<count; i++){
		//printf("%d\n", desc_prc[i]);
		if(pid == desc_prc[i]){
			is_desc = 1;
			break;
		}
	}
	if(is_desc == 0){
		printf("Mentioned process id not a descendant od root process\n");
		exit(EXIT_FAILURE);
	}
}

void getchildprc(pid_t pid){	//print child processes
	int count = getDescCount(pid);
	pid_t *desc_prc = getDescPrc(pid, count);	//array of decendant processes of pid
	int i;
	for(i=0; i<count; i++){
		if(getParentID(desc_prc[i]) == pid){	//if parent id == pid, then it is a child process
			printf("%d ", desc_prc[i]);	//print them
		}
	}
}

void getsibprc(pid_t root_pid, pid_t pid){	//print sibling processes
	int count = getDescCount(root_pid);
	pid_t *desc_prc = getDescPrc(root_pid, count); //array of decendant processes of root_pid
	int i;
	for(i=0; i<count; i++){
		if(getParentID(desc_prc[i]) == getParentID(pid) && desc_prc[i]!=pid){	//if ppid of a descendant process == ppid of pid, then sibling
			printf("%d ", desc_prc[i]);	//print them
		}
	}
}

void getgpprc(pid_t pid){	//print grandparent process
	pid_t ppid = getParentID(pid);	//ppid of pid
	pid_t gppid = getParentID(ppid);	//ppid of ppid, which is gp of pid
	printf("%d", gppid);
}

void getgcprc(pid_t pid){	//prind grandchild processes
	
	int count = getDescCount(pid);
	pid_t *desc_prc = getDescPrc(pid, count);	//get descendant processes
	int i,j, child_count;
	for(i=0; i<count; i++){
		if(getParentID(desc_prc[i]) == pid){	//if this condition true then desc_prc[i] is child of pid
			getchildprc(desc_prc[i]);	//get child of desc_prc[i], whick is grandchild of pid
		}
	}
}

int checkzombie(pid_t pid){	//check if pid is zombie process
	char path[256];
	sprintf(path, "/proc/%d/status", pid);	//file storing status of pid
	FILE *file = fopen(path, "r");
	if(file==NULL){
		printf("error\n");
		exit(EXIT_FAILURE);
	}
	
	char line[1024];
	char *pid_status;
	//extract the status info
	while(fgets(line, 1024, file)!=NULL){
		if(strncmp(line, "State:", 6) == 0){	//line which has the status of the process
			pid_status = strchr(line, ':') + 2; //+2 to remove : and blankspace
			break;
		}
	}
	if(pid_status == NULL){	//error
		printf("Error: unable to detremine if process %d is zombie or not\n", pid);
		exit(EXIT_FAILURE);
	}else if(strncmp(pid_status, "Z (zombie)", 10) == 0){	//is zombie
		return 1;
	}else{	//not zombie
		return 0;
		//printf("not a zombie process\n");
	}
	return 0;
}

void getz(pid_t pid){	//function to check if pid is a zombie processe or no
	int result = checkzombie(pid);
	if(result == 1){
		printf("%d is defunct process\n", pid);
	}else{
		printf("%d is not defunct process\n", pid);
	}
}

void getzl(pid_t pid){	//function to print pid of all pid's child process that are zombie
	int n=0;
	int count = getDescCount(pid);	
	pid_t *desc_prc = getDescPrc(pid, count);	//get descendant processes
	int i, result;
	for(i=0; i<count; i++){
		if(getParentID(desc_prc[i]) == pid){	//check if a child process
			result = checkzombie(desc_prc[i]);	//check if a zombie process
			if(result == 1){
				printf("Child %d of %d process is defunct\n", pid, desc_prc[i]);	//print
				n++;
			}
		}
	}
	if(n==0)
		printf("None of %d's child process is defunct\n", pid);
}

int main(int argc, char *argv[]){
	int temp;
	pid_t ppid;
	
	if(argc<3 || argc>4){
		perror("ERROR: incorrect parameter\n");
		exit(EXIT_FAILURE);
	}
	pid_t root_pid = atoi(argv[1]);
	pid_t pid = atoi(argv[2]);
	
	checkifdescofroot(root_pid, pid);	//check if pid is descendant of root_pid
	
	//printf("root process : %d\n", root_pid);
	printf("Process ID : %d\n", pid);
	
	ppid = getParentID(pid);	//get parent pid
	printf("Parent id: %d\n", ppid);

	if(argc == 3){
		exit(EXIT_SUCCESS);
	}
	
	if(strcmp(argv[3], "-c") == 0){
		printf("Child Processes: ");
		getchildprc(pid);	// for option -c
		printf("\n");
	}else if(strcmp(argv[3], "-s") == 0){
		printf("Sibling Processes: ");
		getsibprc(root_pid, pid);	//for option -s
		printf("\n");
	}else if(strcmp(argv[3], "-gp") == 0){
		printf("Grand parent Process: ");
		getgpprc(pid);	//for option -gp
		printf("\n");
	}else if(strcmp(argv[3], "-gc") == 0){
		printf("Grand child Processes: ");
		getgcprc(pid);	//for option -gc
		printf("\n");
	}else if(strcmp(argv[3], "-z") == 0){
		getz(pid);	//for option -z
	}else if(strcmp(argv[3], "-zl") == 0){
		getzl(pid);	//for option -zl
	}else{
		printf("Error: Incorrect option %s\n", argv[3]);
	}
	
	return 0;
}
