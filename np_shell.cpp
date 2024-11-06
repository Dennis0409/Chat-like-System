#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <algorithm>
#include <unordered_map>
#include "func.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cctype>
#include <algorithm>
using namespace std;
#define MAX_WORDS_IN_LINE 500000
int set_env(string name,string value,int id){
    for(int i=0;i<100;i++){
        if(strcmp(user_env[id][i][0],(char*)name.c_str())==0){
            strcpy(user_env[id][i][1],(char*)value.c_str());
            setenv(user_env[id][i][0],user_env[id][i][1],1);
            return 0;
        }
    }
    user_env[id][env_number[id]][0]=(char*)malloc(sizeof(name.c_str()));
    user_env[id][env_number[id]][1]=(char*)malloc(sizeof(value.c_str()));
    strcpy(user_env[id][env_number[id]][0],name.c_str());
    strcpy(user_env[id][env_number[id]][1],value.c_str());
    env_number[id]++;
    return 0;
}
void init(int id){
    user_env[id][0][0]=(char*)malloc(sizeof("PATH"));
    strcpy(user_env[id][0][0],"PATH");
    user_env[id][0][1]=(char*)malloc(sizeof("/bin:./bin"));
    strcpy(user_env[id][0][1],"/bin:./bin");
    env_number[id]++;
    setenv(user_env[id][0][0],user_env[id][0][1],1);
}
void print_env(int fd,string name){
    char*p=getenv((char*)name.data());
    if(p==NULL) return ;
    dup2(fd,STDOUT_FILENO);
    dup2(fd,STDERR_FILENO);
    cout<<p<<endl;
}
bool isNumber(string&str){
    return !str.empty() && std::all_of(str.begin(),str.end(),::isdigit);
}
int shell(int fd,int id){
    init(id);
    vector<int>wait_proc; // process_idx
    //unordered_map<int,int>wait_num;
    stringstream ss;
    vector<int>bar_pos;
    vector<string>cmd;
    string temp="";
    string c_in;
    int now_bar=0;
    int last_bar=0;
    int p[MAX_PIPE][2]={0};
    //int cur=0;
    bar_pos.clear();
    bar_pos.push_back(-1);
    int pipe_create=0;
    
    ss.clear();
    cmd.clear();
    char buf_in[10000]={};
    read(fd,buf_in,sizeof(buf_in));
    string str(buf_in);
    //cout<<"% ";
    //if(!std::getline(str,c_in,'\n')) return ;
    ss.str(str);
    
    for(int i=0;ss>>temp;i++){
        cmd.push_back(temp);
        if(temp=="|"){
            bar_pos.push_back(i);
        }
    }
    
    if(cmd[0]=="exit"){
        exit_func(id);
        return -1;
    }
    if(cmd[0]=="who"){
        char*title="<ID>    <nickname>  <IP:port>           <indicate me>\n";
        send(fd,title,strlen(title),0);
        for(int i=1;i<MAX_USER;i++){
            if(user[i]==-1) continue;
            cerr<<user_name[i]<<endl;
            string str_i(to_string(i)+"           ");
            send(fd,str_i.c_str(),strlen(str_i.c_str()),0);
            send(fd,(const char*)user_name[i],strlen(user_name[i]),0);
            send(fd,"  ",strlen("  "),0);
            send(fd,user_ip[i],strlen(user_ip[i]),0);
            send(fd,"           ",strlen("           "),0);
            if(i==id) send(fd,"<- me\n",strlen("<- me\n"),0);
            else send(fd,"\n",strlen("\n"),0);
            
        }
        send(fd,"% ",strlen("% "),0);
        return 0;
        
    }
    if(cmd[0]=="name"){
        string new_name;
        for(int i=1;i<cmd.size();i++){
            new_name+=cmd[i]+" ";
        }
        new_name.pop_back();
        rename(new_name,id);
        return 0;
    }
    if(cmd[0]=="yell"){
        string temp="*** "+string(user_name[id])+" yelled ***: ";
        for(int i=1;i<cmd.size();i++){
            temp+=cmd[i]+" ";
        }
        temp.pop_back();
        temp+="\n";
        broadcast(temp);
        broadcast("% ");
        return 0;
    }
    if(cmd[0]=="tell"){
        int target=stoi(cmd[1]);
        if(user[target]==-1){
            string e_mess="*** Error: user #"+to_string(target)+" does not exist yet. ***\n";
            send(fd,e_mess.c_str(),strlen(e_mess.c_str()),0);
            send(fd,"% ",strlen("% "),0);
            return 0;
        }
        string temp;
        for(int i=2;i<cmd.size();i++){
            temp+=cmd[i]+" ";
        }
        temp.pop_back();
        
        return telll(temp,id,target);
    }
    int need_catch=0; // 1:have process need output in the cmd
    for(auto &i:user_wait_num[id]){
        int p=i.first,col=i.second;
        if(col==user_line[id]+1){
            wait_proc.push_back(i.first);
            need_catch=1;
        }
    }
    user_line[id]++;
    for(int i=0;i<cmd.size();i++){
        if(cmd[i]=="|"||cmd[i]==">"||cmd[i][0]=='|'){
            now_bar=std::find(bar_pos.begin(),bar_pos.end(),i)-bar_pos.begin();
            last_bar=now_bar-1;
        }
        
        if(cmd[i]=="|"){
            if(cmd[i-1][0]=='<'){
                continue;
            }
            pipe_create++;
            int now_pipe=pipe_create-1;
            int last_pipe=pipe_create-2;
            if(pipe(p[now_pipe])<0) std::cout<<"create pipe error\n";
            
            if(fork()==0){
                if(need_catch){
                    for(int i=0;i<wait_proc.size();i++){
                        dup2(user_p_num[id][wait_proc[i]][0],STDIN_FILENO);
                        close(user_p_num[id][wait_proc[i]][0]);
                        close(user_p_num[id][wait_proc[i]][1]);
                    }
                    need_catch=0;
                }
                if(last_pipe>=0){
                    dup2(p[last_pipe][0],STDIN_FILENO);
                    close(p[last_pipe][0]);
                    close(p[last_pipe][1]);
                }
                close(p[now_pipe][0]);
                dup2(p[now_pipe][1],STDOUT_FILENO);
                dup2(fd,STDERR_FILENO);
                close(p[now_pipe][1]);
                char*argv[MAX_WORDS_IN_LINE]={0};
                for(int j=0;j<i-bar_pos.at(last_bar)-1;j++){
                    argv[j]=(char*)cmd.at(bar_pos.at(last_bar)+j+1).data();
                    //fprintf(stderr,"argv %s\n",argv[j]);
                }
                argv[i-bar_pos.at(last_bar)-1]=NULL;
                
                if(execvp(argv[0],argv)<0){
                    close(1);
                    fprintf(stderr,"Unknown command: [%s]\n",argv[0]);
                }
                exit(0);
            }else{
                if(last_pipe>=0){
                    close(p[last_pipe][0]);
                    close(p[last_pipe][1]);
                }
                if(!wait_proc.empty()){
                    for(int i=0;i<wait_proc.size();i++){
                        close(user_p_num[id][wait_proc[i]][0]);
                        close(user_p_num[id][wait_proc[i]][1]);
                    }
                }
            }
        }else if(cmd[i][0]=='>'){
            //std::cout<<cmd.at(bar_pos.at(last_bar)+1).data()<<endl;
            int target=-1;
            int fd_op=-1;
            int safe=1;
            if(cmd[i].size()>1){
                string t=cmd[i].substr(1);
                target=stoi(t);
                if(user[target]==-1){
                    string e_mess="*** Error: user #"+to_string(target)+" does not exist yet. ***\n";
                    send(fd,e_mess.c_str(),strlen(e_mess.c_str()),0);
                    safe=0;
                }
                else if(user_p_check[id][target]==1){
                    string e_mess="*** Error: the pipe #"+to_string(id)+"->#"+to_string(target)+" already exists. ***\n";
                    send(fd,e_mess.c_str(),strlen(e_mess.c_str()),0);
                    safe=0;
                }
                else{
                    if(pipe(user_p[id][target])<0) std::cout<<"user_p error\n";
                    user_p_check[id][target]=1;
                }
                cout<<"cmd > \n";
            }else{
                const char*filename=cmd.at(i+1).data();
                fd_op = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                i++;
                
                if (fd_op == -1) {
                    std::cerr << "Error opening file tmp.txt" << std::endl;
                    return 1;
                }
            }
            int last_pipe=pipe_create-1;
            int status;
            
            int pid=fork();
            if(pid==0){
                if(need_catch){
                    for(int i=0;i<wait_proc.size();i++){
                        dup2(user_p_num[id][wait_proc[i]][0],STDIN_FILENO);
                        close(user_p_num[id][wait_proc[i]][0]);
                        close(user_p_num[id][wait_proc[i]][1]);
                    }
                    need_catch=0;
                }
                if(last_pipe>=0){
                    //cout<<"last pipe "<<last_pipe<<endl;
                    dup2(p[last_pipe][0],STDIN_FILENO);
                    dup2(p[last_pipe][0],STDERR_FILENO);
                    close(p[last_pipe][0]);
                    close(p[last_pipe][1]);
                }
                if(fd_op!=-1){
                    //cout<<"fd op :"<<fd_op<<endl;
                    dup2(fd_op,STDOUT_FILENO);
                    close(fd_op);
                }else{
                    if(user_p_check[id][target]==1){
                        dup2(user_p[id][target][1],STDOUT_FILENO);
                        close(user_p[id][target][0]);
                        close(user_p[id][target][1]);
                        string cmd_copy;
                        for(auto c:cmd){
                            cmd_copy+=c+" ";
                        }
                        cmd_copy.pop_back();
                        string mess="*** "+string(user_name[id])+" (#"+to_string(id)+") just piped ’"+cmd_copy+"’ to "+string(user_name[target])+" (#"+to_string(target)+") ***\n";   
                        broadcast(mess);
                    }else{
                        //dup2(fd,STDOUT_FILENO);
                        dup2(fd,STDERR_FILENO);
                    }
                }
                if(safe==0){
                    int fp=open("/dev/null",O_RDWR);
                    int ffp=open("/dev/null",O_RDWR);
                    dup2(fp,STDIN_FILENO);
                    dup2(ffp,STDOUT_FILENO);
                    close(fp);
                    close(ffp);
                }
                char*argv[MAX_WORDS_IN_LINE]={0};
                int flag=0;
                for(int j=0;j<i-bar_pos.at(last_bar)-1;j++){
                    argv[j]=(char*)cmd.at(bar_pos.at(last_bar)+j+1).data();
                    if(strstr(argv[j],"<")!=NULL){
                        argv[j]=NULL;
                        flag=1;
                        if(execvp(argv[0],argv)==-1){
                            if(i==cmd.size()-1) fprintf(stderr,"Unknown command: [%s]\n",argv[0]);
                        }
                        break;
                    }
                }
                if(fd_op!=-1){
                    argv[i-1-bar_pos.at(last_bar)-1]=NULL;
                }
                if(flag==0 && execvp(argv[0],argv)==-1){
                    if(i==cmd.size()-1){
                        fprintf(stderr,"Unknown command: [%s]\n",argv[0]);
                    }
                }
                exit(0);
                
            }else{
                if(last_pipe>=0){
                    close(p[last_pipe][0]);
                    close(p[last_pipe][1]);
                }
                if(!wait_proc.empty()){
                    for(int i=0;i<wait_proc.size();i++){
                        close(user_p_num[id][wait_proc[i]][0]);
                        close(user_p_num[id][wait_proc[i]][1]);
                    }
                }
                int wtr=waitpid(pid,&status,WUNTRACED | WCONTINUED);
                if (wtr == -1) {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }
            }
            
            
        }else if(cmd[i][0]=='<'){
            string t=cmd[i].substr(1);
            int target=stoi(t);
            int safe=1;
            if(user[target]==-1){
                string e_mess="*** Error: user #"+to_string(target)+" does not exist yet. ***\n";
                send(fd,e_mess.c_str(),strlen(e_mess.c_str()),0);
                safe=0;
                
            }else if(user_p_check[target][id]==0){
                string e_mess="*** Error: the pipe #"+to_string(target)+"->#"+to_string(id)+" does not exist yet. ***\n";
                send(fd,e_mess.c_str(),strlen(e_mess.c_str()),0);
                safe=0;
            }
            
            int now_pipe=-1;
            if(i!=cmd.size()-1){
                pipe_create++;
                now_pipe=pipe_create-1;
                
                if(pipe(p[now_pipe])<0) std::cout<<"create pipe error\n";
            }
            
            int pid=fork();
            int status;
            if(pid==0){
                if(user_p_check[target][id]==1){
                    
                    dup2(user_p[target][id][0],STDIN_FILENO);
                    close(user_p[target][id][1]);
                    close(user_p[target][id][0]);
                    
                    string cmd_copy;
                    for(auto c:cmd){
                        cmd_copy+=c+" ";
                    }
                    cmd_copy.pop_back();
                    string mess="*** "+string(user_name[id])+" (#"+to_string(id)+") just received from "+string(user_name[target])+" (#"+to_string(target)+") by ’"+cmd_copy+"’ ***\n";
                    broadcast(mess);
                    
                }
                if(safe==0){
                    int fp=open("/dev/null",O_RDWR);
                    dup2(fp,STDIN_FILENO);
                    close(fp);
                } 
                if(i==cmd.size()-1){
                    
                    dup2(fd,STDOUT_FILENO);
                    dup2(fd,STDERR_FILENO);
                }else{
                    //cout<<"now pipe "<<now_pipe<<endl;
                    dup2(p[now_pipe][1],STDOUT_FILENO);
                    //dup2(p[now_pipe][1],STDERR_FILENO);
                    close(p[now_pipe][0]);
                    close(p[now_pipe][1]);
                }
                
                char*argv[MAX_WORDS_IN_LINE]={0};
                int flag=0;
                for(int j=0;j<i-bar_pos.at(last_bar)-1;j++){
                    argv[j]=(char*)cmd.at(bar_pos.at(last_bar)+j+1).data();
                    if(strstr(argv[j],">")!=NULL){
                        argv[j]=NULL;
                        flag=1;
                        if(execvp(argv[0],argv)==-1){
                            //cout<<"> exec\n";
                            if(i==cmd.size()-1){
                                dup2(fd,STDERR_FILENO);
                                fprintf(stderr,"Unknown command: [%s]\n",argv[0]);
                            }
                        }
                        break;
                        //cout<<"execvp after "<<argv[j]<<endl;
                    }
                    //fprintf(stderr,"argv %s\n",argv[j]);
                }
                argv[i-bar_pos.at(last_bar)-1]=NULL;
                
                if(flag==0 && execvp(argv[0],argv)==-1){
                    if(i==cmd.size()-1){
                        dup2(fd,STDERR_FILENO);
                        fprintf(stderr,"Unknown command: [%s]\n",argv[0]);
                    }
                    //exit(EXIT_FAILURE);
                }
                exit(0);
            }else{
                if(safe==1){
                    close(user_p[target][id][0]);
                    close(user_p[target][id][1]);
                }
                if(i!=cmd.size()-1) close(p[now_pipe][1]);
                
                
                int wtr=waitpid(pid,&status,WUNTRACED | WCONTINUED);
                //send(fd,"wtr\n",strlen("wtr\n"),0);
                
                if (wtr == -1) {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }
                
            }
            if(safe==1) user_p_check[target][id]=0;

        }else if(cmd[i][0]=='|'||cmd[i][0]=='!'){
            int num=cmd[i][1]-'0';
            int wait_now=-1;
            for(auto i:user_wait_num[id]){
                if(user_line[id]+num==i.second){
                    wait_now=i.first;
                }
            }
            if(wait_now==-1){
                user_p_num_idx[id]++;
                wait_now=user_p_num_idx[id]-1;
                user_wait_num[id][wait_now]=user_line[id]+num;
                //fprintf(stderr,"%d\n",user_line[id]+num);
                if(pipe(user_p_num[id][wait_now])<0) std::cout<<"create pipe error\n";
            }
            //fprintf(stderr,"%d\n",user_line[id]+num);
            int last_pipe=pipe_create-1;
            
            int status;
            int pid=fork();
            if(pid==0){
                if(need_catch){
                    for(int i=0;i<wait_proc.size();i++){
                        dup2(user_p_num[id][wait_proc[i]][0],STDIN_FILENO);
                        close(user_p_num[id][wait_proc[i]][0]);
                        close(user_p_num[id][wait_proc[i]][1]);
                    }
                    need_catch=0;
                }
                if(last_pipe>=0){
                    dup2(p[last_pipe][0],STDIN_FILENO);
                    close(p[last_pipe][0]);
                    close(p[last_pipe][1]);
                }
                if(cmd[i][0]=='!'){
                    dup2(user_p_num[id][wait_now][1],STDERR_FILENO);
                }
                dup2(user_p_num[id][wait_now][1],STDOUT_FILENO);
                close(user_p_num[id][wait_now][0]);
                close(user_p_num[id][wait_now][1]);

                char*argv[MAX_WORDS_IN_LINE]={0};
                for(int j=0;j<i-bar_pos.at(last_bar)-1;j++){
                    argv[j]=(char*)cmd.at(bar_pos.at(last_bar)+j+1).data();
                    //fprintf(stderr,"argv %s\n",argv[j]);
                }
                argv[i-bar_pos.at(last_bar)-1]=NULL;

                if(execvp(argv[0],argv)<0){
                    close(1);
                    fprintf(stderr,"Unknown command: [%s]\n",argv[0]);
                }
                exit(0);
            }else{
                if(last_pipe>=0){
                    close(p[last_pipe][0]);
                    close(p[last_pipe][1]);
                }
                if(!wait_proc.empty()){
                    for(int i=0;i<wait_proc.size();i++){
                        close(user_p_num[id][wait_proc[i]][0]);
                        close(user_p_num[id][wait_proc[i]][1]);
                    }
                }
                int wtr=waitpid(pid,&status,WUNTRACED | WCONTINUED);
                if (wtr == -1) {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }
            }

        }
        else if(i==cmd.size()-1){
            pipe_create++;
            int now_pipe=pipe_create-1;
            int last_pipe=pipe_create-2;
            //cout<<"fd \n";
            if(pipe(p[now_pipe])<0) std::cout<<"create pipe error ls \n";
            int status;
            int pid=fork();
            if(pid==0){
                if(last_pipe>=0){
                    
                    dup2(p[last_pipe][0],STDIN_FILENO);
                    close(p[last_pipe][1]);
                    close(p[last_pipe][0]);
                }
                
                if(need_catch){
                    for(int i=0;i<wait_proc.size();i++){
                        //cout<<"catch "<<endl;
                        dup2(user_p_num[id][wait_proc[i]][0],STDIN_FILENO);
                        close(user_p_num[id][wait_proc[i]][0]);
                        close(user_p_num[id][wait_proc[i]][1]);
                    }
                    need_catch=0;
                }
                
                //std::cout<<"last pipe "<<last_pipe<<"now_pipe "<<now_pipe<<std::endl;
                close(p[now_pipe][0]);
                close(p[now_pipe][1]);
                dup2(fd,STDERR_FILENO);
                dup2(fd,STDOUT_FILENO);
                char*argv[MAX_WORDS_IN_LINE]={0};
                for(int j=0;j<i-bar_pos.at(now_bar);j++){
                    argv[j]=(char*)cmd.at(bar_pos.at(now_bar)+j+1).data();
                    //fprintf(stderr,"argv last %s %d\n",argv[j],bar_pos.at(last_bar));
                }
                argv[i-bar_pos.at(now_bar)]=NULL;
                if(execvp(argv[0],argv)<0){
                    //close(1);
                    fprintf(stderr,"Unknown command: [%s]\n",argv[0]);
                }
                exit(0);
            }else{
                close(p[now_pipe][0]);
                close(p[now_pipe][1]);
                if(last_pipe>=0){
                    close(p[last_pipe][0]);
                    close(p[last_pipe][1]);
                }
                if(!wait_proc.empty()){
                    for(int i=0;i<wait_proc.size();i++){
                        close(user_p_num[id][wait_proc[i]][0]);
                        close(user_p_num[id][wait_proc[i]][1]);
                    }
                }
                int wtr=waitpid(pid,&status,WUNTRACED | WCONTINUED);
                if (wtr == -1) {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }
                
                
            }   
        }
    }
    send(fd,(char*)"% ",strlen("% "),0);
}