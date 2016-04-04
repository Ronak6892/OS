/******************************************************************************
 *
 *  File Name........: main.c
 *
 *  Description......: Simple driver program for ush's parser
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "parse.h"

int prev_out_fp=-1;
int prev_err_fp=-1;
int pipe_fd[2];
int pipe2_fd[2];
//int pipe_err_fd[2];

int prev_pipe=0;
int pcnt=0,f=0;
int is_err=0,is_main=0,pipe_abort=0;

char * host;

extern char **environ;

void handle_sig_int(int sig_num)
{
  printf("\r\n"); 
  	//printf("init signum==%d",sig_num);
  if(f==1)
  {
  
  		printf("%s%% ",host);
  }
  fflush(STDIN_FILENO);
}
void handle_sig_term(int sig_num)
{
	//printf("term signum==%d",sig_num);
    killpg(getpgrp(),SIGTERM);
    exit(0);
}

void handle_sig_quit(int sig_num)
{
	//printf("quit signum==%d",sig_num);
	printf("\r\n");
	if(f==1)
	{
		printf("%s%% ",host);
	}
	fflush(STDIN_FILENO);
	//signal(sig_num,SIG_IGN);
}

static void prCmd(Cmd c)
{
  int i;
	//printf("prCmd for cmd = %s\n", c->args[0]);
	
  if ( c ) {
    //printf("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);
    if ( c->in == Tin )
    {
     // printf("<(%s) ", c->infile);
    }
     
     
    if ( c->out != Tnil )
    {
  	
      switch ( c->out ) {
      case Tout:
      {
			//printf(">(%s) ", c->outfile);

			break;
		}
      case Tapp:
      {
			//printf(">>(%s) ", c->outfile);

			break;
		}
      case ToutErr:
      {
			//printf(">&(%s) ", c->outfile);

			break;
		}
      case TappErr:
      {
			//printf(">>&(%s) ", c->outfile);

			break;
		}
      case Tpipe:
      {
			//printf("| ");
			//tout_fp=open(c->outfile,O_WRONLY|O_CREAT);
			//prev_pipe=1;
			break;
		}
      case TpipeErr:
      {
			//printf("|& ");
			//tout_fp=open(c->outfile,O_WRONLY|O_CREAT);
			//prev_pipe=2;
			break;
		}
      default:
      {
			//printf("Shouldn't get here\n");
			exit(-1); 
		}
      }
      
   }

    if ( c->nargs > 1 ) {
     // printf("[");
      for ( i = 1; c->args[i] != NULL; i++ )
	;//printf("%d:%s,", i, c->args[i]);
     // printf("\b]");
    }
   // putchar('\n');
    // this driver understands one command
    if ( !strcmp(c->args[0], "end") )
      exit(0);
  }
  
  //printf("prCmd : EXIT : for cmd = %s\n", c->args[0]);
}

static void update_io_redirect(Cmd c)
{

	if(c==NULL)
	 return;
	 
	//printf("update_to redirect.... for cmd = %s\n", c->args[0]);
	
		
	if ( c ) {
   // printf("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);
   
   
    if ( c->in == Tin )
    {
    	//printf("update_to redirect: for INPUT....cmd = %s \n", c->args[0]);
    	//if(prev_pipe == 0)
    //	{
      	//printf("no previous pipe....cmd = %s \n", c->args[0]);
      
     	 	// open the input file...
     		 int tin_fp=-1;
     		 tin_fp=open(c->infile,O_RDONLY);
     		 if(tin_fp < 0 && is_main!=0)
     		 {
     		 	fprintf(stderr,"%s: Tin No such file or directory.\n",c->infile);
     		 	//is_err=-1;
     		 	//return;
     		  	exit(0);
     		 	
     		 }
		
				// dup2 to replace the std in
		
				if(dup2(tin_fp,STDIN_FILENO)==-1)
				{
					//printf("dup2 error for %s...\n",c->infile);
					//exit(1);
				}
		
				close(tin_fp);
		// }		
	  } 
    
       
     
     
    if ( c->out != Tnil )
    {
    	int tout_fp=-1;
    	//printf("update_to redirect: for OUTPUT....cmd = %s \n", c->args[0]);
    	
      switch ( c->out ) {
      case Tout:
      {
			//printf("output to >(%s) \n", c->outfile);
			tout_fp=open(c->outfile,O_WRONLY|O_CREAT|O_TRUNC,0660);
			
			if(tout_fp < 0)
      	{
      		printf("%s: No such file or directory.\n",c->outfile);
      		//is_err=-1;
     		 	return;
     		}
     		
     		if(dup2(tout_fp,STDOUT_FILENO)==-1)
			{
				//printf("dup2 error for %s...\n",c->outfile);
				//exit(-1);
			}
     		
     		close(tout_fp);
     		
			break;
		}
      case Tapp:
      {
			//printf("output to >>(%s) \n", c->outfile);
			tout_fp=open(c->outfile,O_WRONLY|O_CREAT|O_APPEND,0660);
			if(tout_fp < 0)
      	{
      		printf("%s: No such file or directory.\n",c->outfile);
      		//is_err=-1;
     		}
     		
     		if(dup2(tout_fp,STDOUT_FILENO)==-1)
			{
				//printf("dup2 error for %s...\n",c->outfile);
				//exit(-1);	
			}
     		
     		close(tout_fp);
     
			break;
		}
      case ToutErr:
      {
			//printf("output to >&(%s) \n", c->outfile);
			tout_fp=open(c->outfile,O_WRONLY|O_CREAT|O_TRUNC,0660);
			if(tout_fp < 0)
      	{
      		printf("%s: No such file or directory.\n",c->outfile);
      		//is_err=-1;
     		}
     		
			if(dup2(tout_fp,STDERR_FILENO)==-1)
			{
				//printf("dup2 error for %s...\n",c->outfile);
				//exit(-1);	
			}

     		if(dup2(tout_fp,STDOUT_FILENO)==-1)
			{
				//printf("dup2 error for %s...\n",c->outfile);
				//exit(-1);	
			}
     		
     		close(tout_fp);
     		
			break;
		}
      case TappErr:
      {
			//printf("output to >>&(%s) \n", c->outfile);
			tout_fp=open(c->outfile,O_WRONLY|O_CREAT|O_APPEND,0660);
			if(tout_fp < 0)
      	{
      		printf("%s: No such file or directory.\n",c->outfile);
      		//is_err=-1;
     		 	//return;  
   if ( !strcmp(c->args[0], "end") )
     exit(0);
     		}
     		
			if(dup2(tout_fp,STDERR_FILENO)==-1)
			{
				//printf("dup2 error for %s...\n",c->outfile);
				//exit(-1);	
			}
			
     		if(dup2(tout_fp,STDOUT_FILENO)==-1)
			{
				//printf("dup2 error for %s...\n",c->outfile);
				//exit(-1);	
			}
     		
     		close(tout_fp);

			break;
		}
      case Tpipe:
      {
			//printf("output to pipe |  ::: \n");

			//printf(" EXIT : output io pipe |  ::: \n");
			break;
		}
      case TpipeErr:
      {
			//printf("output to |& :: \n ");

			break;
		}
      default:
      {
			//printf("Shouldn't get here\n");
			exit(-1); 
		}
      }


    }
 
	}
	
	//printf(" ***************** Rdirect exit::::  for cmd = %s \n",c->args[0]);
}


static int execute_built_in(Cmd c)
{
	if(c==NULL)
	 return 1;
	
		//printf(" execute_built_in... for cmd = %s\n", c->args[0]);
		
		// check for built-in commands... 
		// if built-in cmd, no need to fork() ...
		
		// check for cd echo logout nice pwd setenv unsetenv where
		
		// cd..
		// hange the working directory of the shell to dir, provided it is a directory and the shell has the appropriate permissions. 
		// Without an argument, it changes the working directory to the original (home) directory.
		
		
	 if(strcmp(c->args[0],"cd")==0)
		 {
		 	//printf(" execute_cmd: cd command ...\n");
		 	
			if(c->nargs > 1)
			{
				//move cd to arg[1] ....
				//printf("moving cd to passed argument ::: %s \n",c->args[1]);
				if(chdir(c->args[1]) !=0 )
				{
					//printf("cd fails for %s...\n",c->args[1]);	
					fprintf(stderr,"%s: No such file or directory.\n",c->args[1]);
      			//is_err=-1;
     		 		return;
				}
			
			}	else 
			{
				// no argument to cd command...
				// cd to original (home) directory...
				// todo : set home , fprintf std err handling...
				
				//printf("CD with no args ... \n");
				
				if(chdir(getenv("HOME")) !=0 )
				{
					fprintf(stderr,"%s: No such file or directory.\n",c->args[1]);
      			//is_err=-1;
     		 		return;
				}
				
			}
						 	
		 	return 1;
		 } else if (strcmp(c->args[0],"echo")==0)
		 {
		 		
		 	// echo 
		 	
		 	// print all args to std-out file....
		 	// todo  set std-out
		 	
		 	int i=1;  
   if ( !strcmp(c->args[0], "end") )
     exit(0);
		 	
		 	while(i < c->nargs)
		 	{
		 		printf("%s ",c->args[i]);
		 		i++;
		 		
		 	}
		 	printf("\n");
		 	return 1;
		 	
		 } else if (strcmp(c->args[0],"logout")==0)
		 {
		 	//logout
		 	//todo : handle pipe logout
		 	//printf("logout pressed...");
		 	//is_logout=1;
		 	exit(0);
		 	
		 	return 1;
		 	
		 } else if (strcmp(c->args[0],"pwd")==0)
		 {
			// pwd
			// print the current location...
			//printf(" execute_cmd: pwd command ...\n");
			
			//char *buf=(char *)malloc(sizeof(char)*1024);
			char cwd[1024];
			
			if(getcwd(cwd,sizeof(cwd)) != NULL)
			{
					printf("%s\n",cwd);
			} 
					
			return 1;	
		 } else if (strcmp(c->args[0],"nice")==0)
		 {
		 	//nice
		 //	printf(" execute_cmd: nice command ...\n");	
		 	
		 	if(c->nargs ==1)
		 	{
		 		//printf(" execute_cmd: nice command with 1 argument...\n");	
		 		setpriority(PRIO_PROCESS,getpid(),4);
		 		
		 	} else if(c->nargs >= 2)
		 	{
		 		//printf(" execute_cmd: nice command with 2 argument...\n");	
		 		char ch=(c->args[1][strlen(c->args[1])-1]);
		 		int prio=4;	 		
		 		
		 		if(!isdigit(ch))
		 		{
		 			// second argument is cmd, so execute it with priority 4
		 			//printf(" execute_cmd: nice command with 2nd  argument cmd...\n");
		 		   
		 			int status_ch;
		 			int ch_id;
		 		
		 			ch_id=fork();
		 			
		 			if(ch_id < 0)
		 			{
		 				
		 			}		 		
		 			else if(ch_id==0)
		 			{
		 				
		 				setpriority(PRIO_PROCESS,getpid(),prio);
		 			
		 				execvp(c->args[1],&c->args[1]);
							 		
		 			} else 
		 			{
		 				while(wait(&status_ch)!=ch_id);
		 				// error

		 			}
		 			
		 		}
		 		else 
		 		{
		 			//second argument is number, so set main process priority
		 			//printf(" execute_cmd: nice command with 2nd argument num...\n");	
		 			prio=atoi(c->args[1]);
		 			
		 			if(c->nargs==2)
		 			{
		 				setpriority(PRIO_PROCESS,getpid(),prio);
	 				} 
	 				else 
	 				{
	 					//printf(" execute_cmd: nice command with 2nd num + 3rd cmd argument...\n");	
		 				int status_ch;
		 				int ch_id;
		 		
		 				ch_id=fork();
		 				
		 				if(ch_id < 0)
		 				{
		 					//perror("Fork failed...\n");
		 					//exit(0);
		 				}		 		
		 				else if(ch_id==0)
		 				{
				
		 						setpriority(PRIO_PROCESS,getpid(),prio);
		 			
		 						execvp(c->args[2],&c->args[2]);
		 				} 
		 				else 
		 				{
		 					while(wait(&status_ch)!=ch_id);
		 					//todo : errorno
		 				}
	 					
	 				}
		 		}
		 		
		 	}

		 	return 1;
		 } else if (strcmp(c->args[0],"where")==0)
		 {
		 	//where
		//	printf(" execute_cmd: where command ...\n");		 	
		 	
		 	
		 	if(c->nargs == 1)
		 	{
		 		perror("where: Too few arguments.\n");
		 		//exit(0);
		 	}
		 	
 			int i=1;
		 	
		 	
		 			
 			for(i=1;i<(c->nargs);i++)
			{
				if(strcmp(c->args[i],"cd")==0 || strcmp(c->args[i],"echo")==0 || strcmp(c->args[i],"logout")==0 ||
 				strcmp(c->args[i],"nice")==0 || strcmp(c->args[i],"where")==0 || strcmp(c->args[i],"setenv")==0 || strcmp(c->args[i],"unsetenv")==0 ||
 				strcmp(c->args[i],"bg")==0 || strcmp(c->args[i],"fg")==0 || strcmp(c->args[i],"kill")==0 || strcmp(c->args[i],"jobs")==0 )
 				{
		 				
 					printf("%s is a shell built-in \n",c->args[i]);
 				}
		 				
		 		//char **paths;
		 		char * path_var;
		 		
		 		
		 		path_var=(char *)malloc(sizeof(char)*strlen(getenv("PATH")));
		 		
		 		strcpy(path_var,getenv("PATH"));
		 		
		 		/*int num_path=1;
		 		
		 		char *tmp;
		 		tmp=path_var;
		 		
		 		while(strchr(tmp,":")!=NULL)
				{
					num_path++;
					tmp=strchr(tmp,":");
					
				}
				
				paths=(char *)malloc(sizeof(char *)*num_path);
				*/
				
				
				char *temp_path;
				char *mid_path;
				
				int j;
				int prev=0;
				int slen=0;
				//int ind_p=0;				
				int c_len;
				int c_fp;
				
				c_len = strlen(c->args[i]);
				
				//printf(" path variable length = %d  \n", strlen(path_var));
				
				for(j=0;j<(strlen(path_var));j++)
				{
					
					if(path_var[j]==':')
					{
						//printf("path found...at j= %d ,cmd = %s \n",j,c->args[i]);
						slen=j-prev;
						int ll=c_len+slen+1;
						temp_path=(char *)malloc(sizeof(char)*(ll));
						//char tp[ll];
						//strcpy(tp,"");
						
						//printf("malloc done , space = %d\n",ll);
						
						mid_path=&path_var[prev];
						
						temp_path=strndup(mid_path,slen);
						//strncpy(tp,mid_path,slen);
						//strcat(tp,"/");
						//strcat(tp,c->args[i]);
						
						//printf("strncpy done\n");
						
						strcat(temp_path,"/");
						strcat(temp_path,c->args[i]);
						
						//printf("tmp path done : path is ######### : %s\n",temp_path);
						
						
						c_fp=open(temp_path,O_RDONLY,0600);
						
						if(c_fp>0)
						{
							
							printf("%s\n",temp_path);	
						}
						close(c_fp);
						//char * ttmp=temp_path;
						
						//printf("");
						//printf("tmp path size : allocated = %d , actual is= : %d\n",(c_len+slen+1),strlen(temp_path));
						//free(temp_path);
						mid_path=NULL;
						prev=j+1;
						
					}					
					
				}
				
				//printf("for loop finished...\n");
				//last path will be remained...
			//	printf("Last path found...for cmd = %s \n",c->args[i]);
				
				slen=j-prev;
				temp_path=(char *)malloc(sizeof(char)*(c_len+slen+1));
				//printf("malloc done\n");
				
				mid_path=&path_var[prev];
				//printf("strncpy done\n");
				
				temp_path=strndup(mid_path,slen);
				strcat(temp_path,"/");
				strcat(temp_path,c->args[i]);
					
				//printf("tmp path done : path is ######### : %s\n",temp_path);
						
				c_fp=open(temp_path,O_RDONLY,0600);
				
				//printf("tmp path size : allocated = %d , actual is= : %d\n",(c_len+slen+1),strlen(temp_path));
							
				if(c_fp>0)
				{
					printf("%s\n",temp_path);	
				}
				close(c_fp);

				//free(temp_path);
		 		
		 	}
		 	
			return 1;
		 } else if (strcmp(c->args[0],"setenv")==0)
		 {
		 	//setenv
		 	//printf(" execute_cmd: setenv command ...\n");
			 	
		 	if(c->nargs == 1)
		 	{
		 		// setenv without arguments .... print all VARs with their values
		 		int i;
		 		
		 		for(i=0;environ[i]!=NULL;i++)
		 		{
		 			printf("%s\n",environ[i]);
		 		}
		 		
		 	} else if (c->nargs > 1)
			{		 	
		 	 // printf("setenv with 1 or 2 arguments...");
		 		int len=1;
		 		
		 		len+=strlen(c->args[1]);
		 		
		 		if(c->args[2]!=NULL)
		 		  len+=strlen(c->args[2]);
		 		  
		 		char *str=(char *)malloc(sizeof(char)*len);
		 		
		 		strcpy(str,c->args[1]);
		 		strcat(str,"=");
		 		
		 		if(c->nargs > 2)
					strcat(str,c->args[2]);
				else 
					strcat(str,"");
				putenv(str);		 		
		 	
		   } 
		 	
		 	return 1;
		 } else if (strcmp(c->args[0],"unsetenv")==0)
		 {
		 	//unsetenv
		 	//printf("unsetenv cmd ... \n");
		 	
		 	if(c->nargs ==2)
		 	{

				//printf("unsetenv with 1 or 2 arguments...\n");
				
		 		int i=0,j=0;
		 		int ind=0;
		 		
				int var_len=strlen(c->args[1]);		 		
		 		
		 		for(i=0;environ[i]!=NULL;i++)
		 		{
		 				if(!strncmp(environ[i],c->args[1],var_len) && environ[i][var_len]=='=')
						{
							//match found for var passed in argument.. need to remove it...
							ind=i;
							//printf("VAR found at ind=%d \n",ind);
							for(j=ind;environ[j]!=NULL;j++)
							{
								// shift environ variable by one index above...
								environ[j]=environ[j+1];
							}
							
							break;
				
						}		 				
		 		}
				
				if(i>ind)
				{
					//printf("VAR no found... \n");
					
				}	 	
		 	}
		 	
		 	return 1;
		 } else {
		 	
		 	return 0;
		 }
	
		return 0;
}
	
	
static void execute_cmd(Cmd c)
{
	if(c==NULL)
	 return;
	 
	 int is_built_in=0;
	// printf("execute cmd in pipe.... cmd = %s\n", c->args[0]);
	 
	 // update_io_redirect(c);
	 
	 if(c->next == NULL)
	 {
		// Last cmd in pipe....
		// printf("Last cmd in pipe.... cmd = %s\n", c->args[0]);
		
		  int old_stdout=dup(STDOUT_FILENO);
  		  int old_stdin=dup(STDIN_FILENO);
		  int old_stderr=dup(STDERR_FILENO);
  			
  			is_main=0;
		  update_io_redirect(c);
  			
  			is_main=1;
  			
  			/*if(is_err==-1)
  			{
  				//printf("is_err done \n");
  				return;
  			}*/
  			
		is_built_in=execute_built_in(c);
			
		if(is_built_in == 0)
		{
	
		 	// not a built in command...
		 //	printf("execute_cmd : Not a build command : Need to fork... cmd = %s\n", c->args[0]);
		 	// need to fork...
		 	int ch_id;
		 	int status;
		 	
		 	
		 	dup2(old_stdout,STDOUT_FILENO);
  			dup2(old_stdin,STDIN_FILENO);
		  	dup2(old_stderr,STDERR_FILENO);


		 	ch_id=fork();
		 	if(ch_id < 0)
		 	{
		 		//printf("error in fork for %s \n");	
		 	//	exit(1);
		 	} 
		 	else if(ch_id ==0)
		 	{
		 		// child process
		 		//printf("Child process.... \n");
		 		
		 		setpgid(0,getpgrp());
		 		
		 		update_io_redirect(c);
		 		
		 	/*	if(is_err==-1)
	  			{
  					//printf("is_err done \n");
  					return;
  				}*/
		 		
		 		//printf("IO REdirect done for cmd = %s \n",c->args[0]);
		 		
		 		if(execvp(c->args[0],c->args) == -1)	
		 		{
		 			
		 			fprintf(stderr,"%s: %s\n",c->args[0],strerror(errno));	
		 			//todo
		 			
		 			exit(-1);
		 		}
		 	} 
		 	else 
		 	{
		 		//parent process, need to wait till child process ends...
		 		//printf("Parent process.... \n");
		 		
		 		while(wait(&status) != ch_id)
		 		;
		 		//printf("exited from child... exit status = %d \n",status);
		 		
		 		if(WIFEXITED(status))
	 			{
	 			//	printf("exited from child... exit status = %d \n",WEXITSTATUS(status));			
		 				
	 				if(WEXITSTATUS(status)==255)
	 				{
	 					//fprintf(stderr,"%s: Command not found.\n",c->args[0]);
	 					if(c->out == Tpipe) 
	 					{
	 						//printf("Abort pipe...\n");
	 						is_err=-1;
	 					}
	 					//exit(0);
	 				}
	 				/*else if(WEXITSTATUS(status)==126)
	 				{
	 					fprintf(stderr,"%s: Permission denied.\n",c->args[0]);
	 					is_err=-1;
	 					//exit(0); 					
	 					
	 				}*/
		 				
   			}
		 		//printf("Parent wait over for child  %d\n ",ch_id);	
		 		
		 	}
		 	
		 }		
		
	 } else 
	 {
			//printf(" Not last commmand : fork with cmd in pipe.... cmd = %s \n", c->args[0]);
			
		 	// need to fork...
		 	int ch_id;
		 	int status;
		 	
		 	ch_id=fork();
		 	if(ch_id < 0)
		 	{
		 		//printf("error in fork for %s \n");	
		 		exit(1);
		 	} 
		 	else if(ch_id ==0)
		 	{
		 			// child process
	
					// update STD In STD OUT, STD ERR file descriptors....
					setpgid(0,getpgrp());
					
					update_io_redirect(c);
		  			/*if(is_err==-1)
	  				{
  						//printf("is_err done \n");
  						return;
  					}*/
					//printf("IO redirect done with not last cmd = %s \n", c->args[0]);
					
		 		
		 			// check for built-in commands... 
					// if built-in cmd, no need to fork() ...
					is_built_in=0;
					
					is_built_in=execute_built_in(c);
					
					if(is_built_in ==0)
					{
				 		//not a built in command...
				 		//need to exec...
				 	
				 		//printf("Not a built in command... with not last cmd = %s \n", c->args[0]);
				 	
		 				if(execvp(c->args[0],c->args) == -1)	
		 				{
		 					//todo errno
		 					fprintf(stderr,"%s: %s\n",c->args[0],strerror(errno));		
		 					//is_err=-1;
		 					//return;
		 					exit(-1);
		 				}
		 			} else if (is_built_in == 1)
		 			{
		 			 // is built in command and has been executed.... need to exit now from this child process...
		 			 exit(0);	
		 				
		 			}
		 		} 
		 		else 
		 		{
		 			//parent process, need to wait till child process ends...
		 		
		 			while(wait(&status) != ch_id)
		 			;
		 			
		 			if(WIFEXITED(status))
		 			{
		 				//printf("exited from child...");			
		 				
		 				if(WEXITSTATUS(status)==255)
		 				{
		 					//fprintf(stderr,"%s: Command not found.\n",c->args[0]);
	 						if(c->out == Tpipe) 
	 						{
	 							//printf("Abort pipe...\n");
	 							is_err=-1;
	 						}
		 					//exit(0);
		 					
		 				}
	 					/*else if(WEXITSTATUS(status)==126)
	 					{
	 						fprintf(stderr,"%s: Permission denied.\n",c->args[0]);
	 						is_err=-1;
	 						//exit(0); 					
	 					
	 					}*/
		 				
		 			}
		 			//printf("Parent wait over for child  %d\n ",ch_id);	
		 		
		 		}

	 	
	 }
	
}

