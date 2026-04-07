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
const float BASE_SPEED = 200;
const float SCALE = 5;

float obstacle_width;
float obstacle_length;

bool pause = false;

enum GameState
{
	MENU,
	PLAYING,
	GAME_OVER,
	CREDITS
};
GameState currentState = MENU;

struct Layer
{
	Texture tex;
	float speedFactor; // 0.0 to 1.0
	float currentX;
};

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

	void Jump(Sound swim_sfx)
	{
		player_vel.y -= 800.0f;
		player_deg = std::min(player_deg, player_maxdeg);
		PlaySound(swim_sfx);
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

	void Animation(float deltaTime)
	{
		frameTimer += deltaTime;

		if (frameTimer >= 0.1)
		{
			frameTimer = 0.0f;
			if (current_frame < frames.size() - 1)
				current_frame++;
		}
	}

	unsigned int Update(float deltaTime, unsigned int clear_count, bool collision_clear, bool collision_hit, Sound clear_sfx, Sound hit_sfx, Sound swim_sfx)
	{
		if (IsKeyPressed(KEY_SPACE))
		{
			Whale::Get().Jump(swim_sfx);
			current_frame = 0;
		}
		Whale::Get().Animation(deltaTime);

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
	bool speed_flag = false;
	float deltaSpeed = 1;

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
	Sound swim_sfx = LoadSound("sfx/swim.wav");
	Sound congrats_sfx = LoadSound("sfx/congrats.wav");

	// Set background music
	bgm.looping = true;
	PlayMusicStream(bgm);

	// Load textures from the resources directory
	Whale::Get().Frames();

	Obstacle::obstacle_image = LoadTexture("art/sprites/rock.png");
	obstacle_width = Obstacle::obstacle_image.width * SCALE;
	obstacle_length = Obstacle::obstacle_image.height * SCALE;

	std::vector<Layer> layers = {
		{LoadTexture("art/background/water.png"), 0.2f, 0.0f},
		// { LoadTexture("art/background/cave.png"), 0.15f, 0.0f },
		// { LoadTexture("art/background/rocks.png"), 0.40f, 0.0f },
		// { LoadTexture("art/background/sand.png"), 1.00f, 0.0f }
	};

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
			if (IsKeyPressed(KEY_C))
			{
				currentState = CREDITS;
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

				// Slowly increases the speed by 2.5% every 2 cleared obstacles
				if (!speed_flag)
				{
					if (clear_count > 0 && clear_count % 2 == 0)
					{
						deltaSpeed += 0.025f;
						speed_flag = true;
					}
				}
				else
				{
					if (!(clear_count % 2 == 0))
					{
						speed_flag = false;
					}
				}

				float obstacle_speed = BASE_SPEED * deltaSpeed;

				for (Layer &layer : layers)
				{
					layer.currentX -= (obstacle_speed * layer.speedFactor) * deltaTime;

					// Reset when texture moves off-screen
					if (layer.currentX <= -layer.tex.width * SCALE)
					{
						layer.currentX += layer.tex.width * SCALE;
					}
				}

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
					o->x -= obstacle_speed * deltaTime;
				}

				if (collision_hit)
				{
					if (high_score < clear_count)
					{
						high_score = clear_count;
					}
					currentState = GAME_OVER;
				}

				clear_count = Whale::Get().Update(deltaTime, clear_count, collision_clear, collision_hit, clear_sfx, hit_sfx, swim_sfx);
			}
			else
			{
				SetMusicVolume(bgm, 0.025);
			}

			UpdateMusicStream(bgm);
			break;

		case CREDITS:
			if (IsKeyPressed(KEY_C))
			{
				currentState = MENU;
			}
			break;

		case GAME_OVER:
			float deltaTime = GetFrameTime();
			player_hitbox = Whale::Get().Move(deltaTime);
			Whale::Get().Animation(deltaTime);
			SetMusicVolume(bgm, 0.025);
			UpdateMusicStream(bgm);
			if (player_hitbox.y >= (SCREEN_HEIGHT - player_hitbox.width))
			{
				if (IsKeyPressed(KEY_SPACE))
				{
					GameReset(bgm, obstacle_distribution(rng));
					for (Layer &layer : layers)
					{
						layer.currentX = 0.0f;
					}
					deltaSpeed = 1;
					clear_count = 0;
					currentState = PLAYING;
				}
			}
			break;
		}

		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(WHITE);

		for (Layer &layer : layers)
		{
			DrawTextureEx(layer.tex, {layer.currentX, 0}, 0.0f, SCALE, WHITE);
			DrawTextureEx(layer.tex, {layer.currentX + layer.tex.width * SCALE, 0}, 0.0f, SCALE, WHITE);
		}

		if (currentState == PLAYING || currentState == GAME_OVER)
		{
			collision_clear = false;
			collision_hit = false;
			for (Obstacle *o : obstacles)
			{
				DrawTextureEx(o->obstacle_image, {o->x, o->y}, 0.0f, SCALE, WHITE);
				DrawTextureEx(o->obstacle_image, {o->x, o->y + o->gap}, 0.0f, SCALE, WHITE);
				Obstacle::obstacle_top_hitbox = {o->x, o->y, obstacle_width, obstacle_length};
				Obstacle::obstacle_bottom_hitbox = {o->x, o->y + o->gap, obstacle_width, obstacle_length};
				Obstacle::obstacle_clear_hitbox = {o->x + obstacle_width * (SCALE - 3) / SCALE, o->y + obstacle_length, obstacle_width / SCALE, o->gap - obstacle_length};

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

			// Shadow
			DrawText(TextFormat("Cleared: %u", clear_count), 20 + 6, 15 + 6, 75, Fade(BLACK, 0.5f));
			// Text
			DrawText(TextFormat("Cleared: %u", clear_count), 20, 15, 75, WHITE);
		}

		// Overlays
		if (currentState == MENU)
		{
			DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));
			// Shadow
			DrawText("FLOPPY WHALE", (SCREEN_WIDTH - MeasureText("FLOPPY WHALE", 150)) / 2 + 9, SCREEN_HEIGHT / 5 + 9, 150, Fade(BLACK, 0.5f));
			DrawText("Press SPACE to start", (SCREEN_WIDTH - MeasureText("Press SPACE to start", 50)) / 2 + 6, SCREEN_HEIGHT / 2 + 6, 50, Fade(BLACK, 0.5f));
			DrawText("Press C for Credits", (SCREEN_WIDTH - MeasureText("Press C for Credits", 50)) / 2 + 6, SCREEN_HEIGHT / 2 + 80 + 6, 50, Fade(BLACK, 0.5f));
			DrawText("Press ESC to close", (SCREEN_WIDTH - MeasureText("Press ESC to close", 50)) / 2 + 6, SCREEN_HEIGHT / 2 + 160 + 6, 50, Fade(BLACK, 0.5f));
			// Text
			DrawText("FLOPPY WHALE", (SCREEN_WIDTH - MeasureText("FLOPPY WHALE", 150)) / 2, SCREEN_HEIGHT / 5, 150, SKYBLUE);
			DrawText("Press SPACE to start", (SCREEN_WIDTH - MeasureText("Press SPACE to start", 50)) / 2, SCREEN_HEIGHT / 2, 50, LIGHTGRAY);
			DrawText("Press C for Credits", (SCREEN_WIDTH - MeasureText("Press C for Credits", 50)) / 2, SCREEN_HEIGHT / 2 + 80, 50, LIGHTGRAY);
			DrawText("Press ESC to close", (SCREEN_WIDTH - MeasureText("Press ESC to close", 50)) / 2, SCREEN_HEIGHT / 2 + 160, 50, LIGHTGRAY);
		}
		else if (currentState == GAME_OVER)
		{
			DrawCircleGradient(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH * 3 / 5, Fade(RED, 0.4f), ColorBrightness(RED, -0.5f));

			if (player_hitbox.y >= (SCREEN_HEIGHT - player_hitbox.width))
			{
				// Shadow
				DrawText("GAME OVER", (SCREEN_WIDTH - MeasureText("GAME OVER", 150)) / 2 + 9, SCREEN_HEIGHT / 5 + 9, 150, Fade(BLACK, 0.5f));
				DrawText(TextFormat("FINAL SCORE: %u", clear_count), (SCREEN_WIDTH - MeasureText(TextFormat("FINAL SCORE: %u", clear_count), 75)) / 2 + 6, SCREEN_HEIGHT * 2 / 5 + 6, 75, Fade(BLACK, 0.5f));
				DrawText("Press SPACE to try again", (SCREEN_WIDTH - MeasureText("Press SPACE to try again", 50)) / 2 + 6, SCREEN_HEIGHT / 2 + 86, 50, Fade(BLACK, 0.5f));
				DrawText("Press ESC to give up", (SCREEN_WIDTH - MeasureText("Press ESC to give up", 50)) / 2 + 6, SCREEN_HEIGHT / 2 + 146, 50, Fade(BLACK, 0.5f));
				// Text
				DrawText("GAME OVER", (SCREEN_WIDTH - MeasureText("GAME OVER", 150)) / 2, SCREEN_HEIGHT / 5, 150, WHITE);
				DrawText(TextFormat("FINAL SCORE: %u", clear_count), (SCREEN_WIDTH - MeasureText(TextFormat("FINAL SCORE: %u", clear_count), 75)) / 2, SCREEN_HEIGHT * 2 / 5, 75, YELLOW);
				DrawText("Press SPACE to try again", (SCREEN_WIDTH - MeasureText("Press SPACE to try again", 50)) / 2, SCREEN_HEIGHT / 2 + 80, 50, LIGHTGRAY);
				DrawText("Press ESC to give up", (SCREEN_WIDTH - MeasureText("Press ESC to give up", 50)) / 2, SCREEN_HEIGHT / 2 + 140, 50, LIGHTGRAY);

				if (clear_count != high_score || clear_count == 0)
				{
					DrawText(TextFormat("HIGH SCORE: %u", high_score), (SCREEN_WIDTH - MeasureText(TextFormat("HIGH SCORE: %u", high_score), 75)) / 2 + 6, SCREEN_HEIGHT * 2 / 5 + 90 + 6, 75, Fade(BLACK, 0.5f));
					DrawText(TextFormat("HIGH SCORE: %u", high_score), (SCREEN_WIDTH - MeasureText(TextFormat("HIGH SCORE: %u", high_score), 75)) / 2, SCREEN_HEIGHT * 2 / 5 + 90, 75, GREEN);
				}
				else
				{
					DrawText(TextFormat("NEW HIGH SCORE: %u", high_score), (SCREEN_WIDTH - MeasureText(TextFormat("NEW HIGH SCORE: %u", high_score), 75)) / 2 + 6, SCREEN_HEIGHT * 2 / 5 + 90 + 6, 75, Fade(BLACK, 0.5f));
					DrawText(TextFormat("NEW HIGH SCORE: %u", high_score), (SCREEN_WIDTH - MeasureText(TextFormat("NEW HIGH SCORE: %u", high_score), 75)) / 2, SCREEN_HEIGHT * 2 / 5 + 90, 75, GREEN);
					PlaySound(congrats_sfx);
				}
			}
		}
		else if (currentState == CREDITS)
		{
			DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));
			// Shadow
			DrawText("Credits", (SCREEN_WIDTH - MeasureText("Credits", 150)) / 2 + 9, SCREEN_HEIGHT / 5 + 9, 150, Fade(BLACK, 0.5f));
			DrawText("Jacob Reckhard", (SCREEN_WIDTH - MeasureText("Jacob Reckhard", 50)) / 2 + 6, SCREEN_HEIGHT / 2 - 80 + 6, 50, Fade(BLACK, 0.5f));
			DrawText("ManoftheHill7", (SCREEN_WIDTH - MeasureText("ManoftheHill7", 50)) / 2 + 6, SCREEN_HEIGHT / 2 + 6, 50, Fade(BLACK, 0.5f));
			DrawText("ManoftheHill7", 10 + 6, SCREEN_HEIGHT - 30 + 3, 25, Fade(BLACK, 0.5f));
			// Text
			DrawText("Credits", (SCREEN_WIDTH - MeasureText("Credits", 150)) / 2, SCREEN_HEIGHT / 5, 150, GOLD);
			DrawText("Jacob Reckhard", (SCREEN_WIDTH - MeasureText("Jacob Reckhard", 50)) / 2, SCREEN_HEIGHT / 2 - 80, 50, LIGHTGRAY);
			DrawText("ManoftheHill7", (SCREEN_WIDTH - MeasureText("ManoftheHill7", 50)) / 2, SCREEN_HEIGHT / 2, 50, LIGHTGRAY);
			DrawText("Press C to return to menu", 10, SCREEN_HEIGHT - 30, 25, LIGHTGRAY);
		}
		else if (pause)
		{
			DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));
			// Shadow
			DrawText("PAUSED", (SCREEN_WIDTH - MeasureText("PAUSED", 75)) / 2 + 6, SCREEN_HEIGHT / 3 + 6, 75, Fade(BLACK, 0.5f));
			DrawText("Press P to continue", (SCREEN_WIDTH - MeasureText("Press P to continue", 50)) / 2 + 6, SCREEN_HEIGHT / 2 + 80 + 6, 50, Fade(BLACK, 0.5f));
			// Text
			DrawText("PAUSED", (SCREEN_WIDTH - MeasureText("PAUSED", 75)) / 2, SCREEN_HEIGHT / 3, 75, WHITE);
			DrawText("Press P to continue", (SCREEN_WIDTH - MeasureText("Press P to continue", 50)) / 2, SCREEN_HEIGHT / 2 + 80, 50, LIGHTGRAY);
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
	for (Layer &layer : layers)
	{
		UnloadTexture(layer.tex);
	}
	StopMusicStream(bgm);

	UnloadSound(clear_sfx);
	UnloadSound(hit_sfx);
	UnloadSound(swim_sfx);
	UnloadSound(congrats_sfx);
	UnloadMusicStream(bgm);

	CloseAudioDevice();

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
