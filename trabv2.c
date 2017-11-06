#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"

#define SERVIDORES 6
#define ESTAGIARIOS 3
#define N 20
#define FALSE 0

int VAGAS = 4;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t baia = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t chefe_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t lotou = PTHREAD_COND_INITIALIZER;  
pthread_cond_t acorda_chefe = PTHREAD_COND_INITIALIZER;                   
int contador = 0;
int contador_servidores = 0;
int contador_estagiarios = 0;
int chama_chefe = FALSE;

char matriz [N][N];
int baias[5][2];
int baias_ocupadas[5] = {0};

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

void sai_da_baia(int num_baia, int i){
  int linha = baias[num_baia][0];
  int coluna = baias[num_baia][1];
  matriz[linha][coluna - 1] = '_';
  matriz[linha+1][coluna - 1] = '0' + i;
  sleep(1);
  matriz[linha+1][coluna - 1] = '_';
  matriz[linha+1][coluna] = '0' + i;
  sleep(1);
  matriz[linha+1][coluna] = '_';
}

void mostra_que_quer_entrar(){
  pthread_mutex_lock(&mutex);
  contador++;
  pthread_mutex_unlock(&mutex);
}

int servidor_trabalha(int i){
  pthread_mutex_lock(&baia);
  int num_baia;
  for(int k = 0; k <= VAGAS; k++) {
    if(baias_ocupadas[k] == 0){
      baias_ocupadas[k] = 1;
      num_baia = k;
      break;
    } 
  }
  pthread_mutex_unlock(&baia);
    int linha_baia = baias[num_baia][0];
    int coluna_baia = baias[num_baia][1];
    for (int k = 1; k <= linha_baia; k++)
    {
      matriz[k-1][i] = '_';
      matriz[k][i] = '0' + i;
      sleep(1);
    }
     for (int k = i; k < coluna_baia; k++)
     {
       matriz[linha_baia][k-1] = '_';
       matriz[linha_baia][k] = '0' + i;
       sleep(1);
     }
  sleep(10); 
  return num_baia;
}

void servidor_vai_para_casa(int i, int num_baia){
  pthread_mutex_lock(&mutex);
  pthread_mutex_lock(&baia);
  baias_ocupadas[num_baia] = 0;
  pthread_mutex_unlock(&baia);
  pthread_cond_broadcast(&lotou);
  contador--;
  pthread_mutex_unlock(&mutex);
  sai_da_baia(num_baia, i);
  sleep(5);
}

void servidor_fica_esperando(int i){
  pthread_mutex_lock(&mutex);
  contador_servidores++;
  contador--;
  pthread_mutex_lock(&chefe_mutex);
  if(!chama_chefe){
    sleep(5);
    chama_chefe = 1;
    pthread_cond_signal(&acorda_chefe);
  }
  pthread_mutex_unlock(&chefe_mutex);
  while(contador >= VAGAS){
    pthread_cond_wait(&lotou, &mutex);
  }
  contador_servidores--;
  pthread_mutex_unlock(&mutex);
}

void* servidor(void * a){
  int i = *((int *) a);  
  while(1){
  matriz[1][i]='S';
  mostra_que_quer_entrar();
    if(contador<=VAGAS){
      int baia = servidor_trabalha(i);
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
    if(contador <= VAGAS && contador_servidores == 0){    
      pthread_mutex_lock(&mutex);
      contador++;
      pthread_mutex_unlock(&mutex);
      pthread_mutex_lock(&baia);
      int num_baia;
      for(int k = 0; k <= VAGAS; k++) {
        if(baias_ocupadas[k] == 0){
          baias_ocupadas[k] = 1;
          num_baia = k;
          break;
        } 
      }
      pthread_mutex_unlock(&baia);
      int linha_baia = baias[num_baia][0];
      int coluna_baia = baias[num_baia][1];
      for (int k = 1; k <= linha_baia; k++){
        matriz[k-1][i] = '_';
        matriz[k][i] = '0' + i;
        sleep(1);
      }
      for (int k = i; k < coluna_baia; k++){
        matriz[linha_baia][k-1] = '_';
        matriz[linha_baia][k] = '0' + i;
        sleep(1);
      }
      sleep(20); 
      pthread_mutex_lock(&mutex);
      contador--;
      pthread_mutex_unlock(&mutex);
      pthread_mutex_lock(&baia);
      baias_ocupadas[num_baia] = 0;
      pthread_mutex_unlock(&baia);
      sai_da_baia(num_baia, i);
      break;
    }
    pthread_mutex_lock(&mutex);
    while(contador >= VAGAS || contador_servidores > 0){
      pthread_cond_wait(&lotou, &mutex);
    }
    pthread_mutex_unlock(&mutex);
  }
}

void* chefe_vai_ao_almoxarifado(void* a){
  int i = *((int *) a); 
  matriz[i][15] = 'C';
  pthread_mutex_lock(&chefe_mutex);
  while(!chama_chefe){
    pthread_cond_wait(&acorda_chefe, &chefe_mutex);
  }
  pthread_mutex_unlock(&chefe_mutex);
  sleep(10);
  pthread_mutex_lock(&baia);
  matriz[17][19] = 'B';
  baias[4][0] = 17;
  baias[4][1] = 19;
  baias_ocupadas[5] = 0;
  VAGAS++;
  pthread_cond_broadcast(&lotou);
  pthread_mutex_unlock(&baia);
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
    sleep(1);
    system("clear");
  }
}

int main(){

  pthread_t s[100];
  pthread_t e[100];
  pthread_t matriz;
  pthread_t chefe;
  int i;
  int *id;
  preenche_matriz();
  inicializa_baias();

  id = (int *) malloc(sizeof(int));
  *id = i;
  pthread_create(&chefe, NULL, chefe_vai_ao_almoxarifado, (void *) (id));

  id = (int *) malloc(sizeof(int));
  *id = i;
  pthread_create(&matriz, NULL, imprime, (void *) (id));

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

  int num_threads = SERVIDORES;
  while(1){
  time_t t;
  srand((unsigned) time(&t));      
  int random = rand() % 2;
    if(random == 0){
      id = (int *) malloc(sizeof(int));
      *id = num_threads;
      pthread_create(&s[num_threads], NULL, servidor, (void *) (id));
    }else{
      id = (int *) malloc(sizeof(int));
      *id = num_threads;
      pthread_create(&e[num_threads], NULL, estagiario, (void *) (id));
    }
    num_threads++;
    sleep(30);
  }

  for (i = 0; i < SERVIDORES ; i++) {
    pthread_join(s[i],NULL);
  }

  for (i = 0; i < ESTAGIARIOS ; i++) {
    pthread_join(e[i],NULL);
  }

  pthread_join(matriz,NULL);
  pthread_join(chefe,NULL);

  return 0;
}