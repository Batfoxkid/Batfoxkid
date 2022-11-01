#pragma semicolon 1

#include <sourcemod>
#include <discordbot>

#pragma newdecls required

#define BOT_TOKEN	""
#define FILE_THINKER	"discordthinker.txt"

char PingUserId[36];
ConVar CvarThinkerUrl;
ConVar CvarThinkerChance;

public void OnPluginStart()
{
	CvarThinkerUrl = CreateConVar("discord_thinker_url", "", "Webhook URL for Thinker for this server", FCVAR_PROTECTED);
	CvarThinkerChance = CreateConVar("discord_thinker_chance", "0.0", "Chance to post a message", _, true, 0.0, true, 1.0);
	
	AddCommandListener(OnSayCommand, "say");
	AddCommandListener(OnSayCommand, "say_team");
	
	DiscordRequest request = new DiscordRequest(BOT_TOKEN, k_EHTTPMethodGET, "users/@me");
	request.Callback = SetupDiscordBotCallback;
	request.Send();
}

public void SetupDiscordBotCallback(const char[] jsondata)
{
	if(jsondata[0])
	{
		Json json = new Json();
		json.ImportFromString(jsondata);
		json.GetString("id", PingUserId, sizeof(PingUserId));
		delete json;
		
		Format(PingUserId, sizeof(PingUserId), "<@!%s>", PingUserId);
	}
	else
	{
		LogError("[DISCORD] Failed initial setup for bot");
		DiscordRequest request = new DiscordRequest(BOT_TOKEN, k_EHTTPMethodGET, "users/@me");
		request.Callback = SetupDiscordBotCallback;
		request.Send();
	}
}

public void OnMapStart()
{
	if(GetRandomFloat() < CvarThinkerChance.FloatValue)
	{
		char url[PLATFORM_MAX_PATH];
		CvarThinkerUrl.GetString(url, sizeof(url));
		if(url[0])
		{
			char message[1024];
			if(GetRandomMessage(message, sizeof(message)))
			{
				Json json = new Json();
				json.SetString("content", message);
				if(json.JumpToKey("allowed_mentions", true))
				{
					if(json.JumpToKey("parse", true, true))
						json.GoBack();
					
					json.GoBack();
				}
				
				Format(url, sizeof(url), "webhooks/%s", url);
				DiscordRequest request = new DiscordRequest(BOT_TOKEN, k_EHTTPMethodPOST, url);
				request.JsonBody = json;
				request.Send();
			}
		}
	}
}

public Action OnSayCommand(int client, const char[] command, int args)
{
	if(!client)
		return Plugin_Continue;

	static char msg[256];
	GetCmdArgString(msg, sizeof(msg));
	if(StrContains(msg, "rtv", false)!=-1 || StrContains(msg, "nextmap", false)!=-1 || StrContains(msg, "rtd", false)!=-1 || StrContains(msg, "!")!=-1 || StrContains(msg, "/")!=-1 || StrContains(msg, "@")!=-1 || GetRandomInt(0, 49))
		return Plugin_Continue;

	//CRemoveTags(msg, sizeof(msg));
	ReplaceString(msg, sizeof(msg), "\"", "");
	ReplaceString(msg, sizeof(msg), "\\", "");
	ReplaceString(msg, sizeof(msg), "\n", "");
	if(strlen(msg) < 2)
		return Plugin_Continue;

	SaveRandomMessage(msg);
	return Plugin_Continue;
}

bool GetRandomMessage(char[] buffer, int length)
{
	KeyValues kv = new KeyValues("thethinker");
	kv.ImportFromFile(FILE_THINKER);
	
	int index = kv.GetNum("last");
	if(index)
		return false;
	
	index = kv.GetNum("count");
	if(index < 1)
		return false;
	
	index = GetRandomInt(1, index);
	
	char num[12];
	IntToString(index, num, sizeof(num));
	
	kv.GetString(num, buffer, length);
	kv.SetNum("last", index);
	kv.ExportToFile(FILE_THINKER);
	delete kv;
	return true;
}

void SaveRandomMessage(const char[] buffer)
{
	KeyValues kv = new KeyValues("thethinker");
	kv.ImportFromFile(FILE_THINKER);
	
	int index = kv.GetNum("last");
	if(index)
	{
		kv.SetNum("last", 0);
	}
	else
	{
		index = kv.GetNum("count")+1;
		kv.SetNum("count", index);
	}
	
	char num[12];
	IntToString(index, num, sizeof(num));
	
	kv.SetString(num, buffer);
	kv.ExportToFile(FILE_THINKER);
	delete kv;
}