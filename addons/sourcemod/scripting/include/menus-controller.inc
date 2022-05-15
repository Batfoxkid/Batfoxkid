#if defined _menus_controller_included
	#endinput
#endif
#define _menus_controller_included

#include <menus>

#if !defined RedisplayRate
#define RedisplayRate	0.2
#endif

#if defined CanSelectDisabled
#define StyleCheck(%1)	(%1 <= ITEMDRAW_DISABLED)
#else
#define StyleCheck(%1)	(%1 == ITEMDRAW_DEFAULT)
#endif

static StringMap MenuCallbacks;
static bool Ignore;
static Menu Displaying[MAXPLAYERS+1];
static Handle MenuTimer[MAXPLAYERS+1];
static int ItemIndex[MAXPLAYERS+1];

public void OnPluginStart()
{
	MenuCallbacks = new StringMap();
	
	// We're just gonna assume OnPluginStart is already in the plugin
	OnPluginStartMC();
	
	RegConsoleCmd("menu_up", _MC_MenuUp, "Move up an item on supported menus");
	RegConsoleCmd("menu_down", _MC_MenuDown, "Move down an item on supported menus");
	RegConsoleCmd("menu_next", _MC_MenuNext, "Move to the next page on supported menus");
	RegConsoleCmd("menu_back", _MC_MenuBack, "Move to back a page on supported menus");
	RegConsoleCmd("menu_select", _MC_MenuSelect, "Select current item on supported menus");
}

static void SetCallbackOfHandle(Handle handle, MenuHandler handler)
{
	char buffer[16];
	FormatEx(buffer, sizeof(buffer), "%x", handle);
	
	MenuCallbacks.SetValue(buffer, view_as<any>(handler));
}

static MenuHandler GetCallbackOfHandle(Handle handle)
{
	char buffer[16];
	FormatEx(buffer, sizeof(buffer), "%x", handle);
	
	any handler;
	MenuCallbacks.GetValue(buffer, handler);
	return handler;
}

methodmap MenuMC < Menu
{
	public MenuMC(MenuHandler handler, MenuAction actions=MENU_ACTIONS_DEFAULT)
	{
		return CreateMenuMC(handler, actions);
	}
	
	public bool Display(int client, int time)
	{
		return DisplayMenuMC(this, client, time);
	}
	
	public bool DisplayAt(int client, int first_item, int time)
	{
		return DisplayMenuAtItemMC(this, client, first_item, time);
	}
	
	/*public bool DisplayVote(int[] clients, int numClients, int time, int flags=0)
	{
		return VoteMenuMC(this, clients, numClients, time, flags);
	}
	
	public bool DisplayVoteToAll(int time, int flags=0)
	{
		return VoteMenuToAllMC(this, time, flags);
	}*/
}

static void ReshowMenu(int client, int pagination)
{
	if(ItemIndex[client] == -1)
		ItemIndex[client] = 0;
	
	int pos = ItemIndex[client];
	if(pagination)
		pos = pos / pagination * pagination;
	
	Ignore = true;
	
	if(MenuTimer[client])
		delete MenuTimer[client];
	
	#if RedisplayRate != 0.0
	DataPack pack;
	MenuTimer[client] = CreateDataTimer(RedisplayRate, _MC_MenuTimer, pack, TIMER_REPEAT);
	pack.WriteCell(client);
	pack.WriteCell(pos);
	#endif
	
	Displaying[client].DisplayAt(client, pos, MENU_TIME_FOREVER);
	Ignore = false;
}

public Action _MC_MenuUp(int client, int args)
{
	if(client)
	{
		if(Displaying[client])
		{
			char buffer[1];
			bool found;
			int style;
			int pos = ItemIndex[client];
			
			while(pos > 0)
			{
				pos--;
				if(Displaying[client].GetItem(pos, buffer, sizeof(buffer), style) && StyleCheck(style))
				{
					ItemIndex[client] = pos;
					found = true;
					break;
				}
			}
			
			if(!found && Displaying[client].ExitBackButton)
			{
				ClientCommand(client, "slot8");
				FakeClientCommand(client, "menuselect 8");
			}
			else if(!found && Displaying[client].ExitButton && ItemIndex[client] != -1)
			{
				ClientCommand(client, "slot10");
				FakeClientCommand(client, "menuselect 0");
			}
			else
			{
				ReshowMenu(client, Displaying[client].Pagination);
			}
			return Plugin_Handled;
		}
	}
	return Plugin_Continue;
}

