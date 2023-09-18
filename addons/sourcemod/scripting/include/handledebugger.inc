#if defined _handledebug_included
	#endinput
#endif
#define _handledebug_included

#include <clientprefs>
#include <sdktools_trace>

/*
	Excluded handles:
	
	AdminId - Global
	GroupId - Global
	ConVar - Global
	Database - Complex
	Transaction - Skipped
	DHookSetup - Skipped
	DynamicDetour - Skipped
	DynamicHook - Skipped
	GlobalForward - Complex
	PrivateForward - Complex
	MenuStyle - Global
	Profiler - Skipped
	Regex - Skipped
	SDKCall - Skipped
	SMCParser - Skipped
	PluginIterator - Skipped
	TopMenu - Skipped
*/

static StringMap HandleList;
static StringMap HandleMap;

static void AddHandle(Handle handle)
{
	if(!handle)
		return;
	
	if(!HandleList)
	{
		RegServerCmd("sm_handledebug", RunHandleDebugger);
		HandleList = new StringMap();
		HandleMap = new StringMap();
	}
	
	FrameIterator frame = new FrameIterator();
	frame.Next();
	frame.Next();
	
	char code[32];
	frame.GetFunctionName(code, sizeof(code));
	
	frame.Next();
	int line = frame.LineNumber;
	
	char filepath[64], header[32];
	frame.GetFilePath(filepath, sizeof(filepath));
	frame.GetFunctionName(header, sizeof(header));
	
	delete frame;
	
	char buffer[128];
	FormatEx(buffer, sizeof(buffer), "Line %d, %s::%s, %s", line, filepath, header, code);
	
	int count;
	HandleList.GetValue(buffer, count);
	HandleList.SetValue(buffer, count + 1);
	
	CheckHandle(handle);
	
	char name[16];
	IntToString(view_as<int>(handle), name, sizeof(name));
	HandleMap.SetString(name, buffer);
}

static void CheckHandle(Handle handle)
{
	if(!IsValidHandle(handle))
	{
		char name[16], buffer[128];
		IntToString(view_as<int>(handle), name, sizeof(name));
		if(HandleMap.GetString(name, buffer, sizeof(buffer)))
		{
			HandleMap.Remove(name);
			
			int count;
			if(HandleList.GetValue(buffer, count))
			{
				HandleList.SetValue(buffer, count - 1);
			}
			else
			{
				LogError("[HandleDebug] Missing \"%s\"", buffer);
			}
		}
	}
}

static void CheckAllHandles()
{
	StringMapSnapshot snap = HandleMap.Snapshot();
	int length = snap.Length;
	for(int i; i < length; i++)
	{
		int size = snap.KeyBufferSize(i) + 1;
		char[] name = new char[size];
		snap.GetKey(i, name, size);
		
		Handle handle = view_as<Handle>(StringToInt(name));
		CheckHandle(handle);
	}
}

stock Action RunHandleDebugger(int args)
{
	if(HandleList)
	{
		char buffer[128];
		GetPluginFilename(INVALID_HANDLE, buffer, sizeof(buffer));
		PrintToServer(buffer);
		
		CheckAllHandles();
		
		StringMapSnapshot snap = HandleList.Snapshot();
		int length = snap.Length;
		for(int i; i < length; i++)
		{
			snap.GetKey(i, buffer, sizeof(buffer));
			
			int count;
			if(HandleList.GetValue(buffer, count) && count)
				PrintToServer("%s - %d", buffer, count);
		}
	}
	return Plugin_Handled;
}


