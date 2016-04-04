/******************************************************************************
 *
 *  File Name........: speak.c
 *
 *  Description......:
 *	Create a process that talks to the listen.c program.  After 
 *  connecting, each line of input is sent to listen.
 *
 *  This program takes two arguments.  The first is the host name on which
 *  the listen process is running. (Note: listen must be started first.)
 *  The second is the port number on which listen is accepting connections.
 *
 *
 *****************************************************************************/

/*........................ Include Files ....................................*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>

#define LEN	64
 
main (int argc, char *argv[])
{
  int s, rc, len, port,my_port,left_s,right_s,prev_s,next_s,p_id,prev_port,max_fd=-1,np;
  char host[LEN], str[LEN];
  char *l_hname;
  struct hostent *hp,*prev_hp;
  struct sockaddr_in sin,left_sin,right_sin,prev_sin,next_sin;
  fd_set rdfds;
	
  /* read host and port number from command line */
  if ( argc != 3 ) {
    fprintf(stderr, "Usage: %s <host-name> <port-number>\n", argv[0]);
    exit(1);
  }
  
  /* fill in hostent struct */
  hp = gethostbyname(argv[1]); 
  if ( hp == NULL ) {
    fprintf(stderr, "%s: host not found (%s)\n", argv[0], host);
    exit(1);
  }
  
  port = atoi(argv[2]);


	// generate left_port
	
	right_s=socket(AF_INET, SOCK_STREAM, 0);
 	 if (right_s < 0 ) {
    perror("socket:");
    exit(right_s);
 	}

	bzero((char *)&right_sin,sizeof(right_sin));
	right_sin.sin_family = AF_INET;
   memcpy(&right_sin.sin_addr, hp->h_addr_list[0], hp->h_length);
	my_port=port+10;
	int bc=-1;
	
	do{
		right_sin.sin_port = htons(my_port);
		right_sin.sin_addr.s_addr=htonl(INADDR_ANY);
		bc=bind(right_s,(struct sockaddr *)&right_sin, sizeof(right_sin));
		if(bc<0)
		  my_port++;
	 } while(bc<0);
	 
	 //Now my port is generated.... for this player...
	
	/*	
	 bc=listen(right_s,2);
	 if(bc<0)
	 {
    	perror("listen:");
    	exit(bc);
	 }*/
	 
	 
  /* create and connect to a socket */

  /* use address family INET and STREAMing sockets (TCP) */
  s = socket(AF_INET, SOCK_STREAM, 0);
  if ( s < 0 ) {
    perror("socket:");
    exit(s);
  }

  /* set up the address and port */
  bzero((char *)&sin,sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr=htonl(INADDR_ANY);
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  
  /* connect to socket at above addr and port */
  rc = connect(s, (struct sockaddr *)&sin, sizeof(sin));
  if ( rc < 0 ) {
    perror("connect:");
    exit(rc);
  }

	// receive player id from master....
	char tempb[512];
	bzero(tempb,512);
	
	rc=recv(s,tempb,511,0);
	if(rc<0)
	{
		perror("Error: recv pid from master...");
		exit(1);
	}
	
	p_id=atoi(tempb);
	
	printf("Connected as player %d\n",p_id);
	
	
  // send left port info to master...
 
   bzero(tempb,512);
   sprintf(tempb,"%d",my_port);
   
   //printf("left_port=%d\n",my_port);
   
   int sc;
   sc=send(s, tempb, strlen(tempb), 0);
   if(sc!=strlen(tempb))
   {
      perror("Error: send: left port to master");
      exit(1);
   }
   
   
   // Read from master the left right information about ring...
   
   bzero(tempb,512);

   rc=recv(s,tempb,512,0);
   if(rc<0)
   {
		perror("recv: left right...");
      exit(1);
   }
   tempb[rc]='\0';
   //printf("received from master= %s\n",tempb);
   // string format : [k].id  [k-1].left_port  [k-1].hostname...
   int prev_st=0,k=0;
   int cnt=0;
   char *stmp=(char *)malloc(512);
   
   for(k=0;k<strlen(tempb);k++)
   {
   	bzero(stmp,512);
   	if(tempb[k]==' ')
   	{
	     stmp=strndup(tempb+prev_st,k-prev_st+1);
   	
	     if(cnt==0)
	      {
		 np=atoi(stmp);
		 cnt++;
	      }
	     else if(cnt==1)
	      {
		 prev_port=atoi(stmp);
		 cnt++;
	      }

	     prev_st=k+1;
   	}

   	
   }
   
   	bzero(stmp,512);
	l_hname=(char *)malloc(512);
   	l_hname=strndup(tempb+prev_st,k-prev_st+1);
   	
   	//printf("received from master: np=%d, prev port = %d, left hostname=%s\n",np,prev_port,l_hname);
   
	// First player will accpet first...
	// For accept, use local host name, left port which is already binded...

	 bc=listen(right_s,2);
	 if(bc<0)
	 {
		perror("listen:");
		exit(bc);
	 }

	int n_len;
	n_len=sizeof(sin);
		
    if(p_id==0)
	{

		next_s=accept(right_s, (struct sockaddr *)&next_sin, &n_len);
		if ( next_s < 0 ) {
		  perror("Error: next accept:");
		  exit(next_s);
		}
		//printf("pid=%d Accept done to next...\n",p_id);
	} 

	// Now, connect with prev player... using k-1 hostname , k-1 port...
	  /* fill in hostent struct */
	  prev_hp = gethostbyname(l_hname); 
	  if ( prev_hp == NULL ) {
		fprintf(stderr, "host not found (%s)\n",l_hname);
		exit(1);
	  }

	  /* create and connect to a socket */

	  /* use address family INET and STREAMing sockets (TCP) */
	  left_s = socket(AF_INET, SOCK_STREAM, 0);
	  if ( left_s < 0 ) {
		perror("left socket:");
		exit(left_s);
	  }

	  /* set up the address and port */
	  left_sin.sin_family = AF_INET;
	  left_sin.sin_port = htons(prev_port);
	  left_sin.sin_addr.s_addr=htonl(INADDR_ANY);
	  memcpy(&left_sin.sin_addr, prev_hp->h_addr_list[0], prev_hp->h_length);
	  
	  /* connect to socket at above addr and port */
	  rc = connect(left_s, (struct sockaddr *)&left_sin, sizeof(left_sin));
	  if ( rc < 0 ) {
		perror("connect:");
		exit(rc);
	  }
	
	//printf("player :%d Connect done to prev...\n",p_id);
	
	// Accpet if not first player...
	
	if(p_id!=0)
	{
		next_s=accept(right_s, (struct sockaddr *)&next_sin, &n_len);
		if ( next_s < 0 ) {
		  perror("Error: next accept:");
		  exit(next_s);
		}
		//printf("pid=%d Accept done to next...\n",p_id);
	
	}
   
   // After connect to left right, send confirmation to master...
   bzero(tempb,512);
	
   strcpy(tempb,"ACK");
   sc=send(s, tempb, strlen(tempb), 0);
   if(sc!=strlen(tempb))
   {
      perror("Error: send: left port to master");
      exit(1);
   }
   
   //printf("ACK sent to master for neighbours connect \n");
   
   // After connection is made with neighbours and ACK is sent, ready to receive potato
   
   	
	// Set the player id as ther seed for randomness
		
	srand(p_id);
   
   // Set listeners...for s, left_s,next_s
	
	 FD_ZERO(&rdfds);
	
	
	// find the max_fd...
	
		if(s>left_s)
		{
			max_fd=s;
		} 
		else 
		{
			max_fd=left_s;
		}
		
		if(next_s>max_fd)
		{
			max_fd=next_s;
		}
		
		// starting listening loop for receiving data from connections...
		
		int rt_ok=0,msg_size=0,ic=0;

	 	char *peer_msg;
	 	
		while(1)
		{
			FD_ZERO(&rdfds);
			FD_SET(s,&rdfds);
			FD_SET(left_s,&rdfds);
			FD_SET(next_s,&rdfds);
			
			
			rt_ok=select(max_fd+1,&rdfds,NULL,NULL,NULL);
			
			if(FD_ISSET(s,&rdfds))
			{
				ic=ioctl(s,FIONREAD,&msg_size);
				if(ic<0)
				{
					perror("Error: master socket, ioctl ");
					exit(1);
				}
				
				peer_msg=(char *)malloc(sizeof(char)*(msg_size+2));
				bzero(peer_msg,msg_size+2);
				
				rc=recv(s,peer_msg,msg_size,0);
				if(rc<0)
				{
					
					perror("Error: recv error from s");
					exit(1);
				}
				
				peer_msg[rc]='\0';
				
				// if close received from master... break from the listening loop
				
				if(strcmp(peer_msg,"CLOSE")==0)
				{
					break;
				}
			}
			else if(FD_ISSET(left_s,&rdfds))
			{
				ic=ioctl(left_s,FIONREAD,&msg_size);
				if(ic<0)
				{
					perror("Error: master socket, ioctl ");
					exit(1);
				}
				
				peer_msg=(char *)malloc(sizeof(char)*(msg_size+2));
				bzero(peer_msg,msg_size+2);
				
				rc=recv(left_s,peer_msg,msg_size,0);
				if(rc<0)
				{
					
					perror("Error: recv error from s");
					exit(1);
				}
				
				peer_msg[rc]='\0';
				
			}
			else if(FD_ISSET(next_s,&rdfds))
			{
				ic=ioctl(next_s,FIONREAD,&msg_size);
				if(ic<0)
				{
					perror("Error: master socket, ioctl ");
					exit(1);
				}
				
				peer_msg=(char *)malloc(sizeof(char)*(msg_size+2));
				bzero(peer_msg,msg_size+2);
				
				rc=recv(next_s,peer_msg,msg_size,0);
				if(rc<0)
				{
					
					perror("Error: recv error from s");
					exit(1);
				}
				
				peer_msg[rc]='\0';
				
			}
			
			//printf("Recevied Peer msg= %s\n",peer_msg);
			
			// message received from the player/master, need to debug and process it...
			
  			prev_st=0,k=0,cnt=0;
  			int h_size,pt_size=0,hpc=0,f_pc=0;
  			char *hop_cnt=NULL,*p_trace=NULL,*new_str=NULL;
  		
  			for(k=0;k<strlen(peer_msg);k++)
  			{
  			
  				if(peer_msg[k]==' ')
  				{
  					if(cnt==0)
  					{
  						// "potato" is processed...no need of it...
  						cnt++;
  					}
  					else if(cnt==1)
  					{
  						//hops is processed...
  						h_size=k-prev_st+2;
  						hop_cnt=(char *)malloc(sizeof(char)*h_size);
  						hop_cnt=strndup(peer_msg+prev_st,k-prev_st+1);
  						hpc=atoi(hop_cnt);
  						cnt++;
  					}

  					prev_st=k+1;
  				}	
   		}
		
  			if(cnt==2)
  			{
  				// potato trace is processed....No trace would be there....
  				if(k==prev_st)
  				{
  					//printf("First received from master....\n");
  					f_pc=1;
  				}
  				else 
  				{
  					// Not a first player, so trace would be there....
  					pt_size=k-prev_st+2;
  					p_trace=(char *)malloc(sizeof(char)*pt_size);
  					p_trace=strndup(peer_msg+prev_st,k-prev_st+1);	
  					cnt++;
  				}
  			}		
		
			// Build new string...
			
			new_str=(char *)malloc(sizeof(char)*(msg_size+10));
			char this_p[10];
			
			
			// decrement hpc...
			hpc=hpc-1;
			
			strcpy(new_str,"POTATO ");
			
			bzero(this_p,10);
			sprintf(this_p,"%d ",hpc);
			strcat(new_str,this_p);
			if(f_pc==0)
			{
				// Not a first player, so trace would be there....
				strcat(new_str,p_trace);
			}
			
			bzero(this_p,10);
			
			if(hpc>0)
			{
				sprintf(this_p,"%d,",p_id);
			}
			else if(hpc==0)
			{
				sprintf(this_p,"%d",p_id);
			}	
		
			strcat(new_str,this_p);
			
			//printf("Sending New str= %s\n",new_str);
			
			if(hpc==0)
			{
				// Hops count ended... this player is it.... need to print it message and send the potato to master....
				
				printf("I'm it\n");
				
				sc=send(s,new_str,strlen(new_str),0);
				if(sc==-1)
				{
					perror("Error: sending left peer");
					close(s);
					exit(1);	
				}
				
				
			} 
			else if(hpc > 0)
			{
				// Hops count >0 ... need to send it to one of the neighbours....
				
				int ngh;
				ngh=rand();
				ngh=rand()%2;
				
				
				if(ngh==0)
				{
					int l_pl=((p_id-1)+np)%np;
					printf("Sending potato to %d\n",l_pl);
					
					sc=send(left_s,new_str,strlen(new_str),0);
					if(sc==-1)
					{
						perror("Error: sending left peer");
						close(left_s);
						exit(1);	
					}
					
				}
				else if(ngh==1)
				{
					
					int r_pl=(p_id+1)%np;
					printf("Sending potato to %d\n",r_pl);
					
					sc=send(next_s,new_str,strlen(new_str),0);
					if(sc==-1)
					{
						perror("Error: sending left peer");
						close(next_s);
						exit(1);	
					}
				}
					
			}
			
			
						
			
	  }
   
   
   // listening loop ended...
   

   
   
  /* read a string from the terminal and send on socket 
  while ( fgets(str, LEN, stdin) != NULL ) {
    if (str[strlen(str)-1] == '\n')
      str[strlen(str)-1] = '\0';
    len = send(s, str, strlen(str), 0);
    if ( len != strlen(str) ) {
      perror("send");
      exit(1);
    }
  }
	*/
	
   // closing the 
	//printf("Closing sockets... \n");

  close(s);
  close(next_s);
  close(right_s);
  close(left_s);
  exit(0);
}

/*........................ end of speak.c ...................................*/
