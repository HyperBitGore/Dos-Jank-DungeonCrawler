#pragma once
#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <math.h>
#include <algorithm>
#include "GoreEngine.h"
#define P2 1.5708
#define P3 4.71239
#undef main
SDL_Renderer* rend;
Gore gore;
bool push = false;
bool atk = false;
SDL_Surface* surf;
SDL_Surface* wallsurf;
SDL_Surface* enemsurf;
SDL_Texture* enemtex;
SDL_Surface* doorsurf;
float zbuf[800];
float doordist = 100;
int level = 1;
bool textmode = true;
bool dead = false;
std::string leveltext = "Hello\n Arrow keys to move and turn\n Q or Left mouse to\n attack\n Press E to eat dead \nbodies and gain skill points\nKill door to progress\nClick to close";
struct Entity {
	float x;
	float y;
	int health;
	int strength;
	int agility;
};
struct Point {
	float x;
	float y;
};
struct IntPoint {
	int x;
	int y;
};
struct Sprite {
	int x;
	int y;
	int w;
	int h;
	float distance;
	int health;
	SDL_Surface* surf;
	double ftime;
	double time;
	int frames;
	int f;
	bool forward;
	bool dead;
	int type;
};
struct Animate {
	int x;
	int y;
	int w;
	int h;
	double time;
	double ftime;
	Sprite* targ;
	texp sprites;

};
std::vector<Sprite> sprites;
Point camera = { 22, 3 };
Point dir = { -1, 0 };
Point plane = { 0, 0.66 };
Point pushvec = { 0, 0 };

struct {
	bool operator()(Sprite a, Sprite b) const { return a.distance > b.distance; }
} spriteDistanceSort;


int FixAng(int a) { if (a > 6.28319) { a -= 6.28319; } if (a < 0) { a += 6.28319; } return a; }
size_t mw = 16;
size_t mh = 16;
int map1[16][16] = { 
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,0,0,0,1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1,
	1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,3,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,1,1,1,1,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,1,0,0,0,0,2,0,0,0,1,
	1,4,0,0,0,0,1,0,0,0,0,0,0,0,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,};
int map2[16][16] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,
	1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,
	1,0,1,0,1,0,2,2,1,1,1,1,1,1,1,1,
	1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,
	1,0,1,0,0,0,2,0,0,2,0,0,0,0,0,1,
	1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,1,0,0,2,0,2,0,0,0,0,0,0,0,1,
	1,0,0,0,2,2,0,0,2,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,2,2,0,0,0,0,1,
	1,0,0,0,0,0,0,020,0,0,0,2,1,1,1,
	1,0,0,2,2,0,1,0,0,0,0,2,0,0,4,1,
	1,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, };
int map[16][16];
float dist(float ax, float ay, float bx, float by, float ang) {
	return cos((ang)*(bx-ax) - sin(ang) * (by-ay));
}

