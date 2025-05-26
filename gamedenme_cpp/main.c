#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#define MAX_BALL 40

const int screenWidth = 1500;
const int screenHeight = 800;

//enumeration for game screens
typedef enum {TITLE,GAMEPLAY,GAMEOVER}GameScreen;

//define proporties of a target
typedef struct target {
	float x;
	float y;
	float radius;
	Color color;
	bool active; 
	bool moving;
}target;

//node for linked list
typedef struct node {
	target* data;
	struct node* next;
	struct node* previous;
}node;


typedef struct {
	Vector2 ballPos;
	Vector2 ballSpeed;
	float radius;
	Color color;
	bool isFired;
	bool active;
}bullet;

//global variables for game state
GameScreen currentScreen = TITLE;
int maxball = MAX_BALL;
node* head = NULL;
target hedef[999] = { 0 };
bullet mermi = { 0 };
Vector2 mouse = { 0 };
double aimingAngle = 0;
Rectangle health = { 20,20,120,15 };
int healthCounter = 0;
Vector2 hold = {0};
node* holdBallNext = NULL;
node* holdBallPre = NULL;
int activeCounter = 0;
int totalActive = MAX_BALL;
int score = 0;
FILE* fptr = NULL;
bool gameStarted = false;

//functions
void initGame();
void initGame2();
Color giveColor();
void targetCreator(node**, target*);
Color giveColorBullet(node*);
bool isSameColor(Color color1, Color color2);
void bulletFire();
target* createOne(bullet);
node* addTargetBetween(target* newCreated, target* shotTargetIndex);
void stepBack(node*, node*);
void isBoom();
void DrawTargets(node*);

int checkCollision(node*, bullet*);
target* shotTargetIndex(node**, bullet*);
int whereTarget(node*);
void updateTarget(node**);
void updateGame();
int highScore(int);
void freeTargets(node*);

bool gameOver = false;
bool isGameOver(node* head);


//Texture for the game
Texture2D kurbaga; 
Texture2D ending;
Texture2D background;
Texture2D redball;
Texture2D blueball;
Texture2D greenball;
Texture2D yellowball;
Texture2D purpleball;
Texture2D blackball;
Texture2D gameover;
Texture2D marble;
Texture2D mainmenu;
Texture2D play;
Texture2D retry;
Music music;
Sound effect;

int titleToGameplayDelayCounter = 0;

Vector2 texturePosition = { 620,470 };

