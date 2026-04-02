#include "raylib.h"
#include "resource_dir.h" // utility header for SearchAndSetResourceDir

#include <list>
#include <vector>
#include <random>
#include <string>

const bool DEBUG_RENDER = false;
const float SCREEN_WIDTH = 1920;
const float SCREEN_HEIGHT = 1080;
const float OBSTACLE_GAP = 952;
const float OBSTACLE_SPEED = 300;
const float SCALE = 5;

const int MARGIN = 20;
const int FONT_SIZE = 40;

float obstacle_width;
float obstacle_length;
int rand_width;
int rand_gap;

bool pause = false;

enum GameState
{
	MENU,
	PLAYING,
	GAME_OVER
};
GameState currentState = MENU;

class Obstacle
{
public:
	float x, y, gap;
	static Texture obstacle_image;
	static Rectangle obstacle_top_hitbox;
	static Rectangle obstacle_bottom_hitbox;
	static Rectangle obstacle_clear_hitbox;

	Obstacle(float X = SCREEN_WIDTH, float Y = -384.0f, float Gap = OBSTACLE_GAP)
	{
		x = X;
		y = Y;
		gap = Gap;
	}
};

std::list<Obstacle *> obstacles;
Texture Obstacle::obstacle_image;
Rectangle Obstacle::obstacle_top_hitbox;
Rectangle Obstacle::obstacle_bottom_hitbox;
Rectangle Obstacle::obstacle_clear_hitbox;

class Whale
{
private:
	const float GRAVITY = 800.0;
	std::vector<Texture> frames;
	int current_frame = 7;
	bool clear_flag = false;
	bool hit_flag = false;
	float frameTimer = 0.0f;
	float player_maxdeg = -30.0;
	float player_mindeg = 90.0;
	Vector2 player_pos;
	Vector2 player_vel;
	Vector2 player_origin;
	float player_deg;
	Rectangle source_rec;
	Rectangle dest_rec;

	Whale()
	{
		player_pos = {SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2};
		player_vel = {0.0f, 0.0f};
		player_origin = {0.0f, 0.0f};
		player_deg = 0;
	}
	Whale(const Whale &) = delete;
	Whale &operator=(const Whale &) = delete;

public:
	static Whale &Get()
	{
		static Whale instance; // Created only once, the first time this is called.
		return instance;
	}

	void Frames()
	{
		for (int i = 1; i < 9; i++)
		{
			// Build path: "art/sprites/whale[1-8].png"
			frames.push_back(LoadTexture(TextFormat("art/sprites/whale%d.png", i)));
		}
		source_rec = {0.0f, 0.0f, (float)frames[0].width, (float)frames[0].height};
		player_origin = {source_rec.width * SCALE / 2, source_rec.height * SCALE / 2};
	}

	void Jump()
	{
		player_vel.y -= 800.0f;
		player_deg = std::min(player_deg, player_maxdeg);
	}

	Rectangle Move(float deltaTime)
	{
		dest_rec = {player_pos.x, player_pos.y, frames[0].width * SCALE, frames[0].height * SCALE};
		player_vel.y += GRAVITY * deltaTime;
		player_pos.y += player_vel.y * deltaTime;
		player_pos.y = std::min(player_pos.y, SCREEN_HEIGHT - dest_rec.height);
		player_pos.y = std::max(player_pos.y, dest_rec.height / 2);
		player_deg += deltaTime * 100;
		if (player_vel.y < -200.0f)
		{
			player_deg = std::min(player_deg, player_maxdeg);
		}
		else
		{
			player_deg = std::min(player_deg, player_mindeg);
		}
		player_deg = std::max(player_deg, player_maxdeg);

		if (player_pos.y == dest_rec.height / 2)
		{
			player_vel.y = 0;
		}

		if (player_pos.y == SCREEN_HEIGHT - dest_rec.height)
		{
			player_vel.y = std::min(player_vel.y, 0.0f);
		}
		return (dest_rec);
	}

	void Animation()
	{
		if (frameTimer >= 0.1)
		{
			frameTimer = 0.0f;
			if (current_frame < frames.size() - 1)
				current_frame++;
		}
	}

