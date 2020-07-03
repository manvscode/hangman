#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <xtd/console.h>
#include <xtd/time.h>
#include <signal.h>
#include <unistd.h>
#include "hangman.h"


#define LETTERS_COUNT  26
#define ESC_KEY        27
#define MAX_WORD_LEN   16


typedef enum hangman_state {
	TITLE,
	IN_GAME,
	GAME_OVER,
	WIN
} hangman_state_t;

static int count_unique_letters(const char* str, size_t len);
static void hangman_draw_title(hangman_t* game);
static void hangman_draw_word_progress(hangman_t* game, int x, int y);
static void hangman_draw_stick_man(hangman_t* game, int x, int y);
static void hangman_draw_game(hangman_t* game);
static void hangman_draw_chosen_letters(hangman_t* game, int x, int y);
static void hangman_draw_game_over(hangman_t* game);
static void hangman_draw_game_won(hangman_t* game);

struct hangman {
	bool quit;
	sigset_t pending_signals;
	hangman_state_t state;
	bool chosen_letters[LETTERS_COUNT];
	char word[MAX_WORD_LEN];
	size_t word_len;
	int moves_left;
	int winning_moves_count;
	int letters_discovered;
};

hangman_t* hangman_create(const char* dictionary[], size_t dictionary_size )
{
	hangman_t* game = malloc( sizeof(hangman_t) );

	if( game )
	{
		*game = (hangman_t) {
			.quit = false,
			.state = TITLE,
			.moves_left = 6,
			.letters_discovered = 0,
		};

		srand( time(NULL) );

		strcpy( game->word, dictionary[rand() % dictionary_size] );
		game->word_len = strlen(game->word);
		game->winning_moves_count = count_unique_letters(game->word, game->word_len);

		memset( game->chosen_letters, 0, sizeof(bool) * LETTERS_COUNT );

		sigset_t signals;
		sigset_t blocked_signals;

		sigfillset(&signals);
		sigemptyset(&blocked_signals);
		sigaddset(&blocked_signals, SIGINT);
		sigaddset(&blocked_signals, SIGQUIT);
		sigaddset(&blocked_signals, SIGWINCH);

		sigprocmask(SIG_BLOCK, &blocked_signals, &signals);
		sigemptyset(&game->pending_signals);
	}

	return game;
}

void hangman_destroy(hangman_t** game)
{
	if( game && *game )
	{
		free(*game);
		*game = NULL;
	}
}

int count_unique_letters(const char* str, size_t len)
{
	char s[ MAX_WORD_LEN ];
	strcpy(s, str);
	bool alphabet[LETTERS_COUNT] = { false };
	int result = 0;

	for( int i = 0; i < len; i++ )
	{
		if( s[i] != '\0' && !alphabet[ s[i] - 'A' ] )
		{
			char letter = s[i];
			alphabet[ letter - 'A' ] = true;
			result += 1;

			for( int j = i; j < len; j++ )
			{
				if( s[j] == letter )
				{
					s[j] = '\0';
				}
			}
		}
	}

	return result;
}

void hangman_run(hangman_t* game)
{
	while( !game->quit )
	{
		if( !sigpending( &game->pending_signals ) )
		{
			if(sigismember(&game->pending_signals, SIGQUIT) ||
			   sigismember(&game->pending_signals, SIGINT))
			{
				game->quit = true;
			}
			if(sigismember(&game->pending_signals, SIGWINCH))
			{
				hangman_resize(game);
			}
		}

		switch( game->state )
		{
			case TITLE:
				hangman_draw_title(game);
				break;
			case IN_GAME:
				hangman_draw_game(game);
				break;
			case GAME_OVER:
				hangman_draw_game_over(game);
				break;
			case WIN:
				hangman_draw_game_won(game);
				break;
			default:
				break;
		}

		time_msleep(100);
	}
}

void hangman_resize(hangman_t* game)
{
	int new_rows = 0;
	int new_cols = 0;
	console_size(stdout, &new_rows, &new_cols);
}

