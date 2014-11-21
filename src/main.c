#include "main.h"

// Checks for if the game is over: 0 = Not over, 1 = X wins, 2 = O wins
struct winner check_gameover() {
	// Format for who won: player = 1 when player1 wins, player = 2 when player2 wins. Everything else
	// is just used for how the player won (diagnol or straight line, which row/column, etc)
	struct winner winrar;
	winrar.player = -1;

	// Note: Many returns used to avoid having to further check more if statemetns

	// Check for rows
	int i;
	for (i = 0; i < 3; i++) {
		if (board[i][0] == EMPTY) continue;
		if (board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
			winrar.player = board[i][0];
			winrar.wintype = WIN_HORIZONTAL;
			winrar.offset = i;
			return winrar;
		}
	}

	// Check for columns
	for (i = 0; i < 3; i++) {
		if (board[0][i] == EMPTY) continue;
		if (board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
			winrar.player = board[0][i];
			winrar.wintype = WIN_VERTICAL;
			winrar.offset = i;
			return winrar;
		}
	}

	// Check for diagnols
	if (board[0][0] != EMPTY
		&& board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
			winrar.player = board[0][0];
			winrar.wintype = WIN_DN_DIAGNOL;
			winrar.offset = 0;
			return winrar;
	}

	if (board[2][0] != EMPTY
		&& board[2][0] == board[1][1] && board[1][1] == board[0][2]) {
			winrar.player = board[2][0];
			winrar.wintype = WIN_UP_DIAGNOL;
			winrar.offset = 0;
			return winrar;
	}

	return winrar;
}

void draw_piece(int player, int x, int y, Uint32 color) {
	// tl;dr - Fancy math = This makes it centered at half of the screen + 50px down.
	/* Long version:
	 *  Since I am trying to position the pieces in the center of each empty square on the
	 *  board and have some padding in between so the pieces don't touch the board itself,
	 *  I start off by getting half of the padding because I need to put half on each side
	 *  of the piece (left & right, and top & bottom). Next, I offset it by the board's width
	 *  divided by 3 since I need to fit 3 pieces in the row. The 40 represents each bar
	 *  on the board that separates each space, as it is 20px wide on the sprite sheet.
	 *  Finally, I need to offset it by `PIECE_SIZE` because that is where the piece render
	 *  will end, not start. This is all just to get the base position, or the top left.
	 *
	 *  What I do next is move it over according to where the piece is in the index. If the
	 *  piece is supposed to be in the middle of the board, it's position is (1, 1). So I
	 *  simple just move it over PIECE_SIZE + PADDING + the size each bar, or 23px in the way
	 *  I stretched it. But, if it's at (2, 2) or the bottom right, I need to do that same
	 *  process again. So, instead, I just multiply that number by the position in the array.
	 */
	SDL_Rect piece_stretch;
	piece_stretch.x = (PADDING / 2) + (((SCREEN_WIDTH - (PADDING * 2)) - 40) / 3) - PIECE_SIZE;
	piece_stretch.y = (PADDING / 2) + (((SCREEN_HEIGHT - (PADDING * 2)) - 40) / 3) - PIECE_SIZE;
	piece_stretch.x += (PIECE_SIZE + PADDING + 23) * x;
	piece_stretch.y += (PIECE_SIZE + PADDING + 23) * y;
	piece_stretch.w = PIECE_SIZE;
	piece_stretch.h = PIECE_SIZE;

	// This is magenta, but all surfaces are color keyed to magenta, thus this is transparent.
	Uint16 transparent = SDL_MapRGB(screen->format, 0xFF, 0x00, 0xFF);
	Uint16* pixels;

	// Modifies each piece based off given argument
	if (player == 1)
		pixels = (Uint16*)x_img->pixels; // pixel map of `x_img`
	else
		pixels = (Uint16*)o_img->pixels; // pixel map of `o_img`

	// Area of each surface; although they are the same size, I can freely change the sprite
	// and not have to change this code
	int area = player == 1 ? x_img->w * x_img->h : o_img->w * o_img->h;
	int i;
	for (i = 0; i < area; i++) {
		if (pixels[i] != transparent)
			pixels[i] = color;
	}

	// Renders the scaled piece
	SDL_BlitScaled(player == 1 ? x_img : o_img, NULL, screen, &piece_stretch);
}

