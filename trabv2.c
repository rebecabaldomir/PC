#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"

#define VAGAS 4
#define SERVIDORES 6
#define ESTAGIARIOS 3

//SE CONTADOR-SERVIDOR == 0 LOCK(&VEZ)
//LEITOR ESCRITOR

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vez = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t lotou = PTHREAD_COND_INITIALIZER;                   
int contador = 0;
int contador_servidores = 0;
int contador_estagiarios = 0;

void mostra_que_quer_entrar(){
  pthread_mutex_lock(&mutex);
  contador++;
  //printf("Somou contador: %d pelo Servidor %d \n",contador, i );
  pthread_mutex_unlock(&mutex);
}

void servidor_trabalha(int i){
  //pthread_mutex_lock(&mutex);
  printf("Servidor %d: vou trabalhar em uma baia!\n", i);
  //pthread_mutex_unlock(&mutex);
  sleep(5); 
}

void servidor_vai_para_casa(int i){
  pthread_mutex_lock(&mutex);
  printf("Servidor %d: vou deixar a baia.\n", i);
  //if(contador == VAGAS){
    pthread_cond_broadcast(&lotou);
    printf("servidor %d enviou sinal\n", i);
 // }
  contador--;
  pthread_mutex_unlock(&mutex);
  sleep(5);
}

void servidor_fica_esperando(int i){
  pthread_mutex_lock(&mutex);
  printf("Lotado\n");
  printf("Servidor %d: vou esperar >:|\n", i);
  contador_servidores++;
  contador--;
  while(contador >= VAGAS){
    printf("servidor %d esperando sinal\n", i);
    pthread_cond_wait(&lotou, &mutex);
    printf("servidor %d recebeu sinal\n", i);
  }
  
  contador_servidores--;
  //printf("sai do while\n");
  pthread_mutex_unlock(&mutex);
}

void* servidor(void * a){
  int i = *((int *) a);  
  while(1){

  mostra_que_quer_entrar();

    if(contador<=VAGAS){

      servidor_trabalha(i);

      servidor_vai_para_casa(i);

      break;

    }else{

      servidor_fica_esperando(i);

    }
  
  }
}

void* estagiario(void* a){
  int i = *((int *) a); 
  while(1){
   // pthread_mutex_lock(&vez);
    printf("sou estagiario contador: %d  contador servidor: %d\n", contador, contador_servidores);
    if(contador <= VAGAS && contador_servidores == 0){    
      pthread_mutex_lock(&mutex);
      contador++;
      printf("Estagiario %d trabalhando\n", i);
      pthread_mutex_unlock(&mutex);
      sleep(5);
      pthread_mutex_lock(&mutex);
      printf("Estagiario %d saiu\n", i);
      contador--;
      pthread_mutex_unlock(&mutex);
      break;
    }
    pthread_mutex_lock(&mutex);
    while(contador >= VAGAS || contador_servidores > 0){
      printf("estagiario %d vai esperar\n", i);
      pthread_cond_wait(&lotou, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    //pthread_mutex_unlock(&vez);
  }
}



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

   for (i = 0; i < ESTAGIARIOS ; i++) {
         id = (int *) malloc(sizeof(int));
         *id = i;
         pthread_create(&e[i], NULL, estagiario, (void *) (id));
   }

   for (i = 0; i < SERVIDORES ; i++) {
    pthread_join(s[i],NULL);
   }

   for (i = 0; i < ESTAGIARIOS ; i++) {
    pthread_join(e[i],NULL);
   }
   return 0;

}