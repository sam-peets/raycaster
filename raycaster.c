#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>

#define WINDOW_WIDTH 400
#define STEP_SIZE 0.1
#define MAX_DISTANCE 10
#define BACK_STEPS 10
#define MAP_SIZE_X 16
#define MAP_SIZE_Y 16

int map[MAP_SIZE_X][MAP_SIZE_Y] = {
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1,1,1,1,1,1,1,0,1},
	{1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,1,0,1,1,1,1,0,0,0,0,1},
	{1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,1,0,1,0,1,1,1,1,0,0,1},
	{1,0,0,1,0,1,0,1,0,1,0,0,0,0,0,1},
	{1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1},
	{1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1},
	{1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1},
	{1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};


typedef struct vec2 {float x; float y;} vec2;
typedef struct player {vec2 position; float angle; vec2 vel; float look_vel;} player;

void add_vec(vec2* v1, vec2* v2) {
	v1->x += v2->x;
	v1->y += v2->y;
}

float deg_to_rad(float theta) {
	return theta*M_PI/180;
}

void add_delta(vec2* v, float d, float theta) {
	theta = deg_to_rad(theta);
	v->x += sin(theta)*d;
	v->y += cos(theta)*d;
}

void set_pixel(char* p, char r, char g, char b, int x, int y) {
	p[WINDOW_WIDTH*4*y + 4*x + 0] = b;
	p[WINDOW_WIDTH*4*y + 4*x + 1] = g;
	p[WINDOW_WIDTH*4*y + 4*x + 2] = r;
	p[WINDOW_WIDTH*4*y + 4*x + 3] = 255;
}

float cast(player* player, float theta, vec2* ray) {
	ray->x = player->position.x;
	ray->y = player->position.y;

	float distance_traveled = 0;
	while(distance_traveled < MAX_DISTANCE) {
		int cx = (int)floor(ray->x);
		int cy = (int)floor(ray->y);
		if (map[cx][cy] == 1) {
			float division = 2;
			for (int i = 0; i < BACK_STEPS; i++) {
				cx = (int)floor(ray->x);
				cy = (int)floor(ray->y);	
				if (map[cx][cy] == 1) {
					add_delta(ray,-1.0*STEP_SIZE/division, theta);
					distance_traveled -= STEP_SIZE/division;
				} else {
					add_delta(ray,STEP_SIZE/division, theta);
					distance_traveled += STEP_SIZE/division;

				}
				division*=2;
			}
			return distance_traveled * cos(deg_to_rad(abs(theta - player->angle)));
		}
		distance_traveled+=STEP_SIZE;
		add_delta(ray, STEP_SIZE, theta);
	}
	return distance_traveled;
}

void draw_line(char* p, char r, char g, char b, int x, int l) {
	int start = WINDOW_WIDTH/2 - l/2;
	for (int i = 0; i < l; i++) {
		set_pixel(p, r,g,b, x, start+i);
	}
}

void draw_box(char* p, int x1, int y1, int x2, int y2, int r, int g, int b) {
	for (int x = x1; x < x2; x++) {
		for (int y = y1; y < y2; y++) {
			set_pixel(p, r,g,b, x,y);
		}
	}

}

void draw_minimap(char* p, int size, player* player) {
	for (int y = 0; y < MAP_SIZE_Y; y++) {
		for (int x = 0; x < MAP_SIZE_X; x++) {
			int c = map[x][y] == 1 ? 255 : 0;
			draw_box(p, x*size, y*size, x*size + size, y*size + size, c, c, c);
		}
	}

	int x1 = player->position.x*size;
	int y1 = player->position.y*size;
	draw_box(p, x1-size/2,y1-size/2,x1+size/2,y1+size/2,255,0,0);
}

