#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <strings.h>
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include <sys/time.h>

using namespace std;

#define BUFFSIZE 256
#define TIMEOUT 1
const char * SERVER_USER =  "100 LOGIN\r\n";
const char * SERVER_PASSWORD =  "101 PASSWORD\r\n";
const char * SERVER_OK = "200 OK\r\n";
const char * SERVER_LOGIN_FAILED = "300 LOGIN FAILED\r\n";
const char * SERVER_MOVE = "102 MOVE\r\n";
const char * SERVER_PICK_UP = "105 GET MESSAGE\r\n";
const char * SERVER_TURN_LEFT = "103 TURN LEFT\r\n";
const char * SERVER_TURN_RIGHT = "104 TURN RIGHT\r\n";
const char * SERVER_SYNTAX_ERROR = "301 SYNTAX ERROR\r\n";
const char * CLIENT_RECHARGING = "RECHARGING\r\n";
const char * CLIENT_FULL_POWER = "FULL POWER\r\n";
const char * SERVER_LOGIC_ERROR = "302 LOGIC ERROR\r\n";

//bzero(buffer,BUFFSIZE);

void null(char buffer[], uint & bufflen){
    uint i = 0;
    while(buffer[i] != '\r' || buffer[i + 1] != '\n'){
        buffer[i++] = 0;
    }
    buffer[i++] = 0;
    buffer[i++] = 0;
    uint pos = i;
    int tmp = i;
    for (i = 0; pos < bufflen; i++){
        buffer[i] = buffer[pos++];
    }
    while (i < BUFFSIZE){
        buffer[i++] = '\0';
    }
    bufflen -= tmp;
}

void getBuf(char get[], char buffer[], uint & received, uint & bufflen){
    for (uint i = 0; i < received; i++){
        buffer[bufflen++] = get[i];
    }
    received = 0;
}

bool getMessage(char buffer[], string & message, uint & bufflen){
    uint i;
    for (i = 0; i < bufflen - 1; i++){
        if (buffer[i] == '\r' && buffer[i + 1] == '\n'){
            for (uint j = 0; j < i + 2; j++)
                message += buffer[j];
            null(buffer, bufflen);
            return true;
        }
    }
    return false;
}

uint getASCII(string & name){
    uint ascii = 0;
    for (auto it = name.begin(); it != name.end() - 2; ++it)
        ascii += (uint)(*it);
    return ascii;
}

bool getCoordinates(string & message, int & x, int & y){
    auto it = message.begin() + 3;
    string xstr, ystr;
    while (*it != ' '){
        if(((int) *it < 48 || (int) *it > 57) && (int) *it != 45)
            return true;
        xstr += *it++;
    }
    it++;
    while (*it != '\r'){
        if(((int) *it < 48 || (int) *it > 57) && (int) *it != 45)
            return true;
        ystr += *it++;
    }
    x = stoi(xstr);
    y = stoi(ystr);
    return false;
}

bool getDirection(int & direction, int x, int y, int xtmp, int ytmp){
    if (x == xtmp && y == ytmp) return false;
    if (y > ytmp){direction = 0; return true;}
    if (x > xtmp){direction = 1; return true;}
    if (y < ytmp){direction = 2; return true;}
    else{direction = 3; return true;}
}

bool checkOK(string & message){
    if (message.at(0) != 'O' || message.at(1) != 'K' || message.at(2) != ' ')
        return true;
    return false;
}

bool checkBufferForPass(char buffer[], uint bufflen, uint max){
    for (uint i = 0; i < max - 1; i++){
        //cout << buffer[i] << " " << buffer[i+1] << endl;
        if (buffer[i] == '\r' && buffer[i + 1] == '\n')
            return false;
    }
    return true;
}


