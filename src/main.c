#include <stdio.h>
//#include <wchar.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>
#include <xtd/console.h>
#include <xtd/time.h>
#include <signal.h>
#include <unistd.h>

typedef enum game_state {
	TITLE,
	IN_GAME,
	GAME_OVER,
	WIN
} game_state_t;


typedef struct app {
	bool quit;
	game_state_t state;
	bool chosen_letters[26];
	char word[16];
	int moves_left;
	int letters_discovered;
} app_t;

void resize(app_t* app)
{
	int new_rows = 0;
	int new_cols = 0;
	console_size(stdout, &new_rows, &new_cols);
}
#define CURVED_BOX_TLCORNER L'\x256d' // ╭
#define CURVED_BOX_TRCORNER L'\x256e' // ╮
#define CURVED_BOX_BRCORNER L'\x256f' // ╯
#define CURVED_BOX_BLCORNER L'\x2570' // ╰
#define CURVED_BOX_HLINE    L'\x2500' // ╶
#define CURVED_BOX_VLINE    L'\x2502' // ╷

void draw_curved_box(int x, int y, int w, int h, int fg, int bg)
{
	if( x <= 0 ) x = 1;
	if( y <= 0 ) y = 1;
	if( w <= 0 ) w = 3;
	if( h <= 0 ) h = 3;

	int inner_width  = w - 2;
	int inner_height = h - 2;

	if( fg > 0 )
	{
		console_fg_color_8(stdout, fg);
	}

	if( bg > 0 )
	{
		console_bg_color_8(stdout, bg);
	}

	console_goto(stdout, x, y);
	printf("%lc", CURVED_BOX_TLCORNER );
	for( int i = 0; i < inner_width; i++ )
	{
		printf("%lc", CURVED_BOX_HLINE );
	}
	printf("%lc", CURVED_BOX_TRCORNER );

	for( int i = 0; i < inner_height; i++ ) // vertical lines on left
	{
		console_goto(stdout, x, y + i + 1);
		printf("%lc", CURVED_BOX_VLINE );
	}

	for( int i = 0; i < inner_height; i++ ) // vertical lines on right
	{
		console_goto(stdout, x + inner_width + 1, y + i + 1);
		printf("%lc", CURVED_BOX_VLINE );
	}

	console_goto(stdout, x, y + inner_height + 1);
	printf("%lc", CURVED_BOX_BLCORNER );
	for( int i = 0; i < inner_width; i++ )
	{
		printf("%lc", CURVED_BOX_HLINE );
	}
	printf("%lc", CURVED_BOX_BRCORNER );

	if( fg > 0 )
	{
		console_reset_fg_color(stdout);
	}

	if( bg > 0 )
	{
		console_reset_bg_color(stdout);
	}
}


void draw_title(app_t* app)
{
	console_clear_screen_all( stdout );

	console_goto(stdout, 0, 0);
	printf("\n\n");
	printf("            %s   _   _                                         \n", "\033[38;5;14m");
	printf("            %s  | | | |          %sESCAPE THE%s                \n", "\033[38;5;14m", "\033[38;5;11m", "\033[39m");
	printf("            %s  | |_| | __ _ _ __   __ _ _ __ ___   __ _ _ __  \n", "\033[38;5;14m");
	printf("            %s  |  _  |/ _` | '_ \\ / _` | '_ ` _ \\ / _` | '_ \\ \n", "\033[38;5;14m");
	printf("            %s  | | | | (_| | | | | (_| | | | | | | (_| | | | |\n", "\033[38;5;14m");
	printf("            %s  \\_| |_/\\__,_|_| |_|\\__, |_| |_| |_|\\__,_|_| |_|\n", "\033[38;5;14m");
	printf("            %s                      __/ |                      \n", "\033[38;5;14m");
	printf("            %s                     |___/   ", "\033[38;5;14m");
	printf("%scoded by manvscode%s\n", "\033[38;5;13m", "\033[39m");

	console_goto(stdout, 0, 0);
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

	draw_curved_box(20, 14, 12, 3, CONSOLE_COLOR8_GREY_10, -1 );
	console_goto(stdout, 22, 15);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_GREEN);
	printf("Play (p)");
	console_reset_fg_color(stdout);

	draw_curved_box(38, 14, 14, 3, CONSOLE_COLOR8_GREY_10, -1 );
	console_goto(stdout, 40, 15);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_GREEN);
	printf("Quit (ESC)");
	console_reset_fg_color(stdout);

	char key;
	if( scanf("%c", &key ) == 1 )
	{
		switch(key)
		{
			case 27:
				app->quit = true;
				break;
			case 'P':
			case 'p':
				app->state = IN_GAME;
				break;
			default:
				break;
		}
	}

}

