#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#define MAX_BALL 50

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
	int direction; //0sağ/1aşağı/2sol/3yukarı
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
int healthCounter = 0;
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
	kurbaga = LoadTexture("image/crocodile.png");
	background = LoadTexture("image/path1.png");
	redball = LoadTexture("image/papagan.png");
	blueball = LoadTexture("image/kaplan.png");
	greenball = LoadTexture("image/baykus.png");
	yellowball = LoadTexture("image/panda.png");
	purpleball = LoadTexture("image/fil.png");
	blackball = LoadTexture("image/blackball.png");
	ending = LoadTexture("image/ending.png");
	gameover = LoadTexture("image/gameover.png");
	marble = LoadTexture("image/marble.png");
	mainmenu = LoadTexture("image/arkaplan.png");
	play = LoadTexture("image/play.png");
	retry = LoadTexture("image/retry2.png");
	music = LoadMusicStream("sounds/shanty-town.wav");
	effect = LoadSound("sounds/collision.mp3");

	bool musicPlaying = false;

	Vector2 textureCenter = { kurbaga.width / 2.0f+10 ,kurbaga.height / 2.0f-48 }; // position


	Rectangle sourceRec = { 0,0, play.width, play.height }; //play button position
	Rectangle pressBounds = { 630,580, play.width, play.height };

	Rectangle retrySourceRec = { 0,0, retry.width, retry.height }; //retry button position
	Rectangle retryPressBound = { 600,600, retry.width, retry.height };

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
			currentScreen = GAMEOVER;
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

			maxball = MAX_BALL;
			titleToGameplayDelayCounter = 0;
		}

		//retry  funct
		/*if (currentScreen == GAMEOVER && CheckCollisionPointRec(mouse, retryPressBound) && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && totalActive == 0) {

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
		}*/

		// retry button logic
		if (currentScreen == GAMEOVER && CheckCollisionPointRec(mouse, retryPressBound) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {

			// 1. Head listesini temizle
			freeTargets(head);
			head = NULL;

			// 2. Tüm oyun değişkenlerini sıfırla
			SetTargetFPS(120);
			currentScreen = GAMEPLAY;
			gameStarted = true;
			score = 0;
			gameOver = false;
			healthCounter = 0;

			// 3. Yeni topları üret
			maxball = MAX_BALL; // (örneğin: 30 gibi başlangıç değerin neyse)
			initGame();  // veya initGame2();

			// 4. Mermiyi resetle
			mermi.color = giveColorBullet(head);
			mermi.ballPos = texturePosition;
			mermi.ballSpeed = (Vector2){ 0,0 };
			mermi.isFired = false;
			mermi.active = true;
		}

	
		//opening gameplay screen with minor delay
		if (currentScreen == GAMEPLAY) {
			titleToGameplayDelayCounter++;
			if (titleToGameplayDelayCounter > GetFPS() / 10) {
				updateGame();
				updateTarget(&head);
			}
			if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) bulletFire();
			// Müziği sadece bir kere başlat
			if (!musicPlaying) {
				PlayMusicStream(music);
				musicPlaying = true;
			}
		}
		else {
			// GAMEPLAY ekranında değilsek müziği durdur
			if (musicPlaying) {
				StopMusicStream(music);
				musicPlaying = false;
			}
		
			mermi.active = false;
			mermi.isFired = false;
		}

		
		
		if (currentScreen == GAMEPLAY && !gameOver && head != NULL) {
			if (checkCollision(head, &mermi)) {
				node* newBall = createOne(mermi);
				node* inserted = addTargetBetween(newBall, shotTargetIndex(&head, &mermi));
				stepBack(head, inserted);
				isBoom(inserted); // Direkt olarak eklenen düğümle eşleşme kontrolü
			}
		}
		//draw based on current game screen / mevcut oyun ekranına göre çizim
		BeginDrawing();
		switch (currentScreen) {
		case TITLE:
			DrawTexture(mainmenu, 0, 0, WHITE);
			DrawTextureRec(play, sourceRec, (Vector2) { pressBounds.x, pressBounds.y }, WHITE);
			DrawTexture(marble, 580, 100, WHITE);
			break;

		case GAMEPLAY:
			DrawTexture(background, 0, 0, WHITE);
			DrawTargets(head);

			//kurbağa resminin dönüşü
			DrawTexturePro(kurbaga, (Rectangle) { 0, 0, kurbaga.width, kurbaga.height }, 
				(Rectangle) {texturePosition.x+20,texturePosition.y+18,kurbaga.width,kurbaga.height},textureCenter,angle,WHITE);

			DrawTexture(ending, 1000, 315 , WHITE);
			DrawText(TextFormat("%d", score), 150, 14, 50, WHITE);

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

	//oyun varlıklarını başlat
	initGame();
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
	//merminin ilk değerleri
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
			hedef[num].y = 60;
			hedef[num].x = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = (Color){ 255,255,255,0 };
			hedef[num].active = false;
			hedef[num].moving = true;
		}
		//visible
		else if (sonuncuNum - 3 <= num) {
			hedef[num].y = 60;
			hedef[num].x = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = (Color){ 255,255,255,0 };
			hedef[num].active = false;
			hedef[num].moving = true;
		}
		//invisible
		else {
			hedef[num].y = 60;
			hedef[num].x = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = giveColor();
			hedef[num].active = true;
			hedef[num].moving = true;
		}
		targetCreator(&head, &hedef[num]);
	}

	//mermi ilk değerler
	mermi.ballPos = texturePosition;
	mermi.ballSpeed = (Vector2){ 0,0 };
	mermi.radius = 20, 0;
	mermi.color = giveColorBullet(head);
	mermi.isFired = false;
	mermi.active = true;
}

