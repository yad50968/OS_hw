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
int row =12; 
int col= 30;

pthread_cond_t cv;
pthread_mutex_t mu;
int map[20][60];
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

		for(a=0;a<60;a++){
			if(abs(a-b)<10||abs(a-c)<10||abs(a-d)<10){
				map[id][a]=1;
				if(row==t){
					if(abs(col-b)<10||abs(col-c)<10||abs(col-d)<10){
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
		if(b<-10)b=rand()%11+60;
		if(c<-13) c=rand()%11+60;
		if(d<-8) d =rand()%11+60; 
		


		pthread_mutex_unlock(&mu);
		usleep(120000);
	}
	pthread_exit(NULL);

}
void threadtoright(void *t){
	int id = (int )t;
	int on=0;
	int a;
	int b=50;
	int c=30;
	int d=-4;
	while(end==0){
		
		pthread_mutex_lock(&mu);
			if(on==1){
			on =0;
			col++;
			if(col>59)end=1;
		}
		for(a=0;a<60;a++){
			if(abs(a-b)<7||abs(a-c)<7||abs(a-d)<7){   // can move to it 
				map[id][a]=1;
				if(row==id){
					if(abs(col-b)<7||abs(col-c)<7||abs(col-d)<7){ 
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
		if(b>65)b=rand()%10-20;
		if(c>63)c=rand()%10-20;
		if(d>66)d=rand()%10-20;    // difference thread should count it
		
		pthread_mutex_unlock(&mu);
		
		usleep(130000);
	
	
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
			if(row==0||row==12)
				map[row][col+1]=3;
			
		}
		else if(in =='D' && col <59){
			col = col+1;
			map[row][col]=2;
			if(row==0||row==12) 
				map[row][col-1]=3;
		}
		else if(in =='W' &&row>0){
			row = row -1;
			map[row][col]=2;
			if(row==11)
				map[row+1][col]=3;
			if(row==0){
				map[row+1][col]=1;
				win =1;
			}
				
		}
		else if(in =='S' &&row<12){
			row = row +1;
			map[row][col]=2;
			if(row==12)
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
	int a,b;int ke=0 ;
	tid = (int)threadid;
	while(end==0){
			pthread_mutex_lock(&mu);
			printf("\033[2J\033[0;0H");
			for(a=0;a<=12;a++){
				for(b=0;b<60;b++){
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
	if(end ==3){
		printf("\n\n\nQUIT IT\n");
		exit(1);
	}
	if(end ==1){
		printf("\n\n\nLOSE\n");
		exit(1);
	}
	if(end ==2){
		printf("\n\n\nWIN\n");

		exit(1);
	}
	pthread_mutex_unlock(&mu);
	pthread_exit(NULL);
}
int main(){
	int a;
	int b;
	for(b=0;b<=12;b++){
		if(b==0||b==12){
			for(a=0;a<60;a++){
				map[b][a]=3;
			}
		}
		else{
			for(a=30;a<60;a++)
				map[b][a]=0; 
		}
	
	}
	map[row][col]=2;
	pthread_t threads[12];
	pthread_mutex_init(&mu,NULL);
	pthread_cond_init(&cv,NULL);
	
	for(a=1;a<12;a++){
		if(a%2==0)
			pthread_create(&threads[a],NULL,threadtoleft,(void*)a);
		else
			pthread_create(&threads[a],NULL,threadtoright,(void*)a);
	}


	usleep(1000);	
	pthread_create(&threads[0],NULL,doprint,(void*)0);
	pthread_create(&threads[12],NULL,domove,(void*)0);

	for(a=0;a<12;a++){
		pthread_join(threads[a], NULL);

	}

	pthread_cond_destroy(&cv);
	pthread_mutex_destroy(&mu);
	pthread_exit(NULL);
	return 0;
}