bool checkRecharge(int newsockfd, char buffer[], uint & bufflen, string & message, char get[],
                    uint received, fd_set & rfds, struct timeval & tv, bool & closed, bool & repeat){
    if (strcmp(message.c_str(), CLIENT_RECHARGING) == 0){
        message.clear();
        fd_set rfds;
        struct timeval tv;
        int retval;
        while (1){
            tv.tv_sec = 5;
            tv.tv_usec = 0;
            FD_ZERO(&rfds);
            FD_SET(newsockfd, &rfds);
            retval = select(newsockfd + 1, &rfds, NULL, NULL, &tv);
            if (bufflen > 11 && checkBufferForPass(buffer, bufflen, 12)){
                send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
                close(newsockfd);
                closed = true;
                return true;
            }
            if (bufflen != 0 && getMessage(buffer, message, bufflen))
                break;
            if(retval <= 0){
                cout << "zaviram" << endl;
                close(newsockfd);
                closed = true;
                return true;
            }
            received = recv(newsockfd, get, BUFFSIZE, 0);
            getBuf(get, buffer, received, bufflen);
        }
        cout << message << " " << CLIENT_FULL_POWER << endl;
        if (strcmp(message.c_str(), CLIENT_FULL_POWER) == 0){
            repeat = true;
            cout << "ahojky" << endl;
            return false;
        }else{
            send(newsockfd, SERVER_LOGIC_ERROR, strlen(SERVER_LOGIC_ERROR), 0);
            cout << "zaviram na SYNTAX" << endl;
            close(newsockfd);
            closed = true;
            return true;
        }
    }
    return false;
}

bool receiveMessage(int newsockfd, char buffer[], uint & bufflen, string & message, char get[],
                    uint received, fd_set & rfds, struct timeval & tv, bool & closed){
    message.clear();
    bool repeat = false;
    int retval;
    while (1){
        while (1){
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            FD_ZERO(&rfds);
            FD_SET(newsockfd, &rfds);
            retval = select(newsockfd + 1, &rfds, NULL, NULL, &tv);
            cout << "1: " << retval << endl;
            if (bufflen > 11 && checkBufferForPass(buffer, bufflen, 12)){
                send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
                close(newsockfd);
                closed = true;
                return true;
            }
            if (bufflen != 0 && getMessage(buffer, message, bufflen))
                break;
            if (message.size() > 3 && checkOK(message))
                return true;
            cout << "2: " << retval << endl;
            if(retval <= 0){
                cout << "zaviram" << endl;
                close(newsockfd);
                closed = true;
                return true;
            }
            received = recv(newsockfd, get, BUFFSIZE, 0);
            getBuf(get, buffer, received, bufflen);
        }
        if (!checkRecharge(newsockfd, buffer, bufflen, message, get, received, rfds, tv, closed, repeat)){
            if (!repeat)
                break;
            else{
                message.clear();
                repeat = false;
                cout << "ahoj" << endl;
            }
        }else
            return true;                
    }
    return false;
}

void final(int newsockfd, char buffer[], uint & bufflen, fd_set & rfds, timeval & tv, bool & closed){
    string message;
    char get[BUFFSIZE];
    uint received = 0;
    bool repeat = false;
    send(newsockfd, SERVER_PICK_UP, strlen(SERVER_PICK_UP), 0);
    while (1){
        while (1){
            if (bufflen > 99 && checkBufferForPass(buffer, bufflen, 100)){
                send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
                close(newsockfd);
                return;            
            }
            if (bufflen != 0 && getMessage(buffer, message, bufflen))
                break;
            received = recv(newsockfd, get, BUFFSIZE, 0);
            getBuf(get, buffer, received, bufflen);
        }
        if (!checkRecharge(newsockfd, buffer, bufflen, message, get, received, rfds, tv, closed, repeat)){
            if (!repeat)
                break;
            else{
                message.clear();
                repeat = false;
            }
        }else
            return;        
    }
    send(newsockfd, SERVER_OK, strlen(SERVER_OK), 0);
    close(newsockfd);
}