void hangman_draw_title(hangman_t* game)
{
	console_clear_screen_all( stdout );

	console_goto(stdout, 1, 3);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_CYAN);
	printf("                 _   _                                         \n");
	printf("                | | | |                                        \n");
	printf("                | |_| | __ _ _ __   __ _ _ __ ___   __ _ _ __  \n");
	printf("                |  _  |/ _` | '_ \\ / _` | '_ ` _ \\ / _` | '_ \\ \n");
	printf("                | | | | (_| | | | | (_| | | | | | | (_| | | | |\n");
	printf("                \\_| |_/\\__,_|_| |_|\\__, |_| |_| |_|\\__,_|_| |_|\n");
	printf("                                    __/ |                      \n");
	printf("                                   |___/");
	console_reset_fg_color(stdout);

	console_goto(stdout, 36, 4);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_YELLOW);
	printf("ESCAPE THE");
	console_reset_fg_color(stdout);

	console_goto(stdout, 43, 10);
	console_fg_color_8(stdout, CONSOLE_COLOR8_MAGENTA);
	printf("coded by manvscode");
	console_reset_fg_color(stdout);

	console_goto(stdout, 1, 1);
	console_fg_color_8(stdout, CONSOLE_COLOR8_RGB(5, 2, 2));
	printf("      |/|\n");
	printf("      | |\n");
	printf("      |/|\n");
	printf("      | |\n");
	printf("      |/|\n");
	printf("     (___)\n");
	printf("     (___)\n");
	printf("     (___)\n");
	printf("     (___)\n");
	printf("     (___)\n");
	printf("     // \\\\\n");
	printf("    //   \\\\\n");
	printf("   ||     ||\n");
	printf("   ||     ||\n");
	printf("   ||     ||\n");
	printf("    \\\\___//\n");
	printf("     -----\n");
	console_reset_fg_color(stdout);

	console_goto(stdout, 30, 13);
	printf("Press a key");

	console_fg_color_8(stdout, CONSOLE_COLOR8_GREY_10);
	console_curved_box(20, 14, 12, 3);
	console_reset_fg_color(stdout);
	console_goto(stdout, 22, 15);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_GREEN);
	printf("Play (p)");
	console_reset_fg_color(stdout);

	console_fg_color_8(stdout, CONSOLE_COLOR8_GREY_10);
	console_curved_box(38, 14, 14, 3);
	console_reset_fg_color(stdout);
	console_goto(stdout, 40, 15);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_GREEN);
	printf("Quit (ESC)");
	console_reset_fg_color(stdout);

	char key;
	if( scanf("%c", &key ) == 1 )
	{
		switch(key)
		{
			case ESC_KEY:
				game->quit = true;
				break;
			case 'P':
			case 'p':
				game->state = IN_GAME;
				break;
			default:
				break;
		}
	}
}

/*   +------+
 *   |ABCDEF|  Draw a letter box
 *   |GHIJKL|  like this.
 *   |MNOPQR|
 *   |STUVWX|
 *   |YZ    |
 *   +------+
 */
void hangman_draw_chosen_letters(hangman_t* game, int x, int y)
{
	console_fg_color_8(stdout, CONSOLE_COLOR8_GREY_10);
	console_curved_box(x, y, 8, 7);
	console_reset_fg_color(stdout);
	console_fg_color_8(stdout, CONSOLE_COLOR8_YELLOW);
	for( int j = 0; j < 5; j++)
	{
		for( int i = 0; i < 6; i++)
		{
			int idx = j * 6 + i;
			if( idx >= LETTERS_COUNT ) break;

			if( game->chosen_letters[idx] )
			{
				console_goto(stdout, x + i + 1, y + j + 1);
				printf( "%c", 'A' + idx);
			}

		}
	}
	console_reset_fg_color(stdout);
}

/*
 * Draw a word with spaces like this:
 *
 *       A _ _ E _
 */
void hangman_draw_word_progress(hangman_t* game, int x, int y)
{
	console_goto(stdout, x, y);
	for( int i = 0; i < game->word_len; i++)
	{
		bool letter_chosen = game->chosen_letters[ game->word[i] - 'A' ];
		if( letter_chosen )
		{
			console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_YELLOW);
			printf("%c", game->word[i]);
			console_reset_fg_color(stdout);
		}
		else
		{
			console_fg_color_8(stdout, CONSOLE_COLOR8_GREY_10);
			printf("%c", '_');
			console_reset_fg_color(stdout);
		}
		if( i < (game->word_len - 1) ) printf(" ");
	}
}

