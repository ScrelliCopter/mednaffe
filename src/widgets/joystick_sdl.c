/* joystick_sdl.c - Copyright 2023 a dinosaur - SPDX: GPL-3.0-or-later */
#include "joystick.h"
#include <SDL2/SDL.h>

// Enable experimental SDL_GameController support
#define ENABLE_SDL_GAMECONTROLLER


#ifdef ENABLE_SDL_GAMECONTROLLER

typedef struct
{
	SDL_GameController* sdlpad;
	SDL_Joystick* sdljoy;
	SDL_GameControllerButton map_button[SDL_CONTROLLER_BUTTON_MAX];
	SDL_GameControllerAxis map_axis[SDL_CONTROLLER_AXIS_MAX];
} GameController;

#endif


static int IntFromStr(const char* const str, long* out)
{
	errno = 0;
	char* end;
	long value = strtol(str, &end, 10);
	if (end == str)
		return 1;
	if (errno == ERANGE)
		return 2;
	*out = value;
	return 0;
}

static const joy_s* GetJoystickFromGUID(GSList* list, const gchar* id)
{
	if (!list || !id) return NULL;
	for (GSList* it = list; it != NULL; it = it->next)
	{
		const joy_s* joy = it->data;
		if (g_strcmp0(id, joy->id) == 0)
			return joy;
	}
	return NULL;
}

#ifdef ENABLE_SDL_GAMECONTROLLER

static gchar* GameControllerButtonDescription(SDL_GameControllerType type, SDL_GameControllerButton button)
{
	if (type == SDL_CONTROLLER_TYPE_PS3 || type == SDL_CONTROLLER_TYPE_PS4 || type == SDL_CONTROLLER_TYPE_PS5)
	{
		switch (button)
		{
		case SDL_CONTROLLER_BUTTON_A: return "X";
		case SDL_CONTROLLER_BUTTON_B: return "◯";
		case SDL_CONTROLLER_BUTTON_X: return "◻";
		case SDL_CONTROLLER_BUTTON_Y: return "△";
		case SDL_CONTROLLER_BUTTON_GUIDE: return "PS";
		case SDL_CONTROLLER_BUTTON_LEFTSTICK: return "L3";
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return "R3";
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return "L1";
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return "R1";
		case SDL_CONTROLLER_BUTTON_MISC1: return "Mic Mute";
		default: break;
		}
	}
	if (type == SDL_CONTROLLER_TYPE_PS4 || type == SDL_CONTROLLER_TYPE_PS5)
	{
		switch (button)
		{
		case SDL_CONTROLLER_BUTTON_START: return "Options";
		case SDL_CONTROLLER_BUTTON_BACK: return type == SDL_CONTROLLER_TYPE_PS4 ? "Share" : "Create";
		default: break;
		}
	}
	if (type == SDL_CONTROLLER_TYPE_XBOX360 || type == SDL_CONTROLLER_TYPE_XBOXONE)
	{
		switch (button)
		{
		case SDL_CONTROLLER_BUTTON_LEFTSTICK: return "LS";
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return "RS";
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return "LB";
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return "RB";
		case SDL_CONTROLLER_BUTTON_MISC1: return "Share";
		case SDL_CONTROLLER_BUTTON_PADDLE1: return "P1";
		case SDL_CONTROLLER_BUTTON_PADDLE2: return "P2";
		case SDL_CONTROLLER_BUTTON_PADDLE3: return "P3";
		case SDL_CONTROLLER_BUTTON_PADDLE4: return "P4";
		default: break;
		}
	}
	if (type == SDL_CONTROLLER_TYPE_XBOXONE)
	{
		switch (button)
		{
		case SDL_CONTROLLER_BUTTON_BACK: return "View";
		case SDL_CONTROLLER_BUTTON_GUIDE: return "Xbox";
		case SDL_CONTROLLER_BUTTON_START: return "Option";
		default: break;
		}
	}

	switch (button)
	{
	case SDL_CONTROLLER_BUTTON_A: return "A";
	case SDL_CONTROLLER_BUTTON_B: return "B";
	case SDL_CONTROLLER_BUTTON_X: return "X";
	case SDL_CONTROLLER_BUTTON_Y: return "Y";
	case SDL_CONTROLLER_BUTTON_BACK: return "Back";
	case SDL_CONTROLLER_BUTTON_GUIDE: return "Guide";
	case SDL_CONTROLLER_BUTTON_START: return "Start";
	case SDL_CONTROLLER_BUTTON_LEFTSTICK: return "Left Stick Press";
	case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return "Right Stick Press";
	case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return "Left Shoulder";
	case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return "Right Shoulder";
	case SDL_CONTROLLER_BUTTON_DPAD_UP: return "D-Pad Up";
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return "D-Pad Down";
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return "D-Pad Left";
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return "D-Pad Right";
	case SDL_CONTROLLER_BUTTON_MISC1: return "Misc 1";
	case SDL_CONTROLLER_BUTTON_PADDLE1: return "Paddle 1";
	case SDL_CONTROLLER_BUTTON_PADDLE2: return "Paddle 2";
	case SDL_CONTROLLER_BUTTON_PADDLE3: return "Paddle 3";
	case SDL_CONTROLLER_BUTTON_PADDLE4: return "Paddle 4";
	case SDL_CONTROLLER_BUTTON_TOUCHPAD: return "Touchpad";
	default: return NULL;
	}
}