static void prPipe(Pipe p)
{
  int i = 0;
  Cmd c;

  if ( p == NULL )
  {  
  		//printf("NULL pipe came \n");
    	return;
	 }
 
  //pipe(pipe_fd);
 // pipe(pipe_err_fd);
  pcnt=0;  
  prev_pipe=0;
  int old_std_out=dup(STDOUT_FILENO);
  int old_std_in=dup(STDIN_FILENO);
  int old_std_err=dup(STDERR_FILENO);
  int temp_std_in=dup(STDIN_FILENO);
  
  is_err=0;	
  
 // printf("Begin pipe%s\n", p->type == Pout ? "" : " Error");
  
  for ( c = p->head; c != NULL && is_err != -1; c = c->next ) {
   // printf("  Cmd #%d: ", ++i);
   /* if (strcmp(c->args[0],"logout")==0)
    {
    	//printf("logout there, need to exit \n");
    	exit(0);
    }*/
    
    prCmd(c);
   // printf("prPipe     ############              : now execute Cmd \n");
   
  	if((c->out == Tpipe) || (c->out == TpipeErr))  
  	{
   	if(pcnt==0)
  	 	{
   		pipe(pipe_fd);
   	} 
   	else if(pcnt ==1)
	   {
   		pipe(pipe2_fd);
   	}
   
	}
    
    if(c->out == Tpipe)
    {
    	//printf("Pipe FD 1 set to output for cmd = %s\n",c->args[0]);
    	if(pcnt==0)
    	{
    		if(dup2(pipe_fd[1],STDOUT_FILENO)==-1);
    		{
				//printf("dup2 error for input for cmd = %s\n",c->args[0]);
			}
		
			//temp_std_in=dup(pipe_fd[0]);
    		prev_pipe=1;
    		//close(pipe_fd[1]);
    	} else if( pcnt==1)
    	{
    		if(dup2(pipe2_fd[1],STDOUT_FILENO)==-1);
    		{
				//printf("dup2 error for input for cmd = %s\n",c->args[0]);
			}
		
			temp_std_in=dup(pipe2_fd[0]);
    		prev_pipe=1;
    		
    	}
    		
    } 
    else if (c->out == TpipeErr)
    {
    	//printf("Pipe FD 1 set to output for cmd = %s\n",c->args[0]);
    	if(pcnt==0)
    	{
    		if(dup2(pipe_fd[1],STDOUT_FILENO)==-1);
    		{
				//printf("dup2 error for input for cmd = %s\n",c->args[0]);
			}

    		if(dup2(pipe_fd[1],STDERR_FILENO)==-1);
    		{
				//printf("dup2 error for input for cmd = %s\n",c->args[0]);
			}
						
			//temp_std_in=dup(pipe_fd[0]);
    		prev_pipe=1;
    		//close(pipe_fd[1]);
    	} else if( pcnt==1)
    	{
    		if(dup2(pipe2_fd[1],STDOUT_FILENO)==-1);
    		{
				//printf("dup2 error for input for cmd = %s\n",c->args[0]);
			}

    		if(dup2(pipe2_fd[1],STDERR_FILENO)==-1);
    		{
				//printf("dup2 error for input for cmd = %s\n",c->args[0]);
			}
			
			temp_std_in=dup(pipe2_fd[0]);
    		prev_pipe=1;
    		
    	}
    		
    }
    
    execute_cmd(c);
    
    
    if(prev_pipe > 0)
	 {
	    //printf("Pipe FD 0 set to input for cmd = %s\n",c->args[0]);
		//dup2(temp_std_in,pipe_fd[0]);
	
	  if(pcnt==0)
	  {
			if(dup2(pipe_fd[0],STDIN_FILENO)==-1)
			{
				perror("dup2 error...\n");
				exit(0);
			}
			prev_pipe=0;
			close(pipe_fd[1]);
			dup2(old_std_out,STDOUT_FILENO);
			dup2(old_std_err,STDERR_FILENO);
			pcnt=1;
	  } 
	  else if(pcnt==1)
	  {
	  		if(dup2(pipe2_fd[0],STDIN_FILENO)==-1)
			{
				perror("dup2 error...\n");
				exit(0);
			}
			prev_pipe=0;
			close(pipe2_fd[1]);
			dup2(old_std_out,STDOUT_FILENO);
			dup2(old_std_err,STDERR_FILENO);
	  		pcnt=0;
	  }
		//close(pipe_fd[0]);
	 }  
	
	
    //dup2(old_std_out,STDOUT_FILENO);
    //printf("OUTPUT FD back to console, execution done for cmd : %s \n", c->args[0]);
	 
  }
  
  
  //printf("End pipe\n");
  //close(pipe_fd[0]);
  //close(pipe_fd[1]);
  dup2(old_std_out,STDOUT_FILENO);
  dup2(old_std_in,STDIN_FILENO);
  dup2(old_std_err,STDERR_FILENO);
  prPipe(p->next);
}

