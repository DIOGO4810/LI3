#include "controler/usersController.h"
#include "controler/musicsController.h"
#include "controler/albumsController.h"
#include "validacao/validaHistory.h"
#include "utilidades.h"
#include "Entities/users.h"
#include "Entities/history.h"
#include "Input.h"
#include "Output.h"

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "Entities/history.h"
#include "controler/historyController.h"



struct historyData{
    GHashTable* Domingo;
    GHashTable* history;
};


GHashTable* createHistoryTable() {
    GHashTable* Domingo = g_hash_table_new_full(g_str_hash, g_str_equal, free, (GDestroyNotify)freeDomingo);
    if (!Domingo) {
        fprintf(stderr, "Erro: Hash table Domingo não foi criada.\n");
        exit(1);
    } else {
        printf("Hash table Domingo criada com sucesso.\n");
    }
    return Domingo;
}

GHashTable* createUserHistoryTable() {
    GHashTable* history = g_hash_table_new_full(g_int_hash, g_int_equal, free, (GDestroyNotify)freeUserHistory);

    if (!history) {
        fprintf(stderr, "Erro: Hash table history não foi criada.\n");
        exit(1);
    } else {
        printf("Hash table history criada com sucesso.\n");
    }

    return history;
}

Domingo* lookup_domingo(GHashTable* domingo, char* data) {
    if (!domingo) {
        fprintf(stderr, "Erro: Tabela Domingo é NULL.\n");
        return NULL;
    }
    return g_hash_table_lookup(domingo, data);
}

void newDomingo_orNot(HistoryData* controller, char** tokens, MusicData* musicController) {

    char* data = malloc(11 * sizeof(char));
    //tira as horas a que a musica foi ouvida
    strncpy(data, tokens[3]+1, 10);
    data[10] = '\0'; 

    char* domingo_anterior = malloc(11 * sizeof(char));
    //calcula o domingo dessa semana para usar como key da hash table externa
    calcularDomingoAnterior(data, domingo_anterior);

    //liberta-se a data original já que não vai ser mais usada
    free(data);
    //verifica se já eiste uma hashtable com domingo_anterior como key
    if (g_hash_table_contains(controller->Domingo, domingo_anterior)) {
        // Se já existe, adiciona à hash table interna
        Domingo* domingo_atual = lookup_domingo(controller->Domingo, domingo_anterior);
        if (domingo_atual) {
            new_or_add((domingo_atual), tokens, musicController);
        }
    } else {
        // se nao existir essa data então cria um novo domingo
        Domingo* n_domingo = newDomingo(domingo_anterior);
        g_hash_table_insert(controller->Domingo, strdup(domingo_anterior), n_domingo);
    }

    free(domingo_anterior);  //liberta a data
}

History* lookup_UserHistory(HistoryData* historyController,int userId)
{
    return g_hash_table_lookup(historyController->history, &userId);
}


void addhistory(HistoryData* history, char* user_id,char* music_id,char* timestamp,char* duration )
{

        int ano, mes,dia,hora;
        sscanf(timestamp,"\"%d/%d/%d %d",&ano,&mes,&dia,&hora); 
        int musicId=transformaIds(music_id);  
        int userId= transformaIds(user_id);
        int duracao= duration_to_seconds(duration);  
        History* userHistory=lookup_UserHistory(history, userId);
        
    if(userHistory==NULL)
    {
        int* key = malloc(sizeof(int));  // Aloca memória para a chave
        *key = userId;
        userHistory= inicializaUserHistory(userId, musicId, ano,mes,dia,hora, duracao); 
         g_hash_table_insert(history->history,key, userHistory); 
    }
    else
    {    
        adicionaUserHistory(userHistory, musicId, ano,mes,dia,hora, duracao);
    }
}


//Basicamente a ideia desta função é passar todas as hashtables para garrays de 10 já ordenados e apagar a hashtable 
void passa_hastable_para_garray (HistoryData* data){

    GHashTableIter iter;
    gpointer key, value;

    // Itera sobre a GHashTable e transfere os dados para o GArray
    g_hash_table_iter_init(&iter, data->Domingo);

    while (g_hash_table_iter_next(&iter, &key, &value)){

        Domingo* domingo_atual = value;
        passa_Domingo_para_garray(domingo_atual);
    } 
    
}