void draw(char* p, player* player) {
	for (int y = 0; y < WINDOW_WIDTH; y++) {
		for (int x = 0; x < WINDOW_WIDTH; x++) {
			//set_pixel(p,0,0,(int)((float)y/(float)WINDOW_WIDTH * (float)255), x,y);
			set_pixel(p,200,200,200,x,y);
		}
	}

	float fov = 60;
	float fov_step = fov/WINDOW_WIDTH;
	float start_angle = player->angle - fov/2.0;
	float end_angle = fmod(start_angle+fov, 360.0);
	
	vec2* ray = malloc(sizeof(vec2));
	for (int i = 0; i < WINDOW_WIDTH; i++) {
		
		float theta = fmod(start_angle + i*fov_step, 360.0);
		float d = cast(player, theta, ray);
		if (WINDOW_WIDTH/d > WINDOW_WIDTH)
			d = 1;
		float l = WINDOW_WIDTH/d;
		l = l > 0 ? l : 0;

		draw_line(p, 255/d, 255/d, 255/d, i, l);
	}
	free(ray);


}



void handle_keys(SDL_Event* event, player* player ) {
	SDL_KeyboardEvent* key = &(event->key);
	switch (event->type) {
		case SDL_KEYDOWN:
			switch(key->keysym.sym) {
				case SDLK_LEFT:
					player->look_vel = -2;
					break;
				case SDLK_RIGHT:
					player->look_vel = 2;
					break;
				case SDLK_UP:
					player->vel.x = 0;
					player->vel.y = 0;
					add_delta(&(player->vel), 0.05, player->angle);
					break;
				
			}
			break;
		case SDL_KEYUP:
			switch(key->keysym.sym) {
				case SDLK_LEFT: case SDLK_RIGHT:
					player->look_vel = 0;
					break;
				case SDLK_UP:
					player->vel.x = 0;
					player->vel.y = 0;
					break;
			}

	}
}

void update_vel(player* player) {

	if (player->vel.x > 0 || player->vel.y > 0) {
		player->vel.x = 0;
		player->vel.y = 0;
		add_delta(&(player->vel), 0.05, player->angle);
	}
	add_vec(&(player->position), &(player->vel));
	player->angle += player->look_vel;

	// collision
	if (player->position.x <= 1.5)
		player->position.x = 1.5;
	if (player->position.y <= 1.5)
		player->position.y = 1.5;
	if (player->position.x >= 14.5)
		player->position.x = 14.5;
	if (player->position.y >= 14.5)
		player->position.y = 14.5;
}

int main(int argc, char* argv[]) {
	SDL_Event event;
	SDL_Renderer *renderer;
	SDL_Window *window;

	SDL_Init(SDL_INIT_VIDEO);
	
	window = SDL_CreateWindow("test", 100, 100, WINDOW_WIDTH, WINDOW_WIDTH, 0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Surface* surface = SDL_GetWindowSurface(window);

	char* p = malloc(surface->h * surface->pitch * sizeof(char));
	player* player = malloc(sizeof(player));
	player->position.x = 2;
	player->position.y = 8;
	player->vel.x = 0;
	player->vel.y = 0;
	player->look_vel = 0;
	player->angle = 0;
	while (1) {
		int start = SDL_GetTicks();


		if(SDL_MUSTLOCK(surface)){ SDL_LockSurface(surface); }
		update_vel(player);
		draw(p, player);
		draw_minimap(p,4,player);
		memcpy(surface->pixels, p, surface->h * surface->pitch * sizeof(char));

		// fps limit
		int time = SDL_GetTicks() - start;
		if (time < 0) continue;
		int sleepTime = 1000/60 - time;
		if (sleepTime > 0) {
			SDL_Delay(sleepTime);
		}

		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					goto quit;

				case SDL_KEYDOWN: case SDL_KEYUP:
					handle_keys((&event), player);
					break;
			}
		}
		if(SDL_MUSTLOCK(surface)){ SDL_UnlockSurface(surface); }
		if(SDL_UpdateWindowSurface(window) != 0) { printf("LCD::Error - Could not blit\n"); }
	}
quit:
	free(p);
	free(player);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;
}


