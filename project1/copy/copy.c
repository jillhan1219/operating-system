
#include<stdio.h>
#include<stdlib.h>// atoi()
#include<unistd.h>//for fork() pipe() sleep() etc.
#include<sys/wait.h>//waitpid() wait() WIFEXITED() WEXITATUS() etc.
#include<time.h>
#include<fcntl.h>//open() etc.

int main(int argc, char *argv[]){
	if(argc!=4){
		fprintf(stderr,"useage:%s <InputFile> <OutputFile> <BufferSize>\n",argv[0]);
		exit(1);
	}

	char* source_file=argv[1];
	char* destination_file=argv[2];
	int buffer_size=atoi(argv[3]);
	printf("buffer size:%d\n",buffer_size);

	//start timer
	clock_t start_time=clock();

	//create the pipe
	int pipefd[2];
	if(pipe(pipefd)==-1){
		perror("pipe");
		exit(1);
	}

	//fork a child process for writing to the destination file
	pid_t pid=fork();
	if(pid==-1){
		perror("fork");
		exit(1);
	}
	else if(pid==0){
		//child process
		close(pipefd[1]);//close the write end of the pipe
		FILE *output_file=fopen(destination_file,"w+");
		if(output_file==NULL){
			perror("fopen");
			exit(1);
		}
		char buffer[buffer_size];
		int n;
		while((n=read(pipefd[0],buffer,buffer_size))>0){
			if(fwrite(buffer,1,n,output_file)!=n){
				perror("fwrite");
				exit(1);
			}
		}

		printf("read successfully\n");

		fclose(output_file);
		close(pipefd[0]);
		exit(0);
	}
	else{
		FILE *input_file=fopen(source_file,"r");
		if(input_file==NULL){
			perror("fopen");
			exit(1);
		}
		char buffer[buffer_size];
		int n;
		while((n=fread(buffer,1,buffer_size,input_file))>0){
			if(write(pipefd[1],buffer,n)!=n){
				perror("write");
				exit(1);
			}
		}

		printf("write successfully\n");

		fclose(input_file);
		close(pipefd[1]);

		int status;
		waitpid(pid,&status,0);
		if(!WIFEXITED(status)||WEXITSTATUS(status)!=0){
			fprintf(stderr,"error:child process for writing to destination file failed.\n");
			exit(1);
		}
	}

	clock_t end_time=clock();

	double elapsed_time=(double)(end_time-start_time)/CLOCKS_PER_SEC;

	printf("Time used:%f seconds\n",elapsed_time);

	return 0;
}
