// Ty Reid
// Sky High Spy game made using PlayBuffer
// ------------------------------------------

// Implementing the PlayBuffer and activating the PlayManager (important code):
# define PLAY_IMPLEMENTATION // Implementing the PlayBuffer
# define PLAY_USING_GAMEOBJECT_MANAGER // Activating the PlayManager
# include "Play.h" // Including the "Play.h" header file

// PSEUDO CODE:
// ------------------------------------------
// Sky High Spy: As Agent 8
// ------------------------------------------
// Goal: Retreive valuable gems from inside asteroids
// Run around the surface of moving asteroids
// Jump off asteroids, blowing them apart and releasing their gem
// Dodge flaming meteorites to collect gems and land on the next asteroid
// ------------------------------------------
// BEHAVIOURS:
// GEMS:
// If it is created outside of the visible screen area then teleport it inside the visible screen area
// If the player collides with a gem then destroy the gem and increase the player's score
// METEORS:
// A meteor should move forward in the direction it is facing
// If a meteor is completely outside of the screen it should wrap around to the opposite side
// If it collides with Agent 8 then kill the player
// ASTEROID PIECE:
// It should move forward in the direction it is facing
// If it is completely outside of the screen it should destroy itself
// ASTEROID:
// It should move forward in the direction it is facing
// If completely outside of the screen it should wrap around to the opposite side
// AGENT 8:
// Agent 8 has three mutually exclusive behaviours (so only one can occur at a time)...
// ... Flying, where Agent 8 should move forward in the direction he is facing and be able to turn by a moderate amount...
// ... Attached, where Agent 8 will follow the meteor's position and be able to run around the meteor's surface...
// ... Dead, where Agent will continue to move forward in the direction he is facing and the game will restart when he is completely outside of the screen
// While Agent 8 is still alive he can wrap around the screen!
// ------------------------------------------
// FINITE STATE MACHINES:
// These are great for mutually exclusive behaviours (such as with Agent 8)
// START GAME >>> collide with asteroid >>> STATE ATTACHED >>> jump (<<< collide <<<) >>> STATE FLYING >>> collide (meteor) >>> STATE DEAD >>> outside screen >>> END GAME
// You can create these by using a case statement, just like in the original SpyWire example!
// ------------------------------------------
// UPDATE ORDER:
// When Agent 8's game state (finite state) is attatched:
// Agent 8 and the asteroid need to move in unison
// Have a special ASTEROID_ATTACHED GameObjectType
// Change an ASTEROID into an ATTACHED_ASTEROID on collision with Agent 8
// Every frame:
// Set Agent 8's position to the ATTACHED_ASTEROID'S...
// ... The ASTEROIDS need to be updated first (such as their position)...
// ... Otherwise Agent 8 will be one frame behind!
// ------------------------------------------
// SPRITE ORIGINS:
// The "origin" of a sprite is the position on the sprite that it is manipulated from (for drawing and rotating)
// By default this is set to [0,0] (the top left corner)
// PlayBlitter::CentreAllSpriteOrigins() is typically usefull
// PlayBlitter::SetSpriteOrigin() can be used for individual tweaks (such as making meteors rotate around their centre of mass)
// Note that the origin doesnt need to be inside the sprite!
// The origin of Agent 8's walking sprite needs to be below the sprite boundary so that he rotates around the asteroid!
// ------------------------------------------

// GAME STATES AND STRUCTURES:

enum Agent8State // An ennumeration for Agent 8's finite states!
{
	STATE_APPEAR = 0, // For when Agent 8 drops in from the top of the display window!
	STATE_FLYING, // When Agent 8 is flying through space
	STATE_ATTACHED, // When Agent 8 is attached to an asteroid (collides with an asteroid)
	STATE_LAUNCHING, // When Agent 8 has launched of an asteroid
	STATE_DEAD, // When Agent 8 collides with a meteor
};

struct GameState // A global scope structure used to 'remember' the overall state of the game!
{
	// General global game values:

	float timer = 0; // Timer
	int level = 1; // Current level counter
	int spriteId = 0; // ID for the 'current' sprite
	int score = 0; // Current score counter
	int gemCount = 1; // Number of gems remaining counter
	
	// radians for rotaion:
	float rad = 0;
	
	// Boolean values for asteroid spawn conditions:

	bool asteroids_TL = TRUE;
	bool asteroids_TR = TRUE;
	bool asteroids_BL = TRUE;
	bool asteroids_BR = TRUE;

	// Boolean values for meteor spawn conditions:

	bool meteors_L1 = TRUE;
	bool meteors_R1 = TRUE;
	bool meteors_L2 = TRUE;
	bool meteors_R2 = TRUE;

	Agent8State agentState = STATE_APPEAR; // Adding Agent 8's different states as data members of the GameState struct
};
GameState gameState;

// An enumeration to represent the GameObject types in Sky High Spy:
enum GameObjectType 
{
	TYPE_NULL = -1,

	TYPE_AGENT8, // This enumeration will automatically assign the next numerical value (0 in this case) to TYPE_AGENT8, our first GameObject type!
	
	TYPE_ASTEROID, // Asteroid GameObjects
	TYPE_ASTEROID_ATTACHED, // Agent 8 attached to an asteroid GameObject
	TYPE_BROKEN_ASTEROID, // Exploded asteroid pieces GameObjects

	TYPE_GEM, // Gem GameObjects

	TYPE_METEOR, // Meteor GameObjects

	TYPE_DESTROYED, // Destroyed GameObjects
};

// GLOBAL VARIABLES:

float x_asteroidAttached_velocity = {0};
float y_asteroidAttached_velocity = {0};
float asteroidAttached_rotation = {0};

// DECLARING GAME FUNCTIONS:

void HandleFlightControls();

void HandleSpinControls();

void UpdateAsteroids();

void UpdateAsteroidAttached();

void UpdateBrokenAsteroidPieces();

void UpdateGems();

void UpdateMeteors();

void UpdateDestroyed();

void UpdateAgent8();

void UpdateGameLevel();

// CREATING A DISPLAY AREA FOR THE GAME:

// ------------------------------------------
// Pseudo code ideas for the game display area:
// Display area dimensions must be 1280x720 pixels
// Display area must be always on and displaying the "space and Earth" background image
// ------------------------------------------