int main(void) {

	InitWindow(screenWidth, screenHeight, "Marble Puzzle Shoot");
	SetTargetFPS(120);
	initGame();
	InitAudioDevice();

	//load textures and sounds
	kurbaga = LoadTexture("image/crocodile2.png");
	background = LoadTexture("image/arkaplan3.png");
	redball = LoadTexture("image/papagan.png");
	blueball = LoadTexture("image/kaplan.png");
	greenball = LoadTexture("image/baykus.png");
	yellowball = LoadTexture("image/panda.png");
	purpleball = LoadTexture("image/fil.png");
	blackball = LoadTexture("image/blackball.png");
	ending = LoadTexture("image/ending1.png");
	gameover = LoadTexture("image/gameover2.png");
	marble = LoadTexture("image/marble5.png");
	mainmenu = LoadTexture("image/arkaplan1.png");
	play = LoadTexture("image/play1.png");
	retry = LoadTexture("image/retry.png");
	music = LoadMusicStream("sounds/shanty-town.wav");
	effect = LoadSound("sounds/collision.mp3");

	bool musicPlaying = false;

	Vector2 textureCenter = { kurbaga.width / 2.0f+10 ,kurbaga.height / 2.0f-48 }; // position


	Rectangle sourceRec = { 0,0, play.width, play.height }; //play button position
	Rectangle pressBounds = { 630,580, play.width, play.height };

	Rectangle retrySourceRec = { 0,0, retry.width, retry.height }; //retry button position
	Rectangle retryPressBound = { 700,400, retry.width, retry.height };

	while (!WindowShouldClose()) {
	
		UpdateMusicStream(music);

	//game logic and rendering
		ClearBackground(LIGHTGRAY);

		mouse = GetMousePosition();
		//crocodile rotation
		float deltaX = mouse.x - texturePosition.x;
		float deltaY = mouse.y - texturePosition.y;
		float angle = (atan2f(deltaY, deltaX) * (180.0f / PI)) + 90;

		//screen transition
		switch (healthCounter) {
		case 0: case 1: case 2:
			if (currentScreen == TITLE && CheckCollisionPointRec(mouse, pressBounds) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
				currentScreen = GAMEPLAY;
				gameStarted = true;
			}
			break;

			//health counter>=3
		default:
			if (totalActive == 0) currentScreen = GAMEOVER;
			break;
		}

		//chat deneme game over
		if (!gameOver && isGameOver(head)) {
			gameOver = true;
			currentScreen = GAMEOVER;
		}

		//returning variables to their initial values
		if (currentScreen == GAMEOVER) {
			gameStarted = false;
			healthCounter = 0;

			health.width = 120;
			maxball = MAX_BALL;
			titleToGameplayDelayCounter = 0;
		}

		//retry  funct
		if (currentScreen == GAMEOVER && CheckCollisionPointRec(mouse, retryPressBound) && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && totalActive == 0) {
			SetTargetFPS(120);
			currentScreen = GAMEPLAY;
			gameStarted = true;
			score = 0;
			titleToGameplayDelayCounter=0;
			gameOver = false;
			if (titleToGameplayDelayCounter > GetFPS() / 10) {
				updateGame();
				updateTarget(&head);
			}
		}
	
		//opening gameplay screen with minor delay
		if (currentScreen == GAMEPLAY) {
			titleToGameplayDelayCounter++;
			if (titleToGameplayDelayCounter > GetFPS() / 10) {
				updateGame();
				updateTarget(&head);
			}
			// Müziði sadece bir kere baþlat
			if (!musicPlaying) {
				PlayMusicStream(music);
				musicPlaying = true;
			}
		}
		else {
			// GAMEPLAY ekranýnda deðilsek müziði durdur
			if (musicPlaying) {
				StopMusicStream(music);
				musicPlaying = false;
			}
		
			mermi.active = false;
			mermi.isFired = false;
		}

		
		if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) bulletFire();
		if (currentScreen == GAMEPLAY && !gameOver && head != NULL) {
			if (checkCollision(head, &mermi)) {
				createOne(mermi);
				stepBack(head, addTargetBetween(createOne(mermi), shotTargetIndex(&head, &mermi)));
				isBoom();
			}
		}
		//draw based on current game screen / mevcut oyun ekranýna göre çizim
		BeginDrawing();
		switch (currentScreen) {
		case TITLE:
			DrawTexture(mainmenu, 0, 0, WHITE);
			DrawTextureRec(play, sourceRec, (Vector2) { pressBounds.x, pressBounds.y }, WHITE);
			DrawTexture(marble, 580, 0, WHITE);
			break;

		case GAMEPLAY:
			DrawTexture(background, 0, 0, WHITE);
			DrawTargets(head);

			//kurbaða resminin dönüþü
			DrawTexturePro(kurbaga, (Rectangle) { 0, 0, kurbaga.width, kurbaga.height }, 
				(Rectangle) {texturePosition.x+20,texturePosition.y+18,kurbaga.width,kurbaga.height},textureCenter,angle,WHITE);

			DrawTexture(ending, 1000, screenHeight / 2 -85 , WHITE);
			DrawRectangleRec(health, RED);
			DrawText("HEALTH", 45, 20, 18, LIGHTGRAY);
			DrawText(TextFormat("%d", score), 1350, 10, 50, WHITE);

			if (mermi.active == true) {
				if (isSameColor(mermi.color, RED)) {
					DrawTexture(redball, mermi.ballPos.x, mermi.ballPos.y, WHITE);
				}
				if (isSameColor(mermi.color, BLUE)) {
					DrawTexture(blueball, mermi.ballPos.x, mermi.ballPos.y, WHITE);
				}
				if (isSameColor(mermi.color, GREEN)) {
					DrawTexture(greenball, mermi.ballPos.x, mermi.ballPos.y, WHITE);
				}
				if (isSameColor(mermi.color, YELLOW)) {
					DrawTexture(yellowball, mermi.ballPos.x, mermi.ballPos.y, WHITE);
				}
				if (isSameColor(mermi.color, PURPLE)) {
					DrawTexture(purpleball, mermi.ballPos.x, mermi.ballPos.y, WHITE);
				}
				if (isSameColor(mermi.color, BLACK)) {
					DrawTexture(blackball, mermi.ballPos.x, mermi.ballPos.y, WHITE);
				}
			}
			break;
		case GAMEOVER:
			DrawTexture(gameover, 0, 0, WHITE);
			DrawText(TextFormat("%d", score),200,500,100,WHITE);
			DrawText(TextFormat("%d", highScore(score)), 1100, 500, 100, WHITE);
			DrawTextureRec(retry, retrySourceRec, (Vector2) { retryPressBound.x, retryPressBound.y }, WHITE);
			break;
		}
		EndDrawing();
	}
	//temizleme ve kapatma
	freeTargets(head);
	UnloadMusicStream(music);
	CloseAudioDevice();
	CloseWindow();
	return 0;

	//oyun varlýklarýný baþlat
	void initGame();
	int sonuncuNum = maxball;

	for (int num = 1; num <= maxball; num++) {
	//görünmez hedefler
		if (num <= 3) {
			hedef[num].x = 80;
			hedef[num].y = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = (Color){ 255,255,255,0 };
			hedef[num].active = false;
			hedef[num].moving = true;
		}
		//görünür hedefler
		else if (sonuncuNum - 3 >= num) {
			hedef[num].x = 80;
			hedef[num].y = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = (Color){ 255,255,255,0 };
			hedef[num].active = false;
			hedef[num].moving = true;
		}
		//görünmez hedefler
		else {
			hedef[num].x = 80;
			hedef[num].y = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = giveColor();
			hedef[num].active = true;
			hedef[num].moving = true;
		}
		targetCreator(&head, &hedef[num]);
	}
	//merminin ilk deðerleri
	mermi.ballPos = texturePosition;
	mermi.ballSpeed = (Vector2){ 0,0 };
	mermi.radius = 20.0;
	mermi.color = giveColorBullet(head);
	mermi.isFired = false;
	mermi.active=true;
}

