#define _CRT_SECURE_NO_WARNINGS
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <string.h>
#include <stdio.h>
#include "game.h"
#include "shared.h"
#include "utility.h"
#include "scene_game.h"
#include "scene_menu.h"
#include "win.h"
#include "lose.h"
#include "pacman_obj.h"
#include "ghost.h"
#include "map.h"


// [HACKATHON 2-0]
// Just modify the GHOST_NUM to 1
#define GHOST_NUM 4
/* global variables*/
extern const uint32_t GAME_TICK_CD;
extern uint32_t GAME_TICK;
extern ALLEGRO_TIMER* game_tick_timer;
int game_main_Score = 0;
bool game_over = false;

extern ALLEGRO_TIMER* power_up_timer;
extern ALLEGRO_TIMER* pre_timer;
extern bool g_bPower_Time_Up;
extern bool g_bSpark_Time_Up;
extern bool g_bpre_Time_Up;
extern bool g_bEarthQuake;
extern int g_nEarthQuakeCount;


/* Internal variables*/
static const int power_up_duration = 10;
static Pacman* pman;
static Map* basic_map;
static Ghost** ghosts;
bool debug_mode = false;
bool cheat_mode = false;

/* Declare static function prototypes */
static void init(void);
static void step(void);
static void checkItem(void);
static void status_update(void);
static void update(void);
static void draw(void);
static void printinfo(void);
static void destroy(void);
static void on_key_down(int key_code);
static void on_mouse_down(void);
static void render_init_screen(void);
static void draw_hitboxes(void);

static void init(void) {
	game_over = false;
	game_main_Score = 0;
	// create map
	//basic_map = create_map(NULL);
	// [TODO]
	// Create map from .txt file and design your own map !!
	basic_map = create_map("Assets/map_design.txt");
	if (!basic_map) {
		game_abort("error on creating map");
	}	
	// create pacman
	pman = pacman_create();
	ghosts = (Ghost**)malloc(sizeof(Ghost*) * GHOST_NUM);
	if (!pman) {
		game_abort("error on creating pacamn\n");
	}
	
	// allocate ghost memory
	// [HACKATHON 2-1]
	// TODO: Allocate dynamic memory for ghosts array.
	//ghosts = (Ghost**)malloc(sizeof(Ghost*) * GHOST_NUM);
	else {
		// [HACKATHON 2-2]
		// TODO: create a ghost.
		// Try to look the definition of ghost_create and figure out what should be placed here.
		for (int i = 0; i < GHOST_NUM; i++) {
			game_log("creating ghost %d\n", i);
			ghosts[i] = ghost_create(i);  
			if (!ghosts[i])
				game_abort("error creating ghost\n");
		}
	}
	GAME_TICK = 0;

	render_init_screen();
//	power_up_timer = al_create_timer(1.0f); // 1 tick / sec
//	if (!power_up_timer)
//		game_abort("Error on create timer\n");
	return ;
}

