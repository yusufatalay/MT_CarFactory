#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TIRES "tires"
#define CHASIS "chasis"
#define SEATS "seats"
#define ENGINE "engine"
#define TOPCOVER "topcover"
#define PAINT "paint"

#define WORKER_A "TYPE_A"
#define WORKER_B "TYPE_B"
#define WORKER_C "TYPE_C"
#define WORKER_D "TYPE_D"


int NUM_THREAD_A;
int NUM_THREAD_B;
int NUM_THREAD_C;
int NUM_THREAD_D;
int NUM_DAYS;

int CURRENT_DAY;

int NUM_DAILY_CHASIS;
int NUM_DAILY_TIRES;
int NUM_DAILY_SEATS;
int NUM_DAILY_ENGINES;
int NUM_DAILY_TOPCOVERS;
int NUM_DAILY_PAINTS;

int NUM_DAILY_CHASIS_REMAINING;
int NUM_DAILY_TIRES_REMAINING;
int NUM_DAILY_SEATS_REMAINING;
int NUM_DAILY_ENGINES_REMAINING;
int NUM_DAILY_TOPCOVERS_REMAINING;
int NUM_DAILY_PAINTS_REMAINING;

// number of cars to be produced are stored in 2d array fashion	for each day
int **NUM_DAILY_CARS;

typedef struct worker{
	int workerID;
	char *workerType;
	char *currentPartName;
} worker;

// create a car struct with all the parts as a condition variable with a mutex
typedef struct car{
	pthread_mutex_t mutex;
	pthread_cond_t chasis;
	pthread_cond_t tires;
	pthread_cond_t seats;
	pthread_cond_t engine;
	pthread_cond_t topcover;
	worker currentWorker; // current worker is only for logging purposes, since the thread functions only takes single argument
	int CarID;
	_Bool isCarReady; // this value only indicates if the car is ready to be sold. It will not get used for synchronization.
	
} car;

void place_chasis(car *c){
	 // chasis dooes not wait for any condition
	// lock the car's mutex
	pthread_mutex_lock(&c->mutex);
	// assign the worker Id, and part name to the car
	c->currentWorker.workerID = pthread_self();
	c->currentWorker.currentPartName = CHASIS;
	// decrement daily chasis remaining
	NUM_DAILY_CHASIS_REMAINING--;
	// TODO: do some logging here
	printf("%s-%d\t%d\t%s\t%d\n",
	c->currentWorker.workerType,c->currentWorker.workerID,
	c->CarID, c->currentWorker.currentPartName, CURRENT_DAY);

	// unlock the car's mutex
	pthread_mutex_unlock(&c->mutex);
	// signal that chasis is ready
	pthread_cond_broadcast(&c->chasis);


}

void place_tires(car *c){
	// tires wait for chasis to be ready
	pthread_cond_wait(&c->chasis, &c->mutex);
	// lock the car's mutex
	pthread_mutex_lock(&c->mutex);
	c->currentWorker.currentPartName = TIRES;
	// decrement daily tires remaining
	NUM_DAILY_TIRES_REMAINING--;

	// TODO: do some logging here
	printf("%s-%d\t%d\t%s\t%d\n",
	c->currentWorker.workerType,c->currentWorker.workerID,
	c->CarID, c->currentWorker.currentPartName, CURRENT_DAY);

	// unlock the car's mutex
	pthread_mutex_unlock(&c->mutex);

	// signal that chasis is ready	// signal that tires are ready
	pthread_cond_broadcast(&c->tires);

}


void mount_seats(car *c){

	// seats wait for tires to be ready
	pthread_cond_wait(&c->tires, &c->mutex);

	// lock the car's mutex
	pthread_mutex_lock(&c->mutex);
	// assign the worker Id, and part name to the car
	c->currentWorker.currentPartName = SEATS;
	// decrement daily seats remaining
	NUM_DAILY_SEATS_REMAINING--;

	// TODO: do some logging here
	printf("%s-%d\t%d\t%s\t%d\n",
	c->currentWorker.workerType,c->currentWorker.workerID,
	c->CarID, c->currentWorker.currentPartName, CURRENT_DAY);

	// unlock the car's mutex
	pthread_mutex_unlock(&c->mutex);

	// signal that seats are ready
	pthread_cond_broadcast(&c->seats);

}