void initGame() {

	int sonuncuNum = maxball;

	for (int num = 1; num <= maxball; num++) {
	    //invisible
		if (num <= 3) {
			hedef[num].x = 80;
			hedef[num].y = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = (Color){ 255,255,255,0 };
			hedef[num].active = false;
			hedef[num].moving = true;
		}
		//visible
		else if (sonuncuNum - 3 <= num) {
			hedef[num].x = 80;
			hedef[num].y = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = (Color){ 255,255,255,0 };
			hedef[num].active = false;
			hedef[num].moving = true;
		}
		//invisible
		else {
			hedef[num].x = 80;
			hedef[num].y = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = giveColor();
			hedef[num].active = true;
			hedef[num].moving = true;
		}
		targetCreator(&head, &hedef[num]);
	}

	//mermi ilk deðerler
	mermi.ballPos = texturePosition;
	mermi.ballSpeed = (Vector2){ 0,0 };
	mermi.radius = 20, 0;
	mermi.color = giveColorBullet(head);
	mermi.isFired = false;
	mermi.active = true;
}

//ek oyun baþlatma mantýðý
void initGame2() {
	int sonuncuNum = maxball;

	for (int num = 1; num <= maxball; num++) {
		if (num <= 3) {
			hedef[num].x = 80;
			hedef[num].y = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = (Color){ 255,255,255,0 };
			hedef[num].active = false;
			hedef[num].moving = true;
		}
		else if (sonuncuNum - 3 <= num) {
			hedef[num].x = 80;
			hedef[num].y = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = (Color){ 255,255,255,0 };
			hedef[num].active = false;
			hedef[num].moving = true;
		}
		else {
			hedef[num].x = 80;
			hedef[num].y = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = giveColor();
			hedef[num].active = true;
			hedef[num].moving = true;
		}
		targetCreator(&head, &hedef[num]);
	}

	mermi.ballPos = (Vector2){ 100,100 };
	mermi.ballSpeed = (Vector2){ 0,0 };
	mermi.radius = 20.0;
	mermi.color = giveColorBullet(head);
	mermi.isFired = false;
	mermi.active = true;
}