int DISPLAY_WIDTH {1280}; // Variables for the display area dimensions
int DISPLAY_HEIGHT { 720 };
double DISPLAY_SCALE { 1 };

// This is the Windows entry point for a PlayBuffer program:

void MainGameEntry ( PLAY_IGNORE_COMMAND_LINE ) // Function MainGameEntry (uses a macro to skip the command line interface?)
{
	Play::CreateManager ( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE ); // Calling the PlayManager to create a game display of the chosen dimensions

	Play::CentreAllSpriteOrigins(); // Sets the local origin and the centre of each sprite to its centre << radial collisions will be detected from the centre as well!
	
	Play::LoadBackground("Data\\Backgrounds\\spr_background.png"); // Loads the chosen PNG image as the main background (note that a double backslash means an actual backslash)
																								
	Play::StartAudioLoop("snd_music"); // Automatically scans the Data\\Audio directory and plays the first file named "snd_music"

	Play::CreateGameObject(TYPE_AGENT8, { DISPLAY_WIDTH /2, 720 }, 20, "spr_agent8_fly"); // This creates our Agent 8 object!
}

// UPDATING THE GAME:

// The game update function (with LEVEL COUNTER):

bool MainGameUpdate ( float elapsedTime ) // Uses the elapsed time to update the game (including the display area) 60 times per second (60 fps)
{
	//Play::ClearDrawingBuffer ( Play::cCyan ); // Clearing the previous background and drawing the new one each frame!

	Play::DrawBackground(); // Replaces Play::ClearDrawingBuffer() as both completely reset the drawing buffer to draw a new one each frame!

	UpdateAgent8();

	UpdateGameLevel();

	UpdateAsteroids();

	UpdateAsteroidAttached();

	UpdateBrokenAsteroidPieces();

	UpdateGems();

	UpdateMeteors();

	//HandleFlightControls();

	//HandleSpinControls();

	UpdateDestroyed();

	// Testing drawing with the font sprites:
	//Play::DrawFontText( "64px", "Sky High Spy Game!", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, Play::CENTRE ); // Displaying text in the centre of the display area!
	
	// Drawing the background in the display window:
	Play::PresentDrawingBuffer (); // Adding the new background drawn in the DrawingBuffer to the display window
	return Play::KeyDown ( VK_ESCAPE ); // End the game update function (close the game) when the escape key is pressed (virtual key) 
}

// FLYING CONTROLS:

void HandleFlightControls()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8); // Most update functions will begin by retreiving a reference to the player's GameObject!

	// Controls for flying:
	// While flying agent 8 moves in the direction he is facing and can turn by a moderate amount!

	obj_agent8.rotSpeed = 0.0f; // Set Agent 8's rotation to 0 when no directional keys are being pressed!
	obj_agent8.velocity = { 1, -2 }; // Agent 8's constant flight velocity is [ 1, -2 ] (upwards right direction)!

	if (Play::KeyDown(VK_RIGHT) && gameState.agentState != STATE_DEAD && gameState.agentState != STATE_ATTACHED)
	{
		obj_agent8.rotSpeed = 0.015f; // Veer Agent 8 to rotate to the right when the right arrow key is pressed!
		obj_agent8.velocity  *= { 1.5f, 0.5f };
		Play::SetSprite(obj_agent8, "spr_agent8_fly", 1.0f); // Set the sprite animation to play!
	}

	else if (Play::KeyDown(VK_LEFT) && gameState.agentState != STATE_DEAD && gameState.agentState != STATE_ATTACHED)
	{
		obj_agent8.rotSpeed = -0.015f; // Veer Agent 8 to rotate to the left when the left arrow key is pressed!
		obj_agent8.velocity *= { -1.5f, 0.5f };
		Play::SetSprite(obj_agent8, "spr_agent8_fly", 1.0f); // Set the sprite animation to play!
	}

	// Wrapping Agent 8 around the display area:

	// Don't update noObject:
	if (obj_agent8.type == -1) return;
	{
		// Saving the current position in case we need to go back:
		obj_agent8.oldPos = obj_agent8.pos;
		obj_agent8.oldRot = obj_agent8.rotation;
		// Moving Agent 8 according to a very simple physical model:
		obj_agent8.velocity += obj_agent8.acceleration;
		obj_agent8.pos += obj_agent8.velocity;
		obj_agent8.rotation += obj_agent8.rotSpeed;
		// Handling the animation frame update:
		obj_agent8.framePos += obj_agent8.animSpeed;
	}
	if (obj_agent8.framePos > 1.0f)
	{
		obj_agent8.frame++;
		obj_agent8.framePos -= 1.0f;
	}
	// Wrapping Agent 8 around the screen (display area):
	int dWidth = PlayWindow::Instance().GetWidth(); // Store the width of the display area!
	int dHeight = PlayWindow::Instance().GetHeight(); // Store the height of the display area!
	Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_agent8.spriteId);
	if (obj_agent8.pos.x - origin.x - 50 > dWidth) // If Agent 8 is leaving the display area horizontally to the right
	{
		obj_agent8.pos.x = 0.0f - 50 + origin.x;
	}
	else if (obj_agent8.pos.x + origin.x + 50 < 0) // If Agent 8 is leaving the display area horizontally to the left
	{
		obj_agent8.pos.x = dWidth + 50 - origin.x;
	}
	if (obj_agent8.pos.y - origin.y - 50 > dHeight) // If Agent 8 is leaving the display area vertically towards the bottom
	{
		obj_agent8.pos.y = 0.0f - 50 + origin.y;
	}
	else if (obj_agent8.pos.y + origin.y + 50 < 0) // If Agent 8 is leaving the display area vertically towards the top
	{
		obj_agent8.pos.y = dHeight + 50 - origin.y;
	}

	
	Play::SetSprite(obj_agent8, "spr_agent8_fly", 1.0f);
}

// SPINNING CONTROLS:

void HandleSpinControls()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8); // Most update functions will begin by retreiving a reference to the player's GameObject!
	GameObject& obj_asteroid_attached = Play::GetGameObjectByType(TYPE_ASTEROID_ATTACHED); // Retrieving a reference to the asteroid_attached GameObject!
	
	// Making Agent 8 stick to the asteroid:
	if (gameState.agentState == STATE_ATTACHED && !Play::KeyDown(VK_RIGHT) && !Play::KeyDown(VK_LEFT) && !Play::KeyDown(VK_SPACE))
	{
		obj_agent8.rotation = (4.75f); // Orient Agent 8's sprite to be level when he lands on an asteroid!
		obj_agent8.pos = obj_asteroid_attached.pos + Vector2D(20, -55); // Make Agent 8 stick to the top of the asteroid!
	}

	// Making Agent 8 rotate around the asteroid:
	if (Play::KeyDown(VK_RIGHT) && gameState.agentState == STATE_ATTACHED && gameState.agentState != STATE_DEAD && gameState.agentState != STATE_FLYING)
	{
		obj_agent8.pos = obj_asteroid_attached.pos; // Get Agent 8's position on the asteroid
		Play::SetSprite(obj_agent8, "spr_agent8_right_strip7", 0.7f); // Switch to Agent 8's crawling right animation!

		gameState.rad += 0.025; // Incrent the radians (angle of rotation)

		int adjacent = (cos(gameState.rad) * 67); // Find Agent 8's new x-coordiante
		int opposite = (sin(gameState.rad) * 67); // Find Agent 8's new y-coordinate

		obj_agent8.pos.x = obj_agent8.pos.x + adjacent; // Move Agent 8's x position
		obj_agent8.pos.y = obj_agent8.pos.y + opposite; // Move Agent 8's y position

		obj_agent8.rotation = gameState.rad; // Rotate Agent 8's sprite at the same rate as he is moving!
	
	} 

	else if (Play::KeyDown(VK_LEFT) && gameState.agentState == STATE_ATTACHED && gameState.agentState != STATE_DEAD && gameState.agentState != STATE_FLYING)
	{		
		obj_agent8.pos = obj_asteroid_attached.pos; // Get Agent 8's position on the asteroid
		Play::SetSprite(obj_agent8, "spr_agent8_left_strip7", 0.7f); // Switch to Agent 8's crawling left animation!

		gameState.rad -= 0.025; // Incrent the radians (angle of rotation)

		int adjacent = (cos(gameState.rad) * 67); // Find Agent 8's new x-coordiante
		int opposite = (sin(gameState.rad) * 67); // Find Agent 8's new y-coordinate

		obj_agent8.pos.x = obj_agent8.pos.x - adjacent; // Move Agent 8's x position
		obj_agent8.pos.y = obj_agent8.pos.y + opposite; // Move Agent 8's y position

		obj_agent8.rotation = (3 - gameState.rad); // Rotate Agent 8's sprite at the same rate as he is moving!
	}

	// Launching from an asteroid:
	else if (Play::KeyDown(VK_SPACE) && gameState.agentState == STATE_ATTACHED  && gameState.agentState != STATE_DEAD && gameState.agentState != STATE_FLYING)
	{
		Play::SetSprite(obj_agent8, "spr_agent8_fly", 1.0f);

		Play::PlayAudio("snd_combust"); // Play the Agent 8 launch sound effect!
		Play::PlayAudio("snd_explode"); // Play the exploding asteroid sound effect!

		// Making a gem:

		int id = Play::CreateGameObject(TYPE_GEM, obj_asteroid_attached.pos, 25, "spr_gem");
		GameObject& obj_gem = Play::GetGameObject(id);
		obj_gem.velocity = Point2f(Play::RandomRollRange(-1, 1) * 0.75f, Play::RandomRollRange(-1, 1) * 0.75f);
		obj_gem.rotSpeed = 0.05f;

		// Broken asteroid pieces:

		for (float rad{ 0.0f }; rad < 2.0f; rad += 0.66666f) // cycle through 0.0, 0.66666 (recurring) and 1.33333 (recurring) rad (for broken asteroid placement)
		{
			int id = Play::CreateGameObject(TYPE_BROKEN_ASTEROID, obj_asteroid_attached.pos, 0, "spr_asteroid_pieces_strip3");
			GameObject& obj_broken_asteroid_piece = Play::GetGameObject(id);
			obj_broken_asteroid_piece.rotSpeed = 0.1f;
			obj_broken_asteroid_piece.acceleration = { 0.0f, 0.0f }; // Let each piece of the broken asteroid accelerate in its given direction!
			Play::SetGameObjectDirection(obj_broken_asteroid_piece, 16, rad * PLAY_PI);
		}

		// Switching game states:
		gameState.agentState = STATE_LAUNCHING; // Switch Agent 8's state so that he launches of the asteroid!
		obj_asteroid_attached.type = TYPE_DESTROYED; // Destroy the old asteroid_attached GameObject!
	}
	
	// Wrapping Agent 8 around the display area:
	// Don't update noObject:
	if (obj_agent8.type == -1) return;
	{
		// Saving the current position in case we need to go back:
		obj_agent8.oldPos = obj_agent8.pos;
		obj_agent8.oldRot = obj_agent8.rotation;
		// Moving Agent 8 according to a very simple physical model:
		obj_agent8.velocity += obj_agent8.acceleration;
		obj_agent8.pos += obj_agent8.velocity;
		obj_agent8.rotation += obj_agent8.rotSpeed;
		// Handling the animation frame update:
		obj_agent8.framePos += obj_agent8.animSpeed;
	}
	if (obj_agent8.framePos > 1.0f)
	{
		obj_agent8.frame++;
		obj_agent8.framePos -= 1.0f;
	}
	// Wrapping Agent 8 around the screen (display area):
	int dWidth = PlayWindow::Instance().GetWidth(); // Store the width of the display area!
	int dHeight = PlayWindow::Instance().GetHeight(); // Store the height of the display area!
	Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_agent8.spriteId);
	if (obj_agent8.pos.x - origin.x - 50 > dWidth) // If Agent 8 is leaving the display area horizontally to the right
	{
		obj_agent8.pos.x = 0.0f - 50 + origin.x;
	}
	else if (obj_agent8.pos.x + origin.x + 50 < 0) // If Agent 8 is leaving the display area horizontally to the left
	{
		obj_agent8.pos.x = dWidth + 50 - origin.x;
	}
	if (obj_agent8.pos.y - origin.y - 50 > dHeight) // If Agent 8 is leaving the display area vertically towards the bottom
	{
		obj_agent8.pos.y = 0.0f - 50 + origin.y;
	}
	else if (obj_agent8.pos.y + origin.y + 50 < 0) // If Agent 8 is leaving the display area vertically towards the top
	{
		obj_agent8.pos.y = dHeight + 50 - origin.y;
	}
}

