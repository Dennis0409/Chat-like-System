#include "func.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <signal.h>
using namespace std;

void broadcast(string message){
    for(int i=0;i<MAX_USER;i++){
        if(user[i]==-1) continue;
        //cout<<"user "<<user[i]<<endl;
        char*m=(char*)message.data();
        send(user[i],m,strlen(m),0);
        
    }
}
int find_id(int fd){
    for(int i=0;i<MAX_USER;i++){
        if(fd==user[i]) return i;
    }
    return -1;
}

void rename(string new_name,int id){
    char*temp=(char*)malloc(sizeof(new_name));
    strcpy(temp,new_name.c_str());
    for(int i=1;i<MAX_USER;i++){
        if(user[i]==-1) continue;
        if(strcmp(temp,user_name[i])==0){
            string e_mess="*** User ’"+string(temp)+"’ already exists. ***\n";
            send(user[id],e_mess.c_str(),strlen(e_mess.c_str()),0);
            send(user[id],"% ",strlen("% "),0);
            free(temp);
            return ;
        }
    }
    free(user_name[id]);
    user_name[id]=temp;
    string b_name="*** User from "+string(user_ip[id])+":"+string(user_port[id])+" is named "+new_name+". ***\n";
    broadcast(b_name);
    broadcast("% ");
}

void yell(string message,int id){
    string temp="*** "+string(user_name[id])+" yelled ***: "+message+"\n";
    for(int i=1;i<MAX_USER;i++){
        if(user[i]==-1) continue;
        if(i==id) continue;
        send(user[i],temp.c_str(),strlen(temp.c_str()),0);
        send(user[i],"% ",strlen("% "),0);
    }
}

int telll(string message,int id,int target){
    string temp="*** "+string(user_name[id])+" told you ***: "+message+"\n";
    if(user[target]==-1){
        string err="*** Error: user #"+to_string(target)+" does not exist yet. ***\n";
        send(user[id],err.c_str(),strlen(err.c_str()),0);
        send(user[id],"% ",strlen("% "),0);
        return 1;
    }
    send(user[target],temp.c_str(),strlen(temp.c_str()),0);
    send(user[target],"% ",strlen("% "),0);
    send(user[id],"% ",strlen("% "),0);
    return 0;
}

void exit_func(int id){
    string exit_mess="*** User "+string(user_name[id])+" left. ***\n";
    for(int i=1;i<MAX_USER;i++){
        if(user[i]==-1) continue;
        if(i==id) continue;
        send(user[i],exit_mess.c_str(),strlen(exit_mess.c_str()),0);
        send(user[i],"% ",strlen("% "),0);
    }
    user[id]=-1;
    free(user_name[id]);
    free(user_ip[id]);
    free(user_port[id]);
    env_number[id]=0;
    for(int i=0;i<100;i++){
        user_env[id][i][0]="";
        user_env[id][i][1]="";
    }
    for(int i=1;i<MAX_USER;i++){
        if(user_p[id][i][0]!=0 || user_p[id][i][1]!=0){
            close(user_p[id][i][0]);
            close(user_p[id][i][1]);
            user_p_check[id][i]=0;
        }
        if(user_p[i][id][0]!=0 || user_p[i][id][1]!=0){
            close(user_p[i][id][0]);
            close(user_p[i][id][1]);
            user_p_check[i][id]=0;
        }
        
    }
}
void end(int sign){
    exit(0);
}