/********************************************************************************************
This is a template for assignment on writing a custom Shell. 

Students may change the return types and arguments of the functions given in this template,
but do not change the names of these functions.

Though use of any extra functions is not recommended, students may use new functions if they need to, 
but that should not make code unnecessorily complex to read.

Students should keep names of declared variable (and any new functions) self explanatory,
and add proper comments for every logical step.

Students need to be careful while forking a new process (no unnecessory process creations) 
or while inserting the single handler code (should be added at the correct places).

Finally, keep your filename as myshell.c, do not change this name (not even myshell.cpp, 
as you not need to use any features for this assignment that are supported by C++ but not by C).
*********************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()

#define MAXLEN 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

typedef enum{PARALLEL=1,SEQUENTIAL=2,REDIRECTION=3,SIMPLE=4,} command_type;

int parseInput(char inputString[])
{
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
	// It will return the type of the command based on the delimiter (Simple Command , Sequential Commands, Parallel Commands , Redirection).
	
	char inputString_copy[MAXLEN];
	
	//Make a copy of the input
	strcpy(inputString_copy,inputString);
	
	//Initialise the ptrs 

	//Points to the next part after delimiter
	char *in_ptr=inputString_copy;

	//Points to the substring
	char *out_ptr;
	
	out_ptr=strsep(&in_ptr,"&&");
	
	command_type ret_val;
	if(out_ptr!=NULL)
	{	
		//Not a empty input
		if(in_ptr==NULL)
		{
			// Does not have && as the delimiter
			in_ptr=input;
			out_ptr=strsep(&in_ptr,"##");
			
			if(in_ptr==NULL)
			{
				// Does not have ## as the delimiter
				in_ptr=input;
				out_ptr=strsep(&in_ptr,">");
				
				if(in_ptr==NULL)
				{
					// Does not have > or ## or && as the delimiter
					ret_val = SIMPLE;
				}
				else
				{
					//Has > as delimiter
					ret_val = REDIRECTION;
				}
			}
			else
			{
				//Has ## as delimiter
				ret_val = SEQUENTIAL;
			}
		}
		else
		{
			//Has && as the delimiter
			ret_val = PARALLEL;
		}
				
	}
	
	return ret_val;	
	
}

//Signal Handler for CTRL+Z (SIGTSTP) 
void signalhandler_tstp(int sig)
{
	exit(1);
}

//Signal Handler for CTRL+C(SIGINT) 
void signalhandler_int(int sig)
{
	exit(1);
}


void executeCommand(char *input)
{
	// This function will fork a new process to execute a command
	// Will seperate the commands and args as well
	
	char *argv[MAXLIST];
	char *out_ptr;
	char *in_ptr=input;
	
	// Dynamically allocate the memory for each argument
	for(int i=0;i<MAXLIST;i++)
		argv[i]=(char*)malloc(sizeof(char)*MAXLEN);
	
	int i=0;
	out_ptr=strsep(&in_ptr," ");
	
	// Go through the input and seperate the commands and arguments based on 'space'
	// as delimiter(SIMPLE COMMAND)

	while(out_ptr!=NULL)
	{
		strcpy(argv[i],out_ptr);
		out_ptr=strsep(&in_ptr," ");
		i+=1;
	}
	
	// NULL terminated argv
	argv[i]=NULL;

	//Check if it is a built in command like 'cd'
	if(strcmp(argv[0],"cd")==0)
	{
		// Check if it has only one argument
		if(argv[2]==NULL)
		{
			chdir(argv[1]);
		}
		else
			printf("Shell: Incorrect command\n");
	}
	else
	{
		// Fork a new process
		if(fork()==0)
		{
			//Signal handler for child process
			signal(SIGINT,signalhandler_int);
			signal(SIGINT,signalhandler_tstp);

			execvp(argv[0],argv);
			
			// Will execute only if execvp fails
			printf("Shell: Incorrect command\n");
			exit(1);
		}
		else
		{
			//Parent(Terminal) waits for the child to terminate
			int *wstatus=(int*)malloc(sizeof(int));
			wait(wstatus);
		}
	}
}

void executeParallelCommands(char *input)
{
// This function will fork a new process to execute a command
	
	
	char *argv[20][20];
	char *out_ptr;
	char *in_ptr=input;
	char *ptr;
	
	for(int i=0;i<20;i++)
		for(int j=0;j<20;j++)
			argv[j][i]=(char*)malloc(sizeof(char)*20);
	
	int i=0;
	
	
	out_ptr=strsep(&in_ptr,"&&");
	
	char *cptr;
	int j=0;
	while(out_ptr!=NULL)
		{
			i=0;
			
			
			cptr=out_ptr;
			ptr=strsep(&cptr," ");
			
			
			while(ptr!=NULL)
			{
				
				if(strcmp(ptr,"")!=0)
				{
					strcpy(argv[j][i],ptr);
					i+=1;
				}
				ptr=strsep(&cptr," ");
			}
			
			
			argv[j][i]=NULL;
			
			if(in_ptr!=NULL)
			{
				in_ptr++;
			}
			out_ptr=strsep(&in_ptr,"&&");
			
			
			j+=1;
			
			
		}
		
		
		int flag=0;
		for(int i=0;i<j;i++)
		{
		//	printf("%s\n",argv[i][0]);
			if(strcmp(argv[i][0],"cd")==0)
			{
				if(argv[0][2]==NULL)
				{
					
					size_t s=100;
					char *path=(char*)malloc(sizeof(char)*20);
					
					strcat(path,argv[i][1]);
					chdir(path);
				}
				else
					printf("Shell: Incorrect command\n");
			}
			else
			{
					if(fork()==0)
					{	
						
						signal(SIGINT,signalhandler_int);
						signal(SIGINT,signalhandler_tstp);
						execvp(argv[i][0],argv[i]);
						printf("Shell: Incorrect command\n");
						exit(1);
					
					}
			}	
		}
		int *wstatus=(int*)malloc(sizeof(int));
		wait(wstatus);
		
		
	
}

void executeSequentialCommands(char *input)
{	
	// This function will run multiple commands in sequential order
		
	
	char *argv[20][20];
	char *out_ptr;
	char *in_ptr=input;
	char *ptr;
	
	for(int i=0;i<20;i++)
		for(int j=0;j<20;j++)
			argv[j][i]=(char*)malloc(sizeof(char)*200);
	
	int i=0;
	
	
	out_ptr=strsep(&in_ptr,"##");
	
	char *cptr;
	int j=0;
	while(out_ptr!=NULL)
		{
			i=0;
			
			
			cptr=out_ptr;
			ptr=strsep(&cptr," ");
			
			
			while(ptr!=NULL)
			{
				//printf("%s",ptr);
				if(strcmp(ptr,"")!=0)
				{				
					strcpy(argv[j][i],ptr);
					i+=1;
				}
				ptr=strsep(&cptr," ");
				
			}
			
			
			argv[j][i]=NULL;
			
			
			if(in_ptr!=NULL)
			{
				in_ptr++;
			}
			out_ptr=strsep(&in_ptr,"##");
			
			//Start
			if(strcmp(argv[j][0],"cd")==0)
			{
				if(argv[j][2]==NULL)
				{
					
					
					//char *path=(char*)malloc(sizeof(char)*200);
					
					//strcat(path,argv[j][1]);
					chdir(argv[j][1]);
				}
				else
				{
					printf("Shell: Incorrect command\n");
				}
			}
			else
			{
				if(fork()==0)
				{
					
					signal(SIGINT,signalhandler_int);
					signal(SIGINT,signalhandler_tstp);
					
					execvp(argv[j][0],argv[j]);
					printf("Shell: Incorrect command\n");
					exit(1);
					
				}
				else
				{
					int *wstatus=(int*)malloc(sizeof(int));
					wait(wstatus);
					free(wstatus);
		
				}
			}//end
			
			j+=1;
			
			
		}
		
		/*
		int flag=0;
		for(int i=0;i<j;i++)
		{
			
			if(strcmp(argv[i][0],"cd")==0)
			{
				if(argv[i][2]==NULL)
				{
					
					size_t s=100;
					char *path=(char*)malloc(sizeof(char)*20);
					
					printf("%s %s--\n",argv[i][0],argv[i][1]);
					//strcat(path,argv[i][1]);
					//chdir(path);
				}
				else
				{
					printf("Shell: Incorrect command\n");
				}
			}
			else
			{
				printf("%s",argv[i][0]);
				int k=1;
				while(argv[i][k]!=NULL)
				{
					printf("%s-",argv[i][k]);
					k+=1;
				}
				//if(fork()==0)
				//{
					
					//signal(SIGINT,signalhandler_int);
					//signal(SIGINT,signalhandler_tstp);
					
					//execvp(argv[i][0],argv[i]);
					//printf("Shell: Incorrect command\n");
					//exit(1);
					
				//}
				//else
				//{
				//	int *wstatus=(int*)malloc(sizeof(int));
				//	wait(wstatus);
				//	free(wstatus);
		
				//}
			}
		}
	*/
}

