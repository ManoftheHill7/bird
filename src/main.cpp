#include "raylib.h"
#include "resource_dir.h" // utility header for SearchAndSetResourceDir
#include "player.h"

#include <list>
#include <vector>

const float SCREEN_WIDTH = 1920;
const float SCREEN_HEIGHT = 1080;
const float OBSTACLE_WIDTH = 100;
const float OBSTACLE_LENGTH = 500;
const Vector2 OBSTACLE_DIMENSIONS = {OBSTACLE_WIDTH, OBSTACLE_LENGTH};
const float OBSTACLE_GAP = SCREEN_HEIGHT - 400;
const float OBSTACLE_SPEED = 300;
const float SCALE = 4;

float tick;

class Obstacle
{
public:
	float x, y, gap;
	static Texture obstacle_image;

	Obstacle(float X, float Y, float Gap)
	{
		x = X;
		y = Y;
		gap = Gap;
	}

	// Update()
	// {

	// }
};

std::list<Obstacle *> obstacles;
Texture Obstacle::obstacle_image;

class Whale
{
private:
	Vector2 player_pos;
	Vector2 player_vel;
	float player_deg;

	Whale()
	{
		player_pos = {SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2};
		player_vel = {0.0f, 0.0f};
		player_deg = 0;
	}
	Whale(const Whale &) = delete;
	Whale &operator=(const Whale &) = delete;

public:
	static Texture whale;

	static Whale &Get()
	{
		static Whale instance; // Created only once, the first time this is called.
		return instance;
	}

	void Jump()
	{
		player_vel.y -= 800.0f;
		player_deg = std::min(player_deg, -20.0f);
	}

	void Move(float delta)
	{
		player_vel.y += GRAVITY * delta;
		player_pos.y += player_vel.y * delta;
		player_pos.y = std::min(player_pos.y, SCREEN_HEIGHT - whale.height * SCALE);
		player_pos.y = std::max(player_pos.y, 0.0f);
		player_deg += delta * 100;
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

		if (player_pos.y == SCREEN_HEIGHT - whale.height * SCALE)
		{
			player_vel.y = std::min(player_vel.y, 0.0f);
		}
	}

	void Draw()
	{
		DrawTextureEx(whale, player_pos, player_deg, SCALE, WHITE);
	}
};

Texture Whale::whale;

int main()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Floppy Whale");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("assets");

	// Load a texture from the resources directory
	Whale::whale = LoadTexture("art/sprites/whale1.png");
	Obstacle::obstacle_image = LoadTexture("art/sprites/whale2.png");

	// Variables

	Obstacle *i = new Obstacle(SCREEN_WIDTH, 0, OBSTACLE_GAP);
	obstacles.push_back(i);

	// game loop
	while (!WindowShouldClose()) // run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		float delta = GetFrameTime();
		tick += 10 * delta;
		Whale::Get().Move(delta);

		if (IsKeyPressed(KEY_SPACE))
		{
			Whale::Get().Jump();
		}

		for (Obstacle *o : obstacles)
		{
			o->x -= OBSTACLE_SPEED * delta;
			DrawTextureEx(o->obstacle_image, {o->x, o->y}, 0.0f, SCALE, WHITE);
			DrawTextureEx(o->obstacle_image, {o->x, o->y + o->gap}, 0.0f, SCALE, WHITE);
		}

		if (tick > 40)
		{
			Obstacle *n = new Obstacle(SCREEN_WIDTH, 0, OBSTACLE_GAP);
			obstacles.push_back(n);
			tick = 0;
		}

		if ((obstacles.front()->x < 0 - OBSTACLE_WIDTH) && (obstacles.size() > 1))
		{
			obstacles.pop_front();
		}

		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLUE);

		// draw some text using the default font
		// DrawText(screen_dimensions, 200,200,20,WHITE);
		// DrawText(character_loc_ptn_text, 600,200,20,WHITE);
		DrawFPS(100.0f, 100.0f);

		// draw our texture to the screen
		Whale::Get().Draw();
		// TODO draw obstacles

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(Whale::whale);
	UnloadTexture(Obstacle::obstacle_image);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