HistoryData* historyFeed(char* diretoria, MusicData* musicData, ArtistsData* artistData,UsersData* usersData) {

    HistoryData* Hdata = malloc(sizeof(HistoryData));
    if (!Hdata) {
        fprintf(stderr, "Erro: Alocação de memória para HistoryData falhou.\n");
        exit(1);
    }
    
    Output * Erros= iniciaOutput("resultados/history_errors.csv");

    Hdata->Domingo = createHistoryTable();
    Hdata->history=createUserHistoryTable();

    Parser* parserE = newParser(diretoria, "history.csv");
    // Ler a primeira linha do ficheiro  
    char* linha= pegaLinha(parserE);
    //Enviar a linha para o ficheiro "history_erros.csv", esta não será inserida na hashTable
    outputErros(Erros,linha);
    free(linha);

    while (1) {

        parserE = parser(parserE);
        char** tokens = getTokens(parserE);
        if (!tokens) {
         
            freeParser(parserE);
            break;
        }

        char* linhaE=getLineError(parserE);
        int isValid = validaHistory(tokens[5],tokens[4],Erros,linhaE);
        if(isValid) {

            addhistory(Hdata,tokens[1],tokens[2],tokens[3], tokens[4]);
            char* genre = getMusicGenreControl(tokens[2],musicData,'s');
            atualizaPrefsUser(genre,tokens[1],usersData);
            newDomingo_orNot(Hdata, tokens, musicData); 
            atualizaStreams(tokens[2], musicData, artistData);
        }  
            free(linhaE);
            free(getLine(parserE));  
        }
    
        freeOutput(Erros);
        passa_hastable_para_garray(Hdata);
        return Hdata;
}


int getPosicaoAno(HistoryData* historyController,int user_id, int ano)
{
    History* userHistory= lookup_UserHistory(historyController, user_id);
    if(userHistory==NULL) 
    {
        return -1; 
    }
    int i= procuraAno(userHistory,ano);
    return i;
}


char** getNartistasMaisOuvidos(HistoryData* historyController, MusicData*musicController, int user_id,int  posicaoAno,int N)
{
     History* userHistory= lookup_UserHistory(historyController, user_id);
     char** resultados= NartistasMaisOuvidos(userHistory,musicController, posicaoAno,N);
    return resultados;

}



char* getDia(HistoryData*  historyController,int user_id,int ano)
{ 
    History* userHistory= lookup_UserHistory(historyController, user_id);
    char* data=DataMaisMusicas(userHistory,ano); //////Falta dar copia 
    return data;
}

char * getAlbumGenero(HistoryData* historyController,MusicData* musicController, AlbumsData*albumController, int userId,int posicaoAno)
{
    History* userHistory= lookup_UserHistory(historyController, userId);
    char* resultados=   AlbumGenero(musicController, userHistory,albumController, posicaoAno);
    return resultados;
}

char* getHora(HistoryData* historyController,int user_id,int ano)
{
    History* userHistory= lookup_UserHistory(historyController, user_id);
    char* hora= HoraMaisAudicoes(userHistory, ano);//////Falta dar copia
    return hora; 
}

//FAZER PRINT
void print_all_history (HistoryData* Hdata){
    printf("----- Hash Table do HISTORICO -----\n");
    sleep(3);
    g_hash_table_foreach(Hdata->Domingo, print_semana_completa, NULL);

    printf("----- Fim da Hash Table do HISTORICO-----\n");
}

void destroyHistoryData(HistoryData* data) {

        g_hash_table_destroy(data->Domingo);
        g_hash_table_destroy(data->history);
    
    printf("Tabela DOMINGO destruida\n");

}

//Q4

