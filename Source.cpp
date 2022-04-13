#pragma once
#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <math.h>
#include "GoreEngine.h"
#define P2 1.5708
#define P3 4.71239
#undef main
SDL_Renderer* rend;
Gore gore;



struct Entity {
	float x;
	float y;
	float dy;
	float dx;
	float angle;
	int dir;
	int ny;
	int nx;
};
struct Point {
	float x;
	float y;
};
struct IntPoint {
	int x;
	int y;
};
Point camera = { 22, 3 };
Point dir = { -1, 0 };
Point plane = { 0, 0.66 };

int FixAng(int a) { if (a > 6.28319) { a -= 6.28319; } if (a < 0) { a += 6.28319; } return a; }
size_t mw = 16;
size_t mh = 16;
int map[16][16] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,
	1,0,1,0,1,0,0,1,1,1,1,1,1,1,1,1,
	1,0,0,0,1,0,0,1,1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,1,1,1,1,1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,};

float dist(float ax, float ay, float bx, float by, float ang) {
	return cos((ang)*(bx-ax) - sin(ang) * (by-ay));
}


Point normalize(Point p) {
	float rx = 1 / std::abs(p.x);
	float ry = 1 / std::abs(p.y);
	Point e = { p.x * rx, p.y * ry };
	return e;
}

void castRays(Entity* p, SDL_Renderer* rend) {
	int r, mx, my, mp, dof; float rx, ry, ra, xo, yo;
	float retDist = 0;
	ra = p->angle - (0.0174533 * 30); if (ra < 0) { ra += 6.28319; }  if (ra > 6.28319) { ra -= 6.28319; }
	int lasto = 0;
	int off = 0;
	bool skip = false;
	for (r = 0; r < 60; r++) {
		dof = 0;
		float disH = 10000000, hx = p->x, hy = p->y;
		float Ttan = (1.0 / tan(ra));
		if (ra > M_PI) { ry = (((int)p->y >> 6) << 6) - 0.0001; rx = (p->y - ry) * Ttan + p->x; yo = -64; xo = -yo * Ttan; }
		else if (ra < M_PI) { ry = (((int)p->y >> 6) << 6) + 64; rx = (p->y - ry) * Ttan + p->x; yo = 64; xo = -yo * Ttan; }
		else { rx = p->x; ry = p->y; dof = 8; }
		while (dof < 8) {
			mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mw + mx;
			if (mp > 0 && mp < mw * mh && map[mp] == 0) {
				hx = rx;
				hy = ry;
				disH = dist(p->x, p->y, hx, hy, ra);
				dof = 8;
			}
			else {
				rx += xo; ry += yo; dof++;
			}
		}
		dof = 0; 
		float disV = 10000000, vx = p->x, vy = p->y;
		float nTan = tan(ra);
		if (ra > P2 && ra < P3) { rx = (((int)p->x >> 6) << 6) - 0.0001; ry = (p->x - rx) * nTan + p->y; xo = 64; yo = -xo * nTan; }
		else if (ra < P2 || ra > P3) { rx = (((int)p->x >> 6) << 6) + 64; ry = (p->x - rx) * nTan + p->y; xo = -64; yo = -xo * nTan; }
		else { rx = p->x; ry = p->y; dof = 8; }
		while (dof < 8) {
			mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mw + mx;
			if (mp > 0 && mp < mw * mh && map[mp][0] == 1) {
				vx = rx;
				vy = ry;
				disV = dist(p->x, p->y, vx, vy, ra);
				dof = 8;
			}
			else {
				rx += xo; ry += yo; dof++;
			}
		}
		vx = rx;
		vy = ry;
		if (disV < disH) {
				retDist = disV;
				SDL_SetRenderDrawColor(rend, 255, 50, 255, 0);
				SDL_RenderDrawLine(rend, p->x, p->y, vx, vy);
		}
		else if(disH < disV) {
			retDist = disH;
			SDL_SetRenderDrawColor(rend, 40, 50, 255, 0);
			SDL_RenderDrawLine(rend, p->x, p->y, hx, hy);
		}
		//Drawing walls
		int lh = (400) / retDist;
		if (lh > 400) { lh = 400; }
		int lo = 200 - (lh >> 1);
		SDL_SetRenderDrawColor(rend, 255, 50, 100, 0);
		SDL_Rect trect = { r, 200 - lh, 13, 400 + lh };
		//SDL_RenderDrawLine(rend, r * 17, lo, r * 17, lo + lh);
		SDL_RenderFillRect(rend, &trect);
		ra += 0.0174533; if (ra < 0) { ra += 6.28319; }  else if (ra > 6.28319) { ra -= 6.28319; }
	}
}