void place_engine(car *c){

	// engine wait for seats to be ready
	pthread_cond_wait(&c->seats, &c->mutex);

	// lock the car's mutex
	pthread_mutex_lock(&c->mutex);
	c->currentWorker.currentPartName = ENGINE;
	// decrement daily engines remaining
	NUM_DAILY_ENGINES_REMAINING--;
	// TODO: do some logging here
	printf("%s-%d\t%d\t%s\t%d\n",
	c->currentWorker.workerType,c->currentWorker.workerID,
	c->CarID, c->currentWorker.currentPartName, CURRENT_DAY);

	// unlock the car's mutex
	pthread_mutex_unlock(&c->mutex);

	// signal that engine is ready
	pthread_cond_broadcast(&c->engine);

}

void place_topcover(car *c){

	// topcover wait for engine to be ready
	pthread_cond_wait(&c->engine, &c->mutex);
	
	// lock the car's mutex
	pthread_mutex_lock(&c->mutex);
	c->currentWorker.currentPartName = TOPCOVER;
	// decrement daily topcover remaining
	NUM_DAILY_TOPCOVERS_REMAINING--;
	// TODO: do some logging here
	printf("%s-%d\t%d\t%s\t%d\n",
	c->currentWorker.workerType,c->currentWorker.workerID,
	c->CarID, c->currentWorker.currentPartName, CURRENT_DAY);
	// unlock the car's mutex
	pthread_mutex_unlock(&c->mutex);

	// signal that topcover is ready
	pthread_cond_broadcast(&c->topcover);

}

void paint_car(car *c){
	
	// paint wait for topcover to be ready
	pthread_cond_wait(&c->topcover, &c->mutex);
	// lock the car's mutex
	pthread_mutex_lock(&c->mutex);
	// assign the worker Id, and part name to the car
	c->currentWorker.currentPartName = PAINT;
	// decrement daily topcover remaining
	NUM_DAILY_PAINTS_REMAINING--;

	// since car painting is the last step, increase the car counter of the day
	NUM_DAILY_CARS[1][CURRENT_DAY] += 1;

	c->isCarReady = 1;
	printf("%s-%d\t%d\t%s\t%d\n",
	c->currentWorker.workerType,c->currentWorker.workerID,
	c->CarID, c->currentWorker.currentPartName, CURRENT_DAY);


	// unlock the car's mutex
	pthread_mutex_unlock(&c->mutex);
}

// create thread functions for each worker type
void *WORKER_A_WORK(void *args){
	car *c = (car*)args;
	// assign car's worker id to pthread_self()
	c->currentWorker.workerID = pthread_self();
	// assign car's worker type to A
	c->currentWorker.workerType = WORKER_A;
	// worker a can place the chasis 
	place_chasis(c);
	return NULL;
}

void *WORKER_B_WORK(void *args){
	car *c = (car*)args;
	// assign car's worker id to pthread_self()
	c->currentWorker.workerID = pthread_self();
	// assign car's worker type to B
	c->currentWorker.workerType = WORKER_B;	
	// worker b can mount the tires, place the seats and place the engine
	place_tires(c);
	mount_seats(c);
	place_engine(c);
	return NULL;
}

void *WORKER_C_WORK(void *args){
	car *c = (car*)args;
	// assign car's worker id to pthread_self()
	c->currentWorker.workerID = pthread_self();
	// assign car's worker type to C
	c->currentWorker.workerType = WORKER_C;
	// worker c can place the topcover
	place_topcover(c);

	return NULL;
}

void *WORKER_D_WORK(void *args){
	car *c = (car*)args;
	// assign car's worker id to pthread_self()
	c->currentWorker.workerID = pthread_self();
	// assign car's worker type to D
	c->currentWorker.workerType = WORKER_D;
	// worker d can paint the car
	paint_car(c);
	return NULL;
}



// iterate the day for the controller thread

void* iterate_day(void *arg){
	// cast arg to integer
	NUM_DAYS = *(int*)arg;

	for (CURRENT_DAY = 1 ; CURRENT_DAY <= NUM_DAYS; CURRENT_DAY++){
		// in order to make this race condition save I should have lock for each different material, but I don't think it is necessary
		NUM_DAILY_CHASIS_REMAINING  = NUM_DAILY_CHASIS;	
		NUM_DAILY_ENGINES_REMAINING = NUM_DAILY_ENGINES;
		NUM_DAILY_TIRES_REMAINING   = NUM_DAILY_TIRES;
		NUM_DAILY_SEATS_REMAINING   = NUM_DAILY_SEATS;
		NUM_DAILY_TOPCOVERS_REMAINING = NUM_DAILY_TOPCOVERS;
		NUM_DAILY_PAINTS_REMAINING = NUM_DAILY_PAINTS;
		// sleep for 3 seconds
		sleep(3);
	}
	pthread_exit(NULL);


}

