#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

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

int CURRENT_DAY = 1;

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

pthread_mutex_t mutex_chasis;
pthread_mutex_t mutex_tires;
pthread_mutex_t mutex_seats;
pthread_mutex_t mutex_engines;
pthread_mutex_t mutex_topcovers;
pthread_mutex_t mutex_paints;

// number of cars to be produced are stored in 2d array fashion	for each day
int DAILY_CAR = 0;

typedef struct worker
{
	int workerID;
	char *workerType;
	char *currentPartName;
} worker;

// create a car struct with all the parts as a condition variable with a mutex
typedef struct car
{
	pthread_mutex_t mutex;
	sem_t chasis;
	_Bool chasisDone;
	sem_t tires;
	_Bool tiresDone;
	sem_t seats;
	_Bool seatsDone;
	sem_t engine;
	_Bool engineDone;
	sem_t topcover;
	_Bool topcoverDone;
	sem_t paint;
	_Bool paintDone;
	worker currentWorker; // current worker is only for logging purposes, since the thread functions only takes single argument
	int CarID;
	_Bool isCarReady; // this value only indicates if the car is ready to be sold. It will not get used for synchronization.
} car;

// create an array of cars with the size of the smallest amount of material
car *cars;

void place_chasis(car *c)
{
	while (c->isCarReady == 1 || c->chasisDone == 1)
	{
		// if the current car is locked then go to the next car
		c = &cars[c->CarID + 1];
	}
	// chasis dooes not wait for any condition
	// lock the car's mutex
	// try locking the car if not succesfull then go to another car
	pthread_mutex_lock(&c->mutex);

	// assign the worker Id, and part name to the car
	c->currentWorker.workerID = pthread_self();
	c->currentWorker.workerType = WORKER_B;
	c->currentWorker.currentPartName = CHASIS;
	c->chasisDone = 1;
	// decrement daily chasis remaining
	pthread_mutex_lock(&mutex_chasis);
	NUM_DAILY_CHASIS_REMAINING--;
	pthread_mutex_unlock(&mutex_chasis);
	printf("%s-%u\t%d\t%s\t%d\n",
		   c->currentWorker.workerType, c->currentWorker.workerID,
		   c->CarID, c->currentWorker.currentPartName, CURRENT_DAY);

	// signal that chasis is ready
	sem_post(&c->chasis);
	// unlock the car's mutex
	pthread_mutex_unlock(&c->mutex);
}

void place_tires(car *c)
{
	// tires wait for chasis to be ready
	// lock the car's mutex
	// wait for chasis lock
	while (c->tiresDone == 1 || c->isCarReady == 1)
	{
		// if the current car is locked then go to the next car
		c = &cars[c->CarID + 1];
	}
	sem_wait(&c->chasis);
	pthread_mutex_lock(&c->mutex);

	c->currentWorker.workerID = pthread_self();
	c->currentWorker.workerType = WORKER_A;
	c->currentWorker.currentPartName = TIRES;
	c->tiresDone = 1;
	// decrement daily tires remaining
	pthread_mutex_lock(&mutex_tires);
	NUM_DAILY_TIRES_REMAINING--;
	pthread_mutex_unlock(&mutex_tires);

	printf("%s-%u\t%d\t%s\t%d\n",
		   c->currentWorker.workerType, c->currentWorker.workerID,
		   c->CarID, c->currentWorker.currentPartName, CURRENT_DAY);

	// signal that chasis is ready	// signal that tires are ready
	sem_post(&c->tires);

	// unlock the car's mutex
	pthread_mutex_unlock(&c->mutex);
}

void mount_seats(car *c)
{
	while (c->seatsDone == 1 || c->isCarReady == 1)
	{
		// if the current car is locked then go to the next car
		c = &cars[c->CarID + 1];
	}
	sem_wait(&c->tires);
	pthread_mutex_lock(&c->mutex);

	// seats wait for tires to be ready
	// lock the car's mutex
	// assign the worker Id, and part name to the car
	c->currentWorker.workerType = WORKER_C;
	c->currentWorker.workerID = pthread_self();
	c->currentWorker.currentPartName = SEATS;
	c->seatsDone = 1;
	// decrement daily seats remaining
	pthread_mutex_lock(&mutex_seats);
	NUM_DAILY_SEATS_REMAINING--;
	pthread_mutex_unlock(&mutex_seats);

	// TODO: do some logging here
	printf("%s-%u\t%d\t%s\t%d\n",
		   c->currentWorker.workerType, c->currentWorker.workerID,
		   c->CarID, c->currentWorker.currentPartName, CURRENT_DAY);

	// unlock the car's mutex

	// signal that seats are ready
	sem_post(&c->seats);
	pthread_mutex_unlock(&c->mutex);
}

