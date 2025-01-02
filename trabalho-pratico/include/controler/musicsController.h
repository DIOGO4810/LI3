#ifndef MUSICSCONTROLLER_H
#define MUSICSCONTROLLER_H
#include "Entitys/musics.h"
#include "controler/artistsController.h"
#include "controler/albumsController.h"

typedef struct musicData MusicData;

MusicData* musicsFeed(char* diretoria, ArtistsData* artistsData, AlbumsData* albumData);

Music* lookup_musica(MusicData* controller, int music_id);

void print_all_musics(MusicData* musica);

void destroyMusicData(MusicData* data);

int getnumGenerosDif (MusicData* musicController);

void atualizaStreams (char* idMusica, MusicData* musicController, ArtistsData* artistcontroller);

char* getMusicGenreControl(void* idMusic, MusicData* musicController,char type);

int getnumartistaMusicControl (MusicData* musicController, int id);

int* getarrayArtistasMusicControl(MusicData* musicController, int id, int numartistas);


#endif