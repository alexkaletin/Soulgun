#include "DisplayManager.h"

/**
 * Initializes the display manager
 *
 * @param xRenderer External renderer
 * @param xTexture External texture manager
 */
DisplayManager::DisplayManager(SDL_Renderer *xRenderer, TextureManager *xTexture, MapManager *map) {
    renderer = xRenderer;
    txMan = xTexture;
		renderMap = map;

    maxSpawnCooldown = 1000;
    newSpawnCooldown = maxSpawnCooldown;
    firstSpawn = true;
    srand(time(NULL));
}

/**
 * Deconstructs all entities
 */
DisplayManager::~DisplayManager(void) {
    for (int i = 0; i != entities.size(); ++i) {
        delete entities[i];
    }
    entities.clear();
}

/**
 * Pass in window and what you Position you want window to focus on
 */
void DisplayManager::updateWindowPos(Position window_focus){
	point_of_view.h = 5000;
	point_of_view.w = 5000;
	if(window_focus.x >= 512 && window_focus.x <= MAX_TILES*TILE_WIDTH - 512)
	  	point_of_view.x = 512 - window_focus.x;
    // else if (window_focus.x < 512)
    //     point_of_view.x = 512;
    // else
    //     point_of_view.x = MAX_TILES*TILE_WIDTH - 512;

	if(window_focus.y >= 512 && window_focus.y <= MAX_TILES*TILE_HEIGHT - 512)
		point_of_view.y = 512 - window_focus.y;
    // else if (window_focus.y < 512)
    //     point_of_view.y = 512;
    // else
    //     point_of_view.y = MAX_TILES*TILE_WIDTH - 512;

    SDL_RenderSetViewport(renderer, &point_of_view);
    //SDL_RenderSetClipRect(renderer, &point_of_view);
	
}

/**
 * Adds an entity to the manager
 *
 * @param entity Pointer to an entity
 */
void DisplayManager::addEntity(Humanoid *entity) {
    entities.push_back(entity);
}

/**
 * Removes an entity from the manager
 *
 * @param entity Pointer to an entity that is being managed
 */
void DisplayManager::removeEntity(Humanoid *entity) {
    for (int i = 0; i < entities.size(); ++i) {
        if (entities[i] == entity) {
            delete entity;
            entities.erase(entities.begin() + i);
        }
    }
}

/**
 * Spawns enemies as needed
 */
void DisplayManager::spawnEnemies(MapManager *map) {
    int humans = 0;
    int robots = 0;
    Humanoid *player = NULL;

    for (int i = 0; i < entities.size(); ++i) {
        Humanoid *e = entities[i];
        switch (e->getType()) {
            case ET_HUMAN:
                ++humans;
            break;
            case ET_ROBOT:
                ++robots;
            break;
            case ET_PLAYER:
                player = (e);
            break;
        }
    }
    //keep the number of spawns constrained
    if (humans + robots > 40)
        newSpawnCooldown = 50;
    else if (humans + robots < 2)
    {
        newSpawnCooldown = 0;
        firstSpawn = true;
    }
    //spawn enemies at a generally increasing rate
    if (newSpawnCooldown <= 0)
    {
        if (rand() % 4 > 0)
            spawnHumanoid(map, ET_ROBOT, player);
        else if (!firstSpawn)
            spawnHumanoid(map, ET_HUMAN, player);

        newSpawnCooldown = maxSpawnCooldown;
        maxSpawnCooldown -= 15;
        if (maxSpawnCooldown < 500)
            maxSpawnCooldown = 600;
        firstSpawn = false;
    }
    else
        newSpawnCooldown -= 1;
}

/**
 * Spawns a humanoid entity at an appropriate location considering player location and other enemies
 *
 * @param type Type of humanoid to spawn
 * @param player Pointer to the player
 * @returns A pointer to the humanoid spawned
 */