static gchar* GameControllerAxisDescription(SDL_GameControllerType type, SDL_GameControllerAxis axis)
{
	if (type == SDL_CONTROLLER_TYPE_PS3 || type == SDL_CONTROLLER_TYPE_PS4 || type == SDL_CONTROLLER_TYPE_PS5)
	{
		switch (axis)
		{
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT: return "L2";
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: return "R2";
		default: break;
		}
	}
	else if (type == SDL_CONTROLLER_TYPE_XBOX360 || type == SDL_CONTROLLER_TYPE_XBOXONE)
	{
		switch (axis)
		{
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT: return "LT";
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: return "RT";
		default: break;
		}
	}
	switch (axis)
	{
	case SDL_CONTROLLER_AXIS_LEFTX: return "Left Stick X";
	case SDL_CONTROLLER_AXIS_LEFTY: return "Left Stick Y";
	case SDL_CONTROLLER_AXIS_RIGHTX: return "Right Stick X";
	case SDL_CONTROLLER_AXIS_RIGHTY: return "Right Stick Y";
	case SDL_CONTROLLER_AXIS_TRIGGERLEFT: return "Left Trigger";
	case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: return "Right Trigger";
	default: return NULL;
	}
}

#endif

gchar* value_to_text(GSList* listjoy, const gchar* value)
{
	gchar** items = g_strsplit(value, " ", 4);
	if (g_strv_length(items) < 3)
	{
		g_strfreev(items);
		return NULL;
	}

	const joy_s* joy = GetJoystickFromGUID(listjoy, items[1]);
	const gchar* devname = (joy) ? joy->name : "Unknown Device";

	gchar* text = NULL;
#ifdef ENABLE_SDL_GAMECONTROLLER
	if (joy && joy->type == 2)
	{
		const GameController* pad = joy->data1;
		long joyvalue;
		if (items[2][0] == 'b' && !IntFromStr(&items[2][7], &joyvalue))
		{
			for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i)
				if (pad->map_button[i] == joyvalue)
				{
					const gchar* description = GameControllerButtonDescription(SDL_GameControllerGetType(pad->sdlpad), i);
					if (description)
						text = g_strconcat("Button ", description, " (", devname, ")", NULL);
					break;
				}
		}
		else if (items[2][0] == 'a' && !IntFromStr(&items[2][4], &joyvalue))
		{
			for (int i = 0; i < SDL_CONTROLLER_AXIS_MAX; ++i)
				if (pad->map_axis[i] == joyvalue)
				{
					const gchar* description = GameControllerAxisDescription(SDL_GameControllerGetType(pad->sdlpad), i);
					if (description)
					{
						const gchar* findpos = strchr(&items[2][4], '+');
						const gchar* findneg = strchr(&items[2][4], '-');
						const gchar* polarity = "";
						if (findpos && findneg) polarity = findpos < findneg ? findpos : findneg;
						else if (findpos) polarity = findpos;
						else if (findneg) polarity = findneg;
						text = g_strconcat("Axis ", description, polarity, " (", devname, ")", NULL);
					}
					break;
				}
		}
	}

	if (!text)
	{
#endif
		if (items[2][0] == 'b')
			text = g_strconcat("Button ", &items[2][7], " (", devname, ")", NULL);
		else if (items[2][0] == 'a')
			text = g_strconcat("Axis ", &items[2][4], " (", devname, ")", NULL);
#ifdef ENABLE_SDL_GAMECONTROLLER
	}
