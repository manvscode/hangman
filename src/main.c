#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <time.h>
#include <xtd/console.h>
#include "hangman.h"

extern const char* DICTIONARY[];
extern const size_t DICTIONARY_SIZE;

int main( int argc, char* argv[] )
{
	srand(time(NULL));
	const char* random_word = DICTIONARY[rand() % DICTIONARY_SIZE];

	hangman_t* game = hangman_create( random_word );
	if( !game ) return -1;

	setlocale(LC_ALL, "");

	console_hide_cursor(stdout);
	console_echo_disable(stdin);
	console_canonical_disable(stdin);

	//console_set_size(stdout, 80, 25 );
	hangman_resize(game);
	hangman_run(game);

	console_canonical_enable(stdin);
	console_echo_enable(stdin);
	console_show_cursor( stdout );
	console_clear_screen_all( stdout );
	console_reset( stdout );

	hangman_destroy(&game);
	return 0;
}
