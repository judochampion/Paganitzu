#pragma comment(lib, "jvm.lib")

/* This pragma is needed in the object with main, to make JNI work in Visual C++ */

#include "level.h"
#include "game.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <crtdbg.h>
#include "spider.h"

int main(int argc, char *argv[]) {

	//INITIALISATIES EN GEHEUGENALLOCATIES: 

	unsigned int i=0,level_count=0;
	const unsigned int numberoflevels=argc-1;
	LevelState** commando_argument_levels = (LevelState**) malloc(sizeof(LevelState*)*(numberoflevels));
	
	
	//INTERPRETATIE COMMAND LINE ARGUMENTEN
	// Afhankelijk van de argumenten, de juiste levels laden:

	//Als er geen argumenten zijn meegegeven, wordt de vaste lijst van 9 default levels ingelezen:
	if(numberoflevels==0){
		char* default_level_filenames[9] = { "levels/1.lvl", "levels/2.lvl", "levels/3.lvl", "levels/4.lvl", "levels/5.lvl", "levels/6.lvl", "levels/7.lvl", "levels/8.lvl", "levels/9.lvl" };
		LevelState* defaultlevels[9];
		for(i=0; i< 9; i++){
			level_count++;
			defaultlevels[i] = read_level(default_level_filenames[i]);	
		}
		game_loop(defaultlevels, level_count);
	}
	else{
		//Als er wel argumenten worden meegegeven, is argc-1 gelijk aan het aantal argumenten en de argumenten zitten in char * arg[v]

		/* i=0 slaan we over omdat het eerste elemant in argv[], namelijk argv[0]  niet de naam van een level bevat, maar de bestandsnaam*/
		for(i=1; i <= numberoflevels; i++){	
			if(strcmp(argv[i],"example")==0) 
				commando_argument_levels[i-1]=load_example_level();
		
			else
				commando_argument_levels[i-1]=read_level(argv[i]);
		}

		game_loop(commando_argument_levels,numberoflevels);		
	}
	
	/*De gealloceerde rijen met LevelStates dienen niet vrijgegeven te worden, omdat ze 
	al vrijgegeven worden net voor het beëindigen van de gameloop. */
	free(commando_argument_levels);

	printf("Bedankt om te spelen!\n\n");
	_CrtDumpMemoryLeaks();
	return 0;
}

