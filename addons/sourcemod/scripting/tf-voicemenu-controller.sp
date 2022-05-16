#pragma semicolon 1

#include <sourcemod>
#include <menus-controller>

#pragma newdecls required

#define MAXTF2PLAYERS	36

int InMenu[MAXTF2PLAYERS];

public void OnPluginStart()
{
	if(GetEngineVersion() != Engine_TF2)
		SetFailState("Plugin only available in TF2");
	
	RegConsoleCmd("sm_voicemenu", Command_Voicemenu, "TF2 voicemenu but with controller support");
	
	RegConsoleCmd("menu_up", Command_MenuUp, "Move up an item on supported menus");
	RegConsoleCmd("menu_next", Command_MenuNext, "Move to the next page on supported menus");
	RegConsoleCmd("menu_down", Command_MenuDown, "Move down an item on supported menus");
	RegConsoleCmd("menu_back", Command_MenuBack, "Move to back a page on supported menus");
	RegConsoleCmd("menu_select", Command_MenuSelect, "Select current item on supported menus");
}

public Action Command_Voicemenu(int client, int args)
{
	if(!client)
		return Plugin_Continue;
	
	Menu menu = new Menu(Handler_Voicemenu);
	
	int mode = 1;
	switch(InMenu[client])
	{
		case 1:
		{
			mode = 2;
			
			menu.AddItem("1 0", "Incoming");
			menu.AddItem("1 1", "Spy!");
			menu.AddItem("1 2", "Sentry Ahead!");
			menu.AddItem("1 3", "Teleporter Here");
			menu.AddItem("1 4", "Dispenser Here");
			menu.AddItem("1 5", "Sentry Here");
			menu.AddItem("1 6", "Activate Charge!");
			menu.AddItem("1 7", "MEDIC: ÜberCharge Ready");
			menu.AddItem("1 8", "Pass to me!");
		}
		case 2:
		{
			mode = 3;
			
			menu.AddItem("2 0", "Help!");
			menu.AddItem("2 1", "Battle Cry");
			menu.AddItem("2 2", "Cheers");
			menu.AddItem("2 3", "Jeers");
			menu.AddItem("2 4", "Positive");
			menu.AddItem("2 5", "Negative");
			menu.AddItem("2 6", "Nice Shot");
			menu.AddItem("2 7", "Goob Job");
		}
		default:
		{
			menu.AddItem("0 0", "MEDIC!");
			menu.AddItem("0 1", "Thanks!");
			menu.AddItem("0 2", "Go! Go! Go!");
			menu.AddItem("0 3", "Move Up!");
			menu.AddItem("0 4", "Go Left");
			menu.AddItem("0 5", "Go Right");
			menu.AddItem("0 6", "Yes");
			menu.AddItem("0 7", "No");
			menu.AddItem("0 8", "Pass to me!");
		}
	}
	
	menu.Pagination = MENU_NO_PAGINATION;
	menu.ExitButton = true;
	menu.OptionFlags |= MENUFLAG_NO_SOUND;
	menu.Display(client, MENU_TIME_FOREVER);
	
	InMenu[client] = mode;
	return Plugin_Handled;
}

public int Handler_Voicemenu(Menu menu, MenuAction action, int param1, int param2)
{
	switch(action)
	{
		case MenuAction_Cancel:
		{
			InMenu[param1] = 0;
		}
		case MenuAction_Select:
		{
			InMenu[param1] = 0;
			
			char buffer[4];
			if(menu.GetItem(param2, buffer, sizeof(buffer)) && buffer[0])
				FakeClientCommand(param1, "voicemenu %s", buffer);
		}
	}
	return 0;
}

public Action Command_MenuUp(int client, int args)
{
	if(client)
	{
		if(GetClientMenu(client) != MenuSource_Normal)
		{
			ClientCommand(client, "slot1");
			return Plugin_Handled;
		}
	}
	return Plugin_Continue;
}

public Action Command_MenuNext(int client, int args)
{
	if(client)
	{
		if(InMenu[client] && InMenu[client] < 3)
		{
			DataPack pack = new DataPack();
			RequestFrame(Command_MenuFrame, pack);
			pack.WriteCell(GetClientUserId(client));
			pack.WriteCell(InMenu[client]);
			return Plugin_Handled;
		}
		else if(GetClientMenu(client) != MenuSource_Normal)
		{
			ClientCommand(client, "slot2");
			return Plugin_Handled;
		}
	}
	return Plugin_Continue;
}

public Action Command_MenuDown(int client, int args)
{
	if(client)
	{
		if(GetClientMenu(client) != MenuSource_Normal)
		{
			ClientCommand(client, "slot3");
			return Plugin_Handled;
		}
	}
	return Plugin_Continue;
}

public Action Command_MenuBack(int client, int args)
{
	if(client)
	{
		if(InMenu[client] > 1)
		{
			DataPack pack = new DataPack();
			RequestFrame(Command_MenuFrame, pack);
			pack.WriteCell(GetClientUserId(client));
			pack.WriteCell(InMenu[client] - 2);
			return Plugin_Handled;
		}
		else if(GetClientMenu(client) != MenuSource_Normal)
		{
			ClientCommand(client, "slot4");
			return Plugin_Handled;
		}
	}
	return Plugin_Continue;
}

public Action Command_MenuSelect(int client, int args)
{
	if(client)
	{
		if(GetClientMenu(client) != MenuSource_Normal)
		{
			FakeClientCommand(client, "voicemenu 0 0");
			return Plugin_Handled;
		}
	}
	return Plugin_Continue;
}

public void Command_MenuFrame(DataPack pack)
{
	pack.Reset();
	int client = GetClientOfUserId(pack.ReadCell());
	if(client)
	{
		InMenu[client] = pack.ReadCell();
		Command_Voicemenu(client, client);
	}
}