// This function only gets called once and isn't technically needed. However, it is commonly
// used for keeping code "clean" looking and organized.
void initialize() {
	// Makes sure the function is only called once
	if (game_mode != UNINITIALIZED) return;
	game_mode = INITIALIZING; // prevents it from being called again and loading assets twice

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize: %s\n", SDL_GetError());
		game_mode = QUIT;
		return;
	}

	// SDL initialized, time to create window
	window = SDL_CreateWindow(WIN_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
								SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Window could not be created: %s\n", SDL_GetError());
		game_mode = QUIT;
		return;
	} else {
		// The object used to draw to the window
		screen = SDL_GetWindowSurface(window);
	}

	// Initialize SDL_image
	int imgFlags = IMG_INIT_PNG;
	if ( !(IMG_Init(imgFlags) & imgFlags) ) {
		printf("Couldn't initialize SDL_image: %s\n", IMG_GetError());
		game_mode = QUIT;
		return;
	}

	// Initializing SDL_ttf
	if (TTF_Init() == -1) {
		printf("Couldn't initialize SDL_ttf: (no reason specified)\n");
		game_mode = QUIT;
		return;
	}

	// Initialize board
	int x, y;
	for (x = 0; x < 3; x++)
		for (y = 0; y < 3; y++)
			board[x][y] = EMPTY;

	// Load resources
	board_img = load_image("res/img/board.png");
	o_img = load_image("res/img/o.png");
	x_img = load_image("res/img/x.png");

	default_color = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
	win_color = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);

	// Load font (technically, a resource)
	font = TTF_OpenFont("res/font/dejavu-mono.ttf", FONT_SIZE);
	game_font = TTF_OpenFont("res/font/dejavu-mono.ttf", GAME_FONT_SIZE);
	about_font = TTF_OpenFont("res/font/dejavu-mono.ttf", HELP_FONT_SIZE);
	if (font == NULL || game_font == NULL) {
		printf("Unable to load font `res/font/dejavu-mono.ttf`.");
		game_mode = QUIT;
		return;
	}

	game_mode = MAIN_MENU; // Everything worked
}

