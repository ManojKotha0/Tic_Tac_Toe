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
            board[i][j]=' ';
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

void handleGameOver(int socket,struct sockaddr_in addr1,struct sockaddr_in addr2,socklen_t addr_len,const char* result) 
{
    sendto(socket, result, strlen(result), 0, (struct sockaddr*)&addr1, addr_len);
    sendto(socket, result, strlen(result), 0, (struct sockaddr*)&addr2, addr_len);
}

int main() 
{
    int server_fd;
    struct sockaddr_in server_addr,client1_addr,client2_addr;
    socklen_t addr_len=sizeof(struct sockaddr_in);
    char buffer[1024]={0};
    int row, col;
    
    if((server_fd=socket(AF_INET,SOCK_DGRAM,0))==0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_port=htons(PORT);

    if(bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for players to connect...\n");
    int player1_number=1;
    int player2_number=2;

    recvfrom(server_fd,buffer,sizeof(buffer),0,(struct sockaddr*)&client1_addr,&addr_len);
    printf("Player 1 connected.\n");
    sendto(server_fd,&player1_number,sizeof(player1_number),0,(struct sockaddr*)&client1_addr,addr_len);

    recvfrom(server_fd,buffer,sizeof(buffer),0,(struct sockaddr*)&client2_addr,&addr_len);
    printf("Player 2 connected. Starting the game...\n");

    sendto(server_fd,&player2_number,sizeof(player2_number),0,(struct sockaddr*)&client2_addr,addr_len);
    int play_again=0;
    while(!play_again) 
    {
        intialize();
        int game_over=0;
        while(!game_over) 
        {
            sendto(server_fd,board,sizeof(board),0,(struct sockaddr*)&client1_addr,addr_len);
            sendto(server_fd,board,sizeof(board),0,(struct sockaddr*)&client2_addr,addr_len);

            if(currentPlayer==1) 
            {
                int ur_turn_or_not=1;
                sendto(server_fd,&ur_turn_or_not,sizeof(ur_turn_or_not),0,(struct sockaddr*)&client1_addr,addr_len);
                ur_turn_or_not=0;
                sendto(server_fd,&ur_turn_or_not,sizeof(ur_turn_or_not),0,(struct sockaddr*)&client2_addr,addr_len);

                recvfrom(server_fd,&row,sizeof(row),0,(struct sockaddr*)&client1_addr,&addr_len);
                recvfrom(server_fd,&col,sizeof(col),0,(struct sockaddr*)&client1_addr,&addr_len);
            } 
            else 
            {
                int ur_turn_or_not=1;
                sendto(server_fd,&ur_turn_or_not,sizeof(ur_turn_or_not),0,(struct sockaddr*)&client2_addr,addr_len);
                ur_turn_or_not=0;
                sendto(server_fd,&ur_turn_or_not,sizeof(ur_turn_or_not),0,(struct sockaddr*)&client1_addr,addr_len);

                recvfrom(server_fd,&row,sizeof(row),0,(struct sockaddr*)&client2_addr,&addr_len);
                recvfrom(server_fd,&col,sizeof(col),0,(struct sockaddr*)&client2_addr,&addr_len);
            }

            int valid=1;
            if (!validateMove(row,col)) 
            {
                valid=0;
                sendto(server_fd,&valid,sizeof(valid),0,(struct sockaddr*)&client1_addr,addr_len);
                sendto(server_fd,&valid,sizeof(valid),0,(struct sockaddr*)&client2_addr,addr_len);
                continue;
            }
            sendto(server_fd,&valid,sizeof(valid),0,(struct sockaddr*)&client1_addr,addr_len);
            sendto(server_fd,&valid,sizeof(valid),0,(struct sockaddr*)&client2_addr,addr_len);

            board[row][col]=(currentPlayer==1) ? 'X' : 'O';        

            if(winner_or_not()) 
            {
                printf("Player %d wins!\n",currentPlayer);
                char result[50];
                sprintf(result, "Player %d Wins!\n",currentPlayer);
                game_over=1;
                sendto(server_fd,&game_over,sizeof(game_over),0,(struct sockaddr*)&client1_addr,addr_len);
                sendto(server_fd,&game_over,sizeof(game_over),0,(struct sockaddr*)&client2_addr,addr_len);
                sendto(server_fd,board,sizeof(board),0,(struct sockaddr*)&client1_addr,addr_len);
                sendto(server_fd,board,sizeof(board),0,(struct sockaddr*)&client2_addr,addr_len);
                handleGameOver(server_fd,client1_addr,client2_addr, addr_len,result);
                break;
            } 
            else if(draw_or_not()) 
            {
                printf("It's a draw!\n");
                char result[50]="It's a Draw!\n";
                game_over=1;
                sendto(server_fd,&game_over,sizeof(game_over),0,(struct sockaddr*)&client1_addr,addr_len);
                sendto(server_fd,&game_over,sizeof(game_over),0,(struct sockaddr*)&client2_addr,addr_len);
                sendto(server_fd,board,sizeof(board),0,(struct sockaddr*)&client1_addr,addr_len);
                sendto(server_fd,board,sizeof(board),0,(struct sockaddr*)&client2_addr,addr_len);
                handleGameOver(server_fd,client1_addr,client2_addr,addr_len,result);
                break;
            }

            sendto(server_fd,&game_over,sizeof(game_over),0,(struct sockaddr*)&client1_addr,addr_len);
            sendto(server_fd,&game_over,sizeof(game_over),0,(struct sockaddr*)&client2_addr,addr_len);
            if(!game_over)
                switchPlayer();
        }

        sendto(server_fd,"Do you want to play again? (1 for yes, 0 for no): ",50,0,(struct sockaddr*)&client1_addr,addr_len);
        sendto(server_fd,"Do you want to play again? (1 for yes, 0 for no): ",50,0,(struct sockaddr*)&client2_addr,addr_len);
        int player1_play_again=0;
        int player2_play_again=0;
        recvfrom(server_fd,&player1_play_again,sizeof(player1_play_again),0,(struct sockaddr*)&client1_addr,&addr_len);
        recvfrom(server_fd,&player2_play_again,sizeof(player2_play_again),0,(struct sockaddr*)&client2_addr,&addr_len);
        sendto(server_fd,&player2_play_again,sizeof(player2_play_again),0,(struct sockaddr*)&client1_addr,addr_len);
        sendto(server_fd,&player1_play_again,sizeof(player1_play_again),0,(struct sockaddr*)&client2_addr,addr_len);
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
    close(server_fd);
    return 0;
}