// UPDATING ASTEROIDS:

void UpdateAsteroids()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8); // This reference to Agent 8 is used to detect collisions between Agent 8 and asteroids!

	// Spawning asteroids from each corner of the display area (4 corners):

	// The number of asteroids depends on the player's current level...
	// ... Remember we only want 1 asteroid to spawn in each corner (need to limit number of asteroids)!

	// Top left (if the player is on level 1 or higher)!:

	if (gameState.level >= 1 && (Play::RandomRoll(100) == 19) && gameState.asteroids_TL == TRUE) // Roll a 100 sided dice every frame (60 times per second) and check to see if it lands on the number 19 (asteroid spawn rate)
	{
		int id = Play::CreateGameObject(TYPE_ASTEROID, { Play::RandomRollRange(0, 150), 0 }, 100, "spr_asteroid_strip2"); // Type, position, collision radius, sprite name!
		// In the line above the asteroid can be created around the top left corner (from 0 to 150 pixels)!
		GameObject& obj_asteroid = Play::GetGameObject(id);
		Play::SetSprite( obj_asteroid, "spr_asteroid_strip2", 0.9f ); // Set the animation for the asteroid to play!
		obj_asteroid.velocity = { 1, 1 }; // The components of the asteroid's velocity are always [1,1] (slow enough velocity to collide with)!
		obj_asteroid.rotation = (1.0f); // Set the asteroid's rotation so that it faces its direction of movement!
		gameState.asteroids_TL = FALSE; // Boolean condition used to make only one asteroid!
	} 

	// Top right:
	
	if (gameState.level >= 3 && (Play::RandomRoll(100) == 23) && gameState.asteroids_TR == TRUE)
	{
		int id = Play::CreateGameObject(TYPE_ASTEROID, { Play::RandomRollRange(1130, 1280), 0 }, 100, "spr_asteroid_strip2"); // Type, position, collision radius, sprite name!
		// In the line above the asteroid can be created around the top right corner!
		GameObject& obj_asteroid = Play::GetGameObject(id);
		Play::SetSprite(obj_asteroid, "spr_asteroid_strip2", 0.9f); // Set the animation for the asteroid to play!
		obj_asteroid.velocity = Point2f(-1, 1);
		obj_asteroid.rotation = (2.5f);
		gameState.asteroids_TR = FALSE; // Boolean condition used to make only one asteroid!
	}
		
	// Bottom left:

	if (gameState.level >= 4 && (Play::RandomRoll(100) == 24) && gameState.asteroids_BL == TRUE)
	{
		int id = Play::CreateGameObject(TYPE_ASTEROID, { Play::RandomRollRange(0, 150), 720 }, 100, "spr_asteroid_strip2"); // Type, position, collision radius, sprite name!
		// In the line above the asteroid can be created around the bottom left corner!
		GameObject& obj_asteroid = Play::GetGameObject(id);
		Play::SetSprite(obj_asteroid, "spr_asteroid_strip2", 0.9f); // Set the animation for the asteroid to play!
		obj_asteroid.velocity = Point2f(1, -1);
		obj_asteroid.rotation = (5.5f);
		gameState.asteroids_BL = FALSE; // Boolean condition used to make only one asteroid!
	}

	// Bottom right:

	if (gameState.level >=2 && (Play::RandomRoll(100) == 42) && gameState.asteroids_BR == TRUE) // Roll a 100 sided dice every frame (60 times per second) and check to see if it lands on the number 42 (asteroid spawn rate)
	{
		int id = Play::CreateGameObject(TYPE_ASTEROID, { Play::RandomRollRange(1130, 1280), 720 }, 100, "spr_asteroid_strip2"); // Type, position, collision radius, sprite name!
		// In the line above the asteroid can be created around the bottom right corner!
		GameObject& obj_asteroid = Play::GetGameObject(id);
		Play::SetSprite(obj_asteroid, "spr_asteroid_strip2", 0.9f); // Set the animation for the asteroid to play!
		obj_asteroid.velocity = Point2f(-1, -1);
		obj_asteroid.rotation = (4.0f);
		gameState.asteroids_BR = FALSE; // Boolean condition used to make only one asteroid!
	}

	// Asteroid behaviour (collisions and wrapping around the screen):

	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID); // Create a vector to store all of the asteroids!

	// Updating all asteroids:

	for (int id : vAsteroids)
	{
		GameObject& obj_asteroid = Play::GetGameObject(id);
		// Don't update noObject:
		if (obj_asteroid.type == -1) return; 
		// Saving the current position in case we need to go back:
		obj_asteroid.oldPos = obj_asteroid.pos;
		obj_asteroid.oldRot = obj_asteroid.rotation;
		// Moving the asteroid according to a very simple physical model:
		obj_asteroid.velocity += obj_asteroid.acceleration;
		obj_asteroid.pos += obj_asteroid.velocity;
		obj_asteroid.rotation += obj_asteroid.rotSpeed;
		// Handling the animation frame update:
		obj_asteroid.framePos += obj_asteroid.animSpeed;
		if (obj_asteroid.framePos > 1.0f)
		{
			obj_asteroid.frame++;
			obj_asteroid.framePos -= 1.0f;
		}
		
		// Wrapping asteroids around the screen (display area):

		int dWidth = PlayWindow::Instance().GetWidth(); // Store the width of the display area!
		int dHeight = PlayWindow::Instance().GetHeight(); // Store the height of the display area!
		Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_asteroid.spriteId);
		if (obj_asteroid.pos.x - origin.x - 50 > dWidth) // If the asteroid is leaving the display area horizontally to the right
		{
			obj_asteroid.pos.x = 0.0f - 50 + origin.x;
		}
		else if (obj_asteroid.pos.x + origin.x + 50 < 0) // If the asteroid is leaving the display area horizontally to the left
		{
			obj_asteroid.pos.x = dWidth + 50 - origin.x;
		}
		if (obj_asteroid.pos.y - origin.y - 50 > dHeight) // If the asteroid is leaving the display area vertically towards the bottom
		{
			obj_asteroid.pos.y = 0.0f - 50 + origin.y;
		}
		else if (obj_asteroid.pos.y + origin.y + 50 < 0) // If the asteroid is leaving the display area vertically towards the top
		{
			obj_asteroid.pos.y = dHeight + 50 - origin.y;
		}

		// Collision conditions:

		if (gameState.agentState != STATE_DEAD && gameState.agentState != STATE_ATTACHED && Play::IsColliding(obj_asteroid, obj_agent8))
		{
			// Making an asteroid_attached Gameobject:
			int id = Play::CreateGameObject( TYPE_ASTEROID_ATTACHED, { obj_asteroid.pos.x, obj_asteroid.pos.y }, 50, "spr_asteroid_strip2" );
			GameObject& obj_asteroid_attached = Play::GetGameObject(id);
			Play::SetSprite(obj_asteroid_attached, "spr_asteroid_strip2", 0.9f); // Set the animation for the asteroid to play!

			// Storing the original asteroid's movement information to use for asteroid_attached:
			x_asteroidAttached_velocity = obj_asteroid.velocity.x;
			y_asteroidAttached_velocity = obj_asteroid.velocity.y;
			asteroidAttached_rotation = obj_asteroid.rotation;

			obj_asteroid.type = TYPE_DESTROYED; // Destroy the asteroid object since we are making a new one which Agent 8 can crawl across!
			gameState.agentState = STATE_ATTACHED; // Change Agent 8's gameState to STATE_ATTACHED!
		}
		
		// Updating and drawing each asteroid:

		Play::UpdateGameObject(obj_asteroid); // Called to process the asteroid's movement!
		Play::DrawObjectRotated(obj_asteroid); // Called to draw the asteroid so it can be updated to the display area!

		// Update asteroid_attached in the asteroid_attached function!!!!!
	}
}