int id_maiores_ocorrencias(HistoryData* HistoryController, int* maior_n_ocorrencias) {

    GHashTable* domingo = HistoryController->Domingo;
    //Criamos a hash table auxiliar
    GHashTable* hash_auxiliar = g_hash_table_new_full(g_int_hash, g_int_equal, free, free);

    GHashTableIter iter;
    gpointer key, value;

    int max_ocorrencias = -1;
    int mais_freq_artist = -1;

    //Começamos a ver A hashtable externa
    g_hash_table_iter_init(&iter, domingo);

    while (g_hash_table_iter_next(&iter, &key, &value)) {
        //hash table externa
        Domingo* semana_atual = (Domingo*)value;
        //GArray interno
        GArray* top_semanal = get_artistahistory_garray_copy(semana_atual);

        for (int i = 0; i < (int)top_semanal->len; i++) {
            //Vamos iterar o GArray interno
            UmArtista* artista_atual = g_array_index(top_semanal, UmArtista*, i);
            int id_atual = get_artist_id_from_garray(artista_atual);
            // Aloca memória para a chave para evitar usar algo local 
            int* id_atual_ptr = g_new(int, 1);
            *id_atual_ptr = id_atual;
            // Verifica se a chave já existe na hash table
            int* ocorrencia_atual_ptr = g_hash_table_lookup(hash_auxiliar, id_atual_ptr);

            if (!ocorrencia_atual_ptr) {
                // Não existe na tabela; inicializa com 1
                int* nova_ocorrencia = g_new(int, 1);
                *nova_ocorrencia = 1;
                g_hash_table_insert(hash_auxiliar, id_atual_ptr, nova_ocorrencia);
               // g_free(nova_ocorrencia);
            } else {
                // Já existe; incrementa
                (*ocorrencia_atual_ptr)++;
                g_free(id_atual_ptr); // Libera a chave alocada para evitar duplicação
            }
            // Atualiza o artista mais frequente
            int ocorrencia_atual = ocorrencia_atual_ptr ? *ocorrencia_atual_ptr : 1;

            if (ocorrencia_atual > max_ocorrencias || 
                (ocorrencia_atual == max_ocorrencias && id_atual < mais_freq_artist)) {
                max_ocorrencias = ocorrencia_atual;
                mais_freq_artist = id_atual;
                *maior_n_ocorrencias = max_ocorrencias;
            }
        }
        free_garray_with_data(top_semanal);
        //g_array_free(top_semanal, TRUE);
    }
    g_hash_table_destroy(hash_auxiliar);
    return mais_freq_artist;
}




//Assume-se que as datas recebidas por esta funcao já sao os domingos 
int artista_mais_frequente_com_data (HistoryData* HistoryController, char* data_inicio, char* data_fim, int *ocorrencia_final){

    GHashTable* semanas = HistoryController->Domingo;

    int artista_mais_frequente = -1;
    int max_ocorrencias = 0;

    GHashTable* contador_artistas = g_hash_table_new_full(g_int_hash, g_int_equal, free, free);
    // Iterar sobre as semanas no intervalo
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, semanas);

if (data_inicio != NULL && data_fim != NULL && strcmp(data_inicio, "") != 0 && strcmp(data_fim, "") != 0) {
    while (g_hash_table_iter_next(&iter, &key, &value)) {

        char data_atual[11]; 
        calcularDomingoAnterior((char*)key, data_atual);
        // Verificar se está no intervalo
        if ( strcmp(data_atual, data_inicio) >=0 && strcmp(data_atual, data_fim) <= 0) {
        //hash table externa
        Domingo* semana_atual = (Domingo*)value;
        //GArray interno
        GArray* top_semanal = get_artistahistory_garray_copy(semana_atual);

            // Processar o GArray (top 10 artistas)
            for (int i = 0; i < (int)top_semanal->len; i++) {

                UmArtista* artista_atual = g_array_index(top_semanal, UmArtista*, i);

                int id_atual = get_artist_id_from_garray(artista_atual);

                // Aloca memória para a chave para evitar usar algo local 
                int* id_atual_ptr = g_new(int, 1); 
                *id_atual_ptr = id_atual;

                // Verifica se a chave já existe na hash table
                int* ocorrencia_atual_ptr = g_hash_table_lookup(contador_artistas, id_atual_ptr);

                if (!ocorrencia_atual_ptr) {
                    // Não existe na tabela; inicializa com 1
                    int* nova_ocorrencia = g_new(int, 1); 
                    *nova_ocorrencia = 1;
                    g_hash_table_insert(contador_artistas, id_atual_ptr, nova_ocorrencia);

                } else {
                    // Já existe; incrementa
                    (*ocorrencia_atual_ptr)++;
                    g_free(id_atual_ptr); // Liberta a chave para nao correr o risco de haver algum tipo de duplicaçao
                }

                int ocorrencia_atual = ocorrencia_atual_ptr ? *ocorrencia_atual_ptr : 1;
                
                if (ocorrencia_atual > max_ocorrencias 
                || (ocorrencia_atual == max_ocorrencias && id_atual < artista_mais_frequente)){
                    max_ocorrencias = ocorrencia_atual;
                    artista_mais_frequente = id_atual;
                    *ocorrencia_final =  max_ocorrencias;

                }
            }
            free_garray_with_data(top_semanal);
            //g_array_free(top_semanal, TRUE);
        }
    }
}
    //DEBUG
    // if (artista_mais_frequente != -1) {
    //     printf("O artista mais frequente no top 10 foi o ID %d, com %d ocorrências.\n", artista_mais_frequente, max_ocorrencias);
    // } else {
    //     printf("Nenhum artista encontrado no intervalo especificado.\n");
    // }

    g_hash_table_destroy(contador_artistas);
    return artista_mais_frequente;
}