void raycastDDA(Entity* p, SDL_Renderer* rend) {
	Point rayDir;
	for (int x = 0; x < 800; x++) {
		camera.x = 2 * x / float(800) - 1;
		rayDir.x = dir.x + plane.x * camera.x;
		rayDir.y = dir.y + plane.y * camera.x;
		Point sidedist;
		IntPoint mp = { (int)(p->x / 16), (int)(p->y / 16) };
		Point deltadist = { sqrt(1 + (rayDir.y * rayDir.y) / (rayDir.x * rayDir.x)), sqrt(1 + (rayDir.x * rayDir.x) / (rayDir.y * rayDir.y)) };
		float perpwalldist;
		IntPoint step = {0, 0};
		if (rayDir.x < 0) {
			step.x = -1;
			sidedist.x = (p->x/16 - mp.x) * deltadist.x;
		}
		else {
			step.x = 1;
			sidedist.x = (mp.x + 1 - p->x/16) * deltadist.x;
		}
		if (rayDir.y < 0) {
			step.y = -1;
			sidedist.y = (p->y/16 - mp.y) * deltadist.y;
		}
		else {
			step.y = 1;
			sidedist.y = (mp.y + 1 - p->y / 16) * deltadist.y;
		}
		bool hit = false;
		int sidetype = 0;
		float distance = 0;
		while (!hit) {
			if (sidedist.x < sidedist.y) {
				sidedist.x += deltadist.x;
				mp.x += step.x;
				sidetype = 0;
			}
			else {
				sidedist.y += deltadist.y;
				mp.y += step.y;
				sidetype = 1;
			}
			if (map[mp.x][mp.y] != 0) {
				hit = true;
				break;
			}
		}
		Uint8 col = 0;
		if (sidetype == 0) { perpwalldist = sidedist.x - deltadist.x; col = 160; }
		else if(sidetype == 1){ perpwalldist = sidedist.y - deltadist.y; col = 210; }
		int lh;
		if (x < 600 && x > 400 && perpwalldist <= 0.5) {
			lh = 400;
			p->x += pushvec.x;
			p->y += pushvec.y;
			pushvec.x = 0;
			pushvec.y = 0;
		}
		else {
			lh = (int)800 / perpwalldist;
		}
		int drawstart = -lh / 2 + 800 / 2;
		int drawend = lh / 2 + 800 / 2;
		if (map[mp.x][mp.y] == 3 || map[mp.x][mp.y] == 4) {
			SDL_Surface* cursurf = nullptr;
			switch (map[mp.x][mp.y]) {
			case 3:
				cursurf = wallsurf;
				break;
			case 4:
				cursurf = doorsurf;
				doordist = perpwalldist;
				break;
			}
			int pitch = 200;
			float wallX = 0;
			if (sidetype == 0) {
				wallX = p->y + perpwalldist * rayDir.y;
			}
			else {
				wallX = p->x + perpwalldist * rayDir.x;
			}
			//remove this for extremely glitched out textures
			wallX -= (int)wallX;
			//50 is texture width
			int texX = int(wallX * 50.0);
			if (sidetype == 0 && rayDir.x > 0) texX = 50 - texX - 1;
			if (sidetype == 1 && rayDir.y < 0) texX = 50 - texX - 1;
			float step = 1.0 * 50 / lh;
			float texPos = (drawstart - pitch - 50 / 2 + lh / 2) * step;
			for (int y = drawstart; y < drawend; y++) {
				int texY = (int)texPos & (50 - 1);
				texPos += step;
				Uint32 color = gore.GetPixelSurface(cursurf, &texY, &texX);
				if (y >= 0 && y <= 800 && x >= 0 && x <= 800) {
					gore.SetPixelSurface(surf, &y, &x, &color);
				}
			}
		}
		else {
			SDL_SetRenderDrawColor(rend, col, 100, 100, 0);
			SDL_RenderDrawLine(rend, x, drawstart, x, drawend);
		}
		zbuf[x] = perpwalldist;
	}
}
void constructAlphabet(SDL_Renderer* rend, TTF_Font* font, SDL_Color color, texp& head) {
	for (int i = 33; i < 123; i++) {
		char c = i;
		std::string co;
		co.push_back(c);
		SDL_Surface* surf = TTF_RenderText_Solid(font, co.c_str(), color);
		SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);
		SDL_FreeSurface(surf);
		gore.insertTex(head, tex, co);
		//std::cout << co << std::endl;
	}
}
void drawText(SDL_Renderer* rend, texp& head, std::string text, int x, int y, int w, int h, int mw) {
	int sx = x;
	int sy = y;
	int base = 0;
	for (auto& i : text) {
		std::string t;
		t.push_back(i);
		if (i == '\n') {
			sy += h + 1;
			sx = x;
			base = 0;
		}
		else {
			SDL_Rect pos = { sx, sy, w, h };
			SDL_RenderCopy(rend, gore.findTex(head, t.c_str()), NULL, &pos);
			sx += w + 1;
			base += w + 1;
			if (base > mw) {
				sx = x;
				sy += h + 1;
				base = 0;
			}
		}
	}
}

void generateLevel(Entity* p) {
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			map[i][j] = 1;
		}
	}
	int visited[16][16] = { 0 };
	int stx = 1;
	int sty = 1;
	p->x = 30;
	p->y = 10;
	map[stx][sty] = 0;
	while (map[stx][sty] != 4 && stx < 15 && sty < 15) {
		int bcheck = rand() % 2;
		int rcheck = rand() % 2;
		int neg = rand() % 100;
		if (neg > 49) {
			rcheck = -rcheck;
		}
		if (rcheck >= 1 || rcheck <= -1) {
			bcheck = 0;
		}
		int enemsquare = rand() % 100;
		if (visited[stx+bcheck][sty+rcheck] != 1) {
			visited[stx + bcheck][sty + rcheck] = 1;
			map[stx+bcheck][sty+rcheck] = 0;
			if (enemsquare > 48) {
				map[stx + bcheck][sty + rcheck] = 2;
			}
			sty += rcheck;
			stx += bcheck;
		}
	}
	map[0][0];
}