void place_engine(car *c)
{
	while (c->engineDone == 1 || c->isCarReady == 1)
	{
		// if the current car is locked then go to the next car
		c = &cars[c->CarID + 1];
	}
	// engine wait for seats to be ready
	sem_wait(&c->seats);
	pthread_mutex_lock(&c->mutex);

	// lock the car's mutex
	c->currentWorker.workerID = pthread_self();
	c->currentWorker.workerType = WORKER_D;
	c->currentWorker.currentPartName = ENGINE;
	c->engineDone = 1;
	// decrement daily engines remaining
	pthread_mutex_lock(&mutex_engines);
	NUM_DAILY_ENGINES_REMAINING--;
	pthread_mutex_unlock(&mutex_engines);

	printf("%s-%u\t%d\t%s\t%d\n",
		   c->currentWorker.workerType, c->currentWorker.workerID,
		   c->CarID, c->currentWorker.currentPartName, CURRENT_DAY);

	// unlock the car's mutex

	// signal that engine is ready
	sem_post(&c->engine);
	pthread_mutex_unlock(&c->mutex);
}

void place_topcover(car *c)
{
	while (c->topcoverDone == 1 || c->isCarReady == 1)
	{
		// if the current car is locked then go to the next car
		c = &cars[c->CarID + 1];
	}
	sem_wait(&c->engine);
	pthread_mutex_lock(&c->mutex);
	// topcover wait for engine to be ready
	// lock the car's mutex
	c->currentWorker.workerID = pthread_self();
	c->currentWorker.workerType = WORKER_D;
	c->currentWorker.currentPartName = TOPCOVER;
	c->topcoverDone = 1;
	// decrement daily topcover remaining
	pthread_mutex_lock(&mutex_topcovers);
	NUM_DAILY_TOPCOVERS_REMAINING--;
	pthread_mutex_unlock(&mutex_topcovers);
	// TODO: do some logging here
	printf("%s-%u\t%d\t%s\t%d\n",
		   c->currentWorker.workerType, c->currentWorker.workerID,
		   c->CarID, c->currentWorker.currentPartName, CURRENT_DAY);
	// unlock the car's mutex
	sem_post(&c->topcover);
	pthread_mutex_unlock(&c->mutex);

	// signal that topcover is ready
}

void paint_car(car *c)
{
	while (c->paintDone == 1 || c->isCarReady == 1)
	{
		// if the current car is locked then go to the next car
		c = &cars[c->CarID + 1];
	}
	sem_wait(&c->topcover);
	pthread_mutex_lock(&c->mutex);
	// paint wait for topcover to be ready
	// lock the car's mutex
	// assign the worker Id, and part name to the car
	c->currentWorker.workerID = pthread_self();
	c->currentWorker.workerType = WORKER_A;
	c->currentWorker.currentPartName = PAINT;
	c->paintDone = 1;
	// decrement daily topcover remaining
	pthread_mutex_lock(&mutex_paints);
	NUM_DAILY_PAINTS_REMAINING--;
	pthread_mutex_unlock(&mutex_paints);
	// since car painting is the last step, increase the car counter of the day
	DAILY_CAR++;
	c->isCarReady = 1;
	printf("%s-%u\t%d\t%s\t%d\n",
		   c->currentWorker.workerType, c->currentWorker.workerID,
		   c->CarID, c->currentWorker.currentPartName, CURRENT_DAY);

	// unlock the car's mutex
	sem_post(&c->paint);
	pthread_mutex_unlock(&c->mutex);
}

// create thread functions for each worker type
void *WORKER_A_WORK(void *args)
{
	car *c = (car *)args;
	place_tires(c);
	paint_car(c);
	return NULL;
}

void *WORKER_B_WORK(void *args)
{
	car *c = (car *)args;
	place_chasis(c);
	return NULL;
}

void *WORKER_C_WORK(void *args)
{
	car *c = (car *)args;

	mount_seats(c);
	return NULL;
}

void *WORKER_D_WORK(void *args)
{
	car *c = (car *)args;
	place_engine(c);
	place_topcover(c);
	return NULL;
}

// iterate the day for the controller thread

