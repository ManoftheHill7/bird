#include "raylib.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include "player.h"

#include <vector>
const float SCREEN_WIDTH = 1920;
const float SCREEN_HEIGHT = 1080;
const float OBSTACLE_WIDTH = 100;
const float OBSTACLE_LENGTH = 1000;
const Vector2 OBSTACLE_DIMENSIONS = {OBSTACLE_WIDTH, OBSTACLE_LENGTH};
const float GAP = SCREEN_HEIGHT - 300;
const float SPEED = 300;
const float SCALE = 4;

float player_deg = 0;

class Obstacle
{
	public:
	float x;
	float top_y;
	float bottom_y;
};
Obstacle* make_obstacle(float x, float y)
{
	Obstacle* o = new Obstacle();
	o->x = x;
	o->top_y = y - GAP / 2;
	o->bottom_y = y + GAP / 2;
	return o;
}

int main ()
{
	Vector2 player_pos = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
	Vector2 player_vel = {0.0f, 0.0f};
	std::vector<Obstacle*> obstacles;
	obstacles.push_back(make_obstacle(SCREEN_WIDTH, SCREEN_HEIGHT / 4));

	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Floppy Whale");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("assets");

	// Load a texture from the resources directory
	Texture whale = LoadTexture("art/sprites/whale1.png");

	const float SCREEN_TOP = 0;
	const float SCREEN_BOTTOM = SCREEN_HEIGHT - whale.height * SCALE;

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		float delta = GetFrameTime();
		player_vel.y += GRAVITY * delta;
		player_pos.y += player_vel.y * delta;
		player_pos.y = std::min(player_pos.y, SCREEN_BOTTOM);
		player_pos.y = std::max(player_pos.y, SCREEN_TOP);
		player_deg += delta * 100;

		if (IsKeyPressed(KEY_SPACE))
		{
			player_vel.y -= 800.0f;
			player_deg = std::min(player_deg, -20.0f);
		}

		if (player_vel.y < -200.0f)
		{
			player_deg = std::min(player_deg, -20.0f);
		} else {
			player_deg = std::min(player_deg, 20.0f);
		}
		player_deg = std::max(player_deg, -20.0f);

		if (player_pos.y == SCREEN_TOP)
		{
			player_vel.y = 0;
		}

		if (player_pos.y == SCREEN_BOTTOM)
		{
			player_vel.y = std::min(player_vel.y, 0.0f);
		}

		for (Obstacle* o : obstacles)
		{
			o->x -= SPEED * delta;
			DrawRectangleV({o->x, o->top_y}, OBSTACLE_DIMENSIONS, DARKGREEN);
			DrawRectangleV({o->x, o->bottom_y}, OBSTACLE_DIMENSIONS, DARKGREEN);
		}

		//TODO remove obstacles when too far left
		//TODO add new obstacle when removing

		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLUE);

		// draw some text using the default font
		// DrawText(screen_dimensions, 200,200,20,WHITE);
		// DrawText(character_loc_ptn_text, 600,200,20,WHITE);
		DrawFPS(100.0f, 100.0f);

		// draw our texture to the screen
		DrawTextureEx(whale, player_pos, player_deg, SCALE, WHITE);
		//TODO draw obstacles

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(whale);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}


