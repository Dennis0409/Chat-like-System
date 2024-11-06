#define MAX_USER 40
#define MAX_PIPE 10000
#include <vector>
#include <string>
#include <unordered_map>
extern int user_line[MAX_USER]; // number of user cmd
extern int user_p[MAX_USER][MAX_USER][2]; // user to user communicate
extern int user_p_check[MAX_USER][MAX_USER]; // check user to user pipe
extern int user_p_num[MAX_USER][MAX_PIPE][2]; // user wait pipe
extern int user_p_num_idx[MAX_USER]; // user wait pipe number
extern std::unordered_map<int,std::unordered_map<int,int>>user_wait_num; // [user][pipe_number][cmd]


extern int user[MAX_USER]; // user id
extern char* user_env[MAX_USER][100][2]; // user environment
extern int env_number[MAX_USER];// user environment value number
extern char* user_name[MAX_USER];// user name
extern char* user_ip[MAX_USER];// user ip
extern char* user_port[MAX_USER];// user port

int shell(int,int);
void broadcast(std::string);
int find_id(int);
void rename(std::string,int);
void yell(std::string,int);
int telll(std::string,int,int);
void exit_func(int);
void end(int);