void draw_game(app_t* app)
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


	//draw_curved_box(int x, int y, int w, int h, int fg, int bg)
	// +------+
	// |ABCDEF|
	// |GHIJKL|
	// |MNOPQR|
	// |STUVWX|
	// |YZ    |
	// +------+
	draw_curved_box(45, 1, 8, 7, CONSOLE_COLOR8_GREY_10, -1);
	console_fg_color_8(stdout, CONSOLE_COLOR8_YELLOW);
	for( int y = 0; y < 5; y++)
	{
		for( int x = 0; x < 6; x++)
		{
			int idx = y * 6 + x;
			if( idx >= 26 ) break;

			if( app->chosen_letters[idx] )
			{
				console_goto(stdout, 46 + x, 2 + y);
				printf( "%c", 'A' + idx);
			}

		}
	}
	console_reset_fg_color(stdout);


	// Draw word
	console_goto(stdout, 20, 12);
	for( int i = 0; i < 6; i++)
	{
		bool letter_chosen = app->chosen_letters[ app->word[i] - 'A' ];
		if( letter_chosen )
		{
			console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_YELLOW);
			//console_underline_begin(stdout);
			printf("%c", app->word[i]);
			//console_underline_end(stdout);
			console_reset_fg_color(stdout);
		}
		else
		{
			console_fg_color_8(stdout, CONSOLE_COLOR8_GREY_10);
			printf("%c", '_');
			console_reset_fg_color(stdout);
		}
		if( i < 5 ) printf(" ");
	}

	// Draw stickman
	int stick_man_x = 24;
	int stick_man_y = 3;
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_CYAN);
	for( int i = 0; i < 6 - app->moves_left; i++)
	{
		switch( i )
		{
			case 0:
				console_goto(stdout, stick_man_x, stick_man_y);
				printf("0");
				break;
			case 1:
				console_goto(stdout, stick_man_x, stick_man_y + 1);
				printf("|");
				break;
			case 2:
				console_goto(stdout, stick_man_x - 1, stick_man_y + 1);
				printf("/");
				break;
			case 3:
				console_goto(stdout, stick_man_x + 1, stick_man_y + 1);
				printf("\\");
				break;
			case 4:
				console_goto(stdout, stick_man_x - 1, stick_man_y + 2);
				printf("/");
				break;
			case 5:
				console_goto(stdout, stick_man_x + 1, stick_man_y + 2);
				printf("\\");
				break;
		}
	}
	console_reset_fg_color(stdout);



	draw_curved_box(12, 15, 23, 3, CONSOLE_COLOR8_GREY_10, -1 );
	console_goto(stdout, 14, 16);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_GREEN);
	printf("Pick a Letter [A-Z]");
	console_reset_fg_color(stdout);

	draw_curved_box(38, 15, 14, 3, CONSOLE_COLOR8_GREY_10, -1 );
	console_goto(stdout, 40, 16);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_GREEN);
	printf("Quit (ESC)");
	console_reset_fg_color(stdout);


	char key;
	if( scanf("%c", &key ) == 1 )
	{
		switch(key)
		{
			case 27:
				app->quit = true;
				break;
			default:
			{
				if( key >= 'a' && key <= 'z' ) key = toupper(key);

				if( key >= 'A' && key <= 'Z' )
				{
					if( !app->chosen_letters[ key - 'A' ] )
					{
						app->chosen_letters[ key - 'A' ] = true;

						if( !strchr(app->word, key) )
						{
							app->moves_left -= 1;
						}
						else
						{
							app->letters_discovered += 1;
						}
					}
				}

				if( app->letters_discovered == 6 )
				{
					app->state = WIN;
				}
				else if( app->moves_left < 0 )
				{
					app->state = GAME_OVER;
				}
				break;
			}
		}
	}
}