void parseLevel() {
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			if (map[i][j] == 2) {
				map[i][j] = 0;
				Sprite s;
				s.x = (i * 16);
				s.y = j * 16;
				s.surf = enemsurf;
				s.w = 300;
				s.h = 400;
				s.health = 100;
				s.frames = 3;
				s.f = 1;
				s.dead = false;
				s.ftime = 0.2;
				s.forward = true;
				s.type = 0;
				s.time = 0;
				s.distance = 0;
				sprites.push_back(s);
			}
			else if (map[i][j] == 4) {
				Sprite s;
				s.health = 100 * level;
				s.dead = false;
				s.ftime = 0.2;
				switch (level) {
				case 1:
					s.ftime = 1.2;
					break;
				case 2:
					s.ftime = 0.9;
					break;
				case 3:
					s.ftime = 0.6;
					break;
				case 4:
					s.ftime = 0.5;
					break;
				case 5:
					s.ftime = 0.2;
					break;
				}
				s.time = 0;
				s.type = 1;
				s.x = i;
				s.y = j;
				sprites.push_back(s);
			}
		}
	}
}

void endLevelText(Entity* p) {
	doordist = 100;
	sprites.clear();
	switch (level) {
	case 1:
		leveltext = "Oh wow you did it\n Never expected you\n to do it that fast\n";
		textmode = true;
		level++;
		p->x = 40;
		p->y = 50;
		p->health += 100;
		memcpy(map, map2, sizeof(map));
		parseLevel();
		break;
	case 2:
		leveltext = "I lost the textures in the next room\nDon't judge it too harshly\n I worked so hard on it";
		textmode = true;
		level++;
		p->x = 40;
		p->y = 50;
		p->health += 100;
		//memcpy(map, map3, sizeof(map));
		parseLevel();
		break;
	case 3:
		leveltext = "I'm surprised you enjoy it\n so much\nJust a few more levels\n and you're free";
		textmode = true;
		level++;
		p->x = 40;
		p->y = 50;
		p->health += 100;
		//memcpy(map, map4, sizeof(map));
		parseLevel();
		break;
	case 4:
		leveltext = "Can you slow down please?\nI don't know what\n I will do once you're gone...\n";
		textmode = true;
		level++;
		p->x = 40;
		p->y = 50;
		p->health += 100;
		//memcpy(map, map5, sizeof(map));
		parseLevel();
		break;
	case 5:
		leveltext = "That's it\nI don't have anything else for you\nBut please don't go...\nIt's so lonely in here\nPlease don't leave...";
		textmode = true;
		break;
	}
}

void death(Entity* p) {
	p->x = 40;
	p->y = 50;
	p->health = 100;
	level = 1;
	memcpy(map, map1, sizeof(map));
	doordist = 100;
	parseLevel();
}