// This is where the logical phase of the program happens: get user input and handle it
// based off the type of input.
void logic() {
	while (SDL_PollEvent(&event) != 0) {
		if (event.type == SDL_QUIT) {
			game_mode = QUIT;
			break;
		} else if (event.type == SDL_KEYDOWN) {
			// Handle key events differently based on game mode
			if (game_mode == MAIN_MENU) {
				// Up/A and Down/S = select different game mode on menu; Enter = choose selected item
				int key = event.key.keysym.sym;
				if (key == SDLK_UP || key == SDLK_w) {
					option--;
					if (option < 0) option = 2;
				} else if (key == SDLK_DOWN || key == SDLK_s) {
					option++;
					if (option > 2) option = 0;
				} else if (key == SDLK_ESCAPE || key == SDLK_q) {
					game_mode = QUIT;
				} else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
					if (option == 0) {
						game_mode = PLAYING;
						player_turn = X_VALUE;
						piece_position = 4;
						int i;
						for (i = 0; i < 9; i++)
							board[i % 3][i / 3] = EMPTY;
					 } else if (option == 1)
						game_mode = HELP;
					else if (option == 2)
						game_mode = QUIT;
					else {
						option = 0;
						printf("Warning: option > 2; adjust logic\n");
					}
				}
			} else if (game_mode == HELP) {
				int key = event.key.keysym.sym;
				if (key == SDLK_ESCAPE || key == SDLK_q || key == SDLK_RETURN || key == SDLK_KP_ENTER
					|| key == SDLK_BACKSPACE)
						game_mode = MAIN_MENU;
			} else if (game_mode == PLAYING) {
				if (player_turn > 0) {
					int key = event.key.keysym.sym;
					int last_pos = piece_position;

					if (key == SDLK_KP_7)
						piece_position = board[0][0] == EMPTY ? 0 : piece_position;
					else if (key == SDLK_KP_8)
						piece_position = board[1][0] == EMPTY ? 1 : piece_position;
					else if (key == SDLK_KP_9)
						piece_position = board[2][0] == EMPTY ? 2 : piece_position;
					else if (key == SDLK_KP_4)
						piece_position = board[0][1] == EMPTY ? 3 : piece_position;
					else if (key == SDLK_KP_5)
						piece_position = board[1][1] == EMPTY ? 4 : piece_position;
					else if (key == SDLK_KP_6)
						piece_position = board[2][1] == EMPTY ? 5 : piece_position;
					else if (key == SDLK_KP_1)
						piece_position = board[0][2] == EMPTY ? 6 : piece_position;
					else if (key == SDLK_KP_2)
						piece_position = board[1][2] == EMPTY ? 7 : piece_position;
					else if (key == SDLK_KP_3)
						piece_position = board[2][2] == EMPTY ? 8 : piece_position;

					if (key >= SDLK_KP_1 && key <= SDLK_KP_0) {
						int set_piece = 0;
						if (piece_position == last_pos
							&& board[piece_position % 3][piece_position / 3] == EMPTY) {
								board[piece_position % 3][piece_position / 3] = player_turn;
								set_piece = 1;
						}

						if (set_piece == 1) {
							player_turn = player_turn == X_VALUE ? O_VALUE : X_VALUE;

							int empty_piece = 0;
							for (piece_position = 0; piece_position < 9; piece_position++) {
								if (board[piece_position % 3][piece_position / 3] == EMPTY) {
									empty_piece = 1;
									break;
								}
							}

							if (empty_piece == 0)
								player_turn = EMPTY;
						}
					}

					if (key == SDLK_ESCAPE)
						game_mode = QUIT;
					if (key == SDLK_q)
						game_mode = MAIN_MENU;
					else if (key == SDLK_DOWN || key == SDLK_s) {
						if (piece_position < 6) {
							int next = piece_position + 3;
							if (board[next % 3][next / 3] == EMPTY) {
								piece_position += 3;
 							} else {
								next += 3;
								if (next < 9 && board[next % 3][next / 3] == EMPTY)
									piece_position += 6;
								else {
									int repeat;
									next -= 3;
									for (repeat = 0; repeat < 2; repeat++) {
										if (next >= 9) break;
										if (next % 3 > 0) {
											if (board[(next - 1) % 3][(next - 1) / 3] == EMPTY) {
												piece_position = next - 1;
												break;
											}
										}
										if (next % 3 < 2) {
											if (board[(next + 1) % 3][(next + 1) / 3] == EMPTY) {
												piece_position = next + 1;
												break;
											}
										}
										next += 3;
									}
								}
							}
						}
					} else if (key == SDLK_UP || key == SDLK_w) {
						if (piece_position > 2) {
							int next = piece_position - 3;
							if (board[next % 3][next / 3] == EMPTY) {
								piece_position -= 3;
							} else {
								next -= 3;
								if (next >= 0 && board[next % 3][next / 3] == EMPTY)
									piece_position -= 6;
								else {
									int repeat;
									next += 3;
									for (repeat = 0; repeat < 2; repeat++) {
										if (next < 0) break;
										if (next % 3 > 0) {
											if (board[(next - 1) % 3][(next - 1) / 3] == EMPTY) {
												piece_position = next - 1;
												break;
											}
										}
										if (next % 3 < 2) {
											if (board[(next + 1) % 3][(next + 1) / 3] == EMPTY) {
												piece_position = next + 1;
												break;
											}
										}
										next -= 3;
									}
								}
							}
						}
					} else if (key == SDLK_LEFT || key == SDLK_a) {
						if ((piece_position % 3) > 0) {
							int next = piece_position - 1;
							if (board[next % 3][next / 3] == EMPTY) {
								piece_position--;
							} else {
								next--;
								if ((next % 3) >= 0 && board[next % 3][next / 3] == EMPTY)
									piece_position -= 2;
								else {
									int repeat;
									next++;
									for (repeat = 0; repeat < 2; repeat++) {
										if (next % 3 < 0) break;
										if (next - 3 >= 0) {
											if (board[(next - 3) % 3][(next - 3) / 3] == EMPTY) {
												piece_position = next - 3;
												break;
											}
										}
										if (next + 3 < 9) {
											if (board[(next + 3) % 3][(next + 3) / 3] == EMPTY) {
												piece_position = next + 3;
												break;
											}
										}
										next--;
									}
								}
							}
						}
					} else if (key == SDLK_RIGHT || key == SDLK_d) {
						if ((piece_position % 3) < 2) {
							int next = piece_position + 1;
							if (board[next % 3][next / 3] == EMPTY) {
								piece_position++;
							} else {
								next++;
								if ((next % 3) < 6 && board[next % 3][next / 3] == EMPTY)
									piece_position += 2;
								else {
									int repeat;
									next--;
									for (repeat = 0; repeat < 2; repeat++) {
										if (next % 3 > 2) break;
										if (next - 3 >= 0) {
											if (board[(next - 3) % 3][(next - 3) / 3] == EMPTY) {
												piece_position = next - 3;
												break;
											}
											if (board[(next + 3) % 3][(next + 3) / 3] == EMPTY) {
												piece_position = next + 3;
												break;
											}
										}
										next++;
									}
								}
							}
						}
					} else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
						board[piece_position % 3][piece_position / 3] = player_turn;
						int set_piece = 0;
						for (piece_position = 0; piece_position < 9; piece_position++) {
							if (board[piece_position % 3][piece_position / 3] == EMPTY) {
								set_piece = 1;
								break;
							}
						}

						if (set_piece == 0) {
							player_turn = EMPTY;
						} else {
							player_turn = player_turn == X_VALUE ? O_VALUE : X_VALUE;
						}
					}
				} else {
					switch(event.key.keysym.sym) {
						default: {
							int i, j;
							for (i = 0; i < 3; i++)
								for (j = 0; j < 3; j++)
									board[i][j] = 0;
							piece_position = 4;
							game_mode = MAIN_MENU;
							break;
						}
					}
				}
			} else if (game_mode == HELP) {
				int key = event.key.keysym.sym;
				if (key == SDLK_ESCAPE || key == SDLK_BACKSPACE)
					game_mode = MAIN_MENU;
			}
		}
	}
}