// UPDATING ASTEROID_ATTACHED:

void UpdateAsteroidAttached()
{
	GameObject& obj_asteroid_attached = Play::GetGameObjectByType(TYPE_ASTEROID_ATTACHED); // Retrieving a reference to the asteroid_attached GameObject!

	obj_asteroid_attached.velocity = { x_asteroidAttached_velocity, y_asteroidAttached_velocity }; // The components of the asteroid's velocity are always [1,1] (slow enough velocity to collide with)!
	obj_asteroid_attached.rotation = asteroidAttached_rotation; // Set the asteroid's rotation so that it faces its direction of movement!

	// Wrapping asteroid_attached around the display area:
	// Don't update noObject:
	if (obj_asteroid_attached.type == -1) return;
	{
		// Saving the current position in case we need to go back:
		obj_asteroid_attached.oldPos = obj_asteroid_attached.pos;
		obj_asteroid_attached.oldRot = obj_asteroid_attached.rotation;
		// Moving asteroid_attached according to a very simple physical model:
		obj_asteroid_attached.velocity += obj_asteroid_attached.acceleration;
		obj_asteroid_attached.pos += obj_asteroid_attached.velocity;
		obj_asteroid_attached.rotation += obj_asteroid_attached.rotSpeed;
		// Handling the animation frame update:
		obj_asteroid_attached.framePos += obj_asteroid_attached.animSpeed;
	}
	if (obj_asteroid_attached.framePos > 1.0f)
	{
		obj_asteroid_attached.frame++;
		obj_asteroid_attached.framePos -= 1.0f;
	}
	// Wrapping asteroid_attached around the screen (display area):
	int dWidth = PlayWindow::Instance().GetWidth(); // Store the width of the display area!
	int dHeight = PlayWindow::Instance().GetHeight(); // Store the height of the display area!
	Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_asteroid_attached.spriteId);
	if (obj_asteroid_attached.pos.x - origin.x - 50 > dWidth) // If asteroid_attached is leaving the display area horizontally to the right
	{
		obj_asteroid_attached.pos.x = 0.0f - 50 + origin.x;
	}
	else if (obj_asteroid_attached.pos.x + origin.x + 50 < 0) // If asteroid_attached is leaving the display area horizontally to the left
	{
		obj_asteroid_attached.pos.x = dWidth + 50 - origin.x;
	}
	if (obj_asteroid_attached.pos.y - origin.y - 50 > dHeight) // If asteroid_attached is leaving the display area vertically towards the bottom
	{
		obj_asteroid_attached.pos.y = 0.0f - 50 + origin.y;
	}
	else if (obj_asteroid_attached.pos.y + origin.y + 50 < 0) // If asteroid_attached is leaving the display area vertically towards the top
	{
		obj_asteroid_attached.pos.y = dHeight + 50 - origin.y;
	}

	Play::SetSprite(obj_asteroid_attached, "spr_asteroid_strip2", 0.9f); // Make sure that the sprite and animation are working!

	Play::UpdateGameObject(obj_asteroid_attached);
	Play::DrawObjectRotated(obj_asteroid_attached);
}

// UPDATING BROKE ASTEROID PIECES:

void UpdateBrokenAsteroidPieces()
{
	std::vector<int> vBrokenAsteroidPieces = Play::CollectGameObjectIDsByType(TYPE_BROKEN_ASTEROID);

	for (int id_identifier : vBrokenAsteroidPieces) // Simple update loop for the broken asteroid pieces
	{
		GameObject& obj_broken_asteroid_piece = Play::GetGameObject(id_identifier);

		Play::UpdateGameObject(obj_broken_asteroid_piece);
		Play::DrawObjectRotated(obj_broken_asteroid_piece);

		if (!Play::IsVisible(obj_broken_asteroid_piece))
			Play::DestroyGameObject(id_identifier);
	}
}

// UPDATING GEMS:

void UpdateGems()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vGems = Play::CollectGameObjectIDsByType(TYPE_GEM);

	for (int id_identifier : vGems) // Simple update loop for the gems
	{
		GameObject& obj_gem = Play::GetGameObject(id_identifier);

		// Don't update noObject:
		if (obj_gem.type == -1) return;
		// Saving the current position in case we need to go back:
		obj_gem.oldPos = obj_gem.pos;
		obj_gem.oldRot = obj_gem.rotation;
		// Moving the gem according to a very simple physical model:
		obj_gem.velocity += obj_gem.acceleration;
		obj_gem.pos += obj_gem.velocity;
		obj_gem.rotation += obj_gem.rotSpeed;
		// Handling the animation frame update:
		obj_gem.framePos += obj_gem.animSpeed;
		if (obj_gem.framePos > 1.0f)
		{
			obj_gem.frame++;
			obj_gem.framePos -= 1.0f;
		}

		// Wrapping gems around the screen (display area):

		int dWidth = PlayWindow::Instance().GetWidth(); // Store the width of the display area!
		int dHeight = PlayWindow::Instance().GetHeight(); // Store the height of the display area!
		Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_gem.spriteId);
		if (obj_gem.pos.x - origin.x - 50 > dWidth) // If the gem is leaving the display area horizontally to the right
		{
			obj_gem.pos.x = 0.0f - 50 + origin.x;
		}
		else if (obj_gem.pos.x + origin.x + 50 < 0) // If the gem is leaving the display area horizontally to the left
		{
			obj_gem.pos.x = dWidth + 50 - origin.x;
		}
		if (obj_gem.pos.y - origin.y - 50 > dHeight) // If the gem is leaving the display area vertically towards the bottom
		{
			obj_gem.pos.y = 0.0f - 50 + origin.y;
		}
		else if (obj_gem.pos.y + origin.y + 50 < 0) // If the gem is leaving the display area vertically towards the top
		{
			obj_gem.pos.y = dHeight + 50 - origin.y;
		}

		// Collision conditions:

		if (gameState.agentState != STATE_DEAD && Play::IsColliding(obj_gem, obj_agent8))
		{
			// Adding to the player's score:
			Play::PlayAudio("snd_reward");
			gameState.score += 500;
			gameState.gemCount -= 1;

			obj_gem.type = TYPE_DESTROYED; // Destroy the asteroid object since we are making a new one which Agent 8 can crawl across!
		}

		Play::UpdateGameObject(obj_gem);
		Play::DrawObjectRotated(obj_gem);

	}
}

// UPDATING METEORS:

void UpdateMeteors()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8); // This reference to Agent 8 is used to detect collisions between Agent 8 and meteors!

	// Spawning meteors from each side of the display area (2 meteors from each side):

	// The number of meteors depends on the player's current level...
	// ... Remember we only want 2 meteors to spawn each side in total (need to limit number of meteors)!

	// Bottom left (if the player is on level 1 or higher)!:

	if (gameState.level >= 1 && (Play::RandomRoll(100) == 19) && gameState.meteors_L1 == TRUE) // Roll a 100 sided dice every frame (60 times per second) and check to see if it lands on the number 19 (meteor spawn rate)
	{
		int id = Play::CreateGameObject( TYPE_METEOR, { 0, Play::RandomRollRange( 540, 690 ) }, 50, "spr_meteor_strip2" ); // Type, position, collision radius, sprite name!
		// In the line above the meteor can be created around the top left corner (from 0 to 150 pixels)!
		GameObject& obj_meteor = Play::GetGameObject(id);
		Play::SetSprite(obj_meteor, "spr_meteor_strip2", 0.95f); // Set the animation for the meteor to play!
		obj_meteor.velocity = Point2f( 3, 0 ); // The meteors velocity components!
		obj_meteor.rotation = (6.35f); // Set the meteor's rotation so that it faces its direction of movement!
		gameState.meteors_L1 = FALSE; // Boolean condition used to make only one meteor!
	}

	// Top right:

	if (gameState.level >= 2 && (Play::RandomRoll(100) == 23) && gameState.meteors_R1 == TRUE)
	{
		int id = Play::CreateGameObject(TYPE_METEOR, { 0, Play::RandomRollRange( 70, 240 ) }, 50, "spr_meteor_strip2"); // Type, position, collision radius, sprite name!
		// In the line above the meteor can be created around the top left corner (from 0 to 150 pixels)!
		GameObject& obj_meteor = Play::GetGameObject(id);
		Play::SetSprite(obj_meteor, "spr_meteor_strip2", 0.95f); // Set the animation for the meteor to play!
		obj_meteor.velocity = Point2f( -3, 0 ); // The meteors velocity components!
		obj_meteor.rotation = (3.2f);
		gameState.meteors_R1 = FALSE; // Boolean condition used to make only one meteor!
	}

	// Bottom right:

	if (gameState.level >= 3 && (Play::RandomRoll(100) == 24) && gameState.meteors_R2 == TRUE)
	{
		int id = Play::CreateGameObject(TYPE_METEOR, { 0, Play::RandomRollRange( 400, 520 ) }, 50, "spr_meteor_strip2"); // Type, position, collision radius, sprite name!
		// In the line above the meteor can be created around the top left corner (from 0 to 150 pixels)!
		GameObject& obj_meteor = Play::GetGameObject(id);
		Play::SetSprite(obj_meteor, "spr_meteor_strip2", 0.95f); // Set the animation for the meteor to play!
		obj_meteor.velocity = Point2f( -3, 0 ); // The meteors velocity components!
		obj_meteor.rotation = (3.2f);
		gameState.meteors_R2 = FALSE; // Boolean condition used to make only one meteor!
	}

	// Top left:

	if (gameState.level >= 4 && (Play::RandomRoll(100) == 42) && gameState.meteors_L2 == TRUE)
	{
		int id = Play::CreateGameObject(TYPE_METEOR, { 0, Play::RandomRollRange( 250, 390 ) }, 50, "spr_meteor_strip2"); // Type, position, collision radius, sprite name!
		// In the line above the meteor can be created around the top left corner (from 0 to 150 pixels)!
		GameObject& obj_meteor = Play::GetGameObject(id);
		Play::SetSprite(obj_meteor, "spr_meteor_strip2", 0.95f); // Set the animation for the meteor to play!
		obj_meteor.velocity = Point2f(3, 0); // The meteors velocity components!
		obj_meteor.rotation = (6.35f);
		gameState.meteors_L2 = FALSE; // Boolean condition used to make only one meteor!
	}

	// Meteor behaviour (collisions and wrapping around the screen):

	std::vector<int> vMeteors = Play::CollectGameObjectIDsByType(TYPE_METEOR); // Create a vector to store all of the asteroids!

	// Updating all meteors:

	for (int id : vMeteors)
	{
		GameObject& obj_meteor = Play::GetGameObject(id);
		// Don't update noObject:
		if (obj_meteor.type == -1) return;
		// Saving the current position in case we need to go back:
		obj_meteor.oldPos = obj_meteor.pos;
		obj_meteor.oldRot = obj_meteor.rotation;
		// Moving the meteor according to a very simple physical model:
		obj_meteor.velocity += obj_meteor.acceleration;
		obj_meteor.pos += obj_meteor.velocity;
		obj_meteor.rotation += obj_meteor.rotSpeed;
		// Handling the animation frame update:
		obj_meteor.framePos += obj_meteor.animSpeed;
		if (obj_meteor.framePos > 1.0f)
		{
			obj_meteor.frame++;
			obj_meteor.framePos -= 1.0f;
		}

		// Wrapping meteors around the screen (display area):

		int dWidth = PlayWindow::Instance().GetWidth(); // Store the width of the display area!
		int dHeight = PlayWindow::Instance().GetHeight(); // Store the height of the display area!
		Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_meteor.spriteId);
		if (obj_meteor.pos.x - origin.x - 50 > dWidth) // If the meteor is leaving the display area horizontally to the right
		{
			obj_meteor.pos.x = 0.0f - 50 + origin.x;
		}
		else if (obj_meteor.pos.x + origin.x + 50 < 0) // If the meteor is leaving the display area horizontally to the left
		{
			obj_meteor.pos.x = dWidth + 50 - origin.x;
		}
		if (obj_meteor.pos.y - origin.y - 50 > dHeight) // If the meteor is leaving the display area vertically towards the bottom
		{
			obj_meteor.pos.y = 0.0f - 50 + origin.y;
		}
		else if (obj_meteor.pos.y + origin.y + 50 < 0) // If the meteor  is leaving the display area vertically towards the top
		{
			obj_meteor.pos.y = dHeight + 50 - origin.y;
		}

		// Collision conditions:

		if ( gameState.agentState != STATE_DEAD && gameState.agentState != STATE_ATTACHED && Play::IsColliding(obj_meteor, obj_agent8) )
		{
			obj_meteor.type = TYPE_DESTROYED;
			Play::StopAudioLoop("snd_music"); // Stop the main game background music when a collision occurs...
			Play::PlayAudio("snd_clang"); // ... Instead play a death sound effect:(
			// It does not make sense to destroy the player as he would need to be created again to restart the game anyway!
			gameState.agentState = STATE_DEAD;
		}
			
		// Updating and drawing each meteor:

		Play::UpdateGameObject(obj_meteor); // Called to process the meteor's movement!
		Play::DrawObjectRotated(obj_meteor); // Called to draw the meteor so it can be updated to the display area!
	}
}

