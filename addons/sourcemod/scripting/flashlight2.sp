#pragma semicolon 1

#include <sourcemod>
#include <tf2_stocks>
#include <sdkhooks>
#include <sdktools>
#include <clientprefs>

#pragma newdecls required

#define FLASHLIGHT_CLICKSOUND "slender/newflashlight.wav"

#define MAXTF2PLAYERS		36

ConVar CvarColor;
Cookie LightCookie;
bool Fullbright[MAXTF2PLAYERS];
int LightRed[MAXTF2PLAYERS];
int LightBlue[MAXTF2PLAYERS];
int LightGreen[MAXTF2PLAYERS];
int LightLevel[MAXTF2PLAYERS];
int LightWidth[MAXTF2PLAYERS] = {4, ...};
int LightLength[MAXTF2PLAYERS] = {8, ...};
int LightRef[MAXTF2PLAYERS] = {INVALID_ENT_REFERENCE, ...};

public Plugin myinfo =
{
	name		=	"Lobby Flashlight",
	author		=	"Batfoxkid, from Slender Fortress",
	description	=	"Flashlight for Survivors",
	version		=	"manual"
};

public void OnPluginStart()
{
	CvarColor = CreateConVar("flashlight_color", "", "Admin Override for Flashlight Color");
	
	HookEvent("player_death", OnPlayerSpawn, EventHookMode_Pre);
	HookEvent("player_spawn", OnPlayerSpawn, EventHookMode_Pre);
	
	RegConsoleCmd("sm_flashlight", Command, "Personal Lobby Flashlight");
	
	LightCookie = new Cookie("light_cookie", "Your flashlight :)", CookieAccess_Protected);
	
	for(int client = 1; client <= MaxClients; client++)
	{
		if(IsClientInGame(client))
			OnClientPostAdminCheck(client);
	}
}

public void OnPluginEnd()
{
	for(int client = 1; client <= MaxClients; client++)
	{
		if(IsClientInGame(client))
			OnClientDisconnect(client);
	}
}

public void OnClientCookiesCached(int client)
{
	if(CheckCommandAccess(client, "sm_flashlight", 0))
	{
		char buffer[16];
		LightCookie.Get(client, buffer, sizeof(buffer));
		
		if(buffer[0])
		{
			int num = StringToInt(buffer);
			
			CvarColor.GetString(buffer, sizeof(buffer));
			if(!buffer[0] || CheckCommandAccess(client, buffer, ADMFLAG_GENERIC))
			{
				LightRed[client] = num / 100000000;
				num -= (LightLevel[client] * 100000000);
				
				LightGreen[client] = num / 1000000;
				num -= (LightGreen[client] * 1000000);
				
				LightBlue[client] = num / 10000;
				num -= (LightBlue[client] * 10000);
			}
			
			LightLevel[client] = num / 100;
			num -= (LightLevel[client] * 100);
			
			LightLength[client] = num / 10;
			num -= (LightLength[client] * 10);
			
			LightWidth[client] = num;
		}
	}
}

public void OnClientPostAdminCheck(int client)
{
	if(AreClientCookiesCached(client))
		OnClientCookiesCached(client);
}

public void OnClientDisconnect(int client)
{
	if(AreClientCookiesCached(client))
	{
		char buffer[16];
		IntToString((LightRed[client] * 100000000) + (LightGreen[client] * 1000000) + (LightBlue[client] * 10000) + (LightLevel[client] * 100) + (LightLength[client] * 10) + LightWidth[client], buffer, sizeof(buffer));
		LightCookie.Set(client, buffer);
	}
	
	Fullbright[client] = false;
	LightLevel[client] = 0;
	LightLength[client] = 8;
	LightWidth[client] = 4;
	LightRed[client] = 0;
	LightGreen[client] = 0;
	LightBlue[client] = 0;
	
	ClientTurnOffFlashlight(client);
}

