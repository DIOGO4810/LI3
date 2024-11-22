#ifndef UTILIDADES_H
#define UTILIDADES_H

#include<stdlib.h>
#include<stdio.h>
#include <glib.h>
#include"Entitys/users.h"






int calcular_idade( char* data_nascimento_str);

guint get_garray_length(GArray *array);
char* seconds_to_hhmmss(int total_seconds);
char* remove_quotes(char* str);
void freeCleanerArtist(char* clean_id,char* clean_name,char* clean_description,char* ganhos, char* clean_country,char* clean_type);
void freeCleanerMusics(char* music_id,char* music_title,char* music_artist_id,char* music_duration,char* music_genre,char* music_year,char* music_lyrics);
void freeCleanerUsers(char* username,char* email,char* nome ,char*  apelido,char* birth_date,char*  country,char* subscription_type);
int validaData(char* date);
int validaEmail (char* email);
int calculate_num_members( char* grupo);
int validaDuraçao (char* duracao);
void freeArray(int* array) ;
int compararFicheirosPorLinha(char *file1,char *file2, int *ocorrenciasCorretas);


char** divideArtists(char* music_artist);
char** divideGroup(char* group, int numMembros);
int contar_elementos(char *str);
//char** likedSongs(char* songs, int numberS);
int* likedSongs(char* songs, int numberS);

int transformaIds (char* idString);


#endif