methodmap ArrayListD < ArrayList
{
	public ArrayListD(int blocksize=1, int startsize=0)
	{
		ArrayList handle = new ArrayList(blocksize, startsize);
		AddHandle(handle);
		return view_as<ArrayListD>(handle);
	}
	public ArrayListD Clone()
	{
		ArrayList handle = view_as<ArrayList>(this).Clone();
		AddHandle(handle);
		return view_as<ArrayListD>(handle);
	}
}
stock ArrayListD CreateArrayD(int blocksize=1, int startsize=0)
{
	Handle handle = CreateArray(blocksize, startsize);
	AddHandle(handle);
	return view_as<ArrayListD>(handle);
}
stock Handle CloneArrayD(Handle array)
{
	Handle handle = CloneArray(array);
	AddHandle(handle);
	return handle;
}
#define ArrayList ArrayListD
#define CreateArray CreateArrayD
#define CloneArray CloneArrayD

methodmap ArrayStackD < ArrayStack
{
	public ArrayStackD(int blocksize=1)
	{
		ArrayStack handle = new ArrayStack(blocksize);
		AddHandle(handle);
		return view_as<ArrayStackD>(handle);
	}
	public ArrayStackD Clone()
	{
		ArrayStack handle = view_as<ArrayStack>(this).Clone();
		AddHandle(handle);
		return view_as<ArrayStackD>(handle);
	}
}
stock ArrayStackD CreateStackD(int blocksize=1)
{
	Handle handle = CreateStack(blocksize);
	AddHandle(handle);
	return view_as<ArrayStackD>(handle);
}
#define ArrayStack ArrayStackD
#define CreateStack CreateStackD

methodmap StringMapD < StringMap
{
	public StringMapD()
	{
		StringMap handle = new StringMap();
		AddHandle(handle);
		return view_as<StringMapD>(handle);
	}
	public StringMapD Clone()
	{
		StringMap handle = view_as<StringMap>(this).Clone();
		AddHandle(handle);
		return view_as<StringMapD>(handle);
	}
	public StringMapSnapshot Snapshot()
	{
		StringMapSnapshot handle = view_as<StringMap>(this).Snapshot();
		AddHandle(handle);
		return handle;
	}
}
stock StringMapD CreateTrieD()
{
	Handle handle = CreateTrie();
	AddHandle(handle);
	return view_as<StringMapD>(handle);
}
stock Handle CreateTrieSnapshotD(Handle map)
{
	Handle handle = CreateTrieSnapshot(map);
	AddHandle(handle);
	return handle;
}
#define StringMap StringMapD
#define CreateTrie CreateTrieD
#define CreateTrieSnapshot CreateTrieSnapshotD

methodmap CookieD < Cookie
{
	public CookieD(const char[] name, const char[] description, CookieAccess access)
	{
		Cookie handle = new Cookie(name, description, access);
		AddHandle(handle);
		return view_as<CookieD>(handle);
	}
	public static CookieD Find(const char[] name)
	{
		Cookie handle = Cookie.Find(name);
		AddHandle(handle);
		return view_as<CookieD>(handle);
	}
}
stock CookieD RegClientCookieD(const char[] name, const char[] description, CookieAccess access)
{
	Handle handle = RegClientCookie(name, description, access);
	AddHandle(handle);
	return view_as<CookieD>(handle);
}
stock CookieD FindClientCookieD(const char[] name)
{
	Handle handle = FindClientCookie(name);
	AddHandle(handle);
	return view_as<CookieD>(handle);
}
#define Cookie CookieD
#define RegClientCookie RegClientCookieD
#define FindClientCookie FindClientCookieD

methodmap CommandIteratorD < CommandIterator
{
	public CommandIteratorD()
	{
		CommandIterator handle = new CommandIterator();
		AddHandle(handle);
		return view_as<CommandIteratorD>(handle);
	}
}
stock Handle GetCommandIteratorD()
{
	Handle handle = GetCommandIterator();
	AddHandle(handle);
	return handle;
}
#define CommandIterator CommandIteratorD
#define GetCommandIterator GetCommandIteratorD

methodmap DataPackD < DataPack
{
	public DataPackD()
	{
		DataPack handle = new DataPack();
		AddHandle(handle);
		return view_as<DataPackD>(handle);
	}
}
stock DataPackD CreateDataPackD()
{
	Handle handle = CreateDataPack();
	AddHandle(handle);
	return view_as<DataPackD>(handle);
}
#define DataPack DataPackD
#define CreateDataPack CreateDataPackD