public Action _MC_MenuDown(int client, int args)
{
	if(client)
	{
		if(Displaying[client])
		{
			char buffer[1];
			int style;
			int pos = ItemIndex[client];
			
			int items = Displaying[client].ItemCount - 1;
			while(pos < items)
			{
				pos++;
				if(Displaying[client].GetItem(pos, buffer, sizeof(buffer), style) && StyleCheck(style))
				{
					ItemIndex[client] = pos;
					break;
				}
			}
			
			ReshowMenu(client, Displaying[client].Pagination);
			return Plugin_Handled;
		}
	}
	return Plugin_Continue;
}

public Action _MC_MenuBack	(int client, int args)
{
	if(client)
	{
		if(Displaying[client])
		{
			char buffer[1];
			bool found;
			int style;
			int pos = ItemIndex[client];
			int pagination = Displaying[client].Pagination;
			
			if(pos >= pagination)
			{
				pos -= pagination;
				if(Displaying[client].GetItem(pos, buffer, sizeof(buffer), style) && StyleCheck(style))
				{
					ItemIndex[client] = pos;
					found = true;
				}
			}
			
			if(!found && Displaying[client].ExitBackButton)
			{
				ClientCommand(client, "slot8");
				FakeClientCommand(client, "menuselect 8");
			}
			else if(!found && Displaying[client].ExitButton && ItemIndex[client] != -1)
			{
				ClientCommand(client, "slot10");
				FakeClientCommand(client, "menuselect 0");
			}
			else
			{
				ReshowMenu(client, pagination);
			}
			return Plugin_Handled;
		}
	}
	return Plugin_Continue;
}

public Action _MC_MenuNext(int client, int args)
{
	if(client)
	{
		if(Displaying[client])
		{
			char buffer[1];
			bool found;
			int style;
			int pos = ItemIndex[client];
			int pagination = Displaying[client].Pagination;
			
			int items = Displaying[client].ItemCount;
			if(pos + pagination < items)
			{
				pos += pagination;
				if(Displaying[client].GetItem(pos, buffer, sizeof(buffer), style) && StyleCheck(style))
				{
					ItemIndex[client] = pos;
					found = true;
				}
			}
			
			if(!found)
			{
				items--;
				while(pos < items)
				{
					pos++;
					if(Displaying[client].GetItem(pos, buffer, sizeof(buffer), style) && StyleCheck(style))
					{
						ItemIndex[client] = pos;
						break;
					}
				}
			}
			
			ReshowMenu(client, pagination);
			return Plugin_Handled;
		}
	}
	return Plugin_Continue;
}

public Action _MC_MenuSelect(int client, int args)
{
	if(client)
	{
		if(Displaying[client])
		{
			if(ItemIndex[client] == -1)
				ItemIndex[client] = 0;
			
			int pagination = Displaying[client].Pagination;
			if(pagination)
			{
				pagination = (ItemIndex[client] % pagination) + 1;
				ClientCommand(client, "slot%d", pagination);
				FakeClientCommand(client, "menuselect %d", pagination);
			}
			else
			{
				pagination = ItemIndex[client] + 1;
				ClientCommand(client, "slot%d", pagination);
				FakeClientCommand(client, "menuselect %d", pagination);
			}
			return Plugin_Handled;
		}
	}
	return Plugin_Continue;
}

public Action _MC_MenuTimer(Handle timer, DataPack pack)
{
	// Because client thinks otherwise
	
	pack.Reset();
	int client = pack.ReadCell();
	
	Ignore = true;
	Displaying[client].DisplayAt(client, pack.ReadCell(), MENU_TIME_FOREVER);
	Ignore = false;
	
	return Plugin_Continue;
}