/*Color giveColor() {
	int random;
	if (45 > maxball && maxball >= 30) random = GetRandomValue(1, 1);
	else if (65 > maxball && maxball >= 45) random = GetRandomValue(1, 1);
	else if (maxball >= 65) random = GetRandomValue(1, 6);

	random = GetRandomValue(1, 3);

	if (random == 2) return RED;
	else if (random == 3) return BLUE;
	else if (random == 4) return YELLOW;
	else if (random == 5) return PURPLE;
	else if (random == 6) return BLACK;
	else return GREEN;
}*/ 

Color giveColor() {
	int random;
	if (45 > maxball && maxball >= 30) random = GetRandomValue(1, 4);
	else if (65 > maxball && maxball >= 45) random = GetRandomValue(1,5);
	else if (maxball >= 65) random = GetRandomValue(1, 6);  // Bu satýr artýk iþe yarayacak
	else random = GetRandomValue(1,3);
	


	if (random == 2) return RED;
	else if (random == 3) return BLUE;
	else if (random == 4) return YELLOW;
	else if (random == 5) return PURPLE;
	else if (random == 6) return BLACK;
	else return GREEN;
}

void targetCreator(node**head, target*hedef) {
	node* new_node = (node*)malloc(sizeof(node));
	if (new_node == NULL) {
		return;
	}
	new_node->data = (target*)malloc(sizeof(target));
	if (new_node->data == NULL) {
		free(new_node);
		return;
	}

	new_node->data->x = hedef->x;
	new_node->data->y = hedef->y;
	new_node->data->radius = hedef->radius;
	new_node->data->color = hedef->color;
	new_node->data->active = hedef->active;
	new_node->data->moving = hedef->moving;
	new_node->next = NULL;
	new_node->previous = NULL;
	
	//linked list
	if (*head == NULL) {
		*head = new_node;
	}
	else {
		node* current = *head;
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = new_node;
		new_node->previous = current;
	}
}

//get a color for new bullets based on existing targets
/*Color giveColorBullet(node* head) {
	node* current = head;
	int random;
	if (45 > maxball && maxball >= 30) random = GetRandomValue(1, 4);
	else if (65 > maxball && maxball >= 45) random = GetRandomValue(1, 5);
	else if (maxball >= 65) random = GetRandomValue(1, 6);
	else random = GetRandomValue(1, 3);

	current = head;
	while (current->next != NULL) {
		if (random == 1 && isSameColor(current->data->color, RED) && (current->data->active == true)) {
			return RED;
		}
		else if (random == 2 && isSameColor(current->data->color, BLUE) && (current->data->active == true)) {
			return BLUE;
		}
		else if (random == 3 && isSameColor(current->data->color, GREEN) && (current->data->active == true)) {
			return GREEN;
		}
		else if (random == 4 && isSameColor(current->data->color, YELLOW) && (current->data->active == true)) {
			return YELLOW;
		}
		else if (random == 5 && isSameColor(current->data->color, PURPLE) && (current->data->active == true)) {
			return PURPLE;
		}
		else if (random == 6 && isSameColor(current->data->color, BLACK) && (current->data->active == true)) {
			return BLACK;
		}
		current = current->next;
	}
	return giveColorBullet(head);
}*/

Color giveColorBullet(node* head) {
	Color availableColors[6];
	int count = 0;
	node* current = head;

	while (current != NULL) {
		if (current->data->active) {
			Color c = current->data->color;
			bool alreadyExists = false;
			for (int i = 0; i < count; i++) {
				if (isSameColor(availableColors[i], c)) {
					alreadyExists = true;
				}
			}
			if (!alreadyExists) {
				availableColors[count++] = c;
			}
		}
		current = current->next;
	}

	if (count > 0) {
		int index = GetRandomValue(0, count - 1);
		return availableColors[index];
	}

	// Eðer aktif top yoksa varsayýlan olarak RED dön (veya baþka bir iþlem yap)
	return RED;
}


//check if two colors are the same
bool isSameColor(Color color1, Color color2) {
	return (color1.r == color2.r && color1.g == color2.g && color1.b == color2.b && color1.a == color2.a);
}