stock Event CreateEventD(const char[] name, bool force=false)
{
	Handle handle = CreateEvent(name, force);
	AddHandle(handle);
	return view_as<Event>(handle);
}
#define CreateEvent CreateEventD

stock DirectoryListing OpenDirectoryD(const char[] path, bool use_valve_fs=false, const char[] valve_path_id="GAME")
{
	Handle handle = OpenDirectory(path, use_valve_fs, valve_path_id);
	AddHandle(handle);
	return view_as<DirectoryListing>(handle);
}
#define OpenDirectory OpenDirectoryD

stock File OpenFileD(const char[] file, const char[] mode, bool use_valve_fs=false, const char[] valve_path_id="GAME")
{
	Handle handle = OpenFile(file, mode, use_valve_fs, valve_path_id);
	AddHandle(handle);
	return view_as<File>(handle);
}
#define OpenFile OpenFileD

stock Handle CloneHandleD(Handle hndl, Handle plugin=INVALID_HANDLE)
{
	Handle handle = CloneHandle(hndl, plugin);
	
	if(plugin == INVALID_HANDLE)
		AddHandle(handle);
	
	return view_as<File>(handle);
}
#define CloneHandle CloneHandleD

methodmap KeyValuesD < KeyValues
{
	public KeyValuesD(const char[] name, const char[] firstKey="", const char[] firstValue="")
	{
		KeyValues handle = new KeyValues(name, firstKey, firstValue);
		AddHandle(handle);
		return view_as<KeyValuesD>(handle);
	}
}
stock KeyValuesD CreateKeyValuesD(const char[] name, const char[] firstKey="", const char[] firstValue="")
{
	Handle handle = CreateKeyValues(name, firstKey, firstValue);
	AddHandle(handle);
	return view_as<KeyValuesD>(handle);
}
public Action OnClientCommandKeyValues(int client, KeyValues kv)
{
	return Plugin_Continue;
}
#define KeyValues KeyValuesD
#define CreateKeyValues CreateKeyValuesD
#define OnClientCommandKeyValues OnClientCommandKeyValuesD

methodmap PanelD < Panel
{
	public PanelD(Handle hStyle = null)
	{
		Panel handle = new Panel(hStyle);
		AddHandle(handle);
		return view_as<PanelD>(handle);
	}
}
methodmap MenuD < Menu
{
	public MenuD(MenuHandler handler, MenuAction actions=MENU_ACTIONS_DEFAULT)
	{
		Menu handle = new Menu(handler, actions);
		AddHandle(handle);
		return view_as<MenuD>(handle);
	}
	public PanelD ToPanel()
	{
		Handle handle = view_as<Menu>(this).ToPanel();
		AddHandle(handle);
		return view_as<PanelD>(handle);
	}
}
stock PanelD CreatePanelD(Handle hStyle=INVALID_HANDLE)
{
	Handle handle = CreatePanel(hStyle);
	AddHandle(handle);
	return view_as<PanelD>(handle);
}
stock PanelD CreatePanelFromMenuD(Handle menu)
{
	Handle handle = CreatePanelFromMenu(menu);
	AddHandle(handle);
	return view_as<PanelD>(handle);
}
stock MenuD CreateMenuD(MenuHandler handler, MenuAction actions=MENU_ACTIONS_DEFAULT)
{
	Handle handle = CreateMenu(handler, actions);
	AddHandle(handle);
	return view_as<MenuD>(handle);
}
stock MenuD CreateMenuExD(Handle hStyle=INVALID_HANDLE, MenuHandler handler, MenuAction actions=MENU_ACTIONS_DEFAULT)
{
	Handle handle = CreateMenuEx(hStyle, handler, actions);
	AddHandle(handle);
	return view_as<MenuD>(handle);
}
#define Menu MenuD
#define Panel PanelD
#define CreatePanel CreatePanelD
#define CreatePanelFromMenu CreatePanelFromMenuD
#define CreateMenu CreateMenuD
#define CreateMenuEx CreateMenuExD

