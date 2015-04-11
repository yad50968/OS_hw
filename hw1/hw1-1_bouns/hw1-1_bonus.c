#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<errno.h>
#include<signal.h>
#include<string.h>
int count;
int storepid[6];
int parentpid;
int main(int argc,char *argv[]){
	count = argc;
	parentpid = getpid();
	do_process(0,argv,storepid);
	return 0 ;
}
int do_process(int n, char *argv[],int v[5]){

	if(n == count-1){
		v[n] = getpid();
		int i;
		printf("The realtionship is \n");
		for(i=1;i<count-1;i++)
			printf(" %d -> ",storepid[i]);
		printf("%d\n\n\n",storepid[i]);

	}
	else{
		v[n] = getpid();
		pid_t pid = fork();
	
		if(pid<0){
			printf("fork error\n");
		}
		else if(pid == 0){
			do_process(n+1 , argv, storepid);
			printf("process fork!!\n");
			printf("Child process executes This program!\n");
		    printf("Parent PID is %d\n",getpid());
			execl(argv[n+1],NULL);
			exit(1);	
		
		}
		else if(pid > 0){
			getsignal(pid);
		}
	}
}
	

void getsignal(int pid){
	int status;
	waitpid(pid,&status,0);
	printf("Receive a SIGCHLD signal\n\n");
	if(WIFEXITED(status)){
		printf("Normal termination with exit status = %d \n\n\n",WEXITSTATUS(status));
	}
	else if(WIFSIGNALED(status)){
		printf("------ERROR DETECTED-------\n");
		int l = WTERMSIG(status);
		switch(l){

			case SIGHUP: 
				printf("CHILD PROCESS Hangup detected on controlling termainal\nor death of controlling process");
				break;
			case SIGINT: 
				printf("CHILD PROCESS Interrupt from keyboard\n");
				break;
			case SIGQUIT: 
				printf("CHILD PROCESS Quit from keyboard\n");
				break;
			case SIGILL: 
				printf("CHILD PROCESS Illegal Instruction\n");
				break;
			case SIGABRT: 
				printf("CHILD PROCESS Abort signal from abort(3)\n");
				break;
			case SIGFPE: 
				printf("CHILD PROCESS Floating point exception)\n");
				break;
			case SIGKILL: 
				printf("CHILD PROCESS Kill signal\n");
				break;
			case SIGSEGV: 
				printf("CHILD PROCESS Invaild memory reference\n");
				break;
			case SIGPIPE: 
				printf("CHILD PROCESS Broken pipe: write to pipe with no readers\n");
				break;
			case SIGALRM: 
				printf("CHILD PROCESS Timer signal from alarm(2)\n");
				break;
			case SIGTERM: 
				printf("CHILD PROCESS Termination signal\n");
				break;
			case SIGUSR1:
				printf("CHILD PROCESS User-defined signal1\n");
				break;
			case SIGUSR2:
				printf("CHILD PROCESS User-defined signal2\n");
				break;
			case SIGCHLD:
				printf("CHILD PROCESS Child stopped or terminated\n");
			case SIGCONT:
				printf("CHILD PROCESS Continue if stopped\n");
				break;
			case SIGSTOP:
				printf("CHILD PROCESS Stop process\n");
				break;
			case SIGTSTP:
				printf("CHILD PROCESS Stop typed at terminal\n");
				break;
			case SIGTTIN:
				printf("CHILD PROCESS Terminal input for background process\n");
				break;
			case SIGTTOU:
				printf("CHILD PROCESS Terminal output for background process\n");
				break;


		}
		if(WCOREDUMP(status))
			printf("CHILD PROCESS FAILED\n");
	}

	else if(WIFSTOPPED(status)){
		printf("CHILD PROCESS was stopped by signal %d\n",WSTOPSIG(status));


	}	
	else 
		printf("Unexpected signal condition\n");
}


