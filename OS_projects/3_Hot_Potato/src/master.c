/******************************************************************************
 *
 *  File Name........: listen.c
 *
 *  Description......:
 *	Creates a program that establishes a passive socket.  The socket 
 *  accepts connection from the speak.c program.  While the connection is
 *  open, listen will print the characters that come down the socket.
 *
 *  Listen takes a single argument, the port number of the socket.  Choose
 *  a number that isn't assigned.  Invoke the speak.c program with the 
 *  same port number.
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

struct hp_player{
	int id;
	int p_port,l_port;
	int m_sockt;
	//char host_name[64];
	
	struct hostent *ph_addr;
	
	int p_ll,p_rr;
	
}; 

int main (int argc, char *argv[])
{
  char buf[512];
  char host[64];
  int s, p, fp, rc, len, port,np,hops,cpc=0,i,max_fd,okc=0;
  struct hostent *hp, *ihp;
  struct sockaddr_in sin, incoming;
  fd_set rdfds,rdfds_ok;

  /* read port number from command line */
  
  //printf("argc toy= %d\n",argc);
  
  if ( argc < 4 ) {
    fprintf(stderr, "Usage mismatch:  <port-number> <number-of-players> <hops>, argc = %d\n", argc);
    exit(1);
  }
  
  port = atoi(argv[1]);
  np=atoi(argv[2]);
  
  if(np<2)
  {
    fprintf(stderr, "Usage: <number-of-players> must be greater than one. np=%d\n",np);
    exit(1);	
  }
  
  hops=atoi(argv[3]);
 
  if(hops<0)
  {
    fprintf(stderr, "Usage: <number-of-players> must be non-negative. , hops=%d\n",hops);
    exit(1);	
  } 



  /* fill in hostent struct for self */
  gethostname(host, sizeof host);
  // printf("host= %s\n",host);
  //lib-39168.eos.ncsu.edu
  //priestess.ece.ncsu.edu
  //lib-39163.eos.ncsu.edu
  
	printf("Potato Master on %s\n",host);  
   printf("Players = %d\n",np);
   printf("Hops = %d\n",hops);
  
  hp = gethostbyname(host);
  if ( hp == NULL ) {
    fprintf(stderr, "%s: host not found (%s)\n", argv[0], host);
    exit(1);
  }

  /* open a socket for listening
   * 4 steps:
   *	1. create socket
   *	2. bind it to an address/port
   *	3. listen
   *	4. accept a connection
   */

  /* use address family INET and STREAMing sockets (TCP) */
  s = socket(AF_INET, SOCK_STREAM, 0);
  if ( s < 0 ) {
    perror("socket:");
    exit(s);
  }

  /* set up the address and port */
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr=htonl(INADDR_ANY);
  memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  
  /* bind socket s to address sin */
  rc = bind(s, (struct sockaddr *)&sin, sizeof(sin));
  if ( rc < 0 ) {
    perror("bind:");
    exit(rc);
  }

	struct hp_player players[np];
	
	

	// make queue of np connections...
  rc = listen(s, np);
  
  if ( rc < 0 ) {
    perror("listen:");
    exit(rc);
  }

	int sc;
	   
  /* accept connections */
  for(i=0;i<np;i++)
  {
    len = sizeof(sin);
    p = accept(s, (struct sockaddr *)&incoming, &len);
    if ( p < 0 ) {
      perror("bind:");
      exit(rc);
    }
    
    ihp = gethostbyaddr((char *)&incoming.sin_addr, 
			sizeof(struct in_addr), AF_INET);
    //printf(">> Connected to %s\n", ihp->h_name);
    
    
    // load to struct player....
   // players[cpc]=()malloc(sizeof(struct hp_player));
    players[i].id=i;
    players[i].m_sockt=p;
    players[i].ph_addr=ihp;
    
    if((i-1) <0)
	 {
		players[i].p_ll=np-1;
	 } else 
	 {
		players[i].p_ll=i-1;
	 }
		
	 if((i+1)>(np-1))
	 {
		players[i].p_rr=0;
	 } else 
	 {
		players[i].p_rr=i+1;	
	 }
		
		bzero(buf,512);
		char ppid[10];
		sprintf(ppid,"%d",players[i].id);
		strcpy(buf,ppid);
		
	
		sc=send(p,buf,strlen(buf),0);
		if(sc<0)
		{
			perror("Error sending pid...");
			exit(1);	
		}
    
    
     bzero(buf,512);
     len = recv(p, buf, 32, 0);
     if ( len < 0 ) {
			perror("recv");
			exit(1);
     }
     buf[len] = '\0';
           
     players[i].l_port=atoi(buf);
     
    //printf(">> Total players: %d , Connected to player :%d , on host: %s, left_port=%d\n", np, players[i].id,players[i].ph_addr->h_name,players[i].l_port);
    printf("player %d is on %s\n",players[i].id,players[i].ph_addr->h_name);

 
    /* read and print strings sent over the connection */
  /*  while ( 1 ) {
      len = recv(p, buf, 32, 0);
      if ( len < 0 ) {
	perror("recv");
	exit(1);
      }
      buf[len] = '\0';
      if ( !strcmp("close", buf) )
	break;
      else
	printf("%s\n", buf);
    }*/
    // close(p);
    //printf(">> Connection closed\n");
  }
  
	// step 2: connect players to each other...
	//printf("Total= %d Players are connected to master, now need to connect them with their neighbours...\n",i);

			
	for(i=0;i<np;i++)
	{
		int cc,ll,rr;
		cc=np;
		ll=players[i].p_ll;
		rr=players[i].p_rr;
		
		char tmpc[512];
	
		bzero(buf,512);
		bzero(tmpc,512);
		
		sprintf(tmpc,"%d",cc);
		strcpy(buf,tmpc);
		strcat(buf," ");
		
		sprintf(tmpc,"%d",players[ll].l_port);
		strcat(buf,tmpc);
		strcat(buf," ");
		
		strcat(buf,players[ll].ph_addr->h_name);
		

		//printf("sending player[%d] : %s\n",i,buf);
		sc=send(players[i].m_sockt,buf,strlen(buf),0);
		if(sc==-1)
		{
			perror("send: left right...");
      	exit(1);
		}
	
		
	}
  
  //todo : closing sockets...
  
  // Players are connecting with each other, master needs to wait for the acknowledgement....
  char tmpo[512];
	
  okc=0;
  int rt_ok;
  while(1)
  {
		max_fd=-1;
  		FD_ZERO(&rdfds_ok);
  		for(i=0;i<np;i++)
  		{
  			FD_SET(players[i].m_sockt,&rdfds_ok);
  			if(players[i].m_sockt > max_fd)
  			{
  				max_fd=players[i].m_sockt;
  			
  			}
  		}	
  	
  		
  		
  		rt_ok=select(max_fd+1,&rdfds_ok,NULL,NULL,NULL);
  		
  		for(i=0;i<np;i++)
  		{
  			if(FD_ISSET(players[i].m_sockt,&rdfds_ok))
  			{
  				
  				bzero(tmpo,512);
  				rc=recv(players[i].m_sockt,tmpo,512,0);
  				if(rc==-1)
  				{
					perror("recv");
					exit(1);
  				}
  				
  				if(strcmp(tmpo,"ACK")==0)
  				{
  					//printf("ACK from player id=%d\n",i);
  					okc++;
  				}
  			}
  			
  		}
  			
  		// when Ack is received from all players.... Need to break the while loop and start the potato sending...
  		if(okc==np)
  		  break;
  	}
  	
  //	printf("Ack is received from all players, start the game....\n");
  	
  	// check if hops is >0.
  	
  	if(hops>0)
  	{
		// select the first player by random...  		
  		int frst_p=rand();
  		frst_p=frst_p%np;
  		
  		printf("All players present, sending potato to player %d\n",frst_p);
  		
  		char ptt_str[512];
  		bzero(ptt_str,512);
  		strcpy(ptt_str,"POTATO ");
  		
  		bzero(tmpo,512);
  		sprintf(tmpo,"%d ",hops);
  		strcat(ptt_str,tmpo);
  		
  		// Send the ptt_str to the first player...
  		sc=send(players[frst_p].m_sockt,ptt_str,strlen(ptt_str),0);
  		if(sc==-1)
  		{
  			perror("Error: Send potato First player...");
  			exit(1);
  		}
  		
		// Potato is sent, now waiting to receive the potato at the end of the game...  		
  		FD_ZERO(&rdfds);
  		for(i=0;i<np;i++)
  		{
  			FD_SET(players[i].m_sockt,&rdfds);
  		}
  		
  		rt_ok=select(max_fd+1,&rdfds,NULL,NULL,NULL);
  		
		char *it_str;
		int it_size,ic=0;
  		
  		for(i=0;i<np;i++)
  		{
  			if(FD_ISSET(players[i].m_sockt,&rdfds))
  			{
  				
  				ic=ioctl(players[i].m_sockt,FIONREAD,&it_size);
  				if(ic<0)
  				{
  					perror("Error: It message ioctl error...");
  					exit(1);
  				}
  				
  				it_str=(char *)malloc(sizeof(char)*(it_size+1));
  				
  				rc=recv(players[i].m_sockt,it_str,it_size,0);
  				
  				if(rc==-1)
  				{
					perror("Error: recv It end message...");
					exit(1);
  				}
  				
				it_str[rc]='\0';

				break;
				
  			}
  			
  		}
  		
  		// It message received from one of the players... Need to process it to print the traces...
  		
  		int prev_st=0,k=0,cnt=0;
  		int h_size,pt_size=0;
  		char *hop_cnt,*p_trace;
  		
  		for(k=0;k<strlen(it_str);k++)
  		{
  			
  			if(it_str[k]==' ')
  			{
  				if(cnt==0)
  				{
  					// "POTATO" is processed...no need of it...
  					cnt++;
  				}
  				else if(cnt==1)
  				{
  					//hops is processed...
  					h_size=k-prev_st+2;
  					hop_cnt=(char *)malloc(sizeof(char)*h_size);
  					hop_cnt=strndup(it_str+prev_st,k-prev_st+1);	
  					cnt++;
  				}

  				prev_st=k+1;
  			}	
  			
  			
		}
		
  		if(cnt==2)
  		{
  			// potato trace is processed....
  			pt_size=k-prev_st+2;
  			p_trace=(char *)malloc(sizeof(char)*pt_size);
  			p_trace=strndup(it_str+prev_st,k-prev_st+1);	
  			cnt++;
  		}		
		
		
		printf("Trace of potato:\n");
		// check if processed string is directly the trace....

		printf("%s\n",p_trace);
		
  		
  	} else 
  	{
  		// hops ==0 
  		// todo , whether to print the trace or not...
  		
  	}
  	
  	
  	// Trace printed... game over...Need to close the socket....

  

  for(i=0;i<np;i++)
    {
    	bzero(tmpo,512);
    	strcpy(tmpo,"CLOSE");
    	
		sc=send(players[i].m_sockt,tmpo,strlen(tmpo),0);
		if(sc==-1)
		{
			perror("Error: Sending close message...");	
			exit(1);
		}
		
		// closing players socket with master...
		
      close(players[i].m_sockt);
      //printf(">> Connection = %d closed\n",i);
    }
    
  close(s);
      
  //printf(">> Exiting game...\n");
  exit(0);
}

/*........................ end of listen.c ..................................*/