// UPDATING DESTROYED OBJECTS:

void UpdateDestroyed() // Picks up controll of destroyed object types!
{
	std::vector<int> vDead = Play::CollectGameObjectIDsByType(TYPE_DESTROYED);

	for (int id_dead : vDead)
	{
		GameObject& obj_dead = Play::GetGameObject(id_dead);
		obj_dead.animSpeed = 0.2f; // Reduces destroyed objects animation speed so it actually lives for longer (reduced frame rate)
		Play::UpdateGameObject(obj_dead);

		if (obj_dead.frame % 2) // Checks for odd and even frames!
		{
			Play::DrawObjectRotated(obj_dead, (10 - obj_dead.frame) / 10.0f); // Creates a gradual fading out effect!
		}

		if (!Play::IsVisible(obj_dead) || obj_dead.frame >= 10)
		{
			Play::DestroyGameObject(id_dead);
		}		
	}
}

// UPDATING AGENT 8:

void UpdateAgent8()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	switch (gameState.agentState)
	{
	case STATE_APPEAR:
		obj_agent8.velocity = { 0, -2 }; // Begin Agent 8 moving up the screen at a set velocity
		obj_agent8.acceleration = { 0, -0.15f }; // Add extra acceleration to Agent 8 to replicate the effects of Agent 8 moving in a frictionless environment
		Play::SetSprite(obj_agent8, "spr_agent8_fly", 1.0f);
		obj_agent8.rotation = ( 4.75f ); // Orient Agent 8's sprite correctly at the start of the game
		if (obj_agent8.pos.y <= (DISPLAY_HEIGHT / 3)*2 ) // Switch to STATE_FLYING when Agent 8 reaches a third of the way up the screen (display area)
		{
			obj_agent8.acceleration = { 0, 0 }; // Stop Agent 8 from accelerating when he reaches the middle of the screen (now the player controls movement)!
			gameState.agentState = STATE_FLYING;
		}
		break;

	case STATE_FLYING:
		Play::SetSprite(obj_agent8, "spr_agent8_fly", 1.0f);
		HandleFlightControls(); // Use the HandlePlayerControls() function written above to allow the player to control Agent 8 when he is flying!
		break;

	case STATE_ATTACHED:
		Play::SetSprite(obj_agent8, "spr_agent8_right_strip7", 0.0f); // Swap Agent 8 to his crawling sprite but dont animate him yet!
		HandleSpinControls(); // Use the HandleSpinControls() function written above to allow the player to control Agent 8 while he in on an asteroid!
		UpdateAsteroidAttached(); // Updates the asteroid which Agent 8 is currently attched to!
		break;

	case STATE_LAUNCHING:
		Play::SetSprite(obj_agent8, "spr_agent8_fly", 1.0f);
		gameState.agentState = STATE_FLYING;
		break;

	case STATE_DEAD:
		Play::SetSprite(obj_agent8, "spr_agent8_dead_strip2", 0.95f); // Swap to agent 8's death sprite!
		// Rotate Agent 8 faster since he has been killed (collided with a meteor):
		obj_agent8.rotation += 1.5f;
		// Slow Agent 8 down and eventually destroy his gameObject:
		double last_time = timeGetTime();
		double time_scale = 0.001f;
		while (gameState.agentState == STATE_DEAD)
		{
			obj_agent8.acceleration *= 0.99f; // Slow down Agent 8's acceleration gradually
			obj_agent8.rotation *= 0.99f; // Slow down Agent 8's rotation gradually

			if (Play::KeyPressed(VK_SPACE) == true) // If the space bar is pressed while Agent 8 is dead then he is revived and the game restarts!
			{
				obj_agent8.pos.x = ( DISPLAY_WIDTH / 2 );
				obj_agent8.pos.y = 720;
				gameState.agentState = STATE_APPEAR; // Drops Agent 8 in from the top of the display area again
				Play::StartAudioLoop("snd_music"); // Starts looping the game music again
				gameState.level = 1;
				gameState.score = 0;
				gameState.gemCount = gameState.level;

				// Reset all asteroids and meteors and gems:
				for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_ASTEROID))
				{
					Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
				}
				for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_METEOR))
				{
					Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
				}
				for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_GEM))
				{
					Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
				}
				gameState.asteroids_TL = TRUE;
				gameState.asteroids_TR = TRUE;
				gameState.asteroids_BL = TRUE;
				gameState.asteroids_BR = TRUE;

				gameState.meteors_L1 = TRUE;
				gameState.meteors_R1 = TRUE;
				gameState.meteors_L2 = TRUE;
				gameState.meteors_R2 = TRUE;		
			}
			break;
		}

			break; // End of death switchstate for Agent 8
	} // End of switchState for Agent 8

	Play::UpdateGameObject(obj_agent8);
	Play::DrawObjectRotated(obj_agent8);
}

