#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8040

void printBoard(char board[3][3]) 
{
    printf("\n");
    for(int i=0;i<3;i++) 
    {
        for(int j=0;j<3;j++) 
        {
            printf(" %c ",board[i][j]);
            if(j<2) 
                printf("|");
        }
        if(i<2) 
            printf("\n---|---|---\n");
    }
    printf("\n");
}

void takePlayerInput(int* row,int* col) 
{
    printf("Enter your move (row and column): ");
    scanf("%d %d", row, col);
    *row-=1;
    *col-=1;
}

int main() 
{
    int sock=0;
    struct sockaddr_in serv_addr;
    char board[3][3];
    int playerNumber,turn_or_not;
    int row,col;

    if((sock=socket(AF_INET,SOCK_STREAM,0))<0) 
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if(connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0) 
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to the game server.\n");

    recv(sock,&playerNumber,sizeof(playerNumber),0);
    if(playerNumber==1) 
        printf("You are Player 1 (X).\n");
    else if(playerNumber==2) 
        printf("You are Player 2 (O).\n");

    while(1) 
    {
        recv(sock,board,sizeof(board),0);

        printBoard(board);

        recv(sock,&turn_or_not,sizeof(turn_or_not),0);

        if (turn_or_not==1) 
        {
            printf("It's your turn!\n");
            takePlayerInput(&row,&col);

            send(sock,&row,sizeof(row),0);
            send(sock,&col,sizeof(col),0);

        } 
        else 
            printf("Waiting for Player %d to move...\n", (playerNumber==1) ? 2 : 1);
        int valid=1;
        recv(sock,&valid,sizeof(valid),0);
        if(valid==0)
        {
            if(turn_or_not==1)
                printf("Invalid move ! Try again\n");
            continue;
        }
        int gameOver=0;
        recv(sock, &gameOver, sizeof(gameOver),0);
        if(gameOver) 
        {
            recv(sock,board,sizeof(board),0);
            printBoard(board);
            char outcome[50];
            recv(sock,outcome,sizeof(outcome),0);
            printf("%s\n",outcome);

            char replay_message[50];
            recv(sock,replay_message,sizeof(replay_message),0);
            int play_again=0;
            while(1)
            {
                printf("%s",replay_message);

                scanf("%d",&play_again);
                if(play_again==0 || play_again==1)
                {
                    send(sock,&play_again, sizeof(play_again), 0);
                    break;
                }
                else
                {
                    printf("Invalid Input\n");
                }
            }
            int other=0;
            recv(sock,&other,sizeof(other),0);
            if(other==0)
            {
                printf("Player %d wants to end the game\n",(playerNumber==1) ? 2 : 1);
                break;
            }
            if(play_again==0) 
                break;
        }
    }

    close(sock);
    return 0;
}