void hangman_draw_stick_man(hangman_t* game, int x, int y)
{
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_CYAN);
	for( int i = 0; i < 6 - game->moves_left; i++)
	{
		switch( i )
		{
			case 0:
				console_goto(stdout, x, y);
				printf("0");
				break;
			case 1:
				console_goto(stdout, x, y + 1);
				printf("|");
				break;
			case 2:
				console_goto(stdout, x - 1, y + 1);
				printf("/");
				break;
			case 3:
				console_goto(stdout, x + 1, y + 1);
				printf("\\");
				break;
			case 4:
				console_goto(stdout, x - 1, y + 2);
				printf("/");
				break;
			case 5:
				console_goto(stdout, x + 1, y + 2);
				printf("\\");
				break;
		}
	}
	console_reset_fg_color(stdout);
}

void hangman_draw_game(hangman_t* game)
{
	console_clear_screen_all( stdout );

	console_goto(stdout, 1, 1);

	printf("          ______________\n");
	printf("         |             |\n");
	printf("         |              \n");
	printf("         |              \n");
	printf("         |              \n");
	printf("         |___________     ___\n");
	printf("         |          |     | |_\n");
	printf("         |          |     |   |_\n");
	printf("         |__________|_____|_____|\n");

	hangman_draw_chosen_letters(game, 45, 1);
	hangman_draw_word_progress(game, 20, 12);
	hangman_draw_stick_man(game, 24, 3);

	console_fg_color_8(stdout, CONSOLE_COLOR8_GREY_10);
	console_curved_box(12, 15, 23, 3);
	console_reset_fg_color(stdout);

	console_goto(stdout, 14, 16);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_GREEN);
	printf("Pick a Letter [A-Z]");
	console_reset_fg_color(stdout);

	console_fg_color_8(stdout, CONSOLE_COLOR8_GREY_10);
	console_curved_box(38, 15, 14, 3);
	console_reset_fg_color(stdout);

	console_goto(stdout, 40, 16);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_GREEN);
	printf("Quit (ESC)");
	console_reset_fg_color(stdout);


	char key;
	if( scanf("%c", &key ) == 1 )
	{
		switch(key)
		{
			case ESC_KEY:
				game->quit = true;
				break;
			default:
			{
				if( key >= 'a' && key <= 'z' ) key = toupper(key);

				if( key >= 'A' && key <= 'Z' )
				{
					if( !game->chosen_letters[ key - 'A' ] )
					{
						game->chosen_letters[ key - 'A' ] = true;

						if( !strchr(game->word, key) )
						{
							game->moves_left -= 1;
						}
						else
						{
							game->letters_discovered += 1;
						}
					}
				}

				if( game->letters_discovered == game->winning_moves_count )
				{
					game->state = WIN;
				}
				else if( game->moves_left < 0 )
				{
					game->state = GAME_OVER;
				}
				break;
			}
		}
	}
}

void hangman_draw_game_over(hangman_t* game)
{
	console_goto(stdout, 1, 1);
	console_clear_screen_all( stdout );


	printf("   ___________.._______\n");
	printf("  | .__________))______|\n");
	printf("  | | / /      ||\n");
	printf("  | |/ /       ||\n");
	printf("  | | /        ||.-''.\n");
	printf("  | |/         |/  _  \\\n");
	printf("  | |          ||  `/,|\n");
	printf("  | |          (\\\\`_.'\n");
	printf("  | |         .-`--'.\n");
	printf("  | |        /Y . . Y\\\n");
	printf("  | |       // |   | \\\\\n");
	printf("  | |      //  | . |  \\\\\n");
	printf("  | |     ')   |   |   (`\n");
	printf("  | |          ||'||\n");
	printf("  | |          || ||\n");
	printf("  | |          || ||\n");
	printf("  | |          || ||\n");
	printf("  | |         / | | \\ \n");
	printf("  \"\"\"\"\"\"\"\"\"\"|_`-' `-' |\"\"\"|\n");
	printf("  |\"|\"\"\"\"\"\"\"\\ \\       '\"|\"|\n");
	printf("  | |        \\ \\        | |\n");
	printf("  : :         \\ \\       : :\n");
	printf("  . .          `'       . .\n");


	int text_x = 28;
	int text_y = 2;

	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_RED);
	console_goto(stdout, text_x, text_y + 1);  printf(" __   __             _ _        _   _         \n");
	console_goto(stdout, text_x, text_y + 2);  printf(" \\ \\ / /__ _  _   __| (_)___ __| | | |__ _  _ \n");
	console_goto(stdout, text_x, text_y + 3);  printf("  \\ V / _ \\ || | / _` | / -_) _` | | '_ \\ || |\n");
	console_goto(stdout, text_x, text_y + 4);  printf("   |_|\\___/\\_,_| \\__,_|_\\___\\__,_| |_.__/\\_, |\n");
	console_goto(stdout, text_x, text_y + 5);  printf("     | |_  __ _ _ _  __ _(_)_ _  __ _    |__/ \n");
	console_goto(stdout, text_x, text_y + 6);  printf("     | ' \\/ _` | ' \\/ _` | | ' \\/ _` |_       \n");
	console_goto(stdout, text_x, text_y + 7);  printf("     |_||_\\__,_|_||_\\__, |_|_||_\\__, (_)      \n");
	console_goto(stdout, text_x, text_y + 8);  printf("                    |___/       |___/         \n");
	console_reset_fg_color(stdout);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_YELLOW);
	console_goto(stdout, text_x, text_y + 10); printf("       YOUR FAMILY MISSES YOU VERY MUCH.            \n");
	console_goto(stdout, text_x, text_y + 11); printf("              TRY PLAYING AGAIN.                    \n");
	console_reset_fg_color(stdout);

	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_MAGENTA);
	console_goto(stdout, text_x, text_y + 20); printf("          Press any key to continue.                \n");
	console_reset_fg_color(stdout);

	getchar();
	game->quit = true;
}

