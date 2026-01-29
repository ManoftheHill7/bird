#include "raylib.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir




int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(1920, 1080, "bird");
	
	// test code
	Vector2 character_loc = {(float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2};
	const char* screen_dimensions = TextFormat("%i x %i", GetScreenWidth(), GetScreenHeight());

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("assets/art/sprites");

	// Load a texture from the resources directory
	Texture wabbit = LoadTexture("wabbit_alpha.png");

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		// draw some text using the default font
		DrawText(screen_dimensions, 200,200,20,WHITE);

		// draw our texture to the screen
		DrawTextureEx(wabbit, character_loc, 0.0, 3.0, WHITE);
		
		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(wabbit);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