int main(int argc, char *argv[]){
	// open input.txt
	FILE *input = fopen("input.txt", "r");
	if(input == NULL){
		printf("Error opening input.txt\n");
		return 1;
	}

	// read input.txt line by line
	fscanf(input, "%d %d %d %d %d\n%d %d %d %d %d %d", &NUM_THREAD_A, &NUM_THREAD_B, &NUM_THREAD_C, &NUM_THREAD_D, &NUM_DAYS,
		&NUM_DAILY_CHASIS, &NUM_DAILY_TIRES, &NUM_DAILY_SEATS, &NUM_DAILY_ENGINES, &NUM_DAILY_TOPCOVERS, &NUM_DAILY_PAINTS);
	// close input.txt
	fclose(input);


	// redirect the stdout to the output.txt with freopen
	FILE *out = freopen("output.txt", "w", stdout);

	// allocate memory for the daily car counter 
	NUM_DAILY_CARS = (int **)malloc(sizeof(int *) * NUM_DAYS);

	// in order to determine max amunt of cars that can be made, we need to find the smallest amount of material
	// so we can create a reasonable size array of cars
	// get the smallest amount of material among chasis, tires, seats, engines, topcovers, and paints
	int min = NUM_DAILY_CHASIS;
	if(NUM_DAILY_TIRES < min){
		min = NUM_DAILY_TIRES;
	}
	if(NUM_DAILY_SEATS < min){
		min = NUM_DAILY_SEATS;
	}
	if(NUM_DAILY_ENGINES < min){
		min = NUM_DAILY_ENGINES;
	}
	if(NUM_DAILY_TOPCOVERS < min){
		min = NUM_DAILY_TOPCOVERS;
	}
	if(NUM_DAILY_PAINTS < min){
		min = NUM_DAILY_PAINTS;
	}

	// create an array of cars with the size of the smallest amount of material
	car cars[min];
	
	// initialize the mutex and condition variables for each car
	for(int i = 0; i < min; i++){
		cars[i].isCarReady = 0;
		cars[i].CarID = i;
		pthread_mutex_init(&cars[i].mutex, NULL);
		pthread_cond_init(&cars[i].chasis, NULL);
		pthread_cond_init(&cars[i].tires, NULL);
		pthread_cond_init(&cars[i].seats, NULL);
		pthread_cond_init(&cars[i].engine, NULL);
		pthread_cond_init(&cars[i].topcover, NULL);
	}

	// create thread array for each thread type
	pthread_t WORKERS_A[NUM_THREAD_A];
	pthread_t WORKERS_B[NUM_THREAD_B];
	pthread_t WORKERS_C[NUM_THREAD_C];
	pthread_t WORKERS_D[NUM_THREAD_D];

	// create a new thread for updating day value
	pthread_t DAY_ITERATOR;
	pthread_create(&DAY_ITERATOR, NULL, iterate_day, &NUM_DAYS);
	
	// create threads for each worker type
	
	while (CURRENT_DAY < NUM_DAYS){
		for(int i = 0; i < NUM_THREAD_A; i++){
			pthread_create(&WORKERS_A[i], NULL, WORKER_A_WORK, &cars[i]);
		}
		for(int i = 0; i < NUM_THREAD_B; i++){
			pthread_create(&WORKERS_B[i], NULL, WORKER_B_WORK, &cars[i]);
		}
		for(int i = 0; i < NUM_THREAD_C; i++){
			pthread_create(&WORKERS_C[i], NULL, WORKER_C_WORK, &cars[i]);
		}
		for(int i = 0; i < NUM_THREAD_D; i++){
			pthread_create(&WORKERS_D[i], NULL, WORKER_D_WORK, &cars[i]);
		}



		// join all threads
		for(int i = 0; i < NUM_THREAD_A; i++){
			pthread_join(WORKERS_A[i], NULL);
		}
		for (int i = 0; i< NUM_THREAD_B; i++){
			pthread_join(WORKERS_B[i], NULL);
		}
		for (int i = 0; i< NUM_THREAD_C; i++){
			pthread_join(WORKERS_C[i], NULL);
		}
		for (int i = 0; i< NUM_THREAD_D; i++){
			pthread_join(WORKERS_D[i], NULL);
		}
	}

	// join day iterator thread
	pthread_join(DAY_ITERATOR, NULL);


	// log the

	// destroy eveything
	for(int i = 0; i < min; i++){
		pthread_mutex_destroy(&cars[i].mutex);
		pthread_cond_destroy(&cars[i].chasis);
		pthread_cond_destroy(&cars[i].tires);
		pthread_cond_destroy(&cars[i].seats);
		pthread_cond_destroy(&cars[i].engine);
		pthread_cond_destroy(&cars[i].topcover);
	}	

	free(NUM_DAILY_CARS);
	fclose(out);
	return 0;
}





