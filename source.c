#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define THREAD_COUNT 8
#define SUCCESS 1
#define MILLION 100000000

//struktura za prijenos svih potrebnih podataka u thread
struct Sum {
    int result;
    int* data;
    int dataSize;
    int tid;
    double elapsedTime;
};


int* enterData(int*, int);
int* allocateStaticData(int*);
int sumData(int*, int*, int);
int prepareThreadData(struct Sum*, int*);
void* sumDataThread(void*);

int main() {
    struct Sum Sum [THREAD_COUNT];
    int sumSize = 0;
    int result = 0;
    int* staticData = NULL;
    double elapsedTime = 0.0;
    int i = 0;
    int errorCode ;
    void* status;

    pthread_t thread[THREAD_COUNT];
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);   //doduse po defaultu su joinable ionako

    //ovaj dio je za korisnicki unos, ali nema smisla  unositi 50 brojeva koje ce program izracunati u nanosekundi, stoga je napravljen staticki dataset 
    //sa milijun brojeva koji su svi 1, tako da suma mora biti milijun cime provjeravamo jel threadovi dobro računaju tu sumu
    //printf ("Koliko clanova sume zelite unijeti!? \n");
    //scanf(" %d", &sumSize);
    //data = enterData(data, sumSize);

    //alokacija memorije za podatke, korisnik ne unosi podatke pa se zove static data (memorija je zapravo alocirana dinamicki)
    staticData = allocateStaticData(staticData);
    //dodjela potrebnih vrijendnosti strukturi da threadovi mogu obaviti posao
    prepareThreadData(Sum, staticData);

    //sekvencijonalno zbrajanje
    clock_t beginning = clock();
    sumData(&result, staticData, MILLION);
    clock_t ending = clock();
    elapsedTime += (double)(ending - beginning) / CLOCKS_PER_SEC;
    printf("Vrijeme potrebno za izracunati sumu single thread: %f\n", elapsedTime);

    elapsedTime = 0;
    result = 0;
    //put threads to work and start couting time
    beginning = clock();
    for(i; i < THREAD_COUNT; i++){
        errorCode = pthread_create(&thread[i], NULL, sumDataThread, (void*) &(Sum[i]));
        if(errorCode) {
            printf("Thread se nije uspio napraviti \n");
        }
    }
    
    //wait for threads to terminate
    i = 0;
    pthread_attr_destroy(&attr);
    for(i; i < THREAD_COUNT; i++){
        errorCode = pthread_join(thread[i], &status);
        if (errorCode) {
            printf("Unable to join thread \n");
        }
    }
    //calculate final result
    i = 0;
    for(i; i < THREAD_COUNT; i++) {
        result += Sum[i].result;
    }
    ending = clock();
    elapsedTime += (double)(ending - beginning) / CLOCKS_PER_SEC;
    printf("Rezultat sume preko threadova jeste: %d\n", result);
    printf ("Vrijeme potrebno za izracunati sumu preko threadova: %f \n", elapsedTime);


    

    pthread_exit(NULL);

    return 0;
}

int* enterData(int* data, int sumSize) {
    data = (int*) malloc (sumSize * sizeof(int));
    int i = 0;

    if(!data) return data; //error checking
    for (i ; i < sumSize ; i++){
        printf ("Unesi %d. broj\n", i + 1);
        scanf (" %d", &data[i]);
    }
    return data;
}

int* allocateStaticData(int* data){
    int i = 0;
    data = (int*) malloc(MILLION * sizeof(int)); //one million integers
    if(!data) {
        printf("Nedovoljno memorije, malloc failed \n");
        return data;
    }
    for(i; i < MILLION; i++) {
        data[i] = 1;
    }
    return data;
}

int sumData(int* result, int* data, int sumSize) {
    int i = 0;

    for (i; i < sumSize; i++) {
        *result = data[i] + *result;
    }
    printf("Rezultat sume klasičnim putem jeste: %d\n", *result);
    return SUCCESS; //error code, dobra praksa ali beskoristan
}

int prepareThreadData(struct Sum* Sum, int* Data){
    int i = 0;
    double dataChunk = MILLION / THREAD_COUNT;   //količina podataka koju svaki thread obrađuje

    for(i; i < THREAD_COUNT; i++){
        Sum[i].tid = i + 1;
        Sum[i].result = 0;
        Sum[i].elapsedTime = 0.0;
        Sum[i].dataSize = dataChunk;
        Sum[i].data = Data;                     //sve niti dijele pokazivač na početak niza koji sadrži podatke, ali koriste skroz druge dijelove tog niza za račun
    }

}

void* sumDataThread(void* sumPointer) {
    int result = 0;
    struct Sum* Sum;
    Sum = (struct Sum*)sumPointer;
    int * dataPointer = Sum->data;   

    //printf("Inside thread %d\n", Sum->tid);

    int start = (int)((Sum->tid - 1) * Sum->dataSize);     //izračun početka dijela podatka za koji pojedini thread računa sumu
    int end = (int)(Sum->tid * Sum->dataSize);       //za 4 threada = 1 / 4 * velicina podataka, bitno je da velicina podatka bude za red velicine veca od broja niti da se izbjegnu decimalni brojevi
    if (Sum->tid == THREAD_COUNT) end++;               //da se dobija prava suma jer for petlja ide do n-1 elementa    
    
    for(start; start < end; start++){
      result = result + Sum->data[start];
    }
    
    Sum->result = result;
    //printf("Exiting thread %d \n", Sum->tid);
    pthread_exit(NULL);
}