Humanoid *DisplayManager::spawnHumanoid(MapManager *map, EntityType type, Humanoid *player) {
    // Place player at center of map
    if (type == ET_PLAYER) {

	    player = new Humanoid(5, ET_PLAYER, MAP_WIDTH / 2, MAP_HEIGHT / 2, 2, movePlayer, 50, SS_SINGLESHOT, moveDirection, TX_PLAYER);


        addEntity(player);
        return player;
    }

    Position pos = player->getPosition();

    // Have enemies encircle the player
    float unitCircle = 2 * M_PI;

    //initial values and variables
    double x;
    double y;
    Position newPos;
    double speed;
    double health;
    int shootCooldown = 500; //starting value of the cooldown
    ShootStyle ss;
    moveProjectileFunc projMoveFunc;
    double theta = (rand() % 628)*0.01;

    //pick a random available location around player to spawn at
    x = pos.x + cos(theta) * SPAWN_DIST;
    y = pos.y + sin(theta) * SPAWN_DIST;
    newPos.x = x;
    newPos.y = y;
    while (!(map->mapCollision(newPos)))
    {
    		theta = (rand() % 628)*0.01;
        x = pos.x + cos(theta) * SPAWN_DIST;
        y = pos.y + sin(theta) * SPAWN_DIST;
        newPos.x = x;
        newPos.y = y;
    }

    //generate randomized stats
    speed = (type == static_cast<int>(TX_HUMAN)) ? 0.4: 0.2;
    speed += (rand() % 30)*0.05;
    health = (type == static_cast<int>(TX_HUMAN)) ? rand() % 3 + 2: rand() % 2 + 1;
    ss = static_cast<ShootStyle>(rand()%SS_TOTAL);
    if (ss != SS_SINGLESHOT)
        ss = static_cast<ShootStyle>(rand()%SS_TOTAL);
    if (ss == SS_8WAY || ss == SS_SPIRAL)
        ss = static_cast<ShootStyle>(rand()%SS_TOTAL);
    if (ss == SS_8WAY || ss == SS_SPIRAL)
        shootCooldown += rand()%100 + 50;
        
    //generate randomized shooting styles, projectile movements, and appropriate shooting cooldowns
    switch(rand()%(NUM_OF_PROJ_MOVE_FUNCS+5))
    {
        case 0:
        case 1:
        case 2:
        case 3:
            projMoveFunc = moveDirection;
            shootCooldown -= rand()%200 + 100;
            break;
        case 4:
            projMoveFunc = moveSpiral;
            shootCooldown += rand()%100 - 50;
            break;
        case 5:
        case 6:
            projMoveFunc = moveSine;
            shootCooldown -= rand()%100;
            break;
        case 7:
        case 8:
            projMoveFunc = moveCorkscrew;
            shootCooldown += rand()%100 - 50;
            break;
        case 9:
            projMoveFunc = moveBoomerang;
            shootCooldown += rand()%100 - 50;
            break;
        default:
            projMoveFunc = moveDirection;
            break;
    }
    if (firstSpawn) //make sure the first two are very basic to let the player learn how to play
    {
        ss = SS_SINGLESHOT;
        projMoveFunc = moveDirection;
    }

    if (!isNearEnemy(x, y, 5)) {
        Humanoid *e = new Humanoid(health, type, x, y, speed, movePlayer, shootCooldown, ss, projMoveFunc, static_cast<TextureID>(type));
        addEntity(e);
        return e;
    }
    
    return NULL;
}

/**
 * Enemy movement AI
 *
 * @param player Pointer to the player
 */
