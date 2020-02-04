#include <stdio.h>
#include <limits.h>
#include <SDL2/SDL.h>
#include "config.h"
#include "structures.h"
#include "sdlSetup.h"
#include "map.h"

//////////////////////////////////////////////////////////////////////
//VARIABLES INIT//
//////////////////
int gameRunning = FALSE;
int lastFrameTimestamp = 0;
struct Player player;
struct Ray rays[RAYS_NUMBER];

//main game setup - player starting position
void setupGame() {
    player.x = WIN_WIDTH/2;
    player.y = WIN_HEIGHT/2;
    player.size = 10;
    player.turnDir = 0;
    player.walkDIr = 0;
    player.rotationAng = M_PI/2;
    player.walkSpeedMod = 200;
    player.turnSpeedMod = 45 * M_PI / 180;
}

//fetch player input by sdl events
void playerInput() {
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type) {
        case SDL_QUIT: {
            gameRunning = FALSE;
            break;
        } //close game window
        case SDL_KEYDOWN: {
            if(event.key.keysym.sym == SDLK_ESCAPE)
                gameRunning = FALSE;
            if(event.key.keysym.sym == SDLK_UP)
                player.walkDIr = 1;
            if(event.key.keysym.sym == SDLK_DOWN)
                player.walkDIr = -1;
            if(event.key.keysym.sym == SDLK_RIGHT)
                player.turnDir = 1;
            if(event.key.keysym.sym == SDLK_LEFT)
                player.turnDir = -1;
            break;
        } //key pressed down
        case SDL_KEYUP: {
            if(event.key.keysym.sym == SDLK_UP)
                player.walkDIr = 0;
            if(event.key.keysym.sym == SDLK_DOWN)
                player.walkDIr = 0;
            if(event.key.keysym.sym == SDLK_RIGHT)
                player.turnDir = 0;
            if(event.key.keysym.sym == SDLK_LEFT)
                player.turnDir = 0;
            break;
        } //key released
    }
}

//check if point (x,y) is inside wall
int wallCollisionDetector (float x, float y) {
    if (map[(int)y/TILE_SIZE][(int)x/TILE_SIZE]==0)
        return FALSE;
    else
        return TRUE;
}

float calculateDistance (float x1,float y1,float x2,float y2) {
    float distance = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
    return distance;
}


