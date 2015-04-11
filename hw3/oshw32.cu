#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>

#define PAGESIZE 32
#define PHYSICAL_MEM_SIZE 32768
#define STORAGE_SIZE 131072
#define DATAFILE "./data.bin"
#define OUTFILE "./snapshot.bin"
typedef unsigned char uchar;
typedef uint32_t u32;
__device__ __managed__ int t=0;
__device__ __managed__ int PAGE_ENTRIES = 0;
__device__ __managed__ int PAGEFAULT = 0;
__device__ __managed__ uchar storage[STORAGE_SIZE];
__device__ __managed__ uchar results[STORAGE_SIZE];
__device__ __managed__ uchar input[STORAGE_SIZE];
__device__ __managed__ u32 counter[1024];

extern __shared__ u32 pt[];

__device__ u32 paging(uchar *buffer,u32 addr, uchar value)
{
	t = t+1;
	int i;

	for(i=0;i<1024;i++){
		if(pt[i]==addr){
			counter[i] = t;			
			return i*32+value; // hit
		}
	}
	for(i=0;i<1024;i++){
		if(pt[i]== -1){
				PAGEFAULT++;				
				counter[i] = t;
				pt[i]=addr;
				return i*32+value;
		}
	}
	
	    	
		int swap_index=0; 
		int min=counter[0];
		for(int j=0;j<1024;j++){
				if(counter[j]<min){
					min = counter[j];
					swap_index = j;
					
				}	
	 	}
		for(i = 0; i < 32; i++)	// swap
			{
				storage[addr*32 + i] = buffer[swap_index*32 + i];
				buffer[swap_index*32 + i] = input[addr*32 + i];
			}
		PAGEFAULT++;
		counter[swap_index]=t;
        pt[swap_index] = addr;
		return swap_index*32+value;
		
	
}


__device__ uchar Gread(uchar *buffer, u32 addr)
{
	u32 frame_num = addr/PAGESIZE;
	u32 offset = addr%PAGESIZE;
	
	addr = paging(buffer, frame_num, offset);
	return buffer[addr];
}


__device__ void Gwrite(uchar *buffer, u32 addr, uchar value)
{
	u32 frame_num = addr/PAGESIZE;
	u32 offset = addr%PAGESIZE;

	addr = paging(buffer , frame_num, offset);
	buffer[addr] = value;
}

__device__ void snapshot(uchar *results, uchar *buffer ,int offset ,int input_size)
{
	for(int i = 0; i < input_size; i++)
		results[i] = Gread(buffer, i+offset);
}


__device__ static void init_pageTable(int pt_entrie){

	for(int i=0;i<pt_entrie;i++){
		pt[i] = -1 ;
	}


}
__global__ void mykernel(int input_size)
{
	__shared__ uchar data[PHYSICAL_MEM_SIZE];
	
	int pt_entries = PHYSICAL_MEM_SIZE/PAGESIZE;
	init_pageTable(pt_entries);
	//####Gwrite/Gread code section start####
	for(int i = 0; i < input_size; i++)
		Gwrite(data,i,input[i]);
	for(int i = input_size - 1 ; i>= input_size - 10 ;i--)
		int value = Gread(data,i);
	snapshot(results,data,0,input_size);
	//####Gwrite/Gread code section end#### 
	printf("pagefault times = %d\n",PAGEFAULT);
	

} 

int load_binaryFile(const char *INPUT_FILE, uchar *input ,int storesize)
{


	FILE *DATA = fopen(INPUT_FILE,"rb");
	int size = 0;
	uchar in;
	while(fread(&in,sizeof(uchar),1,DATA)){
		input[size++] = in;
	}
	fclose(DATA);
	return size;

}


void write_binaryFile(const char *OUT_FILE, uchar *results ,int size){
	FILE *OUT = fopen(OUT_FILE,"wb");
    int i;
	
	for(i=0;i<size;i++){
		fwrite(&results[i],sizeof(uchar),1,OUT);
	}
    fclose(OUT);

}

int main(){
	
	for(int i=0;i<1024;i++)counter[i]=0;
	int input_size = load_binaryFile(DATAFILE, input,STORAGE_SIZE);
	cudaSetDevice(0);
	mykernel<<< 1, 1, 16384>>>(input_size);
	cudaDeviceSynchronize();
	cudaDeviceReset();
	write_binaryFile(OUTFILE,results,input_size);
	return 0;

}