void DisplayManager::moveEnemies(MapManager *map, Humanoid *player) {
    Position playerPos = player->getPosition();
    Humanoid *h = NULL;

    for (int i = 0; i < entities.size(); ++i) {
        Humanoid *e = entities[i];
        Movement mov;
        int direction = 0;
        int now = SDL_GetTicks();

        Position enemyPos = e->getPosition();

        // All hail Pythagoras
        int distFromPlayer = static_cast<int>(sqrt(pow(abs(playerPos.x - enemyPos.x), 2) + pow(abs(playerPos.y - enemyPos.y), 2)));

        switch (e->getType()) {
            case ET_HUMAN:
                // Humans moves randomly on diagonals
                h = (e);
                if (now - h->moveStartTime > HUMAN_MOVE_TIME) {
                    h->moveStartTime = now;

                    // If too far away, force to move closer to player
                    if (distFromPlayer > ENEMY_MAX_DIST) {
                        mov.right = (playerPos.x > enemyPos.x);
                        mov.up = (playerPos.y < enemyPos.y);
                    }
                    // Otherwise be random
                    else {
                        // Vertical movement
                        direction = rand() % 2;
                        mov.up = direction;

                        // Horizontal movement
                        direction = rand() % 2;
                        mov.right = direction;
                    }
                    mov.down = !mov.up;
                    mov.left = !mov.right;
                    if (map->mapCollision(h->testMove(mov)))
                        h->move(mov);
                }
                else if (map->mapCollision(h->testMove(mov)))
                {
                    h->move(h->moveDirection);
                }
            break;
            case ET_ROBOT:
                // Robots move rigidly and nonstop
                h = (e);

                if (now - h->moveStartTime > ROBOT_MOVE_TIME) {
                    h->moveStartTime = now;

                    // If too far away, force to move closer to player
                    if (distFromPlayer > ENEMY_MAX_DIST) {
                        mov.right = (playerPos.x > enemyPos.x);
                        mov.up = (playerPos.y < enemyPos.y);
                    }
                    // Otherwise be random
                    else {
                        // Vertical movement
                        direction = rand() % 2;
                        mov.up = direction;

                        // Horizontal movement
                        direction = rand() % 2;
                        mov.right = direction;
                    }
                    mov.down = !mov.up;
                    mov.left = !mov.right;

                    // Enforce 90-degree movement
                    if (rand() % 2 == 1) {
                        // Disable vertical
                        mov.up = false;
                        mov.down = false;
                    }
                    else {
                        // Disable horizontal
                        mov.left = false;
                        mov.right = false;
                    }
                    if (map->mapCollision(h->testMove(mov)))
                        h->move(mov);
                }
                else if (map->mapCollision(h->testMove(mov))) {
                    h->move(h->moveDirection);
                }
            break;
        }
    }
}

/**
 * Indicates whether an enemy is located near a coordinate
 *
 * @param x X coorindate
 * @param y Y coordinate
 * @param proximity Distance from the coordinates to be considered "near"
 * @returns True if an enemy is located near this coordinate
 */
bool DisplayManager::isNearEnemy(int x, int y, int proximity) {
    for (int i = 0; i < entities.size(); ++i) {
        Entity *e = entities[i];
        Position pos = e->getPosition();

        // Todo: Do some pythagorean theorem magic to incorporate proximity
        if (pos.x == x && pos.y == y)
            return true;
    }

    return false;
}


//add a projectile to display manager
void DisplayManager::addProjectile(Projectile *proj) {
    projectiles.push_back(proj);
}
//remove a projectile from display manager
void DisplayManager::removeProjectile(Projectile *proj) {
    for (int i = 0; i < projectiles.size(); ++i) {
        if (projectiles[i] == proj) {
            delete proj;
            projectiles.erase(projectiles.begin() + i);
        }
    }
}
//call the shoot function for all enemies and add returned projectiles to the display manager projectiles list
void DisplayManager::fireEnemies(Humanoid *player)
{
    Position playerPos = player->getPosition();
    int posx = playerPos.x;
    int posy = playerPos.y;
    Humanoid *e = nullptr;
    std::vector<Projectile*> p;
    for (int i = 0; i < entities.size(); ++i)
    {
        e = entities[i];
        if (e->getType() != ET_PLAYER)
            p = e->shoot(posx, posy, false);
        for (int i = 0; i < p.size(); ++i)
            addProjectile(p[i]);
    }
}
//move all projectiles using their propper projectileMove functions
void DisplayManager::moveProjectiles(Humanoid *player) {
    Position playerPos = player->getPosition();
    Projectile *p = NULL;
    Movement mov;
    int direction = 0;
    int now = SDL_GetTicks();
    Position enemyPos;
    Position projPos;
    double thetaAim;
    for (int i = 0; i < projectiles.size(); ++i) {
        p = projectiles[i];
        projPos = p->getPosition();
        thetaAim = convertCoordsToRads(projPos.x, projPos.y, playerPos.x, playerPos.y);
 		if (!(p->isSoulBullet()) && player->entityCollision(p->getHitbox()))
        {
            player->damage(p->getPower());
			removeProjectile(p);
        }
        //check if bullet is from player
        else if (p->isSoulBullet())
        {
            for (int i = 0; i < entities.size(); ++i)
            {
                if (entities[i]->getType() != ET_PLAYER)
                {
                    if ((entities[i])->entityCollision(p->getHitbox()))
                    {
                        //if bullet hit another entity(humanoid)
                        if(swapSpots(entities[i])){
                            entities.erase(entities.begin() + i);
                        }//bullet didnt hit a humanoid
                        else if (entities[i]->damage(p->getPower()))
                            removeEntity(entities[i]);
                        removeProjectile(p);
                    }
                }
                else if (p->move(thetaAim) || !renderMap->mapCollision(p->getPosition()))
                    removeProjectile(p);
            }
        }
      	else if (p->move(thetaAim) || !renderMap->mapCollision(p->getPosition()))
            removeProjectile(p);
    }
}


