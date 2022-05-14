#if defined _menus_controller_included
	#endinput
#endif
#define _menus_controller_included

#include <menus>

#if defined CanSelectDisabled
#define StyleCheck(%1)	(%1 <= ITEMDRAW_DISABLED)
#else
#define StyleCheck(%1)	(%1 == ITEMDRAW_DEFAULT)
#endif

static StringMap MenuCallbacks;
static Handle Displaying[MAXPLAYERS+1];
static bool Ignore[MAXPLAYERS+1];
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
	MenuCallbacks.SetArray(buffer, handler);
}

static MenuHandler GetCallbackOfHandle(Handle handle)
{
	char buffer[16];
	FormatEx(buffer, sizeof(buffer), "%x", handle);
	
	MenuHandler handler;
	MenuCallbacks.GetValue(buffer, handler);
	return handler;
}

methodmap MenuMC < Menu
{
	public MenuMC(MenuHandler handler, MenuAction actions=MENU_ACTIONS_DEFAULT)
	{
		Menu menu = new Menu(_MC_MenuHandler, actions);
		SetCallbackOfHandle(menu, handler);
		return view_as<MenuMC>(menu);
	}
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
				if(GetMenuItem(Displaying[client], pos, buffer, sizeof(buffer), style) && StyleCheck(style))
				{
					ItemIndex[client] = pos;
					found = true;
					break;
				}
			}
			
			if(!found && GetMenuExitBackButton(Displaying[client]))
			{
				ClientCommand(client, "slot8");
			}
			else if(!found && GetMenuExitButton(Displaying[client]) && ItemIndex[client] != -1)
			{
				ClientCommand(client, "slot10");
			}
			else
			{
				if(ItemIndex[client] == -1)
					ItemIndex[client] = 0;
				
				pos = ItemIndex[client];
				int pagination = GetMenuPagination(Displaying[client]);
				if(pagination)
					pos = pos / pagination * pagination;
				
				Ignore[client] = true;
				DisplayMenuAtItem(Displaying[client], client, pos, MENU_TIME_FOREVER);
				Ignore[client] = false;
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
			
			int items = GetMenuItemCount(Displaying[client]) - 1;
			while(pos < items)
			{
				pos++;
				if(GetMenuItem(Displaying[client], pos, buffer, sizeof(buffer), style) && StyleCheck(style))
				{
					ItemIndex[client] = pos;
					break;
				}
			}
			
			if(ItemIndex[client] == -1)
				ItemIndex[client] = 0;
			
			pos = ItemIndex[client];
			int pagination = GetMenuPagination(Displaying[client]);
			if(pagination)
				pos = pos / pagination * pagination;
			
			Ignore[client] = true;
			DisplayMenuAtItem(Displaying[client], client, pos, MENU_TIME_FOREVER);
			Ignore[client] = false;
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
			int pagination = GetMenuPagination(Displaying[client]);
			
			if(pos >= pagination)
			{
				pos -= pagination;
				if(GetMenuItem(Displaying[client], pos, buffer, sizeof(buffer), style) && StyleCheck(style))
				{
					ItemIndex[client] = pos;
					found = true;
				}
			}
			
			if(!found && GetMenuExitBackButton(Displaying[client]))
			{
				ClientCommand(client, "slot8");
			}
			else
			{
				if(!found)
				{
					pos = ItemIndex[client];
					while(pos > 0)
					{
						pos--;
						if(GetMenuItem(Displaying[client], pos, buffer, sizeof(buffer), style) && StyleCheck(style))
						{
							ItemIndex[client] = pos;
							break;
						}
					}
				}
				
				if(ItemIndex[client] == -1)
					ItemIndex[client] = 0;
				
				pos = ItemIndex[client];
				if(pagination)
					pos = pos / pagination * pagination;
				
				Ignore[client] = true;
				DisplayMenuAtItem(Displaying[client], client, pos, MENU_TIME_FOREVER);
				Ignore[client] = false;
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
			int pagination = GetMenuPagination(Displaying[client]);
			
			int items = GetMenuItemCount(Displaying[client]);
			if(pos + pagination < items)
			{
				pos += pagination;
				if(GetMenuItem(Displaying[client], pos, buffer, sizeof(buffer), style) && StyleCheck(style))
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
					if(GetMenuItem(Displaying[client], pos, buffer, sizeof(buffer), style) && StyleCheck(style))
					{
						ItemIndex[client] = pos;
						break;
					}
				}
			}
			
			if(ItemIndex[client] == -1)
				ItemIndex[client] = 0;
			
			pos = ItemIndex[client];
			if(pagination)
				pos = pos / pagination * pagination;
			
			Ignore[client] = true;
			DisplayMenuAtItem(Displaying[client], client, pos, MENU_TIME_FOREVER);
			Ignore[client] = false;
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
			
			int pagination = GetMenuPagination(Displaying[client]);
			if(pagination)
			{
				ClientCommand(client, "slot%d", ItemIndex[client] % pagination);
			}
			else
			{
				ClientCommand(client, "slot%d", ItemIndex[client]);
			}
			return Plugin_Handled;
		}
	}
	return Plugin_Continue;
}

public int _MC_MenuHandler(Menu menu, MenuAction action, int param1, int param2)
{
	int value;
	switch(action)
	{
		case MenuAction_Display:
		{
			if(Ignore[param1])
				return 0;
			
			Displaying[param1] = menu;
			
			int items = menu.Pagination;
			if(!items)
				items = menu.ItemCount;
			
			if(ItemIndex[param1] >= items)
				ItemIndex[param1] = 0;
		}
		case MenuAction_Cancel:
		{
			if(Ignore[param1])
				return 0;
			
			Displaying[param1] = null;
			if(param2 == MenuCancel_Disconnected || param2 == MenuCancel_Exit)
				ItemIndex[param1] = -1;
		}
		case MenuAction_DisplayItem:
		{
			if(ItemIndex[param1] == param2)
			{
				int style;
				char buffer[256];
				if(menu.GetItem(param2, buffer, sizeof(buffer), style) && StyleCheck(style))
				{
					StrCat(buffer, sizeof(buffer), " <--");
					value = RedrawMenuItem(buffer);
				}
			}
		}
		default:
		{
			if(Ignore[param1])
				return 0;
		}
	}
	
	MenuHandler handler = GetCallbackOfHandle(menu);
	
	Call_StartFunction(null, handler);
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
	Menu menu = CreateMenu(_MC_MenuHandler, actions);
	SetCallbackOfHandle(menu, handler);
	return view_as<MenuMC>(menu);
}

stock MenuMC CreateMenuExMC(Handle hStyle=INVALID_HANDLE, MenuHandler handler, MenuAction actions=MENU_ACTIONS_DEFAULT)
{
	Menu menu = CreateMenuEx(hStyle, _MC_MenuHandler, actions);
	SetCallbackOfHandle(menu, handler);
	return view_as<MenuMC>(menu);
}

#define OnPluginStart OnPluginStartMC
#define Menu MenuMC
#define CreateMenu CreateMenuMC
#define CreateMenuEx CreateMenuExMC