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

typedef enum{NONE=0,PARALLEL=1,SEQUENTIAL=2,REDIRECTION=3,SIMPLE=4,EXIT=-1} command_type;

int parseInput(char inputString[])
{
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
	// It will return the type of the command based on the delimiter (Simple Command , Sequential Commands, Parallel Commands , Redirection).
	
	//First check for 'exit' or ''

	command_type ret_val;

	if(strcmp("exit",inputString)==0) // When user uses exit command.
	{
		ret_val = EXIT;
	}
	else if(strlen(inputString)==0)
	{
		ret_val = NONE;
	}
	else
	{
	
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
			free(wstatus);
		}
	}

	//Free the memory allocated on heap
	for(int i=0;i<MAXLIST;i++)
		free(argv[i]);
}

void executeParallelCommands(char *input)
{
// This function will fork a new process to execute a command
	
	char *argv[MAXLIST];
	char *out_ptr1;
	char *in_ptr1=input;
	char *out_ptr2;
	
	for(int i=0;i<MAXLIST;i++)
		argv[i]=(char*)malloc(sizeof(char)*MAXLEN);
	
	int i=0;
	
	out_ptr1=strsep(&in_ptr1,"&&");
	
	char *in_ptr2;
	
	while(out_ptr1!=NULL)
	{
		i=0;
			
		in_ptr2=out_ptr1;
		out_ptr2=strsep(&in_ptr2," ");
			
			
		while(out_ptr2!=NULL)
		{	
			if(strcmp(out_ptr2,"")!=0)
			{
				strcpy(argv[i],out_ptr2);
				i+=1;
			}
			out_ptr2=strsep(&in_ptr2," ");
		}
			
			
		argv[i]=NULL;
			
		if(in_ptr1!=NULL)
		{
			in_ptr1++;
		}
		out_ptr1=strsep(&in_ptr1,"&&");
		
		if(strcmp(argv[0],"cd")==0)
		{
			if(argv[2]==NULL)
			{	
				chdir(argv[1]);
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
					
				execvp(argv[0],argv);
				printf("Shell: Incorrect command\n");
				exit(1);
				
			}
		}	

	}

	//After all the child processes are created we wait for all of them to die
	int *wstatus=(int*)malloc(sizeof(int));

	//Wait returns the pid of terminated child
	//the moment all child are terminated ,the condition becomes False 
	while(wait(wstatus)>0);

	// Cleanup the memory
	free(wstatus);

	for(int i=0;i<MAXLIST;i++)
		free(argv[i]);
		
	
}


void executeSequentialCommands(char *input)
{	
	// This function will run multiple commands in sequential order
		
		char *argv[MAXLIST];
	char *out_ptr1;
	char *in_ptr1=input;
	char *out_ptr2;
	
	for(int i=0;i<MAXLIST;i++)
		argv[i]=(char*)malloc(sizeof(char)*MAXLEN);
	
	int i=0;
	
	out_ptr1=strsep(&in_ptr1,"&&");
	
	char *in_ptr2;
	
	while(out_ptr1!=NULL)
	{
		i=0;
			
		in_ptr2=out_ptr1;
		out_ptr2=strsep(&in_ptr2," ");
			
			
		while(out_ptr2!=NULL)
		{	
			if(strcmp(out_ptr2,"")!=0)
			{
				strcpy(argv[i],out_ptr2);
				i+=1;
			}
			out_ptr2=strsep(&in_ptr2," ");
		}
			
			
		argv[i]=NULL;
			
		if(in_ptr1!=NULL)
		{
			in_ptr1++;
		}
		out_ptr1=strsep(&in_ptr1,"&&");
		
		if(strcmp(argv[0],"cd")==0)
		{
			if(argv[2]==NULL)
			{	
				chdir(argv[1]);
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
					
				execvp(argv[0],argv);
				printf("Shell: Incorrect command\n");
				exit(1);
				
			}
			else
			{
				// The parent waits for the first command to execute completely
				// Then executes next command
				int *wstatus=(int*)malloc(sizeof(int));
				wait(wstatus);
				free(wstatus);
			}
		}	

	}

	for(int i=0;i<MAXLIST;i++)
		free(argv[i]);
		

		
}

void executeCommandRedirection(char *input)
{
	// This function will run a single command with output redirected to an output file specificed by user
	char *out_ptr1;
	char *in_ptr1=input;
	char *in_ptr2;
	char *argv[20];
	
	for(int i=0;i<MAXLIST;i++)
		argv[i]=(char*)malloc(sizeof(char)*MAXLEN);
	
	
	out_ptr1=strsep(&in_ptr1,">");
	
	in_ptr2=input;

	char *out_ptr2;
	int i=0;
	
	// Extract the command and args from LHS
	out_ptr2=strsep(&in_ptr2," ");
	while(out_ptr2!=NULL)
	{		
		if(strcmp(out_ptr2,"")!=0)
		{				
			strcpy(argv[i],out_ptr2);
			i+=1;
		}
		out_ptr2=strsep(&in_ptr2," ");
	}
	argv[i]=NULL;
	
	// Fork a new process
	if(fork()==0)
	{
		signal(SIGINT,signalhandler_int);
		signal(SIGINT,signalhandler_tstp);
		
		// File descriptor for file to which the output should be redirected
		int redirect_fd=open(in_ptr1+1,O_CREAT | O_TRUNC | O_WRONLY ); 

		// Make the stdout file descriptor point to the file
		dup2(redirect_fd,STDOUT_FILENO);

		// Close the old file descriptor (just for safety)
		close(redirect_fd);
		
		//Execute the command
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
	
	// Signal Handlers for TERMINAl(IGNORE SIGNALS!!)
	signal(SIGINT,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);

	char currentWorkingDirectory[MAXLEN];
	
	size_t bufsize = MAXLEN;
    size_t characters;
    	
    char *inputString=(char*)malloc(sizeof(char)*MAXLEN);
    command_type type = NONE;

	while(1)	// This loop will keep your shell running until user exits.
	{
		// Print the prompt in format - currentWorkingDirectory$
		getcwd(currentWorkingDirectory,MAXLEN);
		printf("%s$",currentWorkingDirectory);
		
		// accept input with 'getline()'

		characters=getline(&inputString,&bufsize,stdin);
		
		// For the newline(ENTER)
		inputString[characters-1]='\0';
		
		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		// And return the type of the function
		type = parseInput(inputString); 
			
		if(type == EXIT) // When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}
		else if(type == NONE)
		{
			continue;
		}
		else
		{
			if(type == PARALLEL)
				executeParallelCommands(inputString);		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
			else if(type == SEQUENTIAL)
				executeSequentialCommands(inputString);	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
			else if(type == REDIRECTION)
				executeCommandRedirection(inputString);	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
			else if(type == SIMPLE)
				executeCommand(inputString);		// This function is invoked when user wants to run a single commands
		}			
	}
	
	return 0;
}
