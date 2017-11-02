#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"

#define VAGAS 4
#define SERVIDORES 10
#define ESTAGIARIOS 2

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vez = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t lotou = PTHREAD_COND_INITIALIZER;                   
int contador = 0;
int contador_servidores = 0;
int contador_estagiarios = 0;

void* servidor(void * a){
  int i = *((int *) a);  
  while(1){
  pthread_mutex_lock(&mutex);
  contador++;
  //printf("Somou contador: %d pelo Servidor %d \n",contador, i );
  pthread_mutex_unlock(&mutex);
    if(contador<=VAGAS){
      pthread_mutex_lock(&mutex);
      //estaciona
      printf("Servidor %d: vou trabalhar em uma baia!\n", i);
      pthread_mutex_unlock(&mutex);
      //espera
      sleep(5); 
      //sai
      pthread_mutex_lock(&mutex);
      printf("Servidor %d: vou deixar a baia.\n", i);
      contador--;
      //printf("Diminuiu contador: %d\n", contador);
      pthread_cond_signal(&lotou);
      if(contador_servidores > 0) {
        contador_servidores--;
      }
      
      pthread_mutex_unlock(&mutex);
      sleep(5);
    }else{
      //manda sinal, todo mundo fica travado
      pthread_mutex_lock(&mutex);
      printf("Lotado\n");
      printf("Servidor %d: vou esperar >:|\n", i);
      contador_servidores++;
      contador--;
      pthread_cond_wait(&lotou, &mutex);
      pthread_mutex_unlock(&mutex);
    }
  }
}

/*void* estagiario(void * a){
  int i = *((int *) a);  
  while(1){
    while(contador<VAGAS && contador_servidores == 0){
      pthread_mutex_lock(&mutex);
      //estaciona
      printf("Estagiario %d: vou trabalhar em uma baia!\n", i);
      contador++;
      pthread_mutex_unlock(&mutex);
      //espera
      sleep(10); 
      //sai
      pthread_mutex_lock(&mutex);
      printf("Estagiario %d: vou deixar a baia.\n", i);
      contador--;
      pthread_cond_signal(&lotou);
      if(contador_estagiarios > 0){
        contador_estagiarios--;
      }
      pthread_mutex_unlock(&mutex);
    }
    //manda sinal, todo mundo fica travado
    pthread_mutex_lock(&mutex);
    printf("Lotado\n");
    printf("Estagiario %d: vou esperar. :(\n", i);
    contador_estagiarios++;
    while(contador>=VAGAS){
      pthread_cond_wait(&lotou, &mutex);
    }
    pthread_mutex_unlock(&mutex);
  }
}
*/
int main(){

   pthread_t s[SERVIDORES];
   pthread_t e[ESTAGIARIOS];
   int i;
   int *id;
   for (i = 0; i < SERVIDORES ; i++) {
         id = (int *) malloc(sizeof(int));
         *id = i;
         pthread_create(&s[i], NULL, servidor, (void *) (id));
   }

   /*for (i = 0; i < ESTAGIARIOS ; i++) {
         id = (int *) malloc(sizeof(int));
         *id = i;
         pthread_create(&e[i], NULL, estagiario, (void *) (id));
   }*/

   for (i = 0; i < SERVIDORES ; i++) {
    pthread_join(s[i],NULL);
   }
/*
   for (i = 0; i < ESTAGIARIOS ; i++) {
    pthread_join(e[i],NULL);
   }
*/
   return 0;

}