public Action Command(int client, int args)
{
	if(client && GetClientTeam(client) != 2)
		FlashlightMenu(client);
	
	return Plugin_Handled;
}

void FlashlightMenu(int client)
{
	Menu menu = new Menu(FlashlightMenuH);
	
	menu.SetTitle("Lobby Flashlight%s\n ", LightLevel[client] ? "\n \nUse +attack3 to toggle your flashlight\n " : "");
	
	char buffer[64];
	FormatEx(buffer, sizeof(buffer), "Brightness: %d%%", LightLevel[client] * 10);
	menu.AddItem(NULL_STRING, buffer);
	
	FormatEx(buffer, sizeof(buffer), "Width: %d%%", 10 + LightWidth[client] * 10);
	menu.AddItem(NULL_STRING, buffer);
	
	FormatEx(buffer, sizeof(buffer), "Length: %d%%", 10 + LightLength[client] * 10);
	menu.AddItem(NULL_STRING, buffer);
	
	CvarColor.GetString(buffer, sizeof(buffer));
	if(!buffer[0] || CheckCommandAccess(client, buffer, ADMFLAG_GENERIC))
	{
		FormatEx(buffer, sizeof(buffer), "Red: %d%%", 100 - LightRed[client] * 10);
		menu.AddItem(NULL_STRING, buffer);
		
		FormatEx(buffer, sizeof(buffer), "Green: %d%%", 100 - LightGreen[client] * 10);
		menu.AddItem(NULL_STRING, buffer);
		
		FormatEx(buffer, sizeof(buffer), "Blue: %d%%\n ", 100 - LightBlue[client] * 10);
		menu.AddItem(NULL_STRING, buffer);
	}
	
	if(CheckCommandAccess(client, "sm_rcon", ADMFLAG_RCON))
		menu.AddItem(NULL_STRING, Fullbright[client] ? "mat_fullbright 1" : "mat_fullbright 0");
	
	menu.Display(client, MENU_TIME_FOREVER);
}

public int FlashlightMenuH(Menu menu, MenuAction action, int client, int choice)
{
	switch(action)
	{
		case MenuAction_End:
		{
			delete menu;
		}
		case MenuAction_Select:
		{
			switch(choice)
			{
				case 0:
				{
					if(++LightLevel[client] > 10)
						LightLevel[client] = 0;
				}
				case 1:
				{
					if(++LightWidth[client] > 9)
						LightWidth[client] = 0;
				}
				case 2:
				{
					if(++LightLength[client] > 9)
						LightLength[client] = 0;
				}
				case 3:
				{
					if(++LightRed[client] > 10)
						LightRed[client] = 0;
				}
				case 4:
				{
					if(++LightGreen[client] > 10)
						LightGreen[client] = 0;
				}
				case 5:
				{
					if(++LightBlue[client] > 10)
						LightBlue[client] = 0;
				}
				case 6:
				{
					if(Fullbright[client])
					{
						FindConVar("sv_cheats").ReplicateToClient(client, "0");
					}
					else
					{
						FindConVar("sv_cheats").ReplicateToClient(client, "1");
						PrintToChat(client, "Use mat_fullbright 1 to enable");
					}
					
					Fullbright[client] = !Fullbright[client];
					FlashlightMenu(client);
					return 0;
				}
			}
			
			FlashlightMenu(client);
			
			if(LightRef[client] != INVALID_ENT_REFERENCE)
			{
				ClientTurnOffFlashlight(client);
				ClientTurnOnFlashlight(client);
			}
		}
	}
	return 0;
}

public Action OnPlayerSpawn(Event event, const char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(event.GetInt("userid"));
	if(client)
		ClientTurnOffFlashlight(client);
	
	return Plugin_Continue;
}