void hangman_draw_game_won(hangman_t* game)
{
	console_goto(stdout, 1, 1);
	console_clear_screen_all( stdout );

	int text_x = 18;
	int text_y = 3;
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_GREEN);
	console_goto(stdout, text_x, text_y + 0);  printf(" __   __           ___                          _\n");
	console_goto(stdout, text_x, text_y + 1);  printf(" \\ \\ / /__ _  _   | __|___ __ __ _ _ __  ___ __| |\n");
	console_goto(stdout, text_x, text_y + 2);  printf("  \\ V / _ \\ || |  | _|(_-</ _/ _` | '_ \\/ -_) _` |\n");
	console_goto(stdout, text_x, text_y + 3);  printf("   |_|\\___/\\_,_|  |___/__/\\__\\__,_| .__/\\___\\__,_|\n");
	console_goto(stdout, text_x, text_y + 4);  printf("                                  |_|\n");
	console_reset_fg_color(stdout);

	console_goto(stdout, 38, 7);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_YELLOW);
	printf("THE HANGMAN");
	console_reset_fg_color(stdout);

	console_goto(stdout, 30, 12); printf("YOUR FAMILY WANTS YOU TO TURN");
	console_goto(stdout, 30, 13); printf(" AWAY FROM A LIFE OF CRIME.");

	int cowboy_x = 4;
	int cowboy_y = 6;
	console_goto(stdout, cowboy_x, cowboy_y + 0);  printf("        ___\n");
	console_goto(stdout, cowboy_x, cowboy_y + 1);  printf("     __|___|__\n");
	console_goto(stdout, cowboy_x, cowboy_y + 2);  printf("      ('o_o')\n");
	console_goto(stdout, cowboy_x, cowboy_y + 3);  printf("      _\\~-~/_    ______.\n");
	console_goto(stdout, cowboy_x, cowboy_y + 4);  printf("     //\\__/\\ \\ ~(_]---'\n");
	console_goto(stdout, cowboy_x, cowboy_y + 5);  printf("    / )O  O( .\\/_)\n");
	console_goto(stdout, cowboy_x, cowboy_y + 6);  printf("    \\ \\    / \\_/\n");
	console_goto(stdout, cowboy_x, cowboy_y + 7);  printf("    )/_|  |_\\\n");
	console_goto(stdout, cowboy_x, cowboy_y + 8);  printf("   // /(\\/)\\ \\\n");
	console_goto(stdout, cowboy_x, cowboy_y + 9);  printf("   /_/      \\_\\\n");
	console_goto(stdout, cowboy_x, cowboy_y + 10);  printf("  (_||      ||_)\n");
	console_goto(stdout, cowboy_x, cowboy_y + 11);  printf("    \\| |__| |/\n");
	console_goto(stdout, cowboy_x, cowboy_y + 12);  printf("     | |  | |\n");
	console_goto(stdout, cowboy_x, cowboy_y + 13);  printf("     | |  | |\n");
	console_goto(stdout, cowboy_x, cowboy_y + 14);  printf("     |_|  |_|\n");
	console_goto(stdout, cowboy_x, cowboy_y + 15);  printf("     /_\\  /_\\\n");


	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_MAGENTA);
	console_goto(stdout, text_x, text_y + 20); printf("          Press any key to continue.                \n");
	console_reset_fg_color(stdout);
	getchar();
	game->quit = true;
}