void DDAcast(Entity* p, SDL_Renderer* rend) {
	Point rayStart = { p->x, p->y };
	Point rayDir = {cos(p->angle), sin(p->angle)};
	Point rayStep = { sqrt(1 + (rayDir.y * rayDir.y) / (rayDir.x / rayDir.x)), sqrt(1 + (rayDir.x * rayDir.x) / (rayDir.y / rayDir.y)) };
	//std::cout << rayStep.x << " : " << rayStep.y << std::endl;;
	Point mpcheck = rayStart;
	Point rayLength;
	IntPoint step;

	if (rayDir.x < 0) {
		step.x = -1;
		rayLength.x = ((rayStart.x - float(mpcheck.x)) * rayStep.x);
	}
	else {
		step.x = 1;
		rayLength.x = (float(mpcheck.x + 1) - rayStart.x) * rayStep.x;
	}
	if (rayDir.y < 0) {
		step.y = -1;
		rayLength.y = ((rayStart.y - float(mpcheck.y)) * rayStep.y);
	}
	else {
		step.y = 1;
		rayLength.y = (float(mpcheck.y + 1) - rayStart.y) * rayStep.y;
	}
	//std::cout << rayLength.x << ":" << rayLength.y << std::endl;
	bool tilefound = false;
	float distance = 0;
	Point stop = {0, 0};
	while (!tilefound && distance < 600) {
		if (rayLength.x < rayLength.y) {
			mpcheck.x += step.x;
			distance = rayLength.x;
			rayLength.x += rayStep.x;
		}
		else {
			mpcheck.y += step.y;
			distance = rayLength.y;
			rayLength.y += rayStep.y;
		}

		if (map[(int)mpcheck.y * mw + (int)mpcheck.x] != 0) {
			stop.x = rayStart.x + rayDir.x * distance;
			stop.y = rayStart.y + rayDir.y * distance;
			std::cout << stop.x << ":" << stop.y << std::endl;
			tilefound = true;

		}
	}
	SDL_SetRenderDrawColor(rend, 100, 100, 255, 0);
	SDL_RenderDrawLine(rend, p->x, p->y, stop.x, stop.y);

	SDL_SetRenderDrawColor(rend, 255, 0, 255, 0);
	//SDL_RenderDrawLine(rend, p->x, p->y, p->x + rayDir.x * 180 / M_PI, p->y + rayDir.y * 180 / M_PI);


}
void bruteForce(Entity* p, SDL_Renderer* rend) {
	//find cell player is in
	int sx = 0, sy = 0, curcell = 0, n = 0;
	for (auto& i : map) {
		if (p->x < sx + 50 && p->x + 15 > sx && p->y < sy + 50 && p->y + 15 > sy) {
			curcell = n;
			break;
		}
		sx += 50;
		if (sx > 800) {
			sx = 0;
			sy += 50;
		}
		n++;
	}
	//fire "ray" in direction player is looking
	//std::cout << slope << std::endl;
	SDL_SetRenderDrawColor(rend, 0, 50, 255, 0);
	SDL_Rect skyrect = { 0,0, 800, 400 };
	SDL_RenderFillRect(rend, &skyrect);
	float tr = p->x, te = p->y, ta = p->angle - (0.0174533 * 30); if (ta < 0) { ta += 6.28319; }  if (ta > 6.28319) { ta -= 6.28319; }
	int t = 0;
	while (t < 800) {
		curcell = n;
		//convert the angle to slope and travel along that slope
		float slopex = cos(ta) / 200;
		float slopey = sin(ta) / 200;
		while (map[curcell] == 0) {
			tr += slopex * 180 / M_PI;
			te += slopey * 180 / M_PI;
			curcell = ((int)te / 50) * mw + ((int)tr / 50);
		}
		float dis = sqrt(pow(p->x - tr, 2.0) + pow(p->y - te, 2.0));
		if (dis < 1) {
			dis = 1;
		}
		int wh = 500 / (int)dis;
		int col = (((int)dis) / 2) * 255;
		SDL_SetRenderDrawColor(rend, col, 100, 100, 0);
		SDL_Rect rect = { t, 200 - wh, 1, 400 + wh };
		SDL_RenderFillRect(rend, &rect);
		SDL_SetRenderDrawColor(rend, 0, 255, 0, 0);
		SDL_RenderDrawLine(rend, p->x, p->y, tr, te);
		t++;
		//The percantge of one degree we increase by
		ta += 0.001309;
		//ta += 0.00392699;
		if (ta > 6.28319) {
			//std::cout << ta << std::endl;
			//ta = 0;
		}
	}

	//Draw boxes based on cells
}
void raycastDDA(Entity* p, SDL_Renderer* rend) {
	Point rayDir;
	int sx = 0, sy = 0;
	int curcell = 0;
	int n = 0;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			if (p->ny == i && p->nx == j) {
				curcell = n;
				break;
			}
		}
		n++;
	}
	for (int x = 0; x < 800; x++) {
		camera.x = 2 * x / float(800) - 1;
		rayDir.x = dir.x + plane.x * camera.x;
		rayDir.y = dir.y + plane.y * camera.x;
		Point sidedist;
		IntPoint mp = { p->nx, p->ny };
		Point deltadist = { sqrt(1 + (rayDir.y * rayDir.y) / (rayDir.x * rayDir.x)), sqrt(1 + (rayDir.x * rayDir.x) / (rayDir.y * rayDir.y)) };
		float perpwalldist;
		IntPoint step = {0, 0};
		if (rayDir.x < 0) {
			step.x = -1;
			sidedist.x = (p->nx - mp.x) * deltadist.x;
		}
		else {
			step.x = 1;
			sidedist.x = (mp.x + 1 - p->nx) * deltadist.x;
		}
		if (rayDir.y < 0) {
			step.y = -1;
			sidedist.y = (p->ny - mp.y) * deltadist.y;
		}
		else {
			step.y = 1;
			sidedist.y = (mp.y + 1 - p->ny) * deltadist.y;
		}
		bool hit = false;
		int sidetype = 0;
		float distance = 0;
		while (!hit && distance < 600) {
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
				//std::cout << mp.y << " : " << mp.x << std::endl;
				hit = true;
				break;
			}
		}
		SDL_SetRenderDrawColor(rend, 0, 255, 100, 0);
		SDL_RenderDrawLine(rend, p->nx * 50, p->ny * 50, (p->nx * 50) + rayDir.x * sidedist.x, (p->ny * 50) + rayDir.y * sidedist.y);
		Uint8 col = 0;
		if (sidetype == 0) { perpwalldist = sidedist.x - deltadist.x; col = 160; }
		else { perpwalldist = sidedist.y - deltadist.y; col = 210; }
		int lh;
		if (perpwalldist <= 0) {
			lh = 400;
		}
		else {
			lh = (int)800 / perpwalldist;
		}
		SDL_SetRenderDrawColor(rend, col, 100, 100, 0);
		int drawstart = -lh / 2 + 800 / 2;
		int drawend = lh / 2 + 800 / 2;
		SDL_RenderDrawLine(rend, x, drawstart, x, drawend);
		//SDL_Rect rect = { x, drawstart, x * 2, drawend };
		//SDL_RenderFillRect(rend, &rect);

	}
	SDL_SetRenderDrawColor(rend, 100, 255, 100, 0);
	SDL_RenderDrawLine(rend, p->nx * 50, p->ny * 50, (p->nx * 50) + dir.x, p->ny * 50 + dir.y);
}