/**
 * Draws textures on the window where they are currently located
 */
void DisplayManager::refresh(void) {
    // Put textures on screen
    SDL_Rect position;
    SDL_Point size;
    SDL_Texture *texture;
    Humanoid *e;
    Projectile *p;

    // Map rendering
		renderMap->mapDrawer(renderer);

    for (int i = 0; i < entities.size(); ++i) {
        e = entities[i];

        texture = txMan->getTexture(e->getImage());
        size = txMan->getDimensions(e->getImage());
        position.h = size.y;
        position.w = size.x;

        // position.x  = e->x - map->offset_x;
        // position.y  = e->y - map->offset_y;
        Position pos = e->getPosition();
        position.x  = pos.x;
        position.y  = pos.y;

        SDL_RenderCopy(renderer, texture, NULL, &position);
    }
    for (int i = 0; i < projectiles.size(); ++i) {
        p = projectiles[i];

        texture = txMan->getTexture((p->getImage()));
        size = txMan->getDimensions((p->getImage()));
        position.h = size.y;
        position.w = size.x;

        // position.x  = e->x - map->offset_x;
        // position.y  = e->y - map->offset_y;
        Position pos = p->getPosition();
        position.x  = pos.x;
        position.y  = pos.y;

        SDL_RenderCopy(renderer, texture, NULL, &position);
    }
}

void DisplayManager::flashBox(int startx, int starty, int Width, int Height){
    //select the color you want to render (red, green, blue, alpha)
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);

    //creates box
    SDL_Rect box;
    box.x = startx;
    box.y = starty;
    box.w = Width;
    box.h = Height;

    //adds box and displays
    SDL_RenderFillRect(renderer, &box);
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
}

void DisplayManager::flashScreen(){
    //select the color you want to render (red, green, blue, alpha)
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);

    //creates box
    SDL_Rect box;
    box.x = 0;
    box.y = 0;
    box.w = MAP_WIDTH;
    box.h = MAP_HEIGHT;

    //adds box and displays
    SDL_RenderFillRect(renderer, &box);
    SDL_RenderPresent(renderer);
    //SDL_RenderClear(renderer);
}

bool DisplayManager::swapSpots(Humanoid *toSwap){
    //double checks that the first item is the player
    if (entities[0]->getType() == ET_PLAYER){
        if(toSwap->getType() == ET_HUMAN){
            Position newPos = toSwap->getPosition();
            entities[0]->setLocation(newPos);
            entities[0]->setHitboxPos(newPos);
            entities[0]->setProjMoveFunc(toSwap->getProjMoveFunc());
            entities[0]->setShootStyle(toSwap->getShootStyle());
            flashScreen();
            flashBox(newPos.x-5, newPos.y-5, newPos.x+5, newPos.y+5);
            return true;
        }
    }
    return false;
}