void rotate(int newsockfd, int & direction, int rigth, int left, int doubleR, int result,
            char buffer[], uint & bufflen, string & message, char get[], uint received,
            fd_set & rfds, timeval & tv, bool & closed){
    int x, y;
    if (direction == rigth){
        send(newsockfd, SERVER_TURN_RIGHT, strlen(SERVER_TURN_RIGHT), 0);
        if (receiveMessage(newsockfd, buffer, bufflen, message, get, received, rfds, tv, closed) || getCoordinates(message, x, y)){
            if (closed)
                return;
            send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
            close(newsockfd);
            return;                    
        }
    }else if (direction == doubleR){
        send(newsockfd, SERVER_TURN_RIGHT, strlen(SERVER_TURN_RIGHT), 0);
        if (receiveMessage(newsockfd, buffer, bufflen, message, get, received, rfds, tv, closed) || getCoordinates(message, x, y)){
            if (closed)
                return;
            send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
            close(newsockfd);
            return;                    
        }
        send(newsockfd, SERVER_TURN_RIGHT, strlen(SERVER_TURN_RIGHT), 0);
        if (receiveMessage(newsockfd, buffer, bufflen, message, get, received, rfds, tv, closed) || getCoordinates(message, x, y)){
            if (closed)
                return;
            send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
            close(newsockfd);
            return;                    
        }
    }else if (direction == left){
        send(newsockfd, SERVER_TURN_LEFT, strlen(SERVER_TURN_LEFT), 0);
        if (receiveMessage(newsockfd, buffer, bufflen, message, get, received, rfds, tv, closed) || getCoordinates(message, x, y)){
            if (closed)
                return;
            send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
            close(newsockfd);
            return;                    
        }
    }
    direction = result;
}

bool checkPass(string & password){
    if (password.size() > 7)
        return true;
    for (auto it = password.begin(); it != password.end() - 2; ++it)
        if ((int)*it < 48 || (int)*it > 57)
            return true;
    if (password.at(password.size() - 1) != '\n' || password.at(password.size() - 2) != '\r')
        return true;
    return false;
}