void executeCommandRedirection(char *input)
{
	// This function will run a single command with output redirected to an output file specificed by user
	char *out_ptr;
	char *in_ptr=input;
	char *cptr;
	char *argv[20];
	
	for(int i=0;i<20;i++)
		argv[i]=(char*)malloc(sizeof(char)*200);
	
	
	out_ptr=strsep(&in_ptr,">");
	
	cptr=input;
	char *ptr;
	int i=0;
	
	ptr=strsep(&cptr," ");
	while(ptr!=NULL)
			{
				//printf("%s",ptr);
				if(strcmp(ptr,"")!=0)
				{				
					strcpy(argv[i],ptr);
					i+=1;
				}
				ptr=strsep(&cptr," ");
				
			}
	argv[i]=NULL;
	
	//printf("%s",in_ptr+1);
	if(fork()==0)
	{
		signal(SIGINT,signalhandler_int);
		signal(SIGINT,signalhandler_tstp);
		
		
		
		int redirect_fd=open(in_ptr+1,O_CREAT | O_TRUNC | O_WRONLY ); 
		dup2(redirect_fd,STDOUT_FILENO);
		close(redirect_fd);
		
		
		execvp(argv[0],argv);
		
		printf("Shell: Incorrect command\n");
		exit(1);
	}
	else
	{
		int *wstatus=(int*)malloc(sizeof(int));
		wait(wstatus);
		
	}
	
}

int main()
{
	// Initial declarations
	
	signal(SIGINT,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	
	size_t s=100;
	char currentWorkingDirectory[200];
	
	size_t bufsize = 200;
    size_t characters;
    	
    	char *inputString=(char*)malloc(sizeof(char)*200);
    	int execFlag = 0;

	while(1)	// This loop will keep your shell running until user exits.
	{
		// Print the prompt in format - currentWorkingDirectory$
		getcwd(currentWorkingDirectory,s);
		printf("%s$",currentWorkingDirectory);
		
		// accept input with 'getline()'

		characters=getline(&inputString,&bufsize,stdin);
		
		inputString[characters-1]='\0';
		
		
		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		execFlag=parseInput(inputString); 
			
		
		
		
		if(strcmp("exit",inputString)==0) // When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}
		else if(strlen(inputString)==0)
		{
			continue;
		}
		else
		{
		
		
			if(execFlag==1)
				executeParallelCommands(inputString);		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
			else if(execFlag==2)
				executeSequentialCommands(inputString);	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
			else if(execFlag==3)
				executeCommandRedirection(inputString);	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
			else
				executeCommand(inputString);		// This function is invoked when user wants to run a single commands
		}
		
		
				
	}
	
	return 0;
}