static void step(void) {
	if (pman->objData.moveCD > 0)
		pman->objData.moveCD -= pman->speed;
	for (int i = 0; i < GHOST_NUM; i++) {
		// important for movement
		if (ghosts[i]->objData.moveCD > 0)
			ghosts[i]->objData.moveCD -= ghosts[i]->speed;
	}
}
static void checkItem(void) {
	int Grid_x = pman->objData.Coord.x, Grid_y = pman->objData.Coord.y;
	if (Grid_y >= basic_map->row_num - 1 || Grid_y <= 0 || Grid_x >= basic_map->col_num - 1 || Grid_x <= 0)
		return;
	// [HACKATHON 1-3]
	// TODO: check which item you are going to eat and use `pacman_eatItem` to deal with it.
	switch (basic_map->map[Grid_y][Grid_x])
	{
	case '.':
		pacman_eatItem(pman, '.');
		game_main_Score++;
		if (game_main_Score == basic_map->beansCount)
		{
			game_change_scene(scene_win_create());
		}
		break;
	case 'E':
		pacman_eatItem(pman, 'P');
		g_nEarthQuakeCount = 0;
		g_bEarthQuake = true;
		for (int i = 0; i < GHOST_NUM; i++)
		{
			if (ghosts[i]->status == GO_IN)
				continue;
	//		if (ghosts[i]->status == FLEE)
			{
				ghosts[i]->status = GO_IN;
				ghost_collided(&ghosts[i]);
			}
		}
		break;
	case 'P':
		pacman_eatItem(pman, 'P');
		pman->powerUp = true;
		g_bPower_Time_Up = false;
		g_bpre_Time_Up = false;
		al_start_timer(power_up_timer);
		al_start_timer(pre_timer);

		for (int i = 0; i < GHOST_NUM; i++)
		{
			ghost_toggle_FLEE(ghosts[i], 1);
		}
		
		break;
	default:
		break;
	}
	// [HACKATHON 1-4]
	// erase the item you eat from map
	// becareful no erasing the wall block.
	if((basic_map->map[Grid_y][Grid_x] == '.')|| (basic_map->map[Grid_y][Grid_x] == 'P') || (basic_map->map[Grid_y][Grid_x] == 'E'))
		basic_map->map[Grid_y][Grid_x]=' ';
	
}
static void status_update(void) 
{
	if (g_bPower_Time_Up)
	{
		g_bPower_Time_Up = false;
		g_bSpark_Time_Up = true;
		pman->powerUp = false;
		for (int i = 0; i < GHOST_NUM; i++) {
			if (ghosts[i]->status != GO_IN)
			{
				ghosts[i]->status = FREEDOM;
			}
		}
	}

	for (int i = 0; i < GHOST_NUM; i++) {
		if (ghosts[i]->status == GO_IN)
			continue;
		// [TODO]
		// use `getDrawArea(..., GAME_TICK_CD)` and `RecAreaOverlap(..., GAME_TICK_CD)` functions to detect
		// if pacman and ghosts collide with each other.
		// And perform corresponding operations.
		// [NOTE]
		// You should have some branch here if you want to implement power bean mode.
		// Uncomment Following Code
			    RecArea ghostHB = getDrawArea(ghosts[i]->objData, GAME_TICK_CD);
			    RecArea RP = getDrawArea(pman->objData, GAME_TICK_CD);
			    if( (!cheat_mode)  && (RecAreaOverlap(ghostHB, RP))/*collision of pacman and ghost*/)
		        {
					if (ghosts[i]->status == FLEE)
					{
						game_log("collide with ghost666\n");
						ghosts[i]->status = GO_IN;
						ghost_collided(&ghosts[i]);
						//pacman_die();
						//game_over = true;
						//break;
					}
					else
					{
						game_log("collide with ghost\n");
						game_change_scene(scene_lose_create());
						al_rest(1.0);
						pacman_die();
						game_over = true;
						break;
					}
		        }
			
		
		
	}
}

static void update(void) {

	if (game_over) {
		/*
			[TODO]
			start pman->death_anim_counter and schedule a game-over event (e.g change scene to menu) after Pacman's death animation finished
			game_change_scene(...);
		*/
		game_change_scene(scene_menu_create());
		return;
	}

	step();
	checkItem();
	status_update();
	pacman_move(pman, basic_map);
	for (int i = 0; i < GHOST_NUM; i++)
	{
		switch (ghosts[i]->status) {
		case(FLEE):
			ghosts[i]->speed = 1;
			break;
		case(FREEDOM):
			ghosts[i]->speed = 2;
			break;
		case(GO_IN):
			ghosts[i]->speed = 4;
			break;

		}
		ghosts[i]->move_script(ghosts[i], basic_map, pman);
	}
}

