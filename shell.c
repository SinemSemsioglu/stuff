#include "shell.h"

char *strsep(char** stringp, const char *delim);

int main(int argc, char *argv[]){
	FILE* input =stdin;
	stdn = dup(0);
	stdo = dup(1);

	line = alloc(1024*sizeof(char));
	int exit = 0;
	int ch;
	
	buffer = alloc(100*sizeof(char));
	shid = getpid();
//	printf("shell id is %d\n", shid);
	
	printf("shell:\t");
	
	signal_check();
	
	while(fgets(line, 1024, input) && !exit){
		
		init_arrays();
		int argc = get_command(args, 0);
		
		//checks if the command is exit
		if (strcmp(args[0], "exit")==0){
				//printf("you have to exit");
				exit = 1;
				break;
		}
		
		//checks for signals, sends them to handler
		signal_check();
	
		//lets fork the process
		id = fork();
		//there is no id :/
		if ( -1 == id ) {
			perror("fork failed, I am sad");
			return(2);
		}
		
		if (id) {
			//parent
			//printf("I am the parent with PID %d and child %d\n", getpid(), id);
			
			if(strcmp(args[argc-1],"&")==0){
			//	printf("here?\n");
				if (signal(SIGCHLD,sig_handler) == SIG_ERR){
					perror("SIGCHLD: "); 
				}	
			}else{			
				deadchild = waitpid(id,&status,0);
			//	printf("Child %d terminated with status %d\n", deadchild,
			//		 WIFEXITED(status)?WEXITSTATUS(status):-1);
			}	
		
		} else {
			// child
			//printf("I am the child with PID %d and parent %d\n", getpid(), getppid());
			command[0] = '\0';
		//	strcat(command, "/bin/");
			strcat(command, args[0]);
			if(strcmp(args[argc-1],"&")==0){
				args[argc-1] =NULL;
			}	
			execvp(command, args);
			
			perror("Exec failed");
			return(2);
		
		}
		set_io("r", "r");
		//free(tofree);
		line = alloc(1024*sizeof(char));
		printf("shell:\t");
	}
	freeall();
	return 0;
}