//ek oyun başlatma mantığı
void initGame2() {
	int sonuncuNum = maxball;

	for (int num = 1; num <= maxball; num++) {
		if (num <= 3) {
			hedef[num].y = 60;
			hedef[num].x = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = (Color){ 255,255,255,0 };
			hedef[num].active = false;
			hedef[num].moving = true;
		}
		else if (sonuncuNum - 3 <= num) {
			hedef[num].y = 60;
			hedef[num].x = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = (Color){ 255,255,255,0 };
			hedef[num].active = false;
			hedef[num].moving = true;
		}
		else {
			hedef[num].y = 60;
			hedef[num].x = 80 - num * 40;
			hedef[num].radius = 20;
			hedef[num].color = giveColor();
			hedef[num].active = true;
			hedef[num].moving = true;
		}
		targetCreator(&head, &hedef[num]);
	}

	mermi.ballPos = texturePosition;
	mermi.ballSpeed = (Vector2){ 0,0 };
	mermi.radius = 20.0;
	mermi.color = giveColorBullet(head);
	mermi.isFired = false;
	mermi.active = true;
}

Color giveColor() {
	int random;
	if (45 > maxball && maxball >= 30) random = GetRandomValue(1, 4);
	else if (65 > maxball && maxball >= 45) random = GetRandomValue(1,5);
	else if (maxball >= 65) random = GetRandomValue(1, 6);  // Bu satır artık işe yarayacak
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
	// Eğer aktif top yoksa varsayılan olarak RED dön (veya başka bir işlem yap)
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

			// Açı hesapla (atan2f sayesinde tüm çeyrekler düzgün çalışır)
			aimingAngle = atan2f(deltaY, deltaX);

			mermi.isFired = true;
		}
}


target* createOneFromBullet(bullet mermi) {
    target* newCreated = (target*)malloc(sizeof(target));
    if (!newCreated) return NULL;

    newCreated->radius = 20;
    newCreated->color = mermi.color;
    newCreated->active = true;
    newCreated->moving = true;
    newCreated->x = 0;
    newCreated->y = 0;

    return newCreated;
}