void bulletFire() {
	
		if (!mermi.isFired && mermi.active) {
			mermi.ballPos =texturePosition;
			mouse = GetMousePosition();

			// Yön vektörünü hesapla
			float deltaX = mouse.x - mermi.ballPos.x;
			float deltaY = mouse.y - mermi.ballPos.y;

			// Açý hesapla (atan2f sayesinde tüm çeyrekler düzgün çalýþýr)
			aimingAngle = atan2f(deltaY, deltaX);

			mermi.isFired = true;
		}
		/*if (mermi.isFired == false && mermi.active == true) {
		mermi.ballPos = (Vector2){ screenWidth / 2, screenHeight / 2 };
		mouse = GetMousePosition();

		if (mouse.x > screenWidth / 2 && mouse.y < screenHeight / 2) {
			aimingAngle = atan(-(mermi.ballPos.y - mouse.y) / (mouse.x - mermi.ballPos.x));
		}
		if (mouse.x < screenWidth / 2 && mouse.y < screenHeight / 2) {
			aimingAngle = PI- atan(-(mermi.ballPos.y - mouse.y) / (mouse.x - mermi.ballPos.x));
		}
		if (mouse.x < screenWidth / 2 && mouse.y > screenHeight / 2) {
			aimingAngle = PI- atan((mermi.ballPos.y - mouse.y) / (mouse.x - mermi.ballPos.x));
		}
		if (mouse.x > screenWidth / 2 && mouse.y > screenHeight / 2) {
			aimingAngle = (2*PI)- atan((mermi.ballPos.y - mouse.y) / (mouse.x - mermi.ballPos.x));
		}
		mermi.isFired = true;
	}*/

}

target* createOne(bullet mermi) {

	//memory allocation to create a new target ball
	target* newCreated = (target*)malloc(sizeof(target));
	if (newCreated != NULL) {
		node* current = head;

		/*while (current->previous != NULL && current->previous->data != shotTargetIndex) {
			current = current->previous;
		}*/

		target* hedef = shotTargetIndex(&head, &mermi);

		while (current->previous != NULL && current->previous->data != hedef) {
			current = current->previous;
		}


		switch (whereTarget(current)) {
		case 1: //down
			newCreated->x = current->data->x;
			newCreated->y = current->data->y-40;
			break;
		case 2: //up
			newCreated->x = current->data->x;
			newCreated->y = current->data->y + 40;
			break;
		case 3: //right
			newCreated->x = current->data->x-40;
			newCreated->y = current->data->y;
			break;
		case 4: //left
			newCreated->x = current->data->x+40;
			newCreated->y = current->data->y;
			break;
		}

		newCreated->radius = 20;
		//yeni hedef toplarýn rengi mermi rengiyle ayný olmalý
		newCreated->color = mermi.color;
		newCreated->active = true;
		newCreated->moving = true;
	}
	return newCreated;
}

void stepBack(node* head, node* newCreated) {
	node* current = head;

	while (current->next != NULL) {
		current = current->next;
	}
	//yeni ekleneni bul
	while (current->previous != NULL && current->previous != newCreated) {
		current = current->previous;
	}
	//yeni eklenen topun bir öncesine git
	current = current->previous;

	//vurulan ve önceki toplarý kaydýrmak
	while (current != NULL) {
		int hamle = 40;
		switch (whereTarget(current)){
		case 1: //down
			if (current->data->x == 80) {
				while (current->data->y < screenHeight - 80 && hamle>0) {
					current->data->y += 1;
					hamle -= 1;
				}
				while (current->data->y == screenHeight - 80 && hamle > 0) {
					current->data->x += 1;
					hamle -= 1;
				}
			}
			else if (current->data->x == 300) {
				while (current->data->y < screenHeight - 160 && hamle>0) {
					current->data->y += 1;
					hamle -= 1;
				}
				while (current->data->y == screenHeight - 160 && hamle > 0) {
					current->data->x += 1;
					hamle -= 1;
				}
			}
			else if (current->data->x == 550) {
				while (current->data->y < screenHeight / 2 && hamle>0) {
					current->data->y += 1;
					hamle -= 1;
				}
				while (current->data->y >= screenHeight / 2 && hamle > 0) {
					current->data->y = screenHeight / 2;
					hamle -= 1;
				}
			}
			break;
		case 2: //up
			if (current->data->x == screenWidth - 80) {
				while (current->data->y > 80 && hamle > 0) {
					current->data->y -= 1;
					hamle -= 1;
				}
				while (current->data->y == 80 && hamle > 0) {
					current->data->x -= 1;
					hamle -= 1;
				}
			}
			else if (current->data->x == screenWidth - 300) {
				while (current->data->y > 160 && hamle > 0) {
					current->data->y -= 1;
					hamle -= 1;
				}
				while (current->data->y == 160 && hamle > 0) {
					current->data->x -= 1;
					hamle -= 1;
				}
			}
			break;
		case 3: //right
			if (current->data->y == screenHeight - 80) {

				while (current->data->x < screenWidth - 80 && hamle > 0) {
					current->data->x += 1;
					hamle -= 1;
				}
				while (current->data->x == screenWidth - 80 && hamle > 0) {
					current->data->y -= 1;
					hamle -= 1;
				}

			}
			else if (current->data->y == screenHeight - 160) {

				while (current->data->x < screenWidth - 300 && hamle > 0) {
					current->data->x += 1;
					hamle -= 1;
				}
				while (current->data->x == screenWidth - 300 && hamle > 0) {
					current->data->y -= 1;
					hamle -= 1;
				}

			}
			break;

		case 4: // left
			if (current->data->y == 80) {

				while (current->data->x > 300 && hamle > 0) {
					current->data->x -= 1;
					hamle -= 1;
				}
				while (current->data->x == 300 && hamle > 0) {
					current->data->y += 1;
					hamle -= 1;
				}

			}
			else if (current->data->y == 160) {

				while (current->data->x > 550 && hamle > 0) {
					current->data->x -= 1;
					hamle -= 1;
				}
				while (current->data->x == 550 && hamle > 0) {
					current->data->y += 1;
					hamle -= 1;
				}

			}
			break;
		}
		current = current->previous;
	}
	return;
}

