#include <stdio.h> 
#include "mnist.h"
#include <stdlib.h>

int main(int argc , char ** argv){ 	

    LecunDataType train_imgs,train_labels,test_imgs,test_labels,result;
    char t[5];
    int k;
    double acuracia;
    /*Conversão dos argumentos da main para o nome das estruturas*/ 
    sprintf(train_imgs.filename, "%s", argv[1]);
    sprintf(train_labels.filename, "%s", argv[2]);
    sprintf(test_imgs.filename, "%s", argv[3]);
    sprintf(result.filename,"%s",argv[4]);
    sprintf(t,"%s",argv[5]);
    sprintf(test_labels.filename, "%s", argv[6]);
    k = atoi (t);

    /* Leitura dos arquivos e salvamento dos dados nas respectivas estruturas .*/ 

    read_LecunData(&train_imgs); 
    read_LecunData(&train_labels);
    read_LecunData(&test_imgs);
    read_LecunData(&test_labels);

    /* copia os meta dados dos rotulos para a estrutura dos resultados  e alocando memoria.*/
    result.data_bytes = test_labels.data_bytes;
    result.data_type = test_labels.data_type;
    result.n_dimensions = test_labels.n_dimensions;
    result.dimensions = (int *) calloc(result.n_dimensions, sizeof(int));
    *result.dimensions = *test_labels.dimensions;
    result.total_dims = test_labels.total_dims;
    result.data = (unsigned char *) calloc(test_imgs.dimensions[0],sizeof(unsigned char));

    /* Função utilizada para  classificar a nova imagem teste com a classe tomada como resultado da classificação.Foi  usado o algoritimo KNN (K-nearest neighbor) para tal tarefa
    que recebe um dado nao classificado, e calcula a distancia dele com os dados ja classificados que foram usados como treino. Logo apos verifica os K vizinhos com as menores
    distancias da imagem teste, e toma como resultado a classe que mais apareceu entre os k vizinhos. */
    classify(train_imgs, train_labels, test_imgs, &result,k);  

    /* a função accuracy compara os resultados com os rotulos e calcula a % de acerto*/

    printf("/////////////////////////////////////////\n");
    printf("///////    ACCURACY =  %.2lf%%     ///////\n",get_accuracy(test_labels,result));
    printf("/////////////////////////////////////////\n");

    /*cria um arquivo com o resultado*/
    write_LecunData(result);

    return 0;
}


int classify(LecunDataType train, LecunDataType labels, LecunDataType test, LecunDataType *result, int K){
    DistanceType *distancias; /*Variavel que guardara a distancia da imagem teste das usadas no treino .*/
    distancias=(DistanceType *) calloc(train.dimensions[0], sizeof(DistanceType));

    LecunDataType vetor1, vetor2; 
    int i, j, y, a, b, c; /*Variaveis usadas nos loops.*/
    int rotulotest, numerowin;
    double maior;

        
    vetor1.data = (unsigned char *) calloc(784, sizeof(unsigned char));
    vetor2.data = (unsigned char *) calloc(784, sizeof(unsigned char));

        /*Analise da imagem teste uma por uma.*/
        for(i=0;i<test.dimensions[0];i++){
            /*criação do vetor para armazenar a pontuação*/
            int pontos[]={0,0,0,0,0,0,0,0,0,0};
            
            for(j=0;j<train.dimensions[0];j++){

                for(a=0;a<784;a++){
                    /*copiando 1 imagem teste e 1 de treinamento para 2 vetores afim de que se possa trabalhar com cada imagem individualmente. */
                    vetor1.data[a] = test.data[(i*784)+a];
                    vetor2.data[a] = train.data[(j*784)+a];
                        
                }
                /*calculando a distancia da imagem de teste para cada uma de treinamento*/
                distancias[j].data = distance(&vetor1.data[0], &vetor2.data[0], 784);
                distancias[j].idx = j;
    
            }

            /*ordenação do vetor distancias[]*/
            SortK(&distancias[0],train.dimensions[0],K);
            
            /*  Funcao para analisar  o rotulo das K menores disntacias calculadas pós ordenação e incrementar 
            o vetor de pontos na posiçao do respectivo rotulo. */

            for( b=0;b<K;b++){
                  
                rotulotest = distancias[b].idx;
                y  = labels.data[rotulotest];
                pontos[y]++;
                

            }

            maior = pontos[0];
            numerowin = 0;

            /*Verifica o numero mais votado*/
            for(c=1;c<10;c++){

                if(pontos[c] > maior){
                    maior=pontos[c];
                    numerowin = c;
                }
            }
        /*Define a classe da imagem de teste atual*/
        (*result).data[i] = (unsigned char) numerowin;
          
        }

    return 0;
}