//updating game state
void updateState() {
    //wait with update for next frame
    while (!SDL_TICKS_PASSED(SDL_GetTicks(),lastFrameTimestamp
        + FRAME_TIME));

    //delta time
    float timeBeetweenFrames =
        (SDL_GetTicks()-lastFrameTimestamp) / 1000.0f;
    lastFrameTimestamp = SDL_GetTicks();

    //move player
    player.rotationAng += player.turnDir * player.turnSpeedMod * timeBeetweenFrames;
    float updatedX = player.x + cos(player.rotationAng) * player.walkDIr
        * player.walkSpeedMod * timeBeetweenFrames;
    float updatedY = player.y + sin(player.rotationAng) * player.walkDIr
        * player.walkSpeedMod * timeBeetweenFrames;
    if (wallCollisionDetector(updatedX,updatedY)==FALSE) {
        player.x = updatedX;
        player.y = updatedY;
    }

    //cast rays
    float rayAngle = player.rotationAng - (FOV_RAD / 2);
    for (int i = 0; i < RAYS_NUMBER; i++) {
        //rayAngle beetween 0 and 2Pi
        rayAngle = remainder(rayAngle, 2*M_PI);
        if (rayAngle < 0) {
            rayAngle = 2*M_PI + rayAngle;
        }

        //printf("%f  %d: %f\n",FOV_RAD, i, rayAngle);

        //where ray is facing
        int RayDown = rayAngle > 0 && rayAngle < M_PI;
        int RayUp = !RayDown;
        int RayRight = rayAngle < 0.5 * M_PI || rayAngle > 1.5 * M_PI;
        int RayLeft = !RayRight;

        //looking for horizontal wall collision
        float xCollision, yCollision, xStep, yStep, xTempHorizontalHit, yTempHorizontalHit, xHorizontalHit = 0, yHorizontalHit = 0;
        int horizontalHit = FALSE;
        int horizontalHitMaterial = 0;

        //collision with horizontal line
        yCollision = floor(player.y/TILE_SIZE)*TILE_SIZE;
        yCollision += RayDown ? TILE_SIZE : 0;

        //x of horizontal collision
        xCollision = player.x + (yCollision - player.y) / tan(rayAngle);

        //jump length to next horizontal line
        yStep = TILE_SIZE;
        yStep *= RayUp ? -1 : 1;
        xStep = TILE_SIZE / tan(rayAngle);
        xStep *= (RayLeft && xStep > 0) ? -1 : 1;
        xStep *= (RayRight && xStep < 0) ? -1 : 1;

        //jumping on horizontal lines till we find a wall
        xTempHorizontalHit = xCollision;
        yTempHorizontalHit = yCollision;

        while (xTempHorizontalHit >= 0 && xTempHorizontalHit < WIN_WIDTH && yTempHorizontalHit >=0 && yTempHorizontalHit < WIN_HEIGHT) {
            float modifier = RayUp ? -1 : 0;
            if (wallCollisionDetector(xTempHorizontalHit, yTempHorizontalHit + modifier)) {
                //wall hit!
                horizontalHit = TRUE;
                xHorizontalHit = xTempHorizontalHit;
                yHorizontalHit = yTempHorizontalHit;
                horizontalHitMaterial = map [(int)((yTempHorizontalHit + modifier)/TILE_SIZE)][(int)(xTempHorizontalHit/TILE_SIZE)];
                break;
            } else {
                xTempHorizontalHit += xStep;
                yTempHorizontalHit += yStep;
            }
        }

        //looking for vertical wall collision
        float xTempVerticalHit, yTempVerticalHit, xVerticalHit = 0, yVerticalHit = 0;
        int verticalHit = FALSE;
        int verticalHitMaterial = 0;

        //collision with vertical line
        xCollision = floor(player.x/TILE_SIZE)*TILE_SIZE;
        xCollision += RayRight ? TILE_SIZE : 0;

        //y of vertical collision
        yCollision = player.y + (xCollision - player.x) * tan(rayAngle);

        //jump length to next vertical line
        xStep = TILE_SIZE;
        xStep *= RayLeft ? -1 : 1;
        yStep = TILE_SIZE * tan(rayAngle);
        yStep *= (RayUp && yStep > 0) ? -1 : 1;
        yStep *= (RayDown && yStep < 0) ? -1 : 1;

        //jumping on vertical lines till we find a wall
        xTempVerticalHit = xCollision;
        yTempVerticalHit = yCollision;

        while (xTempVerticalHit >= 0 && xTempVerticalHit < WIN_WIDTH && yTempVerticalHit >=0 && yTempVerticalHit < WIN_HEIGHT) {
            float modifier = RayLeft ? -1 : 0;
            if (wallCollisionDetector(xTempVerticalHit + modifier, yTempVerticalHit)) {
                //wall hit!
                verticalHit = TRUE;
                xVerticalHit = xTempVerticalHit;
                yVerticalHit = yTempVerticalHit;
                verticalHitMaterial = map [(int)(yTempVerticalHit/TILE_SIZE)][(int)((xTempVerticalHit + modifier)/TILE_SIZE)];
                break;
            } else {
                xTempVerticalHit += xStep;
                yTempVerticalHit += yStep;
            }
        }

        //check which hit happend?
        //horizontal or vertical?
        float horizontalHitDistance = horizontalHit ? calculateDistance(player.x,player.y,xHorizontalHit,yHorizontalHit) : INT_MAX;
        float verticalHitDistance = verticalHit ? calculateDistance(player.x,player.y,xVerticalHit,yVerticalHit) : INT_MAX;
        if (verticalHitDistance < horizontalHitDistance) {
            rays[i].distance = verticalHitDistance;
            rays[i].hitX = xVerticalHit;
            rays[i].hitY = yVerticalHit;
            rays[i].hitVertical = TRUE;
            rays[i].hitMaterial = verticalHitMaterial;
        } else {
            rays[i].distance = horizontalHitDistance;
            rays[i].hitX = xHorizontalHit;
            rays[i].hitY = yHorizontalHit;
            rays[i].hitVertical = FALSE;
            rays[i].hitMaterial = horizontalHitMaterial;
        }

        //save hit in rays array
        rays[i].angle = rayAngle;
        rays[i].rayUp = RayUp;
        rays[i].rayDown = RayDown;
        rays[i].rayLeft = RayLeft;
        rays[i].rayRight = RayRight;

        float raysNum = RAYS_NUMBER;
        rayAngle += (FOV_RAD / raysNum);
    }
}