//patlama ve zincirleme reaksiyon mantýðý
void isBoom() {
	node* vurulan = head;
	node* eklenen = NULL;

	/*while (vurulan->next != NULL && vurulan->data != shotTargetIndex(&head, &mermi)) {
		vurulan = vurulan->next;
	}*/

	target* hedef = shotTargetIndex(&head, &mermi);

	while (vurulan != NULL && vurulan->data != hedef) {
		vurulan = vurulan->next;
	}


	if (vurulan == NULL || vurulan->next == NULL || vurulan->next->previous == NULL) return;
	
	eklenen = vurulan->next->previous;
	eklenen = eklenen->previous;

	if ((isSameColor(eklenen->data->color, mermi.color) && isSameColor(eklenen->data->color, eklenen->next->data->color) && isSameColor(eklenen->data->color, eklenen->previous->data->color))
		|| (isSameColor(eklenen->data->color, mermi.color) && isSameColor(eklenen->data->color, eklenen->next->data->color) && isSameColor(eklenen->next->data->color, eklenen->next->next->data->color))
		|| (isSameColor(eklenen->data->color, mermi.color)&& isSameColor(eklenen->data->color, eklenen->previous->data->color) && isSameColor(eklenen->previous->data->color, eklenen->previous->previous->data->color))){

		eklenen->data->active = false; //ekleneni yok et
		score += 10;

		hold = (Vector2){ eklenen->previous->data->x, eklenen->previous->data->y };
		holdBallNext = eklenen->next;
		holdBallPre = eklenen->previous;

		node* current = eklenen;
		while (current->data->active == false && isSameColor(current->next->data->color, current->data->color)) {//ayrýca eklenenin arkasýndakileri de yok et
			holdBallNext = current->next;
			current->next->data->active = false;
			score += 10;
			current = current->next;
		}
		holdBallNext = current->next;
		PlaySound(effect);
		current = eklenen;

		while (current->data->active == false && isSameColor(current->previous->data->color, current->data->color)) {//eklenenin önündekileri de yok et
			hold = (Vector2){ current->previous->data->x,current->previous->data->y };
			holdBallPre = current->previous;
			current->previous->data->active = false;
			score += 10;
			current = current->previous;
		}
		holdBallPre = current->previous;

		while (!(holdBallNext->data->x == hold.x && holdBallNext->data->y == hold.y)) {
			while (current->previous != NULL) { //öndekileri durdur
				current->previous->data->moving = false;
				current = current->previous;
			}
			updateTarget(&head);
		}
		current = head;
		while (current->next != NULL) {
			current->data->moving = true;
			current = current->next;
		}

		//linked listleri düzenleme
		holdBallPre->next = holdBallNext;
		holdBallNext->previous = holdBallPre;

	}
}