#endif

	g_strfreev(items);
	return text;
}


static gchar* BuildGuid(const gchar* name, gssize namelen, SDL_Joystick* sdljoy)
{
	// Compute MD5 hash of Joystick name
	GChecksum* hash = g_checksum_new(G_CHECKSUM_MD5);
	if (!hash) return NULL;
	guint8 digest[16];
	gsize digestlen = 16;
	g_checksum_update(hash, (guchar*)name, namelen);
	g_checksum_get_digest(hash, &digest[0], &digestlen);
	g_checksum_free(hash);

	// Build unique ID, a joystick ID is computed by taking the first 64 bits of
	//  the MD5 hash of the name and appending total axis, buttons, hats, and balls.
	gchar* guid = g_malloc(35);
	if (!guid) return NULL;
	snprintf(&guid[0], 3, "0x");
	for (int i = 0; i < 8; ++i)
		snprintf(&guid[2 + i * 2], 3, "%02x", (unsigned char)digest[i]);

	unsigned numaxes = SDL_JoystickNumAxes(sdljoy);
	unsigned numbtns = SDL_JoystickNumButtons(sdljoy);
	unsigned numhats = SDL_JoystickNumHats(sdljoy);
	unsigned numball = SDL_JoystickNumBalls(sdljoy);
	snprintf(&guid[18], 17, "%04x%04x%04x%04x", numaxes, numbtns, numhats, numball);

	return guid;
}


#ifdef ENABLE_SDL_GAMECONTROLLER

static void GetButtonMapping(GameController* pad, gchar* token, SDL_GameControllerButton gcbtn)
{
	long joybtn;
	if (token[0] == 'b' && !IntFromStr(&token[1], &joybtn))
		pad->map_button[gcbtn] = joybtn;
}

static void GetAxisMapping(GameController* pad, gchar* token, SDL_GameControllerAxis gcaxis)
{
	long joyaxis;
	if (token[0] == 'a' && !IntFromStr(&token[1], &joyaxis))
		pad->map_axis[gcaxis] = joyaxis;
}

