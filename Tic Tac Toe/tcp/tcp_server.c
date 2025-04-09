#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8040

char board[3][3];
int currentPlayer=1;

void intialize() 
{
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            board[i][j] = ' ';
}

int winner_or_not() 
{
    for(int i=0;i<3;i++) 
    {
        if(board[i][0]==board[i][1] && board[i][1]==board[i][2] && board[i][0]!=' ') 
            return 1;
        if(board[0][i]==board[1][i] && board[1][i]==board[2][i] && board[0][i]!=' ') 
            return 1;
    }
    if(board[0][0]==board[1][1] && board[1][1]==board[2][2] && board[0][0]!=' ') 
        return 1;
    if(board[0][2]==board[1][1] && board[1][1]==board[2][0] && board[0][2]!=' ') 
        return 1;
    return 0;
}

int draw_or_not() 
{
    for(int i=0;i<3;i++) 
        for(int j=0;j<3;j++) 
            if(board[i][j]==' ') 
                return 0;
    return 1;
}

void switchPlayer() 
{
    currentPlayer=(currentPlayer%2)+1;
}

int validateMove(int row,int col) 
{
    if(row<0 || row>=3 || col<0 || col>=3) 
        return 0;    
    if(board[row][col]!=' ') 
        return 0;
    return 1;
}

void handleGameOver(int socket1,int socket2,const char* result) 
{
    send(socket1,result,strlen(result),0);
    send(socket2,result,strlen(result),0);
}



int main() 
{
    int server_fd,player1_socket,player2_socket,valread;
    struct sockaddr_in address;
    int addrlen=sizeof(address);
    int row,col;

    if((server_fd=socket(AF_INET,SOCK_STREAM,0))==0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(PORT);

    if(bind(server_fd,(struct sockaddr*)&address,sizeof(address))<0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd,2)<0) 
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for players to connect...\n");

    if((player1_socket=accept(server_fd,(struct sockaddr *)&address,(socklen_t*)&addrlen))<0) 
    {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Player 1 connected.\n");

    if((player2_socket=accept(server_fd,(struct sockaddr*)&address,(socklen_t*)&addrlen))<0) 
    {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Player 2 connected. Starting the game...\n");

    int player1_number=1;
    int player2_number=2;
    send(player1_socket,&player1_number,sizeof(player1_number),0);
    send(player2_socket,&player2_number,sizeof(player2_number),0);
    int play_again=0;
    while(!play_again)
    {
        intialize();

        int game_over=0;
        while(!game_over) 
        {
            send(player1_socket,board,sizeof(board),0);
            send(player2_socket,board,sizeof(board),0);

            if(currentPlayer==1) 
            {
                int ur_turn_or_not=1;
                send(player1_socket,&ur_turn_or_not,sizeof(ur_turn_or_not),0);
                ur_turn_or_not=0;
                send(player2_socket,&ur_turn_or_not, sizeof(ur_turn_or_not),0);

                valread=recv(player1_socket,&row,sizeof(row),0);
                valread=recv(player1_socket,&col,sizeof(col),0);
            }
            else 
            {
                int ur_turn_or_not=1;
                send(player2_socket,&ur_turn_or_not,sizeof(ur_turn_or_not),0);
                ur_turn_or_not=0; 
                send(player1_socket,&ur_turn_or_not,sizeof(ur_turn_or_not),0);

                valread=recv(player2_socket,&row,sizeof(row),0);
                valread=recv(player2_socket,&col,sizeof(col),0);
            }

            int valid=1;
            if(!validateMove(row,col)) 
            {
                valid=0;
                send(player1_socket,&valid,sizeof(valid),0);
                send(player2_socket,&valid,sizeof(valid),0);
                continue;
            }
            send(player1_socket,&valid,sizeof(valid),0);
            send(player2_socket,&valid,sizeof(valid),0);

            board[row][col]=(currentPlayer==1)?'X':'O';

            if(winner_or_not()) 
            {
                printf("Player %d wins!\n",currentPlayer);
                char result[50];
                sprintf(result, "Player %d Wins!\n", currentPlayer);
                game_over=1;
                send(player1_socket,&game_over,sizeof(game_over),0);
                send(player2_socket,&game_over, sizeof(game_over),0);
                send(player1_socket,board, sizeof(board),0);
                send(player2_socket,board, sizeof(board),0);
                handleGameOver(player1_socket,player2_socket,result);
                break;
            } 
            else if(draw_or_not()) 
            {
                printf("It's a draw!\n");
                char result[50]="It's a Draw!\n";
                game_over=1;
                send(player1_socket,&game_over,sizeof(game_over),0);
                send(player2_socket,&game_over,sizeof(game_over),0);
                send(player1_socket,board,sizeof(board),0);
                send(player2_socket,board,sizeof(board),0);
                handleGameOver(player1_socket,player2_socket,result);
                break;
            }

            send(player1_socket,&game_over,sizeof(game_over),0);
            send(player2_socket,&game_over,sizeof(game_over),0);
            if(!game_over) 
                switchPlayer();
        }
        send(player1_socket,"Do you want to play again? (1 for yes, 0 for no): ",50,0);
        send(player2_socket,"Do you want to play again? (1 for yes, 0 for no): ",50,0);
        int player1_play_again=0;
        int player2_play_again=0;
        recv(player1_socket,&player1_play_again,sizeof(player1_play_again),0);
        recv(player2_socket,&player2_play_again,sizeof(player2_play_again),0);
        send(player1_socket,&player2_play_again,sizeof(player2_play_again),0);
        send(player2_socket,&player1_play_again,sizeof(player1_play_again),0);
        if(player1_play_again==1 && player2_play_again==1)
        {
            printf("Both player want to play another game. Restarting game...\n");
            play_again=0;
        }
        else if(player1_play_again==0 && player2_play_again==0)
        {
            printf("Both player want to end the game.\n");
            play_again=1;
        }
        else
        {
            printf("Player %d wants to end the game.\n",(player1_play_again==0) ? 1 : 2);
            play_again=1;
        }
    }
    close(player1_socket);
    close(player2_socket);
    close(server_fd);

    return 0;
}