	unsigned int Update(float deltaTime, unsigned int clear_count, bool collision_clear, bool collision_hit, Sound clear_sfx, Sound hit_sfx)
	{
		frameTimer += deltaTime;

		if (IsKeyPressed(KEY_SPACE))
		{
			Whale::Get().Jump();
			current_frame = 0;
		}
		Whale::Get().Animation();

		// For when the player enters the gap
		if (!clear_flag)
		{
			if (collision_clear)
			{
				clear_count++;
				PlaySound(clear_sfx);
				clear_flag = true;
			}
		}
		else
		{
			// If we are already colliding, check to see if the player has left the area
			if (!collision_clear)
			{
				clear_flag = false;
			}
			else
			{
				if (DEBUG_RENDER)
				{
					DrawCircle(dest_rec.x, dest_rec.y, dest_rec.height * 2 / 5, GREEN);
				}
			}
		}

		// For when the player hits an obstacle
		if (!hit_flag)
		{
			if (collision_hit)
			{
				PlaySound(hit_sfx);
				hit_flag = true;
			}
		}
		else
		{
			// If we are already colliding, check to see if the player has left the area
			if (!collision_hit)
			{
				hit_flag = false;
			}
			else
			{
				if (DEBUG_RENDER)
				{
					DrawCircle(dest_rec.x, dest_rec.y, dest_rec.height * 2 / 5, RED);
				}
			}
		}
		return (clear_count);
	}

	void DrawWhale()
	{
		DrawTexturePro(frames[current_frame], source_rec, dest_rec, player_origin, player_deg, WHITE);

		if (DEBUG_RENDER)
		{
			DrawCircle(dest_rec.x, dest_rec.y, dest_rec.height * 2 / 5, Color{230, 41, 55, 127});
		}
	}

	void Reset()
	{
		player_pos = {SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2 - dest_rec.height};
		player_vel = {0.0f, 0.0f};
		player_deg = 0;
	}

	~Whale()
	{
		for (int i = 0; i < frames.size(); i++)
		{
			UnloadTexture(frames[i]);
		}
		frames.clear();
	}
};

void GameReset(Music music, uint_fast32_t rng)
{
	StopMusicStream(music);
	PlayMusicStream(music);
	Whale::Get().Reset();
	for (Obstacle *obs : obstacles)
	{
		delete obs;
	}
	obstacles.clear();
	Obstacle *j = new Obstacle(SCREEN_WIDTH, rng * -1.0f, OBSTACLE_GAP);
	obstacles.push_back(j);
};