static int GetGameControllerMapping(GameController* pad)
{
	char* mapping = SDL_GameControllerMapping(pad->sdlpad);
	if (!mapping) return 1;

	gchar** items = g_strsplit(mapping, ",", 0);
	const unsigned numitems = g_strv_length(items);
	for (unsigned i = 2; i < numitems; ++i)
	{
		gchar** map = g_strsplit(items[i], ":", 2);
		if (g_strcmp0(map[0], "a") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_A);
		else if (g_strcmp0(map[0], "b") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_B);
		else if (g_strcmp0(map[0], "x") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_X);
		else if (g_strcmp0(map[0], "y") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_Y);
		else if (g_strcmp0(map[0], "back") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_BACK);
		else if (g_strcmp0(map[0], "guide") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_GUIDE);
		else if (g_strcmp0(map[0], "start") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_START);
		else if (g_strcmp0(map[0], "leftstick") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_LEFTSTICK);
		else if (g_strcmp0(map[0], "rightstick") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_RIGHTSTICK);
		else if (g_strcmp0(map[0], "leftshoulder") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
		else if (g_strcmp0(map[0], "rightshoulder") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
		else if (g_strcmp0(map[0], "dpup") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_DPAD_UP);
		else if (g_strcmp0(map[0], "dpdown") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_DPAD_DOWN);
		else if (g_strcmp0(map[0], "dpleft") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_DPAD_LEFT);
		else if (g_strcmp0(map[0], "dpright") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
		else if (g_strcmp0(map[0], "misc1") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_MISC1);
		else if (g_strcmp0(map[0], "paddle1") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_PADDLE1);
		else if (g_strcmp0(map[0], "paddle2") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_PADDLE2);
		else if (g_strcmp0(map[0], "paddle3") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_PADDLE3);
		else if (g_strcmp0(map[0], "paddle4") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_PADDLE4);
		else if (g_strcmp0(map[0], "touchpad") == 0)
			GetButtonMapping(pad, map[1], SDL_CONTROLLER_BUTTON_TOUCHPAD);
		else if (g_strcmp0(map[0], "leftx") == 0)
			GetAxisMapping(pad, map[1], SDL_CONTROLLER_AXIS_LEFTX);
		else if (g_strcmp0(map[0], "lefty") == 0)
			GetAxisMapping(pad, map[1], SDL_CONTROLLER_AXIS_LEFTY);
		else if (g_strcmp0(map[0], "rightx") == 0)
			GetAxisMapping(pad, map[1], SDL_CONTROLLER_AXIS_RIGHTX);
		else if (g_strcmp0(map[0], "righty") == 0)
			GetAxisMapping(pad, map[1], SDL_CONTROLLER_AXIS_RIGHTY);
		else if (g_strcmp0(map[0], "lefttrigger") == 0)
			GetAxisMapping(pad, map[1], SDL_CONTROLLER_AXIS_TRIGGERLEFT);
		else if (g_strcmp0(map[0], "righttrigger") == 0)
			GetAxisMapping(pad, map[1], SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
	}

	g_strfreev(items);
	SDL_free(mapping);
	return 0;
}

static joy_s* OpenGameController(int id)
{
	SDL_GameController* sdlpad = SDL_GameControllerOpen(id);
	if (!sdlpad) return NULL;

	GameController* pad = NULL;
	gchar* joyname = NULL, * guid = NULL;

	pad = g_new0(GameController, 1);
	pad->sdlpad = sdlpad;
	pad->sdljoy = SDL_GameControllerGetJoystick(sdlpad);
	for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i)
		pad->map_button[i] = SDL_CONTROLLER_BUTTON_INVALID;
	for (int i = 0; i < SDL_CONTROLLER_AXIS_MAX; ++i)
		pad->map_axis[i] = SDL_CONTROLLER_AXIS_INVALID;

	const char* name = SDL_GameControllerName(sdlpad);
	if (!name) goto OpenGameControllerError;
	const gsize namelen = strlen(name);
	if (!(joyname = g_strndup(name, namelen))) goto OpenGameControllerError;

	guid = BuildGuid(name, namelen, pad->sdljoy);
	if (!guid) goto OpenGameControllerError;

	if (GetGameControllerMapping(pad)) goto OpenGameControllerError;

	joy_s* joy = g_new0(joy_s, 1);
	if (!joy) goto OpenGameControllerError;
	joy->num   = (gint)SDL_JoystickGetDeviceInstanceID(id);
	joy->type  = 2;
	joy->name  = joyname;
	joy->id    = guid;
	joy->data1 = (gpointer)pad;
	return joy;

OpenGameControllerError:
	g_free(guid);
	g_free(joyname);
	g_free(pad);
	SDL_GameControllerClose(sdlpad);
	return NULL;
}

#endif

static joy_s* OpenJoystick(int id)
{
	SDL_Joystick* sdljoy = SDL_JoystickOpen(id);
	if (!sdljoy) return NULL;

	gchar* joyname = NULL, * guid = NULL;

	const char* name = SDL_JoystickName(sdljoy);
	if (!name) goto OpenJoystickError;
	const gsize namelen = strlen(name);
	if (!(joyname = g_strndup(name, namelen))) goto OpenJoystickError;

	guid = BuildGuid(name, namelen, sdljoy);
	if (!guid) goto OpenJoystickError;

	joy_s* joy = g_new0(joy_s, 1);
	if (!joy) goto OpenJoystickError;
	joy->num   = (gint)SDL_JoystickGetDeviceInstanceID(id);
	joy->type  = 1;
	joy->name  = joyname;
	joy->id    = guid;
	joy->data1 = (gpointer)sdljoy;
	return joy;

OpenJoystickError:
	g_free(guid);
	g_free(joyname);
	SDL_JoystickClose(sdljoy);
	return NULL;
}

GSList* init_joys(void)
{
	if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0)
		return NULL;

	GSList* joylist = NULL;
	const int numjoy = SDL_NumJoysticks();
	for (int i = 0; i < numjoy; ++i)
	{
#ifdef ENABLE_SDL_GAMECONTROLLER
		joy_s* joy = SDL_IsGameController(i) ? OpenGameController(i) : OpenJoystick(i);
#else
		joy_s* joy = OpenJoystick(i);
#endif
		if (!joy)
			continue;

		joylist = g_slist_append(joylist, joy);
	}
	return joylist;
}

void close_joys(GSList* list)
{
	for (GSList* it = list; it != NULL; it = it->next)
	{
		joy_s* joy = it->data;
		if (joy->type == 1)
		{
			SDL_JoystickClose(joy->data1);
		}
#ifdef ENABLE_SDL_GAMECONTROLLER
		else if (joy->type == 2)
		{
			GameController* pad = (GameController*)joy->data1;
			SDL_GameControllerClose(pad->sdlpad);
			g_free(pad);
		}
#endif
		g_free(joy->name);
		g_free(joy->id);
		g_free(joy);
	}

	SDL_Quit();
}


gchar* ReadJoystick(const gchar* joyid, SDL_Joystick* sdljoy)
{
	unsigned numaxes = SDL_JoystickNumAxes(sdljoy);
	//unsigned numball = SDL_JoystickNumBalls(sdljoy);
	unsigned numbtns = SDL_JoystickNumButtons(sdljoy);
	//unsigned numhats = SDL_JoystickNumHats(sdljoy);

	for (unsigned i = 0; i < numaxes; ++i)
	{
		Sint16 zero;
		if (SDL_JoystickGetAxisInitialState(sdljoy, i, &zero) == SDL_FALSE)
			zero = 0;

		Sint16 axis =  SDL_JoystickGetAxis(sdljoy, i);
		if (zero <= -0x4000)
		{
			if (axis >= 0)
				return g_strdup_printf("joystick %s abs_%u-+", joyid, i);
		}
		else if (zero >= 0x4000)
		{
			if (axis < 0)
				return g_strdup_printf("joystick %s abs_%u+-", joyid, i);
		}
		else if (axis <= -0x4000 || axis >= 0x4000)
		{
			const gchar polarity = axis < 0 ? '-' : '+';
			return g_strdup_printf("joystick %s abs_%u%c", joyid, i, polarity);
		}
	}

	/*
	for (unsigned i = 0; i < numball; ++i)
	{
		int dx, dy;
		int ballstate = SDL_JoystickGetBall(sdljoy, i, &dx, &dy);
	}
	*/

	for (unsigned i = 0; i < numbtns; ++i)
	{
		if (SDL_JoystickGetButton(sdljoy, i))
			return g_strdup_printf("joystick %s button_%u", joyid, i);
	}

	/*
	for (unsigned i = 0; i < numhats; ++i)
	{
		Uint8 hatstate = SDL_JoystickGetHat(sdljoy, i);
	}
	*/

	return NULL;
}

#ifdef ENABLE_SDL_GAMECONTROLLER

gchar* ReadGameController(const gchar* joyid, GameController* pad)
{
	SDL_GameController* sdlpad = pad->sdlpad;

	for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i)
	{
		if (SDL_GameControllerGetButton(sdlpad, i) == SDL_PRESSED && pad->map_button[i] > SDL_CONTROLLER_BUTTON_INVALID)
			return g_strdup_printf("joystick %s button_%u", joyid, pad->map_button[i]);
	}

	for (int i = 0; i < SDL_CONTROLLER_AXIS_MAX; ++i)
	{
		if (pad->map_axis[i] <= SDL_CONTROLLER_AXIS_INVALID) continue;
		Sint16 axis =  SDL_GameControllerGetAxis(sdlpad, i);
		if ((i == SDL_CONTROLLER_AXIS_TRIGGERLEFT || i == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) && axis >= 0x4000)
		{
			return g_strdup_printf("joystick %s abs_%u-+", joyid, pad->map_axis[i]);
		}
		else if (axis <= -0x4000 || axis >= 0x4000)
		{
			const gchar polarity = axis < 0 ? '-' : '+';
			return g_strdup_printf("joystick %s abs_%u%c", joyid, pad->map_axis[i], polarity);
		}
	}

	//return ReadJoystick(joyid, pad->sdljoy);
	return NULL;
}

#endif

gchar* read_joys(GSList* list)
{
	SDL_PumpEvents();

	gchar* read = NULL;
	for (GSList* it = list; it != NULL; it = it->next)
	{
		joy_s* joy = it->data;
		if (joy->type == 1)
			read = ReadJoystick(joy->id, (SDL_Joystick*)joy->data1);
#ifdef ENABLE_SDL_GAMECONTROLLER
		else if (joy->type == 2)
			read = ReadGameController(joy->id, (GameController*)joy->data1);
#endif
		if (read) break;
	}
	return read;
}

void discard_read(GSList* list)
{
}
