#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"

#define VAGAS 4
#define SERVIDORES 6
#define ESTAGIARIOS 3
#define N 20

//SE CONTADOR-SERVIDOR == 0 LOCK(&VEZ)
//LEITOR ESCRITOR


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t baia = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t lotou = PTHREAD_COND_INITIALIZER;                   
int contador = 0;
int contador_servidores = 0;
int contador_estagiarios = 0;

char matriz [N][N];
int baias[4][2];
int baias_ocupadas[4] = {0};

void inicializa_baias(){
  baias[0][0] = 5;
  baias[0][1] = 19;

  baias[1][0] = 8;
  baias[1][1] = 19;

  baias[2][0] = 11;
  baias[2][1] = 19;

  baias[3][0] = 14;
  baias[3][1] = 19;
}

void mostra_que_quer_entrar(){
  pthread_mutex_lock(&mutex);
  contador++;
  //printf("Somou contador: %d pelo Servidor %d \n",contador, i );
  pthread_mutex_unlock(&mutex);
}

int servidor_trabalhar(int i){
  //pthread_mutex_lock(&mutex);
  pthread_mutex_lock(&baia);
  int num_baia;
  for(int k = 0; k < 4; k++) {
    if(baias_ocupadas[k] == 0){
      baias_ocupadas[k] = 1;
      num_baia = k;
      break;
    } 
  }
  pthread_mutex_unlock(&baia);

  printf("Servidor %d vai pegar a baia %d!\n", i, num_baia);
    
    int linha_baia = baias[num_baia][0];
    int coluna_baia = baias[num_baia][1];
    
    
    for (int k = 1; k <= linha_baia; k++)
    {
      matriz[k-1][i] = '_';
      matriz[k][i] = 'S';
      sleep(2);
    }
    //for (int k = i; k < coluna_baia; k++)
    //{
      //matriz[i][k] = 'S';
      //sleep(2);
    //}
  //pthread_mutex_unlock(&mutex);
  sleep(5); 
  return num_baia;
}

void servidor_vai_para_casa(int i, int num_baia){
  pthread_mutex_lock(&mutex);
  printf("Servidor %d: vou deixar a baia %d.\n", i, num_baia);
  pthread_mutex_lock(&baia);
  baias_ocupadas[num_baia] = 0;
  pthread_mutex_unlock(&baia);
  //if(contador == VAGAS){
    pthread_cond_broadcast(&lotou);
    //printf("servidor %d enviou sinal\n", i);
 // }
  contador--;
  pthread_mutex_unlock(&mutex);
  sleep(5);
}

void servidor_fica_esperando(int i){
  pthread_mutex_lock(&mutex);
  //printf("Lotado\n");
  //printf("Servidor %d: vou esperar >:|\n", i);
  contador_servidores++;
  contador--;
  while(contador >= VAGAS){
    //printf("servidor %d esperando sinal\n", i);
    pthread_cond_wait(&lotou, &mutex);
    //printf("servidor %d recebeu sinal\n", i);
  }
  
  contador_servidores--;
  //printf("sai do while\n");
  pthread_mutex_unlock(&mutex);
}

void* servidor(void * a){
  int i = *((int *) a);  
  while(1){

  matriz[1][i]='S';
  mostra_que_quer_entrar();

    if(contador<=VAGAS){

      int baia = servidor_trabalhar(i);

      servidor_vai_para_casa(i, baia);

      break;

    }else{

      servidor_fica_esperando(i);

    }
  
  }
}

void* estagiario(void* a){
  int i = *((int *) a); 
  while(1){
    matriz[0][i]='E';
   // pthread_mutex_lock(&vez);
    //printf("sou estagiario contador: %d  contador servidor: %d\n", contador, contador_servidores);
    if(contador <= VAGAS && contador_servidores == 0){    
      pthread_mutex_lock(&mutex);
      contador++;
      //printf("Estagiario %d trabalhando\n", i);
      pthread_mutex_unlock(&mutex);
      sleep(5);
      pthread_mutex_lock(&mutex);
      //printf("Estagiario %d saiu\n", i);
      contador--;
      pthread_mutex_unlock(&mutex);
      break;
    }
    pthread_mutex_lock(&mutex);
    while(contador >= VAGAS || contador_servidores > 0){
      //printf("estagiario %d vai esperar\n", i);
      pthread_cond_wait(&lotou, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    //pthread_mutex_unlock(&vez);
  }
}

void preenche_matriz(){
  for (int i = 0; i < N; i++){
      for (int j = 0; j < N; j++){
        matriz[i][j]='_';
        matriz[5][19]='B';
        matriz[8][19]='B';
        matriz[11][19]='B';
        matriz[14][19]='B';
      }
    }
}

void* imprime(){
  while(1){
    for (int i = 0; i < N; i++){
      for (int j = 0; j < N; j++){
        printf("%c",matriz[i][j]);
      }
      printf("\n");
    }
    sleep(3);
    system("clear");
  }
}



int main(){

   pthread_t s[SERVIDORES];
   pthread_t e[ESTAGIARIOS];
   pthread_t matriz;
   int i;
   int *id;
   preenche_matriz();
   inicializa_baias();
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


  id = (int *) malloc(sizeof(int));
  *id = i;
  pthread_create(&matriz, NULL, imprime, (void *) (id));
  pthread_join(matriz,NULL);
  

   for (i = 0; i < SERVIDORES ; i++) {
    pthread_join(s[i],NULL);
   }

   for (i = 0; i < ESTAGIARIOS ; i++) {
    pthread_join(e[i],NULL);
   }

   return 0;
}