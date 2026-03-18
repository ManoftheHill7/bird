#include "raylib.h"
#include "resource_dir.h" // utility header for SearchAndSetResourceDir
#include "player.h"

#include <list>
#include <vector>
#include <random>
#include <string>

const bool DEBUG_RENDER = true;
const float SCREEN_WIDTH = 1920;
const float SCREEN_HEIGHT = 1080;
const float OBSTACLE_GAP = 952;
const float OBSTACLE_SPEED = 300;
const float SCALE = 4;

const int MARGIN = 20;
const int FONT_SIZE = 40;

float obstacle_width;
float obstacle_length;
int rand_width;
int rand_gap;


class Obstacle
{
	// private:
	// 	float obstacle_tick;

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

	// static void Image()
	// {

	// }

	// static void Update(float deltaTime, float obstacle_tick)
	// {

	// }
	// static void UnloadObstacle()
	// {
	// 	UnloadTexture(obstacle_image);
	// }
};

std::list<Obstacle *> obstacles;
Texture Obstacle::obstacle_image;
Rectangle Obstacle::obstacle_top_hitbox;
Rectangle Obstacle::obstacle_bottom_hitbox;
Rectangle Obstacle::obstacle_clear_hitbox;

class Whale
{
private:
	std::vector<Texture> frames;
	int currentFrame = 7;
	u_int clear_count = 0;
	bool clear_flag = false;
	bool hit_flag = false;
	float frameTimer = 0.0f;
	Vector2 player_pos;
	Vector2 player_vel;
	float player_deg;
	Rectangle player_hitbox;

	Whale()
	{
		player_pos = {SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2};
		player_vel = {0.0f, 0.0f};
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

	void Animation()
	{
		for (int i = 1; i < 9; i++)
		{
			// Build path: "art/sprites/whale[1-8].png"
			frames.push_back(LoadTexture(TextFormat("art/sprites/whale%d.png", i)));
		}
	}

	void Jump()
	{
		player_vel.y -= 800.0f;
		player_deg = std::min(player_deg, -20.0f);
	}

	Rectangle Move(float deltaTime)
	{
		player_vel.y += GRAVITY * deltaTime;
		player_pos.y += player_vel.y * deltaTime;
		player_pos.y = std::min(player_pos.y, SCREEN_HEIGHT - frames[0].height * SCALE);
		player_pos.y = std::max(player_pos.y, 0.0f);
		player_deg += deltaTime * 100;
		player_hitbox = {player_pos.x, player_pos.y, frames[0].width * SCALE, frames[0].height * SCALE};
		if (player_vel.y < -200.0f)
		{
			player_deg = std::min(player_deg, -20.0f);
		}
		else
		{
			player_deg = std::min(player_deg, 80.0f);
		}
		player_deg = std::max(player_deg, -20.0f);

		if (player_pos.y == 0)
		{
			player_vel.y = 0;
		}

		if (player_pos.y == SCREEN_HEIGHT - frames[0].height * SCALE)
		{
			player_vel.y = std::min(player_vel.y, 0.0f);
		}
		return (player_hitbox);
	}

	unsigned int Update(float deltaTime, bool collision_clear, bool collision_hit)
	{
		frameTimer += deltaTime;

		if (IsKeyPressed(KEY_SPACE))
		{
			Whale::Get().Jump();
			currentFrame = 0;
		}
		if (frameTimer >= 0.1)
		{
			frameTimer = 0.0f;
			if (currentFrame < frames.size() - 1)
				currentFrame++;
		}

		// For when the player enters the gap
		if (!clear_flag)
		{
			if (collision_clear)
			{
				clear_count++;
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
					DrawRectanglePro(player_hitbox, {0.0f, 0.0f}, player_deg, GREEN);
				}
			}
		}

		// For when the player hits an obstacle
		if (!hit_flag)
		{
			if (collision_hit)
			{
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
					DrawRectanglePro(player_hitbox, {0.0f, 0.0f}, player_deg, RED);
				}
			}
		}
		return (clear_count);
	}

	void DrawWhale()
	{
		DrawTextureEx(frames[currentFrame], player_pos, player_deg, SCALE, WHITE);

		if (DEBUG_RENDER)
		{
			DrawRectanglePro(player_hitbox, {0.0f, 0.0f}, player_deg, Color{230, 41, 55, 127});
		}
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

int main()
{
	unsigned int clear_count;
	bool collision_clear;
	bool collision_hit;
	Rectangle player_hitbox;

	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Floppy Whale");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("assets");

	// Load a texture from the resources directory
	Whale::Get().Animation();
	Obstacle::obstacle_image = LoadTexture("art/sprites/rock.png");
	obstacle_width = (float)Obstacle::obstacle_image.width * SCALE;
	obstacle_length = (float)Obstacle::obstacle_image.height * SCALE;

	Obstacle *i = new Obstacle();
	obstacles.push_back(i);

	// Generate random number
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> obstacle_distribution(0, 384);

	// game loop
	while (!WindowShouldClose()) // run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		float deltaTime = GetFrameTime();

		player_hitbox = Whale::Get().Move(deltaTime);

		if ((obstacles.back()->x < SCREEN_WIDTH * 3 / 4))
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

		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLUE);

		// draw some text using the default font
		// DrawText(screen_dimensions, 200,200,20,WHITE);
		// DrawText(character_loc_ptn_text, 600,200,20,WHITE);

		// DrawFPS(100.0f, 100.0f);

		collision_clear = false;
		collision_hit = false;
		for (Obstacle *o : obstacles)
		{
			o->x -= OBSTACLE_SPEED * deltaTime;
			DrawTextureEx(o->obstacle_image, {o->x, o->y}, 0.0f, SCALE, WHITE);
			DrawTextureEx(o->obstacle_image, {o->x, o->y + o->gap}, 0.0f, SCALE, WHITE);
			Obstacle::obstacle_top_hitbox = {o->x, o->y, obstacle_width, obstacle_length};
			Obstacle::obstacle_bottom_hitbox = {o->x, o->y + o->gap, obstacle_width, obstacle_length};
			Obstacle::obstacle_clear_hitbox = {o->x + obstacle_width * 3 / 4, o->y + obstacle_length, obstacle_width / SCALE, o->gap - obstacle_length};

			if (CheckCollisionRecs(player_hitbox, Obstacle::obstacle_clear_hitbox))
			{
				collision_clear = true;
			}
			if (CheckCollisionRecs(player_hitbox, Obstacle::obstacle_top_hitbox) || CheckCollisionRecs(player_hitbox, Obstacle::obstacle_bottom_hitbox))
			{
				collision_hit = true;
			}

			//Debug
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
		clear_count = Whale::Get().Update(deltaTime, collision_clear, collision_hit);

		DrawText(TextFormat("Cleared: %u", clear_count), MARGIN, MARGIN - 5, FONT_SIZE, YELLOW);

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	Whale::Get().~Whale();
	// Obstacle::UnloadObstacle;
	UnloadTexture(Obstacle::obstacle_image);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