void renderObjects() {
    //SDL uses double buffer
    //it fills back buffer and swaps with front buffer
    //when SDL_RenderPresent is used

    //clear screen
    SDL_SetRenderDrawColor(renderer, 29, 27, 27, 255);
    //clear buffer
    SDL_RenderClear(renderer);

    //render walls
    for (int i = 0; i < RAYS_NUMBER; i++) {
        float FOVDistance = (WIN_WIDTH/2) / tan(FOV_RAD/2);
        float wallDistanceFixed = rays[i].distance*cos(rays[i].angle-player.rotationAng);
        float wallHeight = (TILE_SIZE / wallDistanceFixed) * FOVDistance;
        int shader = (rays[i].hitVertical==TRUE) ? 255 : 125;
        switch (rays[i].hitMaterial) {
            case 1:
                SDL_SetRenderDrawColor(renderer,255,0,0,shader);
                break;
            case 2:
                SDL_SetRenderDrawColor(renderer,0,255,0,shader);
                break;
            case 3:
                SDL_SetRenderDrawColor(renderer,0,0,255,shader);
                break;
        }
        SDL_RenderDrawLine(renderer,i,
            (WIN_HEIGHT/2)-(wallHeight/2),i,(WIN_HEIGHT/2)+(wallHeight/2));
    }

    //render map
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_Rect background = {0*MINIMAP_SCALE,
                0*MINIMAP_SCALE,
                WIN_WIDTH*MINIMAP_SCALE,
                WIN_HEIGHT*MINIMAP_SCALE};
    SDL_RenderFillRect(renderer,&background);
    for (int i = 0; i < MAP_ROWS; i++) {
        for (int j = 0; j < MAP_COLS; j++) {
            switch (map[i][j]) {
            case 1:
                SDL_SetRenderDrawColor(renderer,255,0,0,255);
                break;
            case 2:
                SDL_SetRenderDrawColor(renderer,0,255,0,255);
                break;
            case 3:
                SDL_SetRenderDrawColor(renderer,0,0,255,255);
                break;
            case 0:
                SDL_SetRenderDrawColor(renderer, 29,27, 27, 255);
                break;
            }
            SDL_Rect mapSquare = {j*TILE_SIZE*MINIMAP_SCALE,
                i*TILE_SIZE*MINIMAP_SCALE,
                TILE_SIZE*MINIMAP_SCALE,
                TILE_SIZE*MINIMAP_SCALE};
            SDL_RenderFillRect(renderer,&mapSquare);
        }
    }

    //render rays
    SDL_SetRenderDrawColor(renderer,255,255,255,255);
    for(int i = 0; i < RAYS_NUMBER; i++) {
        SDL_RenderDrawLine(renderer,player.x*MINIMAP_SCALE,
            player.y*MINIMAP_SCALE,
            rays[i].hitX*MINIMAP_SCALE,
            rays[i].hitY*MINIMAP_SCALE);
    }

    //render player
    SDL_SetRenderDrawColor(renderer, 253, 237, 235, 255);
    SDL_Rect playerMarker = {(player.x-player.size/2)*MINIMAP_SCALE,
        (player.y-player.size/2)*MINIMAP_SCALE, player.size*MINIMAP_SCALE,
        player.size*MINIMAP_SCALE
    };
    SDL_RenderFillRect(renderer,&playerMarker);
    SDL_RenderDrawLine(renderer,player.x*MINIMAP_SCALE,
        player.y*MINIMAP_SCALE,
        (player.x+cos(player.rotationAng)*50)*MINIMAP_SCALE,
        (player.y+sin(player.rotationAng)*50)*MINIMAP_SCALE);

    //swap buffers
    SDL_RenderPresent(renderer);
}

int main() {
    //setup
    gameRunning = windowSetup(&window, &renderer);
    setupGame();

    //game loop
    while (gameRunning) {
        playerInput();
        updateState();
        renderObjects();
    }

    //cleaning
    windowDestruction(&window, &renderer);
    return 0;
}