void *iterate_day(void *arg)
{
	for (CURRENT_DAY = 1; CURRENT_DAY <= NUM_DAYS; CURRENT_DAY++)
	{
		sleep(3);

		printf("DAILY CAR AT DAY %d : %d\n", CURRENT_DAY, DAILY_CAR);

		pthread_mutex_lock(&mutex_chasis);
		NUM_DAILY_CHASIS_REMAINING = NUM_DAILY_CHASIS;
		pthread_mutex_unlock(&mutex_chasis);

		pthread_mutex_lock(&mutex_engines);
		NUM_DAILY_ENGINES_REMAINING = NUM_DAILY_ENGINES;
		pthread_mutex_unlock(&mutex_engines);

		pthread_mutex_lock(&mutex_tires);
		NUM_DAILY_TIRES_REMAINING = NUM_DAILY_TIRES;
		pthread_mutex_unlock(&mutex_tires);

		pthread_mutex_lock(&mutex_seats);
		NUM_DAILY_SEATS_REMAINING = NUM_DAILY_SEATS;
		pthread_mutex_unlock(&mutex_seats);

		pthread_mutex_lock(&mutex_topcovers);
		NUM_DAILY_TOPCOVERS_REMAINING = NUM_DAILY_TOPCOVERS;
		pthread_mutex_unlock(&mutex_topcovers);

		pthread_mutex_lock(&mutex_paints);
		NUM_DAILY_PAINTS_REMAINING = NUM_DAILY_PAINTS;
		pthread_mutex_unlock(&mutex_paints);
		DAILY_CAR = 0;
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	// open input.txt
	FILE *input = fopen("input.txt", "r");
	if (input == NULL)
	{
		printf("Error opening input.txt\n");
		return 1;
	}

	// read input.txt line by line
	fscanf(input, "%d %d %d %d %d\n%d %d %d %d %d %d", &NUM_THREAD_A, &NUM_THREAD_B, &NUM_THREAD_C, &NUM_THREAD_D, &NUM_DAYS,
		   &NUM_DAILY_CHASIS, &NUM_DAILY_TIRES, &NUM_DAILY_SEATS, &NUM_DAILY_ENGINES, &NUM_DAILY_TOPCOVERS, &NUM_DAILY_PAINTS);
	// close input.txt
	fclose(input);

	// redirect the stdout to the output.txt with freopen
	// FILE *out = freopen("output.txt", "w", stdout);

	// in order to determine max amunt of cars that can be made, we need to find the smallest amount of material
	// so we can create a reasonable size array of cars
	// get the smallest amount of material among chasis, tires, seats, engines, topcovers, and paints
	int min = NUM_DAILY_CHASIS;
	if (NUM_DAILY_TIRES < min)
	{
		min = NUM_DAILY_TIRES;
	}
	if (NUM_DAILY_SEATS < min)
	{
		min = NUM_DAILY_SEATS;
	}
	if (NUM_DAILY_ENGINES < min)
	{
		min = NUM_DAILY_ENGINES;
	}
	if (NUM_DAILY_TOPCOVERS < min)
	{
		min = NUM_DAILY_TOPCOVERS;
	}
	if (NUM_DAILY_PAINTS < min)
	{
		min = NUM_DAILY_PAINTS;
	}

	int arraySize = sizeof(car) * min;
	cars = malloc(arraySize);

	// initialize the mutex and condition variables for each car
	for (int i = 0; i < min; i++)
	{
		cars[i].isCarReady = 0;
		cars[i].CarID = i;
		cars[i].chasisDone = 0;
		cars[i].tiresDone = 0;
		cars[i].seatsDone = 0;
		cars[i].engineDone = 0;
		cars[i].topcoverDone = 0;
		cars[i].paintDone = 0;
		pthread_mutex_init(&cars[i].mutex, NULL);
		sem_init(&cars[i].chasis, 0, 0);
		sem_init(&cars[i].tires, 0, 0);
		sem_init(&cars[i].seats, 0, 0);
		sem_init(&cars[i].engine, 0, 0);
		sem_init(&cars[i].topcover, 0, 0);
	}
	// create thread array for each thread type
	pthread_t WORKERS_A[NUM_THREAD_A];
	pthread_t WORKERS_B[NUM_THREAD_B];
	pthread_t WORKERS_C[NUM_THREAD_C];
	pthread_t WORKERS_D[NUM_THREAD_D];

	// create a new thread for updating day value
	pthread_t DAY_ITERATOR;
	pthread_create(&DAY_ITERATOR, NULL, iterate_day, NULL);

	// create threads for each worker type

	for (int i = 0; i < NUM_THREAD_A; i++)
	{
		pthread_create(&WORKERS_A[i], NULL, WORKER_A_WORK, &cars[0]);
	}
	for (int i = 0; i < NUM_THREAD_B; i++)
	{
		pthread_create(&WORKERS_B[i], NULL, WORKER_B_WORK, &cars[0]);
	}
	for (int i = 0; i < NUM_THREAD_C; i++)
	{
		pthread_create(&WORKERS_C[i], NULL, WORKER_C_WORK, &cars[0]);
	}
	for (int i = 0; i < NUM_THREAD_D; i++)
	{
		pthread_create(&WORKERS_D[i], NULL, WORKER_D_WORK, &cars[0]);
	}

	// wait for days to finish
	pthread_join(DAY_ITERATOR, NULL);

	// join all threads
	for (int i = 0; i < NUM_THREAD_A; i++)
	{
		pthread_join(WORKERS_A[i], NULL);
	}
	for (int i = 0; i < NUM_THREAD_B; i++)
	{
		pthread_join(WORKERS_B[i], NULL);
	}
	for (int i = 0; i < NUM_THREAD_C; i++)
	{
		pthread_join(WORKERS_C[i], NULL);
	}
	for (int i = 0; i < NUM_THREAD_D; i++)
	{
		pthread_join(WORKERS_D[i], NULL);
	}

	// log the
	// destroy eveything
	for (int i = 0; i < min; i++)
	{
		pthread_mutex_destroy(&cars[i].mutex);
		sem_destroy(&cars[i].chasis);
		sem_destroy(&cars[i].tires);
		sem_destroy(&cars[i].seats);
		sem_destroy(&cars[i].engine);
		sem_destroy(&cars[i].topcover);
	}

	free(cars);
	// fclose(out);
	return 0;
}
