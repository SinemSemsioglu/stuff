#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define READ_END 0
#define WRITE_END 1

pid_t zombiechild;
pid_t deadchild;
pid_t id;
int stdn;
int stdo;
int status;
int shid;
//int argc;
char* args[100];
char * args2[100];
char command[100];
char* line;
char* buffer;

char *strdup(const char *s);
char *strsep(char** stringp, const char *delim);
int get_command(char** args, int argc);
char* clear_space(char * arr);
void set_io(char * input, char * output);
void signal_check(void);
void *alloc( int size );
int kill(pid_t pid, int sig);
//Function to free everything that has been malloc'd, just in case.
void freeall();
//Initializes input and output files based on args, if invalid input quits and prints error message.
//void init_io( int argc, char *argv[]);


//External variables

struct ptrlst *alloced;	//Pointer to the head of ptrlst.
//FILE *input, *output;	//Input and output file pointers.
//struct ptrlst *children = alloc(sizeof(struct ptrlst));

void kill_children(pid_t, int);

void sig_handler(int sig) {
    switch (sig) {
	case SIGTSTP:
	    printf("But I'm not sleepy!\n");
	    break;
	case SIGINT:
		printf("trapped? id is %d\n", getpid());
		if(getppid()!=shid && getpid()!=shid){
			printf("how? shid is %d id is %d\n", shid, getpid());
			kill_children(id, sig);
			waitpid(0, &status, WUNTRACED);
		}else printf("shell: \t");
		break;
	case SIGCHLD:
	//	printf("background");
		zombiechild = waitpid(-1, &status, WNOHANG);
	//	printf("Bg_child %d terminated with status %d\n", deadchild,
	//				 WIFEXITED(status)?WEXITSTATUS(status):-1);
		break;
	default:
	    printf("shell:\t");	
    }
    signal(sig, sig_handler);
}

void kill_children(pid_t id, int sig){
	if(sig == SIGINT){
		printf("Parent is sending an interrupt signal to the child\n");
		kill(id, SIGINT);
		sleep(1);
	}
	if(sig == SIGQUIT){
		printf("Parent is sending a quit signal to the child\n");
		kill(id, SIGQUIT);
	}
}

struct node{
	pid_t id;
};

//Linked list used for keeping track of what is malloc'd.
struct ptrlst{
	void *data;	//Pointer to something that has been malloc'd.
	struct ptrlst *next;	//Pointer to next item is malloced.
};

struct node *create_node(){
	struct node *ptr;
    ptr = alloc( sizeof(struct node) );

    if (ptr) {
        ptr->id = 0;
    }

    return ptr;
}


//Functions (descriptions at top with headers)

void *alloc( int size ){
	void *dude = malloc(size);
	if( dude ) {
		struct ptrlst *thisdude = malloc(sizeof(struct ptrlst));
		if( thisdude ) {	//Add this item to the list of all malloced items, make thisdude the new head.
			thisdude->data = dude;
			if( alloced ) thisdude->next = alloced;
			alloced = thisdude;
			return dude;
		}
		else free(dude);
	}

	//If the program gets here one of the mallocs failed.
	fprintf(stderr, "\nError:  Could not malloc!  Exiting!\n");
	exit(0);
	return NULL;
}


void freeall(){
	while( alloced ){
		if( alloced->data ) free(alloced->data);
		struct ptrlst *jimmy = alloced->next;
		free(alloced);
		alloced = jimmy;
	}
}

//changes the input and output files as desired
void set_io(char *input, char* output){
	int in, out;
	//change input to the given file if exists, dups the previous in?
	if(input!=NULL){
		if(strcmp(input, "r")==0) in = stdn;
		else{
		//	printf("passed path is %s out is %s.\n", input, output);
		 	in = open(input, O_RDONLY);
		}
		if (-1 == in) {
     			perror("opening input file");
    	}
		if(close(0)==-1){
			perror("closing stdin");
			//exit(1);
		}
		if(dup2(in, 0)==-1){
			perror("duppin");
			//exit(1);
		}
	}
	if(output!=NULL){	
		if(strcmp(output, "r")==0) out = stdo;
		else out = open(output, O_WRONLY | O_CREAT , 0777);
		if (-1 == out) {
			perror("opening output file");
			//exit(1);
			//break;
    	}
		if(close(1)==-1){
		   perror("closing stout");
		  // exit(1);
		}
		if(dup2(out, 1)==-1){
			perror("dupdup");
			//exit(1);
		}
	}

}