void draw_game_over(app_t* app)
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

#if 0
	console_goto(stdout, 30, 1);
	printf("__   __                _ _          _   _           \n");
	printf("\\ \\ / /               | (_)        | | | |          \n");
	printf(" \\ V /___  _   _    __| |_  ___  __| | | |__  _   _ \n");
	printf("  \\ // _ \\| | | |  / _` | |/ _ \\/ _` | | '_ \\| | | |\n");
	printf("  | | (_) | |_| | | (_| | |  __/ (_| | | |_) | |_| |\n");
	printf("  \\_/\\___/ \\__,_|  \\__,_|_|\\___|\\__,_| |_.__/ \\__, |\n");
	printf("                                               __/ |\n");
	printf("     _                       _                |___/ \n");
	printf("    | |                     (_)                     \n");
	printf("    | |__   __ _ _ __   __ _ _ _ __   __ _          \n");
	printf("    | '_ \\ / _` | '_ \\ / _` | | '_ \\ / _` |         \n");
	printf("    | | | | (_| | | | | (_| | | | | | (_| |_        \n");
	printf("    |_| |_|\\__,_|_| |_|\\__, |_|_| |_|\\__, (_)       \n");
	printf("                        __/ |         __/ |         \n");
	printf("                       |___/         |___/          \n");
	printf("                                                    \n");
#endif
	getchar();
	app->quit = true;
}

void draw_game_won(app_t* app)
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

	console_goto(stdout, 41, 2);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_YELLOW);
	printf("TODAY");
	console_reset_fg_color(stdout);


	console_goto(stdout, 38, 7);
	console_fg_color_8(stdout, CONSOLE_COLOR8_BRIGHT_YELLOW);
	printf("THE HANGMAN");
	console_reset_fg_color(stdout);

	console_goto(stdout, 30, 12); printf("YOUR FAMILY WANTS YOU TO TURN");
	console_goto(stdout, 30, 13); printf("AWAY FROM A LIFE OF CRIME.");

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
	app->quit = true;
}

int main( int argc, char* argv[] )
{
	app_t app = (app_t) {
		.quit = false,
		.state = TITLE,
		.moves_left = 6,
		.letters_discovered = 0,
	};


	strcpy(app.word, "GENIUS");
	memset( app.chosen_letters, 0, sizeof(bool) * 16 );

	setlocale(LC_ALL, "");

	sigset_t signals;
	sigset_t blocked_signals;
	sigset_t pending_signals;

	sigfillset(&signals);
	sigemptyset(&blocked_signals);
	sigaddset(&blocked_signals, SIGINT);
	sigaddset(&blocked_signals, SIGQUIT);
	sigaddset(&blocked_signals, SIGWINCH);

	sigprocmask(SIG_BLOCK, &blocked_signals, &signals);
	sigemptyset(&pending_signals);

	console_hide_cursor(stdout);
	console_echo_disable(stdin);
	console_canonical_disable(stdin);

	console_set_size(stdout, 80, 25 );
	resize(&app);


	while( !app.quit )
	{
		if( !sigpending( &pending_signals ) )
		{
			if(sigismember(&pending_signals, SIGQUIT) ||
			   sigismember(&pending_signals, SIGINT))
			{
				app.quit = true;
			}
			if(sigismember(&pending_signals, SIGWINCH))
			{
				resize(&app);
			}
		}

		switch( app.state )
		{
			case TITLE:
				draw_title(&app);
				break;
			case IN_GAME:
				draw_game(&app);
				break;
			case GAME_OVER:
				draw_game_over(&app);
				break;
			case WIN:
				draw_game_won(&app);
				break;
			default:
				break;
		}

		time_msleep(100);
	}

	console_canonical_enable(stdin);
	console_echo_enable(stdin);
	console_show_cursor( stdout );
	console_clear_screen_all( stdout );
	console_reset( stdout );

	return 0;
}