void stepBack(node* head, node* newCreated) {
	node* current = head;

	// Liste sonuna git
	while (current->next != NULL) {
		current = current->next;
	}

	// Yeni eklenen topu bul
	while (current->previous != NULL && current->previous != newCreated) {
		current = current->previous;
	}

	// Yeni eklenen topun bir öncesine git
	current = current->previous;

	// Geri kaydırma işlemi
	while (current != NULL) {
		int hamle = 40;
		switch (whereTarget(current)) {
		case 1: // sağa gitme
			if (current->data->y == 60) {
				while (current->data->x < screenWidth - 40 && hamle > 0) {
					current->data->x += 1;
					hamle--;
				}
				while (current->data->x == screenWidth - 40 && hamle > 0) {
					current->data->y += 1;
					hamle--;
				}
			}
			else if (current->data->y == 300) {
				while (current->data->x < screenWidth - 160 && hamle > 0) {
					current->data->x += 1;
					hamle--;
				}
				while (current->data->x == screenWidth - 160 && hamle > 0) {
					current->data->y += 1;
					hamle--;
				}
			}
			else if (current->data->y == 550) {
				while (current->data->x < screenWidth / 2 && hamle > 0) {
					current->data->x += 1;
					hamle--;
				}
				while (current->data->x == screenWidth / 2 && hamle > 0) {
					current->data->y += 1;
					hamle--;
				}
			}
			break;

		case 2: // sola gitme
			if (current->data->y == screenHeight - 80) {
				while (current->data->x > 80 && hamle > 0) {
					current->data->x -= 1;
					hamle--;
				}
				while (current->data->x == 80 && hamle > 0) {
					current->data->y -= 1;
					hamle--;
				}
			}
			else if (current->data->y == screenHeight - 300) {
				while (current->data->x > 160 && hamle > 0) {
					current->data->x -= 1;
					hamle--;
				}
				while (current->data->x == 160 && hamle > 0) {
					current->data->y -= 1;
					hamle--;
				}
			}
			break;

		case 3: // aşağı gitme
			if (current->data->x == screenWidth - 40) {
				while (current->data->y < screenHeight - 80 && hamle > 0) {
					current->data->y += 1;
					hamle--;
				}
				while (current->data->y == screenHeight - 80 && hamle > 0) {
					current->data->x -= 1;
					hamle--;
				}
			}
			else if (current->data->x == screenWidth - 160) {
				while (current->data->y < screenHeight - 300 && hamle > 0) {
					current->data->y += 1;
					hamle--;
				}
				while (current->data->y == screenHeight - 300 && hamle > 0) {
					current->data->x -= 1;
					hamle--;
				}
			}
			break;

		case 4: // yukarı gitme
			if (current->data->x == 80) {
				while (current->data->y > 300 && hamle > 0) {
					current->data->y -= 1;
					hamle--;
				}
				while (current->data->y == 300 && hamle > 0) {
					current->data->x += 1;
					hamle--;
				}
			}
			else if (current->data->x == 160) {
				while (current->data->y > 550 && hamle > 0) {
					current->data->y -= 1;
					hamle--;
				}
				while (current->data->y == 550 && hamle > 0) {
					current->data->x += 1;
					hamle--;
				}
			}
			break;
		}
		current = current->previous;
	}
}

/*void isBoom() {
	printf("isBoom() çağrıldı.\n");

	node* vurulan = head;
	node* eklenen = NULL;
	target* hedef = shotTargetIndex(&head, &mermi);
	printf("shotTargetIndex çalıştı. hedef pointer'ı: %p\n", hedef);

	while (vurulan != NULL && vurulan->data != hedef) {
		vurulan = vurulan->next;
	}

	if (vurulan == NULL) {
		printf("Hedef vurulan listede bulunamadı.\n");
		return;
	}

	// Vurulanın bir öncesi: eklenen topun geldiği yer
	eklenen = vurulan->previous;

	if (eklenen == NULL) {
		printf("Eklenen NULL, işlem yapılamıyor.\n");
		return;
	}

	printf("Eklenen top: x=%d, y=%d\n", eklenen->data->x, eklenen->data->y);

	// Zinciri tara
	int sayac = 1;
	node* temp = eklenen;

	// Sola doğru
	while (temp->previous != NULL && isSameColor(temp->previous->data->color, eklenen->data->color) && temp->previous->data->active) {
		sayac++;
		temp = temp->previous;
	}

	node* solUcu = temp;
	temp = eklenen;

	// Sağa doğru
	while (temp->next != NULL && isSameColor(temp->next->data->color, eklenen->data->color) && temp->next->data->active) {
		sayac++;
		temp = temp->next;
	}

	node* sagUcu = temp;

	if (sayac >= 3) {
		printf("%d top eşleşti. Yok ediliyor.\n", sayac);

		// Silme
		node* current = solUcu;
		while (current != sagUcu->next) {
			current->data->active = false;
			score += 10;
			current = current->next;
		}

		// Zinciri bağla
		if (solUcu->previous != NULL) {
			solUcu->previous->next = sagUcu->next;
		}
		if (sagUcu->next != NULL) {
			sagUcu->next->previous = solUcu->previous;
		}

		PlaySound(effect);
		updateTarget(&head);
		printf("Patlama sonrası zincir güncellendi.\n");

	}
	else {
		printf("Eşleşme yok, patlama olmadı.\n");
	}
	updateTarget(&head);
}*/