static void draw(void) {

	if (g_bEarthQuake)
	{
		if (g_nEarthQuakeCount < 30)
			g_nEarthQuakeCount++;
		else
			g_bEarthQuake = false;
    }

	if (g_bEarthQuake)
   	  al_clear_to_color(al_map_rgb(200, 0, 0));
	else
	  al_clear_to_color(al_map_rgb(0, 0, 0));
	
	//	[TODO]
	//	Draw scoreboard, something your may need is sprintf();
	

	char ch[100];

	sprintf(&ch, "SCORE: %5d", game_main_Score);

	al_draw_text(menuFont,
			al_map_rgb(255, 255, 0),
			0, 0,
			ALLEGRO_ALIGN_LEFT,
			ch);
	

	draw_map(basic_map);

	pacman_draw(pman);
	if (game_over)
		return;
	// no drawing below when game over
	for (int i = 0; i < GHOST_NUM; i++)
		ghost_draw(ghosts[i]);
	
	//debugging mode
	if (debug_mode) {
		draw_hitboxes();
	}

}

static void draw_hitboxes(void) {
	RecArea pmanHB = getDrawArea(pman->objData, GAME_TICK_CD);
	al_draw_rectangle(
		pmanHB.x, pmanHB.y,
		pmanHB.x + pmanHB.w, pmanHB.y + pmanHB.h,
		al_map_rgb_f(1.0, 0.0, 0.0), 2
	);

	for (int i = 0; i < GHOST_NUM; i++) {
		RecArea ghostHB = getDrawArea(ghosts[i]->objData, GAME_TICK_CD);
		al_draw_rectangle(
			ghostHB.x, ghostHB.y,
			ghostHB.x + ghostHB.w, ghostHB.y + ghostHB.h,
			al_map_rgb_f(1.0, 0.0, 0.0), 2
		);
	}

}

static void printinfo(void) {
	game_log("pacman:\n");
	game_log("coord: %d, %d\n", pman->objData.Coord.x, pman->objData.Coord.y);
	game_log("PreMove: %d\n", pman->objData.preMove);
	game_log("NextTryMove: %d\n", pman->objData.nextTryMove);
	game_log("Speed: %f\n", pman->speed);
}


static void destroy(void) {
	/*
		[TODO]
		free map array, Pacman and ghosts
	*/
}

static void on_key_down(int key_code) {
	switch (key_code)
	{
		// [HACKATHON 1-1]	
		// TODO: Use allegro pre-defined enum ALLEGRO_KEY_<KEYNAME> to controll pacman movement
		// we provided you a function `pacman_NextMove` to set the pacman's next move direction.
		
		case ALLEGRO_KEY_W:
			pacman_NextMove(pman,UP);
			break;
		case ALLEGRO_KEY_A:
			pacman_NextMove(pman,LEFT);
			break;
		case ALLEGRO_KEY_S:
			pacman_NextMove(pman,DOWN);
			break;
		case ALLEGRO_KEY_D:
			pacman_NextMove(pman,RIGHT);
			break;
		case ALLEGRO_KEY_C:
			cheat_mode = !cheat_mode;
			if (cheat_mode)
				printf("cheat mode on\n");
			else 
				printf("cheat mode off\n");
			break;
		case ALLEGRO_KEY_G:
			debug_mode = !debug_mode;
			break;
		
	default:
		break;
	}

}

static void on_mouse_down(void) {
	// nothing here

}

static void render_init_screen(void) {
	al_clear_to_color(al_map_rgb(0, 0, 0));

	draw_map(basic_map);
	pacman_draw(pman);
	for (int i = 0; i < GHOST_NUM; i++) {
		ghost_draw(ghosts[i]);
	}

	al_draw_text(
		menuFont,
		al_map_rgb(255, 255, 0),
		400, 400,
		ALLEGRO_ALIGN_CENTER,
		"READY!"
	);

	al_flip_display();
	al_rest(2.0);

}
// Functions without 'static', 'extern' prefixes is just a normal
// function, they can be accessed by other files using 'extern'.
// Define your normal function prototypes below.

// The only function that is shared across files.
Scene scene_main_create(void) {
	Scene scene;
	memset(&scene, 0, sizeof(Scene));
	scene.name = "Start";
	scene.initialize = &init;
	scene.update = &update;
	scene.draw = &draw;
	scene.destroy = &destroy;
	scene.on_key_down = &on_key_down;
	scene.on_mouse_down = &on_mouse_down;
	// TODO: Register more event callback functions such as keyboard, mouse, ...
	game_log("Start scene created");
	return scene;
}