// All of the drawing happens here: without this function, nothing would happen visually.
void render() {
	if (game_mode == UNINITIALIZED) return;

	// Clears the screen
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
	switch(game_mode) {
		case MAIN_MENU: {
			// Normal font color and selected option font color, respectively
			SDL_Color unselect = {154, 154, 154};
			SDL_Color select = {255, 255, 255};

			// Draws the menu, and if the menu choice is selected, encases it with square brackets
			int i;
			for (i = 0; i < 3; i++) {
				char menu_text[10]; // buffer
				int width, height; // used to center the text

				// Encasement of text
				if (option == i) {
					strcpy(menu_text, "[ ");
					strcat(menu_text, MENU_OPTIONS[i]);
					strcat(menu_text, " ]");
				} else {
					strcpy(menu_text, MENU_OPTIONS[i]);
				}
				TTF_SizeText(font, menu_text, &width, &height); // gets text size

				// Creates offset and text itself, then draws it to the window
				SDL_Rect offset;
				SDL_Surface* menu = TTF_RenderText_Solid(font, menu_text, option == i ? select : unselect);
				offset.x = (SCREEN_WIDTH / 2) - (width / 2);
				offset.y = (SCREEN_HEIGHT / 2) - ((-(i - 1) * (height * 3)) / 2) + 50;
				SDL_BlitSurface(menu, NULL, screen, &offset);
				SDL_FreeSurface(menu);
			}
			break;
		}

		case HELP: {
			const char* htp[] = {
				"HOW TO PLAY",
				"The goal of the game is to make a line of 3 pieces in a row.",
				"You can move the pieces around with arrow keys or WASD,",
				"ordirectly pick a position using the keypad (i.e. '5' = ",
				"center, '9' = top left, and tapping the key twice = place)"
			};

			const char* about[] = {
				"ABOUT",
				"This software is free to use for anyone and is not under a",
				"license. If any issues are found, please report them to",
				"http://github.com/DealerNextDoor/PiTacToe. Created by",
				"Chauncey Hoover."
			};

			SDL_Color title = {0, 255, 255};
			SDL_Color paragraph = {255, 255, 255};

			int x = PADDING;
			int y = PADDING;
			int i, w, h;
			for (i = 0; i < 5; i++) {
				TTF_SizeText(font, htp[i], &w, &h);
				SDL_Surface* message = TTF_RenderText_Solid(i == 0 ? font : about_font, htp[i],
						i == 0 ? title : paragraph);
				SDL_Rect offset;
				offset.x = x;
				offset.y = y;

				y += h + (PADDING / 2);
				SDL_BlitSurface(message, NULL, screen, &offset);
				SDL_FreeSurface(message);
			}

			y += PADDING;

			for (i = 0; i < 5; i++) {
				TTF_SizeText(font, about[i], &w, &h);
				SDL_Surface* message = TTF_RenderText_Solid(i == 0 ? font : about_font, about[i],
						i == 0 ? title : paragraph);
				SDL_Rect offset;
				offset.x = x;
				offset.y = y;

				y += h + (PADDING / 2);
				SDL_BlitSurface(message, NULL, screen, &offset);
				SDL_FreeSurface(message);
			}
			break;
		}

		case PLAYING: {
			// Stretches the board to fit the screen, with some padding to separate it from the edge
			SDL_Rect stretch;
			stretch.x = PADDING;
			stretch.y = PADDING;
			stretch.w = SCREEN_WIDTH - (PADDING * 2);
			stretch.h = SCREEN_HEIGHT - (PADDING * 2);
			SDL_BlitScaled(board_img, NULL, screen, &stretch);

			// Drawing each piece on the board
			int x, y;
			for (x = 0; x < 3; x++) {
				for (y = 0; y < 3; y++) {
					if (board[x][y] == EMPTY) continue; // If empty, continue to next position
					draw_piece(board[x][y], x, y, default_color);
				}
			}

			struct winner winrar = check_gameover();
			if (game_mode == PLAYING) {
				if (winrar.player > 0) {
					switch(winrar.wintype) {
						case WIN_VERTICAL:
							draw_piece(winrar.player, 0, winrar.offset, win_color);
							draw_piece(winrar.player, 1, winrar.offset, win_color);
							draw_piece(winrar.player, 2, winrar.offset, win_color);
						break;

						case WIN_HORIZONTAL:
							draw_piece(winrar.player, winrar.offset, 0, win_color);
							draw_piece(winrar.player, winrar.offset, 1, win_color);
							draw_piece(winrar.player, winrar.offset, 2, win_color);
						break;

						case WIN_DN_DIAGNOL:
							draw_piece(winrar.player, 0, 0, win_color);
							draw_piece(winrar.player, 1, 1, win_color);
							draw_piece(winrar.player, 2, 2, win_color);
							break;

						case WIN_UP_DIAGNOL:
							draw_piece(winrar.player, 0, 2, win_color);
							draw_piece(winrar.player, 1, 1, win_color);
							draw_piece(winrar.player, 2, 0, win_color);
							break;

						default:
							printf("Unhanlded winning situation (probably some hacker; go away!)\n");
							break;
					}

					const char* message = winrar.player == 1 ? "PLAYER 1 WINS" : "PLAYER 2 WINS";
					SDL_Rect offset;
					TTF_SizeText(game_font, message, &offset.w, &offset.h);
					SDL_Color msg_color = {255, 0, 255};

					SDL_Surface* win_message = TTF_RenderText_Solid(game_font, message, msg_color);
					offset.x = (SCREEN_WIDTH / 2) - (offset.w / 2);
					offset.y = (SCREEN_HEIGHT / 2) - (offset.h / 2);
					SDL_BlitSurface(win_message, NULL, screen, &offset);
					player_turn = -1;
					SDL_FreeSurface(win_message);
				} else if (piece_position == 9) {
					const char* message = "GAME TIED!";
					SDL_Color msg_color = {255, 0, 255};

					SDL_Surface* tie_message = TTF_RenderText_Solid(game_font, message, msg_color);
					SDL_Rect offset;
					TTF_SizeText(game_font, message, &offset.w, &offset.h);
					offset.x = (SCREEN_WIDTH / 2) - (offset.w / 2);
					offset.y = (SCREEN_HEIGHT / 2) - (offset.h / 2);

					SDL_BlitSurface(tie_message, NULL, screen, &offset);
					SDL_FreeSurface(tie_message);
				} else {
					draw_piece(player_turn, piece_position % 3, piece_position / 3,
							SDL_MapRGB(screen->format, 127, 127, 127));
				}
			}
			break;
		}

		default:
			// mayhaps error
			break;
	}

	// Updates the actual window so all renders can be visible
	SDL_UpdateWindowSurface(window);
}

