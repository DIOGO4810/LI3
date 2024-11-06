#include "../../include/controler/usersController.h"
#include "../../include/controler/musicsController.h"
#include "controler/usersController.h"
#include "utilidades.h"
#include "Entitys/musics.h"
#include "Entitys/users.h"
#include "querie/querie3.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <unistd.h> 

struct querie3
{
   char* genero;
   int numMusicas;
};

void querie3(int num, int min , int max, UsersData* userController)
{
   struct querie3 array[15];
   for (int i = 0; i < 15; i++)
   {
         array[i].numMusicas = -1;
   array[i].genero = NULL;
   }
   



   for (int i = min; i < max + 1; i++) {
      for (int j = 0; j < getUBANGeneros(userController, i); j++) {     
         int inserido = 0;
         int a;
         char* genero = getUBAGenero(userController, i, j);   
         int numSongs = getUBANSongs(userController, i, j); 

         // Verificar se o gênero já está presente no array
         for (a = 0; array[a].numMusicas != -1 && !inserido; a++) {
            if (strcmp(array[a].genero, genero) == 0) {
               array[a].numMusicas += numSongs;
               inserido = 1;
            }
         }
         
         // Insere o gênero no array se não estiver presente
         if (!inserido) {
            array[a].genero = strdup(genero);
            array[a].numMusicas = numSongs;
         }
         free(genero);  // Liberar o gênero temporário obtido de getUBAGenero
      }
   }

   // Organiza o array
   if (array[0].numMusicas != -1) {
      for (int i = 1; array[i].numMusicas != -1; i++) {
         struct querie3 key = array[i];
         int nm = array[i].numMusicas;
         char* gnr = array[i].genero;
         int j = i - 1;

         while (j >= 0 && (array[j].numMusicas < nm || (array[j].numMusicas == nm && strcmp(gnr, array[j].genero) < 0))) {
            array[j + 1] = array[j];
            j = j - 1;
         }
         array[j + 1] = key;
      }
   }

   char *filename = malloc(sizeof(char) * 256);
   sprintf(filename, "resultados/command%d_output.txt", num + 1);
   FILE *output_file = fopen(filename, "w");

   if (array[0].numMusicas == -1) {
      fprintf(output_file, "\n");
   } else {
      for (int i = 0; array[i].numMusicas != -1; i++) {
         fprintf(output_file, "%s;%d\n", array[i].genero, array[i].numMusicas);
         free(array[i].genero);  // Liberar cada string de gênero após escrever no arquivo
      }
   }

   fclose(output_file);
   free(filename);
}
