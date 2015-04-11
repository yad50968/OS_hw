#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<math.h>
#include<unistd.h>
#include <termios.h>
int lastrow,lastcol;
int win=0;
int end=0;
int row =5; 
int col= 15;

pthread_cond_t cv;
pthread_mutex_t mu;
int map[6][30];
int getch (void)
{
    int ch;
    struct termios oldt, newt;
 
    tcgetattr(STDIN_FILENO, &oldt);
    memcpy(&newt, &oldt, sizeof(newt));
    newt.c_lflag &= ~( ECHO | ICANON | ECHOE | ECHOK |
                       ECHONL | ECHOPRT | ECHOKE | ICRNL);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
 
    return ch;
}
void threadtoleft(void *t){
	int id = (int)t;
	int a;
	int b=35;
	int c=20;
	int d=5;
	int on=0;
	while(end==0){
		
		pthread_mutex_lock(&mu);
		if(on==1){
			on =0; 
			col--;
			if(col<0)end=1;
		}

		for(a=0;a<30;a++){
			if(abs(a-b)<6||abs(a-c)<6||abs(a-d)<6){
				map[id][a]=1;
				if(row==t){
					if(abs(col-b)<6||abs(col-c)<6||abs(col-d)<6){
						map[id][col] = 2;
						on =1;
					}
					else
						end =1;
				}
				
			}
			else 
				map[id][a]=0;
		}
		b--; c--; d--;
		if(b<-5)b=35;
		if(c<0) c=35;
		if(d<-3) d =35; 
		
		pthread_mutex_unlock(&mu);
		usleep(rand()%10*10000);
	}
	pthread_exit(NULL);

}
void threadtoright(void *t){
	int id = (int )t;
	int on=0;
	int a;
	int b=20;
	int c=10;
	int d=-4;
	while(end==0){
		
		pthread_mutex_lock(&mu);
			if(on==1){
			on =0;
			col++;
			if(col>29)end=1;
		}
		for(a=0;a<30;a++){
			if(abs(a-b)<4||abs(a-c)<4||abs(a-d)<4){   // can move to it 
				map[id][a]=1;
				if(row==id){
					if(abs(col-b)<4||abs(col-c)<4||abs(col-d)<4){ 
					 //i on it 
					map[id][col] = 2;
					on =1;
					}
					
					else
						end =1;
				}
			}
			else 
				map[id][a]=0;
		}		
		b++;
		c++;
		d++;
		if(b>30)b=0;
		if(c>33)c=-3;
		if(d>36)d=-6;    // difference thread should count it
		
		pthread_mutex_unlock(&mu);
		
		usleep(rand()%10*10000);
	
	
	}
	pthread_exit(NULL);

}
void domove(void  *threadid){
	char in;
	int d;
	while(end==0){
		in = getch();

		if(in =='A' && col>0){ 
			col=col-1;
			map[row][col]=2;
			if(row==0||row==5)
				map[row][col+1]=3;
			
		}
		else if(in =='D' && col <29){
			col = col+1;
			map[row][col]=2;
			if(row==0||row==5) 
				map[row][col-1]=3;
		}
		else if(in =='W' &&row>0){
			row = row -1;
			map[row][col]=2;
			if(row==4)
				map[row+1][col]=3;
			if(row==0){
				map[row+1][col]=1;
				win =1;
			}
				
		}
		else if(in =='S' &&row<5){
			row = row +1;
			map[row][col]=2;
			if(row==5)
				col = col-1;
		
		}
		else if(in =='Q')
			end =3;

		usleep(10000);
	}
	pthread_exit(NULL);
}
void doprint(void *threadid){
	int tid;
	int a,b;
	tid = (int)threadid;
	while(end==0){
			pthread_mutex_lock(&mu);
			printf("\033[2J\033[0;0H");
		
			for(a=0;a<6;a++){
				for(b=0;b<30;b++){
					if(map[a][b]==0)
						printf(" ");
					else if(map[a][b]==1) 
						printf("=");
					else if(map[a][b]==2){
						printf("o");}
					else if(map[a][b]==3)
						printf("+");
				}
				printf("\n");
			}
			if(win==1)
				end=2;
		pthread_mutex_unlock(&mu);
		usleep(10000);
	}
	pthread_mutex_lock(&mu);
	if(end ==3)
		printf("\n\n\nQUIT IT\n");
	if(end ==1)
		printf("\n\n\nLOSE\n");
	if(end ==2)
		printf("\n\n\nWIN\n");
	pthread_mutex_unlock(&mu);
	pthread_exit(NULL);
}
int main(){
	int a;
	int b;
	for(a=0;a<30;a++){
		map[0][a]=3;
		map[1][a]=0; map[2][a]=0;;map[3][a]=0;map[4][a]=0;
		map[5][a]=3;		
		map[row][col] = 2;
	}
	pthread_t threads[7];
	pthread_mutex_init(&mu,NULL);
	pthread_cond_init(&cv,NULL);
	pthread_create(&threads[1],NULL,threadtoleft,(void*)1);
	pthread_create(&threads[2],NULL,threadtoright,(void*)2);
	pthread_create(&threads[3],NULL,threadtoleft,(void*)3);
	pthread_create(&threads[4],NULL,threadtoright,(void*)4);
	usleep(1000);	
	pthread_create(&threads[5],NULL,doprint,(void*)0);
	pthread_create(&threads[6],NULL,domove,(void*)0);

	for(a=1;a<=6;a++){
		pthread_join(threads[a], NULL);

	}

	pthread_cond_destroy(&cv);
	pthread_mutex_destroy(&mu);
	pthread_exit(NULL);
	return 0;
}