//https://github.com/ssloy/tinyraycaster/wiki/Part-1:-crude-3D-renderings
//https://www.youtube.com/watch?v=gYRrGTC7GtA
//https://permadi.com/1996/05/ray-casting-tutorial-table-of-contents/
//Try DDA instead of whatever the fuck this is
//https://lodev.org/cgtutor/raycasting.html
//https://www.youtube.com/watch?v=NbSee-XM7WA&list=WL&index=1
int main() {
	if (!SDL_Init(SDL_INIT_EVERYTHING)) {
		std::cout << "SDL failed to load" << std::endl;
	}
	if (TTF_Init()) {
		std::cout << "TTF failed to init" << std::endl;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		std::cout << "Mixer failed" << std::endl;
	}
	SDL_Window* wind = SDL_CreateWindow("Dungeon Crawler", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_SHOWN);
	rend = SDL_CreateRenderer(wind, -1, 0);
	bool exitf = false;
	Entity p = {150.0, 200};
	p.angle = 0;
	p.dx = cos(p.angle) * 5;
	p.dy = -sin(p.angle) * 5;
	p.ny = 3;
	p.nx = 3;
	p.dir = 0;
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	SDL_Event e;
	double delta;
	double btimer = 0;
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
		SDL_PumpEvents();
		if (p.ny < 0) {
			p.ny = 0;
		}
		else if (p.ny > 15) {
			p.ny = 15;
		}
		if (p.nx < 0) {
			p.nx = 0;
		}
		else if (p.nx > 15) {
			p.nx = 15;
		}
		if (btimer > 0.05) {
		if (keys[SDL_SCANCODE_W]) {
			p.y -= 100 * delta;
			p.ny--;
			btimer = 0;
			//p.x -= p.dx * delta * 100; p.y += p.dy * delta * 100;
		}
		else if (keys[SDL_SCANCODE_S]) {
			p.y += 100 * delta;
			p.ny++;
			btimer = 0;
			//p.x += p.dx * delta * 100; p.y -= p.dy * delta * 100;
		}
		else if (keys[SDL_SCANCODE_A]) {
			p.x -= 100 * delta;
			p.nx--;
			btimer = 0;
		}
		else if (keys[SDL_SCANCODE_D]) {
			p.x += 100 * delta;
			p.nx++;
			btimer = 0;
		}
			if (keys[SDL_SCANCODE_RIGHT]) {
				//p.angle -= 1.5708;
				p.angle += 0.01;
				if (p.angle < 0) { p.angle += 6.28319; }
				p.dx = cos(p.angle);
				p.dy = -sin(p.angle);
				//std::cout << p.angle << std::endl;
				float olddirx = dir.x;
				dir.x = dir.x * cos(-delta * 3) - dir.y * sin(-delta * 3);
				dir.y = olddirx * sin(-delta * 3) + dir.y * cos(-delta * 3);
				double oldPlaneX = plane.x;
				plane.x = plane.x * cos(-delta * 3) - plane.y * sin(-delta * 3);
				plane.y = oldPlaneX * sin(-delta * 3) + plane.y * cos(-delta * 3);
				btimer = 0;
			}
			else if (keys[SDL_SCANCODE_LEFT]) {
				//p.angle += 1.5708;
				p.angle -= 0.01;
				if ((p.angle) > 6.28319) { p.angle -= 6.28319; }
				//std::cout << p.angle << std::endl;
				p.dx = cos(p.angle);
				p.dy = -sin(p.angle);
				double olddirx = dir.x;
				dir.x = dir.x * cos(delta * 3) - dir.y * sin(delta * 3);
				dir.y = olddirx * sin(delta * 3) + dir.y * cos(delta * 3);
				double oldPlaneX = plane.x;
				plane.x = plane.x * cos(delta * 3) - plane.y * sin(delta * 3);
				plane.y = oldPlaneX * sin(delta * 3) + plane.y * cos(delta * 3);
				btimer = 0;
			}
		}
		SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
		SDL_RenderClear(rend);
		SDL_SetRenderDrawColor(rend, 100, 100, 20, 0);
		SDL_RenderDrawLineF(rend, p.x, p.y, p.x - p.dx * 10, p.y - p.dy * 10);
		for (size_t j = 0; j < mh; j++) {
			for (size_t i = 0; i < mw; i++) {
				if (i == p.nx && j == p.ny) {
					SDL_SetRenderDrawColor(rend, 50, 255, 50, 0);
					SDL_Rect prect = { p.nx * 50, p.ny * 50, 15, 15 };
					SDL_RenderFillRect(rend, &prect);
				}
				if (map[j][i] != 0) {
					size_t rx = i * 50;
					size_t ry = j * 50;
					SDL_Rect rect = { rx, ry, 50, 50 };
					SDL_SetRenderDrawColor(rend, 255, 50, 50, 0);
					SDL_RenderDrawRect(rend, &rect);
				}
			}
		}
		//bruteForce(&p, rend);
		//castRays(&p, rend);
		//DDAcast(&p, rend);
		raycastDDA(&p, rend);
		SDL_RenderPresent(rend);
	}
	TTF_Quit();
	Mix_CloseAudio();
	SDL_Quit();
	return 0;
}