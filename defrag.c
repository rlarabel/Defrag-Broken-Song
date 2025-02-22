#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


void* traverseDirectory(void *);

// Shared Data
char** frags;
size_t length = 0;


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv) {
	if (argc != 3) return -1;
	FILE* mp3file = fopen(argv[2], "wb");
	fclose(mp3file);
	mp3file = fopen(argv[2], "ab");
	
	struct dirent *entry;
	chdir(argv[1]);
	char start_dir[1000];
   	getcwd(start_dir, sizeof(char)*1000);
	DIR *current = opendir(".");
	int num_threads = 0, err;
	pthread_t* sub_threads = NULL;
	char** sub_dir = NULL;
	
	
	// Read all of first level sub dir (assuming no files in first level)
	while( (entry = readdir(current)) ) {
		if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0) && (entry->d_type == DT_DIR)) {
			
			//Make str for the location of the sub directory
			sub_dir = realloc(sub_dir, sizeof(char*)*(num_threads+1));
			sub_dir[num_threads] = NULL;
			sub_dir[num_threads] = realloc(sub_dir[num_threads], sizeof(char)*(strlen(start_dir)+strlen(entry->d_name)+2));
			sub_dir[num_threads] = strcpy(sub_dir[num_threads], start_dir);
			strcat(sub_dir[num_threads], "/");
			strcat(sub_dir[num_threads], entry->d_name);
			
			// Spawns list of Threads
			sub_threads = realloc(sub_threads, sizeof(pthread_t)*(num_threads+1));
			err = pthread_create(&sub_threads[num_threads], NULL, traverseDirectory, (void *)sub_dir[num_threads]);
			num_threads++;
			

			if(err != 0) {
				perror("can't create thread");
				return -1;
			}
		} 	
	}
	
	// Wait for all threads to finish
	for(int i = 0; i < num_threads; i++) {
		pthread_join(sub_threads[i], NULL);
		
	}
	for(int i = 0; i < num_threads; i++) free(sub_dir[i]);
	chdir(start_dir);
	if (length > 0) {
		
		for(int i = 0; i < length+1; i++) {
			fwrite(frags[i], sizeof(char), strlen(frags[i]), mp3file);
			free(frags[i]);
		}
	}
	closedir(current);
	free(sub_dir);
	free(sub_threads);
	free(frags);
	fclose(mp3file);

	return 0;
}


// Recursive function to traverse sub dir
void* traverseDirectory(void *arg) {
	struct dirent *entry;
	int index;
	char* buf;
	size_t size;
	

	//Target directory
	char* t_dir = (char *) arg;
	
	pthread_mutex_lock(&lock);
	chdir(t_dir);
	DIR* current = opendir(".");
	pthread_mutex_unlock(&lock);
	
	char* next = NULL, *file = NULL;
	
	if (current != NULL) {
		while( (entry = readdir(current)) ){
			if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0) && (entry->d_type == DT_DIR)) {
				// Recursively Treverse a given directory w/ threads for each subpaths (dirs)
				next = malloc(sizeof(char)*(strlen(t_dir)+strlen(entry->d_name)+2));
				strcpy(next, t_dir);
				strcat(next, "/");
				strcat(next, entry->d_name);
				
				traverseDirectory(next);
				if(next != NULL) free(next);
			} else if (entry->d_type == DT_REG) {
				// Get file exact path
				file = malloc(sizeof(char)*(strlen(t_dir)+strlen(entry->d_name)+2));
				strcpy(file, t_dir);
				strcat(file, "/");
				strcat(file, entry->d_name);
				
				// Open fragmenent file
				FILE* frag_bin = fopen(file, "rb");
				
				// Read fragmenent file
				fseek(frag_bin, 0, SEEK_END);
				size = ftell(frag_bin);
				fseek(frag_bin, 0, SEEK_SET);
				buf = malloc(sizeof(char)*size);
				fread(buf, sizeof(char), size, frag_bin);
				fclose(frag_bin);
				
				// Write to buf to list of frag files
				index = atoi(entry->d_name);
				pthread_mutex_lock(&lock);
				if(length < index) {
					length = index;
					frags = realloc(frags, sizeof(char*)*(index + 1));
				}
				frags[index] = strdup(buf);
				
				pthread_mutex_unlock(&lock);
				free(file);
				free(buf);
					
			} 
		}
	
	}
	closedir(current);
	return NULL;
}