int main()
{
	unsigned int clear_count = 0;
	bool collision_clear;
	bool collision_hit;
	Rectangle player_hitbox;

	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Floppy Whale");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("assets");

	// Load local high score
	unsigned int high_score = 0;
	int bytesRead = 0;
	unsigned char *loadedData = LoadFileData("high_score.bin", &bytesRead);
	if (loadedData == nullptr)
	{
		printf("No high score file found. Making high_score.bin on shutdown.\n");
	}
	else
	{
		high_score = *(unsigned int *)loadedData;
	}
	UnloadFileData(loadedData);

	// Load sounds
	SetAudioStreamBufferSizeDefault(8192);
	InitAudioDevice();
	Music bgm = LoadMusicStream("sfx/music.ogg");
	Sound clear_sfx = LoadSound("sfx/da_ding.wav");
	Sound hit_sfx = LoadSound("sfx/thump.wav");

	// Set background music
	bgm.looping = true;
	PlayMusicStream(bgm);

	// Load a texture from the resources directory
	Whale::Get().Frames();
	Obstacle::obstacle_image = LoadTexture("art/sprites/rock.png");
	obstacle_width = Obstacle::obstacle_image.width * SCALE;
	obstacle_length = Obstacle::obstacle_image.height * SCALE;

	// Generate random number
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> obstacle_distribution(0, 384);

	// game loop
	while (!WindowShouldClose()) // run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// Switch between Menu, Playing, and Game Over game states
		switch (currentState)
		{
		case MENU:
			if (IsKeyPressed(KEY_SPACE))
			{
				GameReset(bgm, obstacle_distribution(rng));
				clear_count = 0;
				currentState = PLAYING;
			}
			break;

		case PLAYING:
			if (IsKeyPressed(KEY_P))
			{
				pause = !pause;
			}

			// Game logic updates while not paused
			if (!pause)
			{
				SetMusicVolume(bgm, 0.1);
				float deltaTime = GetFrameTime();

				player_hitbox = Whale::Get().Move(deltaTime);

				if ((obstacles.back()->x < SCREEN_WIDTH * 2 / 3))
				{
					Obstacle *n = new Obstacle(SCREEN_WIDTH, obstacle_distribution(rng) * -1.0f, OBSTACLE_GAP);
					obstacles.push_back(n);
				}

				if (obstacles.front()->x < 0 - obstacle_width)
				{
					Obstacle *front = obstacles.front();
					delete front;
					obstacles.pop_front();
				}

				for (Obstacle *o : obstacles)
				{
					o->x -= OBSTACLE_SPEED * deltaTime;
				}

				if (collision_hit)
				{
					if (high_score < clear_count)
					{
						high_score = clear_count;
					}
					currentState = GAME_OVER;
				}

				clear_count = Whale::Get().Update(deltaTime, clear_count, collision_clear, collision_hit, clear_sfx, hit_sfx);
			}
			else
			{
				SetMusicVolume(bgm, 0.025);
			}

			UpdateMusicStream(bgm);
			break;

		case GAME_OVER:
			SetMusicVolume(bgm, 0.025);
			UpdateMusicStream(bgm);
			if (IsKeyPressed(KEY_ENTER))
			{
				GameReset(bgm, obstacle_distribution(rng));
				clear_count = 0;
				currentState = PLAYING;
			}
			break;
		}

		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLUE);
		if (currentState != MENU)
		{
			collision_clear = false;
			collision_hit = false;
			for (Obstacle *o : obstacles)
			{
				DrawTextureEx(o->obstacle_image, {o->x, o->y}, 0.0f, SCALE, WHITE);
				DrawTextureEx(o->obstacle_image, {o->x, o->y + o->gap}, 0.0f, SCALE, WHITE);
				Obstacle::obstacle_top_hitbox = {o->x, o->y, obstacle_width, obstacle_length};
				Obstacle::obstacle_bottom_hitbox = {o->x, o->y + o->gap, obstacle_width, obstacle_length};
				Obstacle::obstacle_clear_hitbox = {o->x + obstacle_width * (SCALE - 1) / SCALE, o->y + obstacle_length, obstacle_width / SCALE, o->gap - obstacle_length};

				if (CheckCollisionCircleRec({player_hitbox.x, player_hitbox.y}, player_hitbox.height * 2 / 5, Obstacle::obstacle_clear_hitbox))
				{
					collision_clear = true;
				}

				if (CheckCollisionCircleRec({player_hitbox.x, player_hitbox.y}, player_hitbox.height * 2 / 5, Obstacle::obstacle_top_hitbox) || CheckCollisionCircleRec({player_hitbox.x, player_hitbox.y}, player_hitbox.height * 2 / 5, Obstacle::obstacle_bottom_hitbox))
				{
					collision_hit = true;
				}

				// Debug
				if (DEBUG_RENDER && collision_clear)
				{
					DrawRectangleRec(Obstacle::obstacle_clear_hitbox, Color{0, 228, 48, 127});
				}
				if (DEBUG_RENDER && collision_hit)
				{
					DrawRectangleRec(Obstacle::obstacle_top_hitbox, Color{230, 41, 55, 127});
					DrawRectangleRec(Obstacle::obstacle_bottom_hitbox, Color{230, 41, 55, 127});
				}
			}

			Whale::Get().DrawWhale();

			DrawText(TextFormat("High Score: %u", high_score), MARGIN, MARGIN - 5, FONT_SIZE, GREEN);
			DrawText(TextFormat("Cleared: %u", clear_count), MARGIN, MARGIN * 3, FONT_SIZE, YELLOW);
		}

		// Overlays
		if (currentState == MENU)
		{
			DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.4f));
			DrawText("FLOPPY WHALE", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 50, 60, WHITE);
			DrawText("Press SPACE to start", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 + 30, 20, LIGHTGRAY);
		}
		else if (currentState == GAME_OVER)
		{
			DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(RED, 0.5f));
			DrawText("GAME OVER", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 50, WHITE);
			DrawText(TextFormat("FINAL SCORE: %u", clear_count), SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 + 10, 25, YELLOW);
			DrawText("Press ENTER to try again", SCREEN_WIDTH / 2 - 110, SCREEN_HEIGHT / 2 + 60, 20, RAYWHITE);
			DrawText("Press ESC to give up", SCREEN_WIDTH / 2 - 110, SCREEN_HEIGHT / 2 + 100, 20, RAYWHITE);
		}
		else if (pause)
		{
			// Draw a semi-transparent black rectangle over the whole screen
			DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));
			DrawText("PAUSED", SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 20, FONT_SIZE, WHITE);
			DrawText("Press P to Resume", SCREEN_WIDTH / 2 - 110, SCREEN_HEIGHT / 2 + 30, 20, LIGHTGRAY);
		}

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	SaveFileData("high_score.bin", &high_score, sizeof(high_score));

	// cleanup
	// unload our texture so it can be cleaned up
	// Whale::Get().~Whale();
	// Obstacle::UnloadObstacle;
	UnloadTexture(Obstacle::obstacle_image);
	StopMusicStream(bgm);

	UnloadSound(clear_sfx);
	UnloadSound(hit_sfx);
	UnloadMusicStream(bgm);

	CloseAudioDevice();

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