void isBoom() {
	printf("isBoom() çağrıldı.\n");

	node* vurulan = head;
	node* eklenen = NULL;
	target* hedef = shotTargetIndex(&head, &mermi);
	if (!hedef) {
		printf("shotTargetIndex başarısız. Hedef bulunamadı.\n");
		return;
	}

	// Hedefe karşılık gelen node'u bul
	while (vurulan && vurulan->data != hedef) {
		vurulan = vurulan->next;
	}

	if (!vurulan || !vurulan->previous) {
		printf("Vurulan top bulunamadı veya previous NULL.\n");
		return;
	}

	eklenen = vurulan->previous;
	printf("Eklenen top: x=%d, y=%d\n", eklenen->data->x, eklenen->data->y);

	// Zinciri tara
	int sayac = 1;
	node* solUcu = eklenen;
	node* sagUcu = eklenen;

	// Sola doğru
	while (solUcu->previous && isSameColor(solUcu->previous->data->color, eklenen->data->color) && solUcu->previous->data->active) {
		solUcu = solUcu->previous;
		sayac++;
	}

	// Sağa doğru
	while (sagUcu->next && isSameColor(sagUcu->next->data->color, eklenen->data->color) && sagUcu->next->data->active) {
		sagUcu = sagUcu->next;
		sayac++;
	}

	if (sayac >= 3) {
		printf("%d top eşleşti. Yok ediliyor.\n", sayac);

		// Silinenleri işaretle ve puan ver
		node* current = solUcu;
		while (current != sagUcu->next) {
			current->data->active = false;
			score += 10;
			current = current->next;
		}

		// Zinciri bağla
		if (solUcu->previous) {
			solUcu->previous->next = sagUcu->next;
		}
		if (sagUcu->next) {
			sagUcu->next->previous = solUcu->previous;
		}

		// Hafızayı temizle (aktif olmayanları)
		current = head;
		while (current) {
			node* sonraki = current->next;
			if (!current->data->active) {
				if (current->previous) current->previous->next = current->next;
				if (current->next) current->next->previous = current->previous;
				if (current == head) head = current->next;
				free(current->data);
				free(current);
			}
			current = sonraki;
		}

		PlaySound(effect);
		updateTarget(&head);

		// Zincirleme kontrolü
		isBoom();
	}
	else {
		printf("Eşleşme yok, patlama olmadı.\n");
	}

	updateTarget(&head);
}

//çarpışma tespiti
int checkCollision(node* head, bullet* mermi) {
	node* current = head;

	while (current != NULL) {
		Vector2 hedefCenter = { current->data->x,current->data->y };
		Vector2 mermiCenter = { mermi->ballPos.x,mermi->ballPos.y };

		//merminin görünmez hedef toplara çarpmasını engellemek
		if (current->data->active == true && CheckCollisionCircles(hedefCenter, 20, mermiCenter, 20)) {
			printf("Çarpışma bulundu! Hedef pozisyonu: (%f, %f)\n", current->data->x, current->data->y);
			mermi->active = false;
			mermi->isFired = false;
			return 1;
		}
		current = current->next;
	}
	return 0;
}

//atılan hedef topun verilerini verir

