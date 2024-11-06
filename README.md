# Chat-like System
## Command
1. who : show information of all users.
2. tell : send a message to another user.
3. yell : send a message to all users.
4. name : change your name.
### shell
1. number : Add a number to each line of input.
2. removetag : Remove HTML tags and output to STDOUT.
3. removetag0 : Same as removetag, but outputs error messages to STDERR.

## Scenario
1. Welcome message
```
bash$ telnet 127.0.0.1 7000
***************************************
 ** Welcome to the information server **
***************************************            # Welcome message
```
2. who
```
*** User (no name) entered from 127.0.0.1:55494. ***
$ who
<ID>    <nickname>  <IP:port>           <indicate me>
1           (no name)  127.0.0.1           <- me
```
3. name
```
% name dennis
*** User from 127.0.0.1:55494 is named dennis. ***
```
4. Shell
```
% ls
bin
client
client.cpp
func.h
function.cpp
Makefile
np_shell.cpp
server
server.cpp
test.html
```
5. User 2 and User 3 logins
```
% *** User (no name) entered from 127.0.0.1:45262. ***   # user 2 logins
% who
<ID>    <nickname>  <IP:port>           <indicate me>
1           dennis  127.0.0.1           <- me
2           (no name)  127.0.0.1
% *** User from 127.0.0.1:45262 is named leo. ***  # user 2 inputs 'name leo'
% who
<ID>    <nickname>  <IP:port>           <indicate me>
1           dennis  127.0.0.1           <- me
2           leo  127.0.0.1
% *** User (no name) entered from 127.0.0.1:38648. ***   # user 3 logins
% who
<ID>    <nickname>  <IP:port>           <indicate me>
1           dennis  127.0.0.1           <- me
2           leo  127.0.0.1
3           (no name)  127.0.0.1
```
6. Chat command
```
% yell who knows how to do project2? help me plz!
*** dennis yelled ***: who knows how to do project2? help me plz!
% *** (no name) yelled ***: Sorry I don't know     # user 3 yells
% *** leo yelled ***: I know            # user 2 yells
% tell 2 Plz help me
% *** leo told you ***: yes, let me show you how to send files to you! # user 2 tell user 1
% *** leo (#2) just piped ’cat test.html >1’ to dennis (#1) ***
*** leo told you ***: you can 'cat <2' to show it!
```
7. User pipe
```
% cat <5
*** Error: user #5 does not exist yet. ***
% cat <2                       # receive from the user pipe
*** dennis (#1) just received from leo (#2) by ’cat <2’ ***
<!test.html>
<TITLE>Test</TITLE>
<BODY>This is a <b>test</b> program
for ras.
</BODY>
% tell 2 It's works! great!
% *** leo (#2) just piped ’number test.html >1’ to dennis (#1) ***
*** leo told you ***: you can receive by your program! try 'number <2' !
% number <2
*** dennis (#1) just received from leo (#2) by ’number <2’ ***
   1    1 <!test.html>
   2    2 <TITLE>Test</TITLE>
   3    3 <BODY>This is a <b>test</b> program
   4    4 for ras.
   5    5 </BODY>
% tell 2 cool! you're genius! thank you
% *** leo told you ***: you're welcome!
% *** User leo left. ***
% exit