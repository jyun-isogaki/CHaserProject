/*


                     TTPC, the template program for CHaser.
                            Version 0.9, 2013/08/02


              Copyright (c) 2013, Jun ISOGAKI, Ohta Technical Highschool.
              All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Ohta Technical Highschool nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL JUN ISOGAKI BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
  For detail, see followings.

    CHaser, Zenjyo-ken, programming contest.(Written in Japanese)
    http://www.zenjouken.com/?page_id=62

    It can be compiled by Microsoft Visual C++ series.
*/

/* Team name and server. */
#define TEAM_NAME "太田工業" /* Team name up to 4 byte. DBCS is acceptable.*/
#define DEFAULT_SERVER_ADDR "127.0.0.1"
#define DEFAULT_SERVER_PORT 2010

/* Suppressing strcpy() warnings */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

/* Prototype definition */

void create_sock(int argc,char *argv[]);
void send_team_name();
void turn_start();
void turn_done();
void send_get_ready();
void get_ret_value();

void walk_up();
void walk_down();
void walk_left();
void walk_right();
void look_up();
void look_down();
void look_left();
void look_right();
void search_up();
void search_down();
void search_left();
void search_right();
void put_up();
void put_down();
void put_left();
void put_right();

/* Data buffer for recv() call. */
#define BUFSIZE 2048

SOCKET sockfd;                      /* Socket descriptor. */

/*
    value[]: Control and side information.
        value[0]: control information.
            value: CONTROL_END('0'), CONTROL_PROGRESS('1').
        value[1]-value[9]: side information.
            value: OBJ_NONE('0'), OBJ_ENEMY('1'), OBJ_WALL('2').
            See CHaser specification for detail.
    WARNING: Please assign enough margin.
             It will be directly copied from socket buffer by strcpy().
*/
char value[BUFSIZE];

/* For value[0] */
#define CONTROL_END '0'
#define CONTROL_PROGRESS '1'
/* For value[1] - value[9] */
#define OBJ_NONE '0'
#define OBJ_ENEMY '1'
#define OBJ_WALL '2'

/*
   action_done: Debug purpose flag.
   It will be set when the turn is started.
   It will be unset when action(walk, put etc..) is done.
*/
int action_done=0;

int main(int argc, char *argv[]){

    /*
       COMMAND LINE EXAMPLE #1

          C:\CHaser> c201302.exe 192.168.0.1 2010

             argv[1]: Server IP address.  i.e.) 192.168.0.1
             argv[2]: Server port number. i.e.) 2010

       COMMAND LINE EXAMPLE #2

          C:\CHaser> c201302.exe

             argv[1]: Server IP address.  ex) 127.0.0.1
             argv[2]: Server port number. ex) 2010
    */

    /* Initialize winsock and connect server. */
    create_sock(argc,argv);

    /* Sending team name. */
    send_team_name();

    while(1){
        turn_start();
        send_get_ready();
        get_ret_value();    /* An array value[] will be updated. */

        if(value[0]==CONTROL_END){
            /* 制御情報が CONTROL_END('0') のときは試合終了。 */
            printf("end!\n");
            return 0;
        }

/*
        ここからアクションのロジック
##############################################################################
*/
        /*
           value[] の値を見て、walk_up() や put_down(), search_left()
           などを必ず1度呼び出すこと。過去の情報を参照しても良い。
           呼び出しが終わったら、action_endのラベルまでgotoすること。
        */

        if(value[2]!=OBJ_WALL){
            walk_up();
            goto action_end;
        }
        if (value[4]!=OBJ_WALL){
            walk_left();
            goto action_end;
        }
    	search_left();

action_end:

        get_ret_value();    /* the value[] will be updated. */

/*
##############################################################################
        ここまでアクションのロジック
*/

        if(value[0]==CONTROL_END){
            /* 制御情報 value[0]が CONTROL_END('0') のときは試合終了。 */
            printf("end\n");
            return 0;
        }
        turn_done(); /* ターン終了 */
    }
    closesocket(sockfd);    
    return 0;

}

void turn_start(){
    char    buf[BUFSIZE];
    /* Repeating 1 byte recv() call until receiving '@'. */
    int n;
    action_done=0;
    do{
        n = recv(sockfd, buf, 1, 0);
        buf[n] = '\0';
    }while(buf[0]!='@');
}

void send_get_ready(){
    /* Sending GetReady. */
    int send_ans;
    char command[BUFSIZE];
    strcpy((char *)command, "gr\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void get_ret_value(){
    /*
       get_ret_value(): Receiving command result.
       value[0]-value[9] will be updated.
    */
    int n;
    char    buf[BUFSIZE];
    Sleep(100);
    memset(buf, '*', sizeof(buf));
    n = recv(sockfd, buf, sizeof(buf), 0);
    buf[n] = '\0';
    if(n==12){
        strcpy(value,buf);
    } else if (n==23) {
        strcpy(value,&buf[11]);
    } else {
        printf("error n!=12 && n!=23\n");
    }
    printf("get_ret_value(): n=%d(\n%s\nvalue=%s)\n",n,buf,value);
}

void walk_up(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "wu\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void walk_down(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "wd\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void walk_left(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "wl\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void walk_right(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "wr\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}


void look_up(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "lu\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void look_down(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "ld\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void look_left(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "ll\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void look_right(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "lr\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void search_up(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "su\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void search_down(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "sd\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void search_left(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "sl\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void search_right(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "sr\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void put_up(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "pu\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void put_down(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "pd\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void put_left(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "pl\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void put_right(){
    char command[BUFSIZE];
    int send_ans;
    action_done=1;
    strcpy(command, "pr\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void turn_done(){
    char command[BUFSIZE];
    int send_ans;
    if(action_done==0){
        printf("warning: done() is executed before the action.\n");
    }
    strcpy(command, "#\r\n");
    send_ans = send(sockfd, command, strlen(command), 0);
}

void send_team_name(){
    int send_ans;
    send_ans = send(sockfd, TEAM_NAME"\r\n", strlen(TEAM_NAME"\r\n"), 0);
}

void create_sock(int argc, char *argv[]){
    struct sockaddr_in  client_addr;    /* sockaddr_in structure for socket. */
    static WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("socket : %d\n", WSAGetLastError());
        exit(1);
    }

    memset((char *)&client_addr, '\0', sizeof(client_addr));
    client_addr.sin_family = PF_INET;

    if(argc==3){
        /*
            The argument will be used if the server passed from command line.
        */
        client_addr.sin_addr.s_addr = inet_addr(argv[1]);
        client_addr.sin_port = htons(atoi(argv[2]));
    } else if(argc==1){
        client_addr.sin_addr.s_addr = inet_addr(DEFAULT_SERVER_ADDR);
        client_addr.sin_port = htons(DEFAULT_SERVER_PORT);
    }

    if (connect(sockfd, (struct sockaddr *)&client_addr, 
        sizeof(client_addr)) != 0) {
            printf("error: client connect");
            closesocket(sockfd);
            exit(1);
    }
}