node* shotTargetNode(node* head, bullet* mermi) {
    node* current = head;
    while (current != NULL) {
        Vector2 hedefCenter = { current->data->x, current->data->y };
        Vector2 mermiCenter = { mermi->ballPos.x, mermi->ballPos.y };

        if (current->data->active && CheckCollisionCircles(hedefCenter, 20, mermiCenter, 20)) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}


/*node* addTargetBetween(target* newCreated, target* shotTargetIndex) {
	node* current = head;

	// Bellek ayır
	node* newNode = (node*)malloc(sizeof(node));
	if (newNode == NULL || newCreated == NULL || shotTargetIndex == NULL) {
		printf("Hatalı parametre veya bellek ayıramadı.\n");
		return NULL;
	}

	// Çarpışan hedefin bulunduğu node'u bul
	while (current != NULL && current->data != shotTargetIndex) {
		current = current->next;
	}

	if (current == NULL) {
		printf("Hedef top listede bulunamadı.\n");
		return NULL;
	}

	if (current->previous == NULL) {
		printf("Hedef topun öncesi yok, başa ekleme yapılamaz.\n");
		return NULL;
	}

	// Yeni node'u araya ekle
	node* onceki = current->previous;

	newNode->data = newCreated;
	newNode->previous = onceki;
	newNode->next = current;

	onceki->next = newNode;
	current->previous = newNode;

	// Topun konumunu hizala
	newNode->data->x = onceki->data->x;
	newNode->data->y = onceki->data->y;

	printf("Yeni top başarıyla eklendi.\n");

	return newNode;
}*/
node* addTargetBetween(target* newCreated, target* shotTargetIndex) {
	node* current = head;

	node* newNode = (node*)malloc(sizeof(node));
	if (newNode == NULL || newCreated == NULL || shotTargetIndex == NULL) {
		printf("Hatalı parametre veya bellek ayıramadı.\n");
		return NULL;
	}

	while (current != NULL && current->data != shotTargetIndex) {
		current = current->next;
	}

	if (current == NULL) {
		printf("Hedef top listede bulunamadı.\n");
		return NULL;
	}

	if (current->previous == NULL) {
		printf("Hedef topun öncesi yok, başa ekleme yapılamaz.\n");
		return NULL;
	}

	node* onceki = current->previous;

	// Bağlantıları ayarla
	newNode->data = newCreated;
	newNode->previous = onceki;
	newNode->next = current;

	onceki->next = newNode;
	current->previous = newNode;
/*
	// 📏 İki top arasındaki yönü bul ve sabit adım geri git
	float dx = current->data->x - onceki->data->x;
	float dy = current->data->y - onceki->data->y;

	// Normalize et (yön vektörü)
	float length = sqrtf(dx * dx + dy * dy);
	if (length == 0) length = 1; // bölme hatası engelle

	float nx = dx / length;
	float ny = dy / length;

	// Mermiyi hedef topun önüne sabit mesafe kadar geri koy
	float step = 1.0f; // toplar arası mesafe kadar

	newNode->data->x = current->data->x - nx * step;
	newNode->data->y = current->data->y - ny * step;

	printf("Yeni top düzgün şekilde eklendi ve hizalandı.\n");

	return newNode;*/

	float step = 40.0f; // toplar arası mesafe kadar
	/*switch (current->data->direction) {
	case 0: // sağa gidiyorsa, topu biraz sola yerleştir
		newNode->data->x = current->data->x - step;
		newNode->data->y = current->data->y;
		break;
	case 1: // aşağı gidiyorsa
		newNode->data->x = current->data->x;
		newNode->data->y = current->data->y - step;
		break;
	case 2: // sola gidiyorsa
		newNode->data->x = current->data->x + step;
		newNode->data->y = current->data->y;
		break;
	case 3: // yukarı gidiyorsa
		newNode->data->x = current->data->x;
		newNode->data->y = current->data->y + step;
		break;
	}*/
	switch (shotTargetIndex->direction) {
	case 0: // sağa gidiyorsa, yeni topu soluna koy
		newNode->data->x = shotTargetIndex->x - step;
		newNode->data->y = shotTargetIndex->y;
		break;
	case 1: // aşağıya gidiyorsa, yeni topu yukarıya koy
		newNode->data->x = shotTargetIndex->x;
		newNode->data->y = shotTargetIndex->y - step;
		break;
	case 2: // sola gidiyorsa, yeni topu sağına koy
		newNode->data->x = shotTargetIndex->x + step;
		newNode->data->y = shotTargetIndex->y;
		break;
	case 3: // yukarı gidiyorsa, yeni topu aşağıya koy
		newNode->data->x = shotTargetIndex->x;
		newNode->data->y = shotTargetIndex->y + step;
		break;
	default:
		// bilinmeyen yön, aynı konuma koy (güvenlik önlemi)
		newNode->data->x = shotTargetIndex->x;
		newNode->data->y = shotTargetIndex->y;
		break;
	}
	printf("Yeni top başarıyla yönlü eklendi.\n");

	return newNode;
}

int whereTarget(node* given) {
	node* current = given;

	target* selected = current->data;

	if ((abs(selected->y - (60)) <= 1) && (selected->x < screenWidth - 90)) return 1; //going right
	if ((abs(selected->x - (screenWidth - 90)) <= 1) && (selected->y < screenHeight - 80)) return 3; //going down
	if ((abs(selected->y - (screenHeight - 80)) <= 1) && (selected->x > 75)) return 2; //going left
	if ((abs(selected->x - (75)) <= 1) && (selected->y > 190)) return 4; //going up
	if ((abs(selected->y - (190)) <= 1) && (selected->x < screenWidth - 280)) return 1;

	if ((abs(selected->x - (screenWidth - 280)) <= 1) && (selected->y < screenHeight - 195) && (selected->y != 80)) return 3;
	if ((abs(selected->y - (screenHeight - 195)) <= 1) && (selected->x > 230) && (selected->x != screenWidth - 80)) return 2;
	if ((abs(selected->x - (230)) <= 1) && (selected->y > 350) && (selected->y != screenHeight - 80)) return 4;
	if ((abs(selected->y - (350)) <= 1) && (selected->x < 1020) && (selected->x != 80)) return  1;

	else return 0;
}

void updateTarget(node** head) {
	node* current = *head;

	while (current != NULL) {
		target* selected = current->data;

		if (selected->moving == true && selected->active == true) {
			if ((abs(selected->y - (60)) <= 1) && (selected->x < screenWidth - 90)) {
				selected->x++;
				selected->direction = 0; // sağ
			}
			else if ((abs(selected->x - (screenWidth - 90)) <= 1) && (selected->y < screenHeight - 80)) {
				selected->y++;
				selected->direction = 1; // aşağı
			}
			else if ((abs(selected->y - (screenHeight - 80)) <= 1) && (selected->x > 75)) {
				selected->x--;
				selected->direction = 2; // sol
			}
			else if ((abs(selected->x - (75)) <= 1) && (selected->y > 190)) {
				selected->y--;
				selected->direction = 3; // yukarı
			}
			else if ((abs(selected->y - (190)) <= 1) && (selected->x < screenWidth - 280)) {
				selected->x++;
				selected->direction = 0; // sağ
			}
			else if ((abs(selected->x - (screenWidth - 280)) <= 1) && (selected->y < screenHeight - 195)) {
				selected->y++;
				selected->direction = 1; // aşağı
			}
			else if ((abs(selected->y - (screenHeight - 195)) <= 1) && (selected->x > 230)) {
				selected->x--;
				selected->direction = 2; // sol
			}
			else if ((abs(selected->x - (230)) <= 1) && (selected->y > 350)) {
				selected->y--;
				selected->direction = 3; // yukarı
			}
			else if ((abs(selected->y - (350)) <= 1) && (selected->x < 1020)) {
				selected->x++;
				selected->direction = 0; // sağ
			}

			if ((abs(selected->x - (1020)) <= 1) && (abs(selected->y - (350)) <= 1)) {
				if (selected->active) {  // sadece bir kez saysın
						healthCounter++;
						selected->active = false; // bir daha saymasın
				}
			}
		}
		current = current->next;
	}
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
	while (current != NULL) {
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

//yüksek puanı bul
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

//hedefleri hareket ettirmek ve oyun üzerindeki koşulları kontrol etmek gibi oyun mantığını güncelleyin
void updateGame(){

	activeCounter = 0;

	node* current = head;
	while (current->next != NULL) {
		if (current->data->active == true) activeCounter++;
		current = current->next;
	}
	totalActive = activeCounter;
	if (totalActive == 0 && healthCounter < 2) {
		maxball += 10;
		initGame2();
		SetTargetFPS(120);
	}
	else activeCounter = 0;

	mermi.ballPos.x += mermi.ballSpeed.x;
	mermi.ballPos.y += mermi.ballSpeed.y;

	//mermi hareketini başlat
	if (mermi.isFired == true) {
		mermi.ballPos.x += cos(aimingAngle) * 10.0f;
		mermi.ballPos.y += sin(aimingAngle) * 10.0f;
	}

	//merminin ekranın dışına çıkıp çıkmadığını kontrol et
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

			updateTarget(&head);
		}
	//yeni dalgadan önce bir renk verin
		else if (totalActive <= 0) {
			mermi.color = YELLOW;
			mermi.ballPos = texturePosition;
			mermi.ballSpeed.x = 0.0;
			mermi.ballSpeed.y = 0.0;
			mermi.isFired = false;
			mermi.active = true;
		}
	}

	//yeni dalga gelmeden önce ateş etmeyi önlemek
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