stock Handle TR_TraceHullExD(const float pos[3], const float vec[3], const float mins[3], const float maxs[3], int flags)
{
	Handle handle = TR_TraceHullEx(pos, vec, mins, maxs, flags);
	AddHandle(handle);
	return handle;
}
#define TR_TraceHullEx TR_TraceHullExD

stock Handle TR_TraceHullFilterExD(const float pos[3], const float vec[3], const float mins[3], const float maxs[3], int flags, TraceEntityFilter filter, any data=0)
{
	Handle handle = TR_TraceHullFilterEx(pos, vec, mins, maxs, flags, filter, data);
	AddHandle(handle);
	return handle;
}
#define TR_TraceHullFilterEx TR_TraceHullFilterExD

stock Handle TR_TraceRayExD(const float pos[3], const float vec[3], int flags, RayType rtype)
{
	Handle handle = TR_TraceRayEx(pos, vec, flags, rtype);
	AddHandle(handle);
	return handle;
}
#define TR_TraceRayEx TR_TraceRayExD

stock Handle TR_TraceRayFilterExD(const float pos[3], const float vec[3], int flags, RayType rtype, TraceEntityFilter filter, any data=0)
{
	Handle handle = TR_TraceRayFilterEx(pos, vec, flags, rtype, filter, data);
	AddHandle(handle);
	return handle;
}
#define TR_TraceRayFilterEx TR_TraceRayFilterExD

methodmap FrameIteratorD < FrameIterator
{
	public FrameIteratorD()
	{
		FrameIterator handle = new FrameIterator();
		AddHandle(handle);
		return view_as<FrameIteratorD>(handle);
	}
}
#define FrameIterator FrameIteratorD

methodmap GameDataD < GameData
{
	public GameDataD(const char[] file)
	{
		GameData handle = new GameData(file);
		AddHandle(handle);
		return view_as<GameDataD>(handle);
	}
}
stock GameDataD LoadGameConfigFileD(const char[] file)
{
	Handle handle = LoadGameConfigFile(file);
	AddHandle(handle);
	return view_as<GameDataD>(handle);
}
#define GameData GameDataD
#define LoadGameConfigFile LoadGameConfigFileD
/*
methodmap PluginIteratorD < PluginIterator
{
	public PluginIteratorD(const char[] file)
	{
		PluginIterator handle = new PluginIterator();
		AddHandle(handle);
		return view_as<PluginIteratorD>(handle);
	}
}
stock PluginIteratorD GetPluginIteratorD()
{
	Handle handle = GetPluginIterator();
	AddHandle(handle);
	return view_as<PluginIteratorD>(handle);
}
#define PluginIterator PluginIteratorD
#define GetPluginIterator GetPluginIteratorD
*/
stock Handle CreateTimerD(float interval, Timer func, any data=0, int flags=0)
{
	Handle handle = CreateTimer(interval, func, data, flags);
	AddHandle(handle);
	return handle;
}
stock Handle CreateDataTimerD(float interval, Timer func, Handle &datapack, int flags=0)
{
	Handle handle = CreateDataTimer(interval, func, datapack, flags);
	AddHandle(handle);
	return handle;
}
#define CreateTimer CreateTimerD
#define CreateDataTimer CreateDataTimerD

stock Handle StartMessageD(const char[] msgname, const int[] clients, int numClients, int flags=0)
{
	Handle handle = StartMessage(msgname, clients, numClients, flags);
	AddHandle(handle);
	return handle;
}
stock Handle StartMessageExD(UserMsg msg, const int[] clients, int numClients, int flags=0)
{
	Handle handle = StartMessageEx(msg, clients, numClients, flags);
	AddHandle(handle);
	return handle;
}
#define StartMessage StartMessageD
#define StartMessageEx StartMessageExD