//çarpýþma tespiti
int checkCollision(node* head, bullet* mermi) {
	node* current = head;

	while (current->next != NULL) {
		Vector2 hedefCenter = { current->data->x,current->data->y };
		Vector2 mermiCenter = { mermi->ballPos.x,mermi->ballPos.y };

		//merminin görünmez hedef toplara çarpmasýný engellemek
		if (current->data->active == true && CheckCollisionCircles(hedefCenter, 20, mermiCenter, 20)) {
			mermi->active = false;
			mermi->isFired = false;
			return 1;
		}
		current = current->next;
	}
	return 0;
}

//atýlan hedef topun verilerini verir
target* shotTargetIndex(node** head, bullet* mermi) {
	node* current = *head;

	while (current->next != NULL) {
		Vector2 hedefCenter = { current->data->x, current->data->y };
		Vector2 mermiCenter = { mermi->ballPos.x, mermi->ballPos.y };

		if (current->data->active == true && CheckCollisionCircles(hedefCenter, 20, mermiCenter, 20)) {
			return current->data;
		}
		current = current->next;
	}
	return NULL;
}

//seçilen hedefin konumunu verir
int whereTarget(node* given) {
	node* current = given;

	target* selected = current->data;

	if ((selected->x == 80) && (selected->y < screenHeight - 80)) return 1; //going down
	if ((selected->y == screenHeight - 80) && (selected->x < screenWidth - 80)) return 3; //going right
	if ((selected->x == screenWidth - 80) && (selected->y > 80)) return 2; //going up
	if ((selected->y == 80) && (selected->x > 300)) return 4; //going left
	if ((selected->x == 300) && (selected->y < screenHeight - 160)) return 1; 

	if ((selected->y == screenHeight - 160) && (selected->x < screenWidth - 300) && (selected->x != 80)) return 3;
	if ((selected->x == screenWidth - 300) && (selected->y > 160) && (selected->y != screenHeight - 80)) return 2;
	if ((selected->y == 160) && (selected->x > 550) && (selected->x != screenWidth - 80)) return 4;
	if ((selected->x == 550) && (selected->y < screenHeight / 2) && (selected->y != 80)) return  1;

	else return 0;
}

//arada bir node oluþtur
node* addTargetBetween(target* newCreated, target* shotTargetIndex) {
	node* current = head;
	node* shotted = NULL;

	//yeni node için malloc
	node* new = (node*)malloc(sizeof(node));
	if (new == NULL) return NULL;

	while (current->next != NULL) {
		current = current->next;
	}

	//atýlan topu bul
	while (current->previous != NULL && current->previous->data != shotTargetIndex) {
		current = current->previous;
	}

	//linked liste yeni nod ekleme
	shotted = current->previous;

	new->data = newCreated;

	new->next = current;
	new->previous = shotted;
	current->previous = new;
	shotted->next = new;

	return new;
}

//hedefler için rota tanýmlama
void updateTarget(node** head) {
	node* current = *head;

	while (current->next != NULL) {
		target* selected = current->data;
		if (selected->moving == true) {

			if ((selected->x == 80) && (selected->y < screenHeight - 80))selected->y++;
			if ((selected->y == screenHeight - 80) && (selected->x < screenWidth - 80)) selected->x++;
			if ((selected->x == screenWidth - 80) && (selected->y > 80))selected->y--;
			if ((selected->y == 80) && (selected->x > 300)) current->data->x--;
			if ((selected->x == 300) && (selected->y < screenHeight - 160)) current->data->y++;

			if ((selected->y == screenHeight - 160) && (selected->x < screenWidth - 300) && (selected->x != 80)) selected->x++;
			if ((selected->x == screenWidth - 300) && (selected->y > 160) && (selected->y != screenHeight - 80)) selected->y--;
			if ((selected->y == 160) && (selected->x > 550) && (selected->x != screenWidth - 80)) selected->x--;
			if ((selected->x == 550) && (selected->y < screenHeight / 2) && (selected->y != 80)) selected->y++;

			//deactiveting the ball which has reached to the finishing position 
			if ((selected->x == 550) && (selected->y == screenHeight / 2)) selected->active = false;

			//health 
			if ((selected->x == 550) && (selected->y == screenHeight / 2 - 10) && (selected->moving == true) && (selected->active == true)) {
				healthCounter++;
			}

			if ((health.width >= 80) && (healthCounter == 1)) health.width--;
			if ((health.width >= 40) && (healthCounter == 2)) health.width--;
			if ((health.width >= 0) && (healthCounter == 3)) health.width--;

		}
		current = current->next;
	}
}

