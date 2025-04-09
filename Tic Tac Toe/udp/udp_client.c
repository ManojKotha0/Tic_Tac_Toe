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
    struct sockaddr_in server_addr;
    socklen_t addr_len=sizeof(struct sockaddr_in);
    char board[3][3];
    int playerNumber,turn_or_not;
    int row,col;

    if((sock=socket(AF_INET,SOCK_DGRAM,0))<0) 
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(PORT);

    if(inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    printf("Connected to the game server.\n");

    sendto(sock,"connected",strlen("connected"),0,(struct sockaddr*)&server_addr,addr_len);
    recvfrom(sock,&playerNumber,sizeof(playerNumber),0,(struct sockaddr*)&server_addr,&addr_len);
    while(1) 
    {
        recvfrom(sock,board,sizeof(board),0,(struct sockaddr*)&server_addr,&addr_len);
        printBoard(board);
    
        recvfrom(sock,&turn_or_not,sizeof(turn_or_not),0,(struct sockaddr*)&server_addr,&addr_len);

        if(turn_or_not==1) 
        {
            printf("It's your turn!\n");
            takePlayerInput(&row,&col);
            sendto(sock,&row,sizeof(row),0,(struct sockaddr*)&server_addr,addr_len);
            sendto(sock,&col,sizeof(col),0,(struct sockaddr*)&server_addr,addr_len);
        } 
        else 
            printf("Waiting for the other player to move...\n");

        int valid=1;
        recvfrom(sock,&valid,sizeof(valid),0,(struct sockaddr*)&server_addr,&addr_len);
        if(valid==0) 
        {
            if(turn_or_not==1)
                printf("Invalid move! Try again\n");
            continue;
        }

        int gameOver=0;
        recvfrom(sock,&gameOver,sizeof(gameOver),0,(struct sockaddr*)&server_addr,&addr_len);
        if(gameOver)           
        {
            recvfrom(sock,board,sizeof(board),0,(struct sockaddr*)&server_addr,&addr_len);
            printBoard(board);
            char outcome[50];
            recvfrom(sock,outcome,sizeof(outcome),0,(struct sockaddr*)&server_addr,&addr_len);
            printf("%s\n",outcome);

            char replay_message[50];
            recvfrom(sock,replay_message,sizeof(replay_message),0,(struct sockaddr*)&server_addr,&addr_len);
            int play_again;
            while(1)
            {
                printf("%s",replay_message);

                scanf("%d",&play_again);
                if(play_again==0 || play_again==1)
                {
                    sendto(sock,&play_again,sizeof(play_again),0,(struct sockaddr*)&server_addr,addr_len);
                    break;
                }
                else
                {
                    printf("Invalid Input\n");
                }
            }
            int other=0;
            recvfrom(sock,&other,sizeof(other),0,(struct sockaddr*)&server_addr,&addr_len);
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