public Action OnPlayerRunCmd(int client, int &buttons, int &impulse, float vel[3], float angles[3], int &weapon)
{
	if(LightLevel[client] && IsPlayerAlive(client) && GetClientTeam(client) == 3)
	{
		int entity = INVALID_ENT_REFERENCE;
		if(LightRef[client] != INVALID_ENT_REFERENCE)
		{
			entity = EntRefToEntIndex(LightRef[client]);
			if(entity != INVALID_ENT_REFERENCE)
			{
				TeleportEntity(entity, NULL_VECTOR, view_as<float>({ 0.0, 0.0, 0.0 }), NULL_VECTOR);

				if(GetClientHealth(client) < 31)
				{
					SetEntProp(entity, Prop_Data, "m_LightStyle", 10);
				}
				else
				{
					SetEntProp(entity, Prop_Data, "m_LightStyle", 0);
				}
			}
			else
			{
				LightRef[client] = INVALID_ENT_REFERENCE;
			}
		}

		static bool holding[MAXTF2PLAYERS];
		if(holding[client])
		{
			if(!(buttons & IN_ATTACK3))
				holding[client] = false;

			return Plugin_Continue;
		}
		else if(buttons & IN_ATTACK3)
		{
			ClientCommand(client, "playgamesound " ... FLASHLIGHT_CLICKSOUND);
			holding[client] = true;
			
			ClientTurnOffFlashlight(client);
			if(entity == INVALID_ENT_REFERENCE)
				ClientTurnOnFlashlight(client);
		}
	}
	return Plugin_Continue;
}

void ClientTurnOnFlashlight(int client)
{
	float flEyePos[3];
	GetClientEyePosition(client, flEyePos);
	
	// Spawn the light which only the user will see.
	int ent = CreateEntityByName("light_dynamic");
	if (ent != -1)
	{
		TeleportEntity(ent, flEyePos, NULL_VECTOR, NULL_VECTOR);
		DispatchKeyValue(ent, "targetname", "WUBADUBDUBMOTHERBUCKERS");
		DispatchKeyValue(ent, "rendercolor", "255 255 255");
		SetVariantFloat((LightWidth[client] + 1.0) * 128.0);
		AcceptEntityInput(ent, "spotlight_radius");
		SetVariantFloat((LightLength[client] + 1.0) * 128.0);
		AcceptEntityInput(ent, "distance");
		SetVariantInt(LightLevel[client] - 1);
		AcceptEntityInput(ent, "brightness");
		
		static int color[4] = {255, ...};
		color[0] = 255 - (25 * LightRed[client]);
		color[1] = 255 - (25 * LightGreen[client]);
		color[2] = 255 - (25 * LightBlue[client]);
		SetVariantColor(color);
		AcceptEntityInput(ent, "Color");
		
		// Convert WU to inches.
		float cone = 55.0;
		cone *= 0.75;
		
		SetVariantInt(RoundToFloor(cone));
		AcceptEntityInput(ent, "_inner_cone");
		SetVariantInt(RoundToFloor(cone));
		AcceptEntityInput(ent, "_cone");
		DispatchSpawn(ent);
		ActivateEntity(ent);
		SetVariantString("!activator");
		AcceptEntityInput(ent, "SetParent", client);
		AcceptEntityInput(ent, "TurnOn");
		
		LightRef[client] = EntIndexToEntRef(ent);
		
		SDKHook(ent, SDKHook_SetTransmit, Hook_FlashlightSetTransmit);
	}
}

void ClientTurnOffFlashlight(int client)
{
	int ent = EntRefToEntIndex(LightRef[client]);
	if(ent && ent != INVALID_ENT_REFERENCE) 
	{
		AcceptEntityInput(ent, "TurnOff");
		AcceptEntityInput(ent, "Kill");
	}
	
	LightRef[client] = INVALID_ENT_REFERENCE;
}

public Action Hook_FlashlightSetTransmit(int ent, int other)
{
	return EntRefToEntIndex(LightRef[other])==ent ? Plugin_Continue : Plugin_Handled;
}