//yüksek puaný bul
int highScore(int score) {
	int oldScore = 0;
	fptr = fopen("highscore.txt", "r");
	fscanf(fptr, "%d", &oldScore);
	fclose(fptr);

	if (score > oldScore) {
		fptr = fopen("highscore.txt", "w");
		fprintf(fptr, "%d", score);
		fclose(fptr);
		return score;
	}
	return oldScore;
}

void freeTargets(node* head) {
	node* current = head;

	while (current != NULL) {
		node* next = current->next;
		free(current->data);
		free(current);
		current = next;
	}
}

void DrawTargets(node* head) {
	node* current = head;
	while(current->next != NULL) {
		if (current->data->active == true) {
			if (isSameColor(current->data->color, RED)) {
				DrawTexture(redball, current->data->x, current->data->y, WHITE);
			}
			if (isSameColor(current->data->color, BLUE)) {
				DrawTexture(blueball, current->data->x, current->data->y, WHITE);
			}
			if (isSameColor(current->data->color, GREEN)) {
				DrawTexture(greenball, current->data->x, current->data->y, WHITE);
			}
			if (isSameColor(current->data->color, YELLOW)) {
				DrawTexture(yellowball, current->data->x, current->data->y, WHITE);
			}
			if (isSameColor(current->data->color, PURPLE)) {
				DrawTexture(purpleball, current->data->x, current->data->y, WHITE);
			}
			if (isSameColor(current->data->color, BLACK)) {
				DrawTexture(blackball, current->data->x, current->data->y, WHITE);
			}
		}
		current = current->next;
	}
}

//hedefleri hareket ettirmek ve oyun üzerindeki koþullarý kontrol etmek gibi oyun mantýðýný güncelleyin
void updateGame(){

	activeCounter = 0;

	node* current = head;
	while (current->next != NULL) {
		if (current->data->active == true) activeCounter++;
		current = current->next;
	}
	totalActive = activeCounter;
	if (totalActive == 0 && healthCounter < 3) {
		maxball += 5;
		initGame2();
		SetTargetFPS(120);
	}
	else activeCounter = 0;

	mermi.ballPos.x += mermi.ballSpeed.x;
	mermi.ballPos.y += mermi.ballSpeed.y;

	//mermi hareketini baþlat
	if (mermi.isFired == true) {
		mermi.ballPos.x += cos(aimingAngle) * 10.0f;
		mermi.ballPos.y += sin(aimingAngle) * 10.0f;
	}

	//merminin ekranýn dýþýna çýkýp çýkmadýðýný kontrol et
	if (mermi.ballPos.x>(float)screenWidth+20.0||mermi.ballPos.x<-20.0||mermi.ballPos.y>(float)screenHeight+20.0||mermi.ballPos.y<-20.0) {
		mermi.active = false;
	}
	if (mermi.active == false) {
		if (totalActive > 0) {
			mermi.color = giveColorBullet(head);
			mermi.ballPos =texturePosition;
			mermi.ballSpeed.x = 0.0;
			mermi.ballSpeed.y = 0.0;
			mermi.isFired = false;
			mermi.active = true;
		}
	//yeni dalgadan önce bir renk verin
		else if (totalActive <= 0) {
			mermi.color = YELLOW;
			mermi.ballPos = (Vector2){ screenWidth / 2,screenHeight / 2 };
			mermi.ballSpeed.x = 0.0;
			mermi.ballSpeed.y = 0.0;
			mermi.isFired = false;
			mermi.active = true;
		}
	}

	//yeni dalga gelmeden önce ateþ etmeyi önlemek
	if (mermi.active == true && totalActive <= 0) {
		mermi.isFired = false;
	}

	if (isGameOver(head)) {
		gameOver = true;
	}
}

bool isGameOver(node* head) {
	node* current = head;
	while (current != NULL) {
		if (current->data->active) {
			return false;
		}
		current = current->next;
	}
	return true;
}