//https://github.com/ssloy/tinyraycaster/wiki/Part-1:-crude-3D-renderings
//https://www.youtube.com/watch?v=gYRrGTC7GtA
//https://permadi.com/1996/05/ray-casting-tutorial-table-of-contents/
//https://lodev.org/cgtutor/raycasting.html
//https://www.youtube.com/watch?v=NbSee-XM7WA&list=WL&index=1
//Handmade levels
//Add waiting
//Add sound effects
//Make enemies static, maybe(path via tiles)
int main() {
	srand(time(NULL));
	if (!SDL_Init(SDL_INIT_EVERYTHING)) {
		std::cout << "SDL failed to load" << std::endl;
	}
	if (TTF_Init()) {
		std::cout << "TTF failed to init" << std::endl;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		std::cout << "Mixer failed" << std::endl;
	}
	memcpy(map, map1, sizeof(map));
	SDL_Window* wind = SDL_CreateWindow("Dungeon Crawler", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_SHOWN);
	rend = SDL_CreateRenderer(wind, -1, 0);
	TTF_Font* font = TTF_OpenFont("DelaGothicOne-Regular.ttf", 12);
	texp texthead = NULL;
	constructAlphabet(rend, font, {150, 200, 250}, texthead);
	bool exitf = false;
	texp atkhead = gore.loadTextureList({ "atk5.png", "atk4.png", "atk3.png", "atk2.png", "atk1.png" }, { 400, 400, 400, 400, 400 }, { 400, 400, 400, 400, 400 }
	, SDL_PIXELFORMAT_RGBA8888, rend, "Sprites/");
	surf = SDL_CreateRGBSurfaceWithFormat(0, 800, 800, 32, SDL_PIXELFORMAT_RGBA8888);
	wallsurf = gore.loadPNG("Sprites/wall1.png", SDL_PIXELFORMAT_RGBA8888, 50, 50);
	enemsurf = gore.loadPNG("Sprites/enemy1.png", SDL_PIXELFORMAT_RGBA8888, 300, 400);
	SDL_Surface* enemsurf2 = gore.loadPNG("Sprites/enemy1_2.png", SDL_PIXELFORMAT_RGBA8888, 300, 400);
	SDL_Surface* enemsurf3 = gore.loadPNG("Sprites/enemy1_3.png", SDL_PIXELFORMAT_RGBA8888, 300, 400);
	SDL_Surface* deadsurf = gore.loadPNG("Sprites/deadenemy.png", SDL_PIXELFORMAT_RGBA8888, 300, 400);
	doorsurf = gore.loadPNG("Sprites/door1.png", SDL_PIXELFORMAT_RGBA8888, 50, 50);
	enemtex = SDL_CreateTextureFromSurface(rend, enemsurf);
	Entity p = {40, 50, 100};
	p.strength = 1;
	p.agility = 1;
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	SDL_Event e;
	std::vector<Animate> animates;
	//generateLevel(&p);
	double delta;
	double btimer = 0, atktimer = 0, atkcool = 1.0;
	bool upgrade = false;
	parseLevel();
	std::sort(sprites.begin(), sprites.end(), spriteDistanceSort);
	Sprite* up = nullptr;
	while (!exitf) {
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				exitf = true;
				break;
			}
		}
		delta = gore.getDelta();
		btimer += delta;
		atktimer += delta;
		SDL_PumpEvents();
		if (keys[SDL_SCANCODE_W] && !upgrade) {
			p.x += dir.x * delta * 50;
			p.y += dir.y * delta * 50;
			pushvec.x = -(dir.x * delta * 50);
			pushvec.y = -(dir.y * delta * 50);
		}
		else if (keys[SDL_SCANCODE_S] && !upgrade) {
			p.x -= dir.x * delta * 50;
			p.y -= dir.y * delta * 50;
			pushvec.x = dir.x * delta * 50;
			pushvec.y = dir.y * delta * 50;
			
		}
		if (btimer > 0.1) {
			if (keys[SDL_SCANCODE_RIGHT] && !upgrade) {
				float olddirx = dir.x;
				dir.x = dir.x * cos(-1.5708) - dir.y * sin(-1.5708);
				dir.y = olddirx * sin(-1.5708) + dir.y * cos(-1.5708);
				double oldPlaneX = plane.x;
				plane.x = plane.x * cos(-1.5708) - plane.y * sin(-1.5708);
				plane.y = oldPlaneX * sin(-1.5708) + plane.y * cos(-1.5708);
				btimer = 0;
			}
			else if (keys[SDL_SCANCODE_LEFT] && !upgrade) {
				double olddirx = dir.x;
				dir.x = dir.x * cos(1.5708) - dir.y * sin(1.5708);
				dir.y = olddirx * sin(1.5708) + dir.y * cos(1.5708);
				double oldPlaneX = plane.x;
				plane.x = plane.x * cos(1.5708) - plane.y * sin(1.5708);
				plane.y = oldPlaneX * sin(1.5708) + plane.y * cos(1.5708);
				btimer = 0;
			}
		}
		Sprite* range = nullptr;
		bool drawehealth = false;
		if (atktimer > atkcool) {
			int mx, my;
			if (keys[SDL_SCANCODE_Q] && !upgrade || SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT) && !upgrade) {
				Sprite* sp = nullptr;
				bool skip = false;
				if (doordist < 1.5) {
					Animate a;
					a.x = 100;
					a.y = 100;
					a.w = 400;
					a.h = 400;
					a.time = 0;
					a.ftime = 0.05;
					a.sprites = atkhead;
					for (auto& i : sprites) {
						if (i.type == 1) {
							a.targ = &i;
						}
					}
					animates.push_back(a);
					atktimer = 0;
					skip = true;
				}
				if (!skip) {
					for (auto& i : sprites) {
						if (i.distance < 1.2) {
							sp = &i;
							atk = true;
							//move this to raycasting part
							Animate a;
							a.x = 100;
							a.y = 100;
							a.w = 400;
							a.h = 400;
							a.time = 0;
							a.ftime = 0.05;
							a.sprites = atkhead;
							a.targ = sp;
							animates.push_back(a);
							atktimer = 0;
							break;
						}
					}
				}
				
			}
			else if (keys[SDL_SCANCODE_E]) {
				for (auto& i : sprites) {
					if (i.distance < 1.2 && i.dead) {
						range = &i;
						upgrade = true;
						up = &i;
						break;
					}
				}
			}
		}
		

		SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
		SDL_RenderClear(rend);
		if (!dead) {
			raycastDDA(&p, rend);

			if (!upgrade) {
				int n = 0;
				for (auto& i : sprites) {
					i.distance = (sqrt(((p.x - i.x) * (p.x - i.x) + (p.y - i.y) * (p.y - i.y)))) / 16;
					//std::cout << i.distance << std::endl;
					if (!i.dead) {
						if (i.type == 0) {
							if (i.distance <= 1.2) {
								range = &i;
								drawehealth = true;
								i.time += delta;
								if (i.time > i.ftime) {
									if (i.forward) {
										i.f++;
										if (i.f > i.frames) {
											i.f = i.frames;
											i.forward = false;
										}
									}
									else {
										i.f--;
										if (i.f < 1) {
											i.f = 1;
											p.health -= 5;
											i.forward = true;
										}
									}
									switch (i.f) {
									case 1:
										i.surf = enemsurf;
										break;
									case 2:
										i.surf = enemsurf2;
										break;
									case 3:
										i.surf = enemsurf3;
										break;
									}
									i.time = 0;
								}
							}
							else {
								i.surf = enemsurf;
							}
							if (i.health <= 0) {
								i.health = 1;
								i.dead = true;
							}
						}
						else if (i.type == 1) {
							i.time += delta;
							if (doordist < 1.2) {
								drawehealth = true;
								range = &i;
								if (i.time > i.ftime) {
									p.health -= 10 * level;
									i.time = 0;
								}
							}
							if (i.health <= 0) {
								i.dead = true;
								map[i.x][i.y] = 0;
								endLevelText(&p);
							}
						}
					}
					else {
						i.surf = deadsurf;
						if (i.health <= 0) {
							sprites.erase(sprites.begin() + n);
						}
					}
					n++;
				}
			}
			std::sort(sprites.begin(), sprites.end(), spriteDistanceSort);
			std::cout << "X: " << p.x << " Y: " << p.y << std::endl;
			//std::cout << doordist << std::endl;
			for (int i = 0; i < sprites.size(); i++) {
				if (sprites[i].type == 0) {
					float spx = sprites[i].x - p.x;
					float spy = sprites[i].y - p.y;
					float invDet = 1.0 / (plane.x * dir.y - dir.x * plane.y);
					float tfx = invDet * (dir.y * spx - dir.x * spy);
					float tfy = invDet * (-plane.y * spx + plane.x * spy);

					int spritescreenx = int((800 / 2) * (1 + tfx / tfy));
					int spriteheight = std::abs(int(10000 / tfy));

					int drawstarty = -spriteheight / 2 + 800 / 2;
					if (drawstarty < 0) drawstarty = 0;
					int drawEndY = spriteheight / 2 + 800 / 2;
					if (drawEndY >= 800) drawEndY = 800 - 1;

					int spriteWidth = std::abs(int(10000 / (tfy)));

					int drawStartX = -spriteWidth / 2 + spritescreenx;
					if (drawStartX < 0) drawStartX = 0;
					int drawEndX = spriteWidth / 2 + spritescreenx;
					if (drawEndX >= 800) drawEndX = 800 - 1;

					int n = 1;
					int cy = 0;
					for (int stripe = drawStartX; stripe < drawEndX; stripe++, n++)
					{
						int texX = int(256 * (stripe - (-spriteWidth / 2 + spritescreenx)) * 300 / spriteWidth) / 256;
						if (tfy > 0 && stripe > 0 && stripe < 800 && sprites[i].distance < zbuf[stripe])
							for (int y = drawstarty; y < drawEndY; y++) //for every pixel of the current stripe
							{
								int d = (y) * 256 - 800 * 128 + spriteheight * 128; //256 and 128 factors to avoid floats
								int texY = ((d * 400) / spriteheight) / 256;
								if (texY >= 0 && texY <= 400 && texX >= 0 && texX <= 300) {
									Uint32 color = gore.GetPixelSurface(sprites[i].surf, &texY, &texX);
									gore.SetPixelSurface(surf, &y, &stripe, &color);
								}
							}
					}
				}
			}
			SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);
			SDL_Rect frect = { 0, 0, 800, 800 };
			SDL_RenderCopy(rend, tex, NULL, &frect);
			SDL_DestroyTexture(tex);
			gore.clearSurface(surf);
			if (upgrade) {
				SDL_SetRenderDrawColor(rend, 150, 255, 50, 0);
				SDL_Rect rect = { 200, 200, 270, 220 };
				SDL_RenderFillRect(rend, &rect);
				//Button to upgrade strength
				//Button upgrade agility
				SDL_SetRenderDrawColor(rend, 255, 50, 150, 0);
				SDL_Rect frect = { 210, 210, 250, 100 };
				SDL_RenderFillRect(rend, &frect);
				drawText(rend, texthead, "Upgrade Strength", 220, 220, 26, 28, 200);
				SDL_SetRenderDrawColor(rend, 100, 50, 250, 0);
				SDL_Rect erect = { 210, 310, 250, 100 };
				SDL_RenderFillRect(rend, &erect);
				drawText(rend, texthead, "Upgrade Agility", 220, 320, 26, 28, 200);
				int mx, my;
				if (SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
					if (mx < 210 + 250 && mx + 1 > 210 && my < 210 + 100 && my + 1 > 210) {
						p.strength++;
						upgrade = false;
						p.health += 25;
						up->health = 0;
						atktimer = 0;
					}
					else if (mx < 210 + 250 && mx + 1 > 210 && my < 310 + 100 && my + 1 > 310) {
						p.agility++;
						atkcool -= 0.05;
						upgrade = false;
						p.health += 25;
						up->health = 0;
						atktimer = 0;
					}
				}

			}
			std::string temp = "Health: " + std::to_string(p.health);
			drawText(rend, texthead, temp, 10, 10, 20, 20, 500);
			if (drawehealth) {
				std::string temp2 = "Enemy Health: " + std::to_string(range->health);
				drawText(rend, texthead, temp2, 350, 10, 20, 20, 500);
			}
			std::string temp3 = "Strength: " + std::to_string(p.strength);
			drawText(rend, texthead, temp3, 250, 30, 20, 20, 500);
			std::string temp4 = "Agility: " + std::to_string(p.agility);
			drawText(rend, texthead, temp4, 250, 60, 20, 20, 500);
			int j = 0;
			for (auto& i : animates) {
				i.time += delta;
				if (i.time > i.ftime) {
					i.sprites = i.sprites->next;
					i.time = 0;
				}
				if (i.sprites == NULL) {
					animates.erase(animates.begin() + j);
					i.targ->health -= 5 * p.strength;
					std::cout << i.targ->health << std::endl;
				}
				else {
					SDL_Rect rect = { i.x, i.y, i.w, i.h };
					SDL_RenderCopy(rend, i.sprites->current, NULL, &rect);
				}
				j++;
			}
			if (textmode) {
				SDL_SetRenderDrawColor(rend, 50, 100, 255, 0);
				SDL_Rect rect = { 100, 100, 400, 400 };
				SDL_RenderFillRect(rend, &rect);
				drawText(rend, texthead, leveltext, 110, 110, 15, 15, 350);
				if (btimer > 0.15) {
					int mx, my;
					if (SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
						textmode = false;
					}
					btimer = 0;
				}
			}
			if (p.health <= 0) {
				dead = true;
			}
		}
		else {
		drawText(rend, texthead, "YOU DIED!", 250, 300, 25, 25, 600);
		drawText(rend, texthead, "Left click to try again", 250, 400, 25, 25, 600);
			if (btimer > 0.15) {
				int mx, my;
				if (SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
					death(&p);
					dead = false;
				}
				btimer = 0;
			}
		}
		SDL_RenderPresent(rend);
	}
	TTF_Quit();
	Mix_CloseAudio();
	SDL_Quit();
	return 0;
}