// Loads an image from the given path
SDL_Surface* load_image(const char* path) {
	// "Optimized" image = uses same depth buffer and a few other minor things as window
	SDL_Surface* optimized;

	// Loads image
	SDL_Surface* image = IMG_Load(path);
	if (image == NULL) {
		printf("Unable to load `%s`: %s\n", path, SDL_GetError());
	} else {
		// Replaces all magenta with transparency and attempts to optimize the image
		// Note: if image would not be optimized, every time the image is rendered, it would have to be
		// converted to all the same specs as the screen itself, which is a waste.
		SDL_SetColorKey(image, SDL_TRUE, SDL_MapRGB(image->format, 0xFF, 0x00, 0xFF));
		optimized = SDL_ConvertSurface(image, screen->format, 0);
		if (optimized == NULL) {
			printf("Unable to optimize image `%s`: %s", path, SDL_GetError());
			SDL_FreeSurface(optimized);
		} else {
			// Deletes the loaded image and returns the optimized one
			SDL_FreeSurface(image);
			return optimized;
		}
	}

	// Regardless of if the image loaded or not, something gets returned.
	return image;
}

// Main entry point for the program - the first thing that gets called
int main(int argc, char* args[]) {
	// Initialize program
	initialize();

	while (game_mode != QUIT) {
		logic();
		render();
	}

	// Deallocate surfaces
	SDL_FreeSurface(board_img);
	SDL_FreeSurface(x_img);
	SDL_FreeSurface(o_img);
	board_img = NULL;
	o_img = NULL;
	x_img = NULL;

	// Close the fonts
	TTF_CloseFont(font);
	TTF_CloseFont(game_font);
	TTF_CloseFont(about_font);

	// Destroys the window instance
	SDL_DestroyWindow(window);
	SDL_FreeSurface(screen);
	window = NULL;
	screen = NULL;

	// Quits SDL: this also cleans up any SDL_Surface pointers (or any pointers) that were not
	// manually freed via SDL_FreeSurface or the respective function.
	IMG_Quit();
	SDL_Quit();
	TTF_Quit();
	return 0;
}