// UPDATING THE LEVELS:

void UpdateGameLevel()
{
	// LEVEL COUNTER:
	// ------------------------------------------
	if (gameState.level <= 4)
	{
		// Draw game text:
		Play::DrawFontText("64px", "LEVEL: " + std::to_string(gameState.level), { 100, 50 }, Play::CENTRE); // Using DrawFontText() to draw the player's current level!
		Play::DrawFontText("132px", "REMAINING GEMS: " + std::to_string(gameState.gemCount), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE); // Using DrawFontText() to draw the levels remaining gems!
		Play::DrawFontText("64px", "SCORE: " + std::to_string(gameState.score), { 1180, 50 }, Play::CENTRE); // Using DrawFontText() to draw the player's current score!
		Play::DrawFontText("64px", "USE THE ARROW KEYS FOR ROTATION AND THE SPACE BAR TO LAUNCH!", { DISPLAY_WIDTH / 2, 690 }, Play::CENTRE); // Using DrawFontText() to draw the game controls!
	}
	if (gameState.level - gameState.gemCount == gameState.level) // If all of the gems in a level are collected...
	{
		// Reset all asteroids and meteors and gems:
		for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_ASTEROID))
		{
			Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
		}
		for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_METEOR))
		{
			Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
		}
		for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_GEM))
		{
			Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
		}
		gameState.asteroids_TL = TRUE; // Spawn all of the new asteroids meteors and gems for the next level!
		gameState.asteroids_TR = TRUE;
		gameState.asteroids_BL = TRUE;
		gameState.asteroids_BR = TRUE;

		gameState.meteors_L1 = TRUE;
		gameState.meteors_R1 = TRUE;
		gameState.meteors_L2 = TRUE;
		gameState.meteors_R2 = TRUE;

		gameState.level += 1; // ... Progress to the next level:)
		gameState.gemCount = gameState.level; // Add all of the gems into the level!
	}
	else if (gameState.level > 4) // If the current level is higher than the last level in the game
	{
		// Destroy all asteroids and meteors and gems:
		for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_ASTEROID))
		{
			Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
		}
		for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_METEOR))
		{
			Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
		}
		for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_GEM))
		{
			Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
		}
		gameState.asteroids_TL = FALSE;
		gameState.asteroids_TR = FALSE;
		gameState.asteroids_BL = FALSE;
		gameState.asteroids_BR = FALSE;

		gameState.meteors_L1 = FALSE;
		gameState.meteors_R1 = FALSE;
		gameState.meteors_L2 = FALSE;
		gameState.meteors_R2 = FALSE;

		Play::StopAudioLoop("snd_music"); // Stop the main game background music because the player has finished the game

		Play::DrawFontText("132px", "CONGRATULATIONS YOU WIN!!!", { DISPLAY_WIDTH /2, DISPLAY_HEIGHT /2 }, Play::CENTRE); // Congratulations message:)
		Play::DrawFontText("64px", "PRESS THE SPACE BAR TO REPLAY OR THE ESCAPE KEY TO QUIT!", { DISPLAY_WIDTH / 2, 690 }, Play::CENTRE); // Game reset/quit message
		
		if (Play::KeyPressed(VK_SPACE) == true) // Restarting the game!
		{
			gameState.level = 1;
			gameState.gemCount = gameState.level;
			gameState.score = 0;

			gameState.asteroids_TL = TRUE;
			gameState.asteroids_TR = TRUE;
			gameState.asteroids_BL = TRUE;
			gameState.asteroids_BR = TRUE;

			gameState.meteors_L1 = TRUE;
			gameState.meteors_R1 = TRUE;
			gameState.meteors_L2 = TRUE;
			gameState.meteors_R2 = TRUE;

			gameState.agentState = STATE_APPEAR;
			Play::StartAudioLoop("snd_music"); // Start the main game background music because the player has restarted the game
		}
	}
	// ------------------------------------------
}

// This function gets called once when the player quits the game (VK_ESCAPE):

int MainGameExit( void )
{
	Play::DestroyManager(); // Clears all game information to free up space
	return PLAY_OK;
}

// Code to check sprite origins:

// int SOX = Play::GetSpriteOrigin("spr_agent8_left_strip7").x; (sprite origin x)
// int SOY = Play::GetSpriteOrigin("spr_agent8_left_strip7").y; (sprite origin y)

// Play::DrawFontText("64px", "SOX: " + std::to_string(SOX), { 100, 300 }, Play::CENTRE); 
// Play::DrawFontText("64px", "SOY: " + std::to_string(SOY), { 100, 400 }, Play::CENTRE); 