static void read_ush_rc(Pipe p)
{
	
	char * path_ushrc;
	const char * home_var="HOME";
	int rc_fp=-1;
	int old_std_in_fp=dup(STDIN_FILENO);
	int old_std_out_fp=dup(STDOUT_FILENO);
	int old_std_err_fp=dup(STDERR_FILENO);	
	
	// get the path home from environ...
	path_ushrc=(char *)malloc(sizeof(char)*(strlen(getenv(home_var))+8));
	
	strcpy(path_ushrc,getenv(home_var));
	strcat(path_ushrc,"/.ushrc");
	
	
	rc_fp=open(path_ushrc,O_RDONLY,0600);
	
	if(rc_fp<0)
	{
		printf("%s: No such file or directory. \n",path_ushrc);
	}
	else 
	{
			// dup2 with std in to this file ....
			//printf("opened .ushrc file from HOME = %s\n",getenv(home_var));
			if(dup2(rc_fp,STDIN_FILENO) == -1)
			{
				perror("dup2 error for /.ushrc file \n");	
				exit(0);			
			}	
			else 
			{
				p=parse();
				while(p !=NULL && strcmp(p->head->args[0],"end")!=0)
				{
					fflush(stdout);
					prPipe(p);  // this will print the pipe and execute the pipe as well
					//fflush(stdout);				
					freePipe(p);
					p=parse();
				}
			
			}

			close(rc_fp);
			
		 dup2(old_std_out_fp,STDOUT_FILENO);
 		 dup2(old_std_in_fp,STDIN_FILENO);
 		 dup2(old_std_err_fp,STDERR_FILENO);
	}
	
}



int main(int argc, char *argv[])
{
  Pipe p;
  host = "armadillo";
  host=(char *)malloc(sizeof(char)*256);
  gethostname(host,256);
  
	//temp_fp=open("log_temp.txt",O_WRONLY|O_CREAT|O_APPEND,0660);
	
  signal(SIGTERM,handle_sig_term);
  signal(SIGQUIT, handle_sig_quit);
  signal(SIGINT,handle_sig_int);
  //signal(SIGTSTP,handle_sig_int);
  
  // read from ~/.ushrc file when shell starts....  
  read_ush_rc(p);
  
 // printf(" USHRC read and executed.... \n");
  
  setpgid(0,0);

  while ( 1 ) {
    printf("%s%% ", host);
    fflush(stdout);
    f=1;
    p = parse();
    f=0;
    prPipe(p);
    freePipe(p);
    //close(temp_fp);
  }
}

/*........................ end of main.c ....................................*/