void oneRobot (int newsockfd){
	char get[BUFFSIZE];
	char buffer[1000];
    uint received = 0;
    uint bufflen = 0;
    bool closed = false;
    bool repeat = false;
    int retval;

    fd_set rfds;
    struct timeval tv;       
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    //PRIHLASENI
	string name;
    string password;
	send(newsockfd, SERVER_USER, strlen(SERVER_USER), 0);

    while (1){
        while (1){
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            FD_ZERO(&rfds);
            FD_SET(newsockfd, &rfds);
            retval = select(newsockfd + 1, &rfds, NULL, NULL, &tv);
            if (bufflen > 99 && checkBufferForPass(buffer, bufflen, 100)){
                send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
                close(newsockfd);
                return;            
            }
            if (bufflen != 0 && getMessage(buffer, name, bufflen))
                break;
            if(retval <= 0){
                cout << "zaviram" << endl;
                close(newsockfd);
                return;                    
            }
            received = recv(newsockfd, get, BUFFSIZE, 0);
            getBuf(get, buffer, received, bufflen);
        }
        if (!checkRecharge(newsockfd, buffer, bufflen, name, get, received, rfds, tv, closed, repeat)){
            if (!repeat)
                break;
            else{
                name.clear();
                repeat = false;
            }
        }else
            return;        
    }
    uint validPass = getASCII(name);
    cout << validPass << endl;
    send(newsockfd, SERVER_PASSWORD, strlen(SERVER_PASSWORD), 0);
    while (1){
        while (1){
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            FD_ZERO(&rfds);
            FD_SET(newsockfd, &rfds);
            retval = select(newsockfd + 1, &rfds, NULL, NULL, &tv);
            cout << "1: " << retval << endl;
            if (bufflen > 11 && checkBufferForPass(buffer, bufflen, 12)){
                send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
                close(newsockfd);
                return;            
            }
            if (bufflen != 0 && getMessage(buffer, password, bufflen))
                break;
            cout << "0: " << retval << endl;
            if(retval <= 0){
                cout << "zaviram" << endl;
                close(newsockfd);
                return;                    
            }
            received = recv(newsockfd, get, BUFFSIZE, 0);
            getBuf(get, buffer, received, bufflen);
        }
        if (!checkRecharge(newsockfd, buffer, bufflen, password, get, received, rfds, tv, closed, repeat)){
            if (!repeat)
                break;
            else{
                password.clear();
                repeat = false;
            }
        }else
            return;
    }
    if (checkPass(password)){
        send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
        close(newsockfd);
        return;                    
    }
    uint pass = stoi(password);
    cout << pass << endl;
    if (pass != validPass){
        send(newsockfd, SERVER_LOGIN_FAILED, strlen(SERVER_LOGIN_FAILED), 0);
        close(newsockfd);
        return;
    }
    send(newsockfd, SERVER_OK, strlen(SERVER_OK), 0);



    //ZISKANI STARTOVNI POZICE
    int x, y, xtmp, ytmp;
    int direction;
    string message;
    send(newsockfd, SERVER_MOVE, strlen(SERVER_MOVE), 0);
    if (receiveMessage(newsockfd, buffer, bufflen, message, get, received, rfds, tv, closed) || getCoordinates(message, xtmp, ytmp)){
        if (closed)
            return;
        send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
        close(newsockfd);
        return;                    
    }
    if (xtmp == 0 && ytmp == 0){
        final(newsockfd, buffer, bufflen, rfds, tv, closed);
        return;
    }
    while(1){
        message.clear();
        send(newsockfd, SERVER_MOVE, strlen(SERVER_MOVE), 0);
        if (receiveMessage(newsockfd, buffer, bufflen, message, get, received, rfds, tv, closed) || getCoordinates(message, x, y)){
            if (closed)
                return;
            send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
            close(newsockfd);
            return;                    
        }
        if (getDirection(direction, x, y, xtmp, ytmp))
            break;
    }

    //JED DOLEVA
    if (x > 0 && direction != 3){
        rotate(newsockfd, direction, 2, 0, 1, 3, buffer, bufflen, message, get, received, rfds, tv, closed);
        if (closed)
            return;
    }
    while(x > 0){
        send(newsockfd, SERVER_MOVE, strlen(SERVER_MOVE), 0);
        if (receiveMessage(newsockfd, buffer, bufflen, message, get, received, rfds, tv, closed) || getCoordinates(message, x, y)){
            if (closed)
                return;
            send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
            close(newsockfd);
            return;                    
        }
    }

    //JED DOPRAVA
    if (x < 0 && direction != 1){
        rotate(newsockfd, direction, 0, 2, 3, 1, buffer, bufflen, message, get, received, rfds, tv, closed);
        if (closed)
            return;
    }
    while(x < 0){
        send(newsockfd, SERVER_MOVE, strlen(SERVER_MOVE), 0);
        if (receiveMessage(newsockfd, buffer, bufflen, message, get, received, rfds, tv, closed) || getCoordinates(message, x, y)){
            if (closed)
                return;
            send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
            close(newsockfd);
            return;                    
        }
    }

    //JED NAHORU
    if (y < 0 && direction != 0){
        rotate(newsockfd, direction, 3, 1, 2, 0, buffer, bufflen, message, get, received, rfds, tv, closed);
        if (closed)
            return;
    }
    while(y < 0){
        send(newsockfd, SERVER_MOVE, strlen(SERVER_MOVE), 0);
        if (receiveMessage(newsockfd, buffer, bufflen, message, get, received, rfds, tv, closed) || getCoordinates(message, x, y)){
            if (closed)
                return;
            send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
            close(newsockfd);
            return;                    
        }
    }

    //JED DOLU
    if (y > 0 && direction != 2){
        rotate(newsockfd, direction, 1, 3, 0, 2, buffer, bufflen, message, get, received, rfds, tv, closed);
        if (closed)
            return;
    }
    while(y > 0){
        send(newsockfd, SERVER_MOVE, strlen(SERVER_MOVE), 0);
        if (receiveMessage(newsockfd, buffer, bufflen, message, get, received, rfds, tv, closed) || getCoordinates(message, x, y)){
            if (closed)
                return;
            send(newsockfd, SERVER_SYNTAX_ERROR, strlen(SERVER_SYNTAX_ERROR), 0);
            close(newsockfd);
            return;                    
        }
    }

    //VEM ZPRAVU
    final(newsockfd, buffer, bufflen, rfds, tv, closed);
    return;
}

int main(int argc, char * argv[]){
	int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    stringstream strValue;
    strValue << argv[1];
    unsigned int intValue;
    strValue >> intValue;

    portno = intValue;

    sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);

    bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    listen(sockfd,5);

    while(1){
    	clilen = sizeof(cli_addr);
    	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    	if (newsockfd > 0){
            pid_t pid = fork();
            if (pid == 0){
                close(sockfd);
                oneRobot(newsockfd);
            }
            close(newsockfd);
        }
	}
    close(newsockfd);
	return 0;
}