public int _MC_MenuHandler(Menu menu, MenuAction action, int param1, int param2)
{
	int value;
	switch(action)
	{
		case MenuAction_Display:
		{
			if(Ignore)
				return 0;
			
			Displaying[param1] = menu;
			
			int items = menu.Pagination;
			if(!items)
				items = menu.ItemCount;
			
			if(ItemIndex[param1] >= items)
				ItemIndex[param1] = 0;
			
			char buffer[1];
			while(ItemIndex[param1] >= 0)
			{
				if(menu.GetItem(ItemIndex[param1], buffer, sizeof(buffer), items) && StyleCheck(items))
					break;
				
				ItemIndex[param1]--;
			}
		}
		case MenuAction_Cancel:
		{
			if(Ignore)
				return 0;
			
			Displaying[param1] = null;
			
			delete MenuTimer[param1];
			MenuTimer[param1] = null;
			
			if(param2 == MenuCancel_Disconnected)
				ItemIndex[param1] = -1;
		}
		case MenuAction_DisplayItem:
		{
			if(ItemIndex[param1] == param2)
			{
				int style;
				char buffer[1], display[256];
				if(menu.GetItem(param2, buffer, sizeof(buffer), style, display, sizeof(display)) && StyleCheck(style))
				{
					StrCat(display, sizeof(display), " <--");
					value = RedrawMenuItem(display);
				}
			}
		}
		case MenuAction_Select:
		{
			Displaying[param1] = null;
			
			delete MenuTimer[param1];
			MenuTimer[param1] = null;
		}
		default:
		{
			if(Ignore)
				return 0;
		}
	}
	
	Call_StartFunction(null, GetCallbackOfHandle(menu));
	Call_PushCell(menu);
	Call_PushCell(action);
	Call_PushCell(param1);
	Call_PushCell(param2);
	
	if(value)
	{
		Call_Finish();
	}
	else
	{
		Call_Finish(value);
	}
	
	return value;
}

stock MenuMC CreateMenuMC(MenuHandler handler, MenuAction actions=MENU_ACTIONS_DEFAULT)
{
	Menu menu = CreateMenu(_MC_MenuHandler, actions|MenuAction_Display|MenuAction_DisplayItem);
	SetCallbackOfHandle(menu, handler);
	return view_as<MenuMC>(menu);
}

stock MenuMC CreateMenuExMC(Handle hStyle=INVALID_HANDLE, MenuHandler handler, MenuAction actions=MENU_ACTIONS_DEFAULT)
{
	Menu menu = CreateMenuEx(hStyle, _MC_MenuHandler, actions|MenuAction_Display|MenuAction_DisplayItem);
	SetCallbackOfHandle(menu, handler);
	return view_as<MenuMC>(menu);
}

stock bool DisplayMenuMC(Handle menu, int client, int time)
{
	if(MenuTimer[client])
		delete MenuTimer[client];
	
	DataPack pack;
	MenuTimer[client] = CreateDataTimer(RedisplayRate, _MC_MenuTimer, pack, TIMER_REPEAT);
	pack.WriteCell(client);
	pack.WriteCell(0);
	return DisplayMenu(menu, client, time);
}
	
stock bool DisplayMenuAtItemMC(Handle menu, int client, int first_item, int time)
{
	if(MenuTimer[client])
		delete MenuTimer[client];
	
	DataPack pack;
	MenuTimer[client] = CreateDataTimer(RedisplayRate, _MC_MenuTimer, pack, TIMER_REPEAT);
	pack.WriteCell(client);
	pack.WriteCell(first_item);
	return DisplayMenuAtItem(menu, client, first_item, time);
}

#define OnPluginStart OnPluginStartMC
#define Menu MenuMC
#define CreateMenu CreateMenuMC
#define CreateMenuEx CreateMenuExMC
#define DisplayMenu DisplayMenuMC
#define DisplayMenuAtItem DisplayMenuAtItemMC
//#define VoteMenu VoteMenuMC
//#define VoteMenuToAll VoteMenuToAllMC