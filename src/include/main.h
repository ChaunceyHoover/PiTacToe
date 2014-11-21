// Include guards - this is used so multiple files can include this header,
// but all of the content will only be defined once. To do this, the program
// checks if a variable "MAIN_H" has been defined, and if it hasn't, it
// defines it.
#ifndef MAIN_H
#define MAIN_H

// Standard library
#include <stdio.h>

// SDL library
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// Enums
typedef enum { UNINITIALIZED, INITIALIZING, MAIN_MENU, PLAYING, HELP, QUIT } Game_Mode;
typedef enum { WIN_VERTICAL, WIN_HORIZONTAL, WIN_DN_DIAGNOL, WIN_UP_DIAGNOL } Win_Type;

// Structure for getting victory information
struct winner {
	int player;
	int offset;
	Win_Type wintype;
};

// Variables
Game_Mode game_mode			= UNINITIALIZED;
int player_turn				= 0;
int piece_position			= 4;
int option					= 0;
Uint32 default_color		= NULL; // set in `initialize()`
Uint32 win_color			= NULL; // requires `format` variable, so has to be set after images loaded

// SDL Variables
SDL_Window* window			= NULL;
SDL_Surface* screen			= NULL;

// Images used in program
SDL_Surface* board_img		= NULL;
SDL_Surface* x_img			= NULL;
SDL_Surface* o_img			= NULL;

// Fonts
TTF_Font* font				= NULL;
TTF_Font* game_font			= NULL;
TTF_Font* about_font		= NULL;

// SDL Event - used for handling input
SDL_Event event;

// Constants
const char* WIN_TITLE		= "Project Raspberry Pi: Tic-Tac-Toe";
const char* MENU_OPTIONS[]	= {"Play", "About", "Exit"};
const int   SCREEN_WIDTH	= 512;
const int   SCREEN_HEIGHT	= 512;

const int 	PADDING			= 16;
const int	PIECE_SIZE		= 128;

const int   EMPTY			= -1;
const int   X_VALUE			= 1;
const int   O_VALUE			= 2;

const int	HELP_FONT_SIZE	= 14;
const int 	FONT_SIZE 		= 28;
const int	GAME_FONT_SIZE	= 56;

// The board - static size, so no need to make variable for board size
// Note: Unlike most modern languages, C has no native way of getting the
// size of an array. To obtain it, the programmer would have to specify the
// variable somewhere else, or pass it as a separate parameter
int board[3][3]				= {{0}};

// Functions (description in source file)
struct winner check_gameover();
void draw_piece(int player, int x, int y, Uint32 color);
void initialize();
void logic();
void render();

// SDL related functions
SDL_Surface* load_image(const char* path);

#endif
