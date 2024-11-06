#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <errno.h>
#include <string>
#include <iostream>
#include <string.h>
#include "func.h"
#include <arpa/inet.h>
#include <signal.h>
using namespace std;
int user_line[MAX_USER]={0};
int user_p[MAX_USER][MAX_USER][2]={0};
int user_p_check[MAX_USER][MAX_USER]={0};
int user_p_num[MAX_USER][MAX_PIPE][2]={0};
int user_p_num_idx[MAX_USER]={0};
int user[MAX_USER]={0};
int env_number[MAX_USER]={0};
char*user_env[MAX_USER][100][2]={};
char* user_name[MAX_USER]={0};
char* user_ip[MAX_USER]={0};
char* user_port[MAX_USER]={0};
unordered_map<int,unordered_map<int,int>>user_wait_num;
int main(int argc,char*argv[]){
    //socket的建立
    char inputBuffer[256] = {};
    string message="***************************************\n";
    string message2=" ** Welcome to the information server **\n";
    string message3="***************************************\n";
    int sockfd = 0,forClientSockfd = -1;

    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if (sockfd == -1){
        printf("Fail to create a socket.");
    }
    struct sockaddr_in serverInfo,clientInfo;

    bzero(&serverInfo,sizeof(serverInfo));

    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(7000);
    bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    listen(sockfd,MAX_USER);
    //vector<int>clientfds;
    int mx_fd=sockfd;
    int user_n=1;
    for(int i=1;i<MAX_USER;i++)user[i]=-1;
    while(1){
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(sockfd,&readset);
        //int client_len=clientfds.size();
        for(int i=0;i<MAX_USER;i++){
            if(user[i]!=-1){
                //printf("clientfds : %d\n",user[i]);
                FD_SET(user[i],&readset);
            }
        }
        signal(SIGINT,end);
        //printf("sockfd %d ",sockfd);
        //printf("-----\n");
        timeval tm;
        tm.tv_sec=5;
        tm.tv_usec=0;
        int ret=select(100,&readset,NULL,NULL,&tm);
        if(ret==-1){
            if(errno!=EINTR) break;
        }else if(ret==0){
            //cout<<"ret==0\n";
            continue;
        }else{
            if(FD_ISSET(sockfd,&readset)){
                struct sockaddr_in clientInfo;
                socklen_t client_len=sizeof(clientInfo);
                forClientSockfd = accept(sockfd,(struct sockaddr*) &clientInfo, &client_len);
                
                int g_ret=getpeername(forClientSockfd,(struct sockaddr*)&clientInfo,&client_len);
                if(g_ret!=0) printf("error\n");
                
                
                char*ip=(char*)malloc(sizeof(inet_ntoa(clientInfo.sin_addr)));
                char*name=(char*)malloc(sizeof((char*)"(no name)"));
                string str_port=to_string(ntohs(clientInfo.sin_port));
                char*port=(char*)malloc(sizeof(str_port.c_str()));
                strcpy(port,str_port.c_str());
                strcpy(ip,inet_ntoa(clientInfo.sin_addr));
                strcpy(name,(char*)"(no name)");

                int user_n;
                for(int i=0;i<MAX_USER;i++){
                    if(user[i]==-1){
                        user[i]=forClientSockfd;
                        user_ip[i]=ip;
                        user_name[i]=name;
                        user_port[i]=port;
                        user_n=i;
                        //printf("%s %s %s %d\n",user_name[i],user_ip[i],user_port[i],i);
                        break;
                    }
                }
                send(forClientSockfd,message.c_str(),strlen(message.c_str()),0);
                send(forClientSockfd,message2.c_str(),strlen(message2.c_str()),0);
                send(forClientSockfd,message3.c_str(),strlen(message3.c_str()),0);
                string broadcast_user="*** User "+string(user_name[user_n])+" entered from "+string(user_ip[user_n])+":"+string(user_port[user_n])+". ***\n";
                broadcast(broadcast_user);
                broadcast("% ");
                if(mx_fd<sockfd){
                    mx_fd=sockfd;
                }
            }
            for(int i = 0; i < 100+1; i++){
                //printf("%d %d\n",i,FD_ISSET(i,&readset));
                if(i != sockfd && FD_ISSET(i, &readset)){
                    int idx=find_id(i);
                    ret=shell(i,idx);
                    if(ret==-1){
                        close(i);
                        user[idx]=-1;
                        FD_CLR(i,&readset);
                    }
                }
                
            }
        }
        
        
    }
    return 0;
}