void init_arrays(){
	//initialize/reinitialize the array
		for(int j=0; j<100; j++){
			args[j]=NULL;
			command[j]='\0';
			args2[j]=NULL;
		}
}

//a method to help handle signals
//to avoid them being handled by the real shell and crashing
void signal_check(){
	if (signal(SIGINT, sig_handler) == SIG_ERR)
			   perror("SIGINT: ");
	if (signal(SIGQUIT,sig_handler) == SIG_ERR)
			   perror("SIGQUIT: "); 
}

//a method to help format commands
char* clear_space(char * arr){
	char * curr = alloc(100*sizeof(char));
	curr = arr;
	int index = strlen(curr)-1;
	while(isspace(arr[index])){
		curr[index]='\0';
		index--;
	}
	return curr;
}

//does piping
int pipe_me(char* cmd1, char* cmd2, char** args2){
	pid_t pid;
    int fd[2];

    pipe(fd);
    int i;
    pid=fork();

    if (pid==0) {

//        printf("i'm the child used for pipe %s \n", cmd1);
        dup2(fd[WRITE_END], STDOUT_FILENO);
        close(fd[WRITE_END]);
        //execlp("ls", "ls", "-al", NULL);
        execvp(cmd1, args);
        perror("Exec failed");
		return(2);
		waitpid(id,&status,0);

   } else { 
    pid=fork();

    if (pid==0) {
   //     printf("i'm in the second child, which will be used to run %s\n", cmd2);
        dup2(fd[READ_END], STDIN_FILENO);
        close(fd[READ_END]);

		execvp(cmd2, args2);
		perror("Exec failed");
		return(2);
       //execlp("grep", "grep","alpha",NULL);
       waitpid(id,&status,0);
    }

    }

}

//gets commands up to 100 characters
//calls set_io to return to stdin and stdout
int get_command(char** arr, int argc){
	//	if(argc == 1) printf("here bitch\n");
		int check = argc;
		char *token, *tofree;
		//tofree = line;
		//int argc=0;	
	
		char * in = NULL;
		char * out = NULL;
		
		//get the commands
		while((token=strsep(&line," "))!=NULL){
//			printf("%s\n", token);
			if(strcmp(token, ">")==0 && (token = strsep(&line, " "))!=NULL){
				buffer = strcat(getcwd(buffer, 100),"/") ;
				buffer = strcat(buffer,token) ;
				out = strdup(buffer);
				out = clear_space(out);
			//	printf("out is initialized and is %s\n", out);
			
			}else if(strcmp(token, "<")==0 && (token = strsep(&line, " "))!=NULL){
				buffer = strcat(getcwd(buffer, 100),"/") ;
				buffer = strcat(buffer,token) ;
				in = strdup(buffer);
				in = clear_space(in);
			//	printf("in is initialized and is %s\n", in);
			}else if(strcmp(token, "|")==0 && (token = strsep(&line, " "))!=NULL){
			
				
				char cmd1[100];
				char cmd2[100];
				cmd1[0] = '\0';
				strcat(cmd1, "/bin/");
				strcat(cmd1, arr[0]);
				if(strcmp(arr[argc-1],"&")==0){
					arr[argc-1] =NULL;
				}
				
				
				int index = strlen(token)-1;
				while(isspace(token[index])){
					token[index]='\0';
					index--;
				}
				args2[0]=token;
				
//				printf("this is it %s\n", args2[0]);
				
				int count = get_command(args2,1);
				
				cmd2[0] = '\0';
				strcat(cmd2, "/bin/");
				strcat(cmd2, args2[0]);
				if(strcmp(args2[count-1],"&")==0){
					args2[count-1] =NULL;
				}
				//pipe_me(
//				printf("pipe here\n");
				pipe_me(cmd1, cmd2, args2);
				check = 1;
			
			}else{
				int index = strlen(token)-1;
				while(isspace(token[index])){
					token[index]='\0';
					index--;
				}
				arr[argc++]=token;
			}
		}
		
		//if(check == 0){
			set_io(in, out);
		//}		
		return argc;

}
