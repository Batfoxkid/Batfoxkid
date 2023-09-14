#if SOURCEMOD_V_MAJOR >= 1 && SOURCEMOD_V_MINOR >= 11
	#error github.com/alliedmodders/sourcepawn/issues/671
#endif

#if defined _discordbot_included
	#endinput
#endif
#define _discordbot_included

#include <SteamWorks>
#include <jsonkv>

#pragma dynamic	131072	// Blame SteamWorks_GetHTTPResponseBodyCallback

#define DISCORD_API_URL	"https://discord.com/api/"

static StringMap RateReset;
static StringMap RateLeft;
static StringMap RateLimit;
static char buffer[16384];

methodmap DiscordRequest < StringMap
{
	public DiscordRequest(const char[] token, EHTTPMethod method, const char[] route, const char[] data="", any ...)
	{
		if(data[0])
		{
			VFormat(buffer, sizeof(buffer), data, 5);
			Format(buffer, sizeof(buffer), "%s%s%s", DISCORD_API_URL, route, buffer);
		}
		else
		{
			Format(buffer, sizeof(buffer), "%s%s", DISCORD_API_URL, route);
		}
		
		StringMap map = new StringMap();
		map.SetValue("method", method);
		map.SetString("route", route);
		map.SetString("url", buffer);
		
		if(!StrContains(token, "Bot "))
		{
			map.SetString("token", token);
		}
		else
		{
			Format(buffer, sizeof(buffer), "Bot %s", token);
			map.SetString("token", buffer);
		}
		
		return view_as<DiscordRequest>(map);
	}
	
	public bool GetRoute(char[] route, int length)
	{
		return this.GetString("route", route, length);
	}
	
	property Json JsonBody
	{
		public set(Json json)
		{
			this.SetValue("json", json);
		}
		public get()
		{
			Json json;
			this.GetValue("json", json);
			return json;
		}
	}
	
	property SteamWorksHTTPBodyCallback Callback
	{
		public set(SteamWorksHTTPBodyCallback callback)
		{
			DataPack pack;
			if(this.GetValue("callback", pack))
				delete pack;
			
			pack = new DataPack();
			pack.WriteFunction(view_as<Function>(callback));
			this.SetValue("callback", pack);
		}
		public get()
		{
			DataPack pack;
			if(!this.GetValue("callback", pack))
				return INVALID_FUNCTION;
			
			pack.Reset();
			return view_as<SteamWorksHTTPBodyCallback>(pack.ReadFunction());
		}
	}
	
	property SteamWorksHTTPDataReceived DataCallback
	{
		public set(SteamWorksHTTPDataReceived callback)
		{
			DataPack pack;
			if(this.GetValue("databack", pack))
				delete pack;
			
			pack = new DataPack();
			pack.WriteFunction(view_as<Function>(callback));
			this.SetValue("databack", pack);
		}
		public get()
		{
			DataPack pack;
			if(!this.GetValue("databack", pack))
				return _discordddatareceived;
			
			pack.Reset();
			return view_as<SteamWorksHTTPDataReceived>(pack.ReadFunction());
		}
	}
	
	property bool SupressError
	{
		public set(bool value)
		{
			this.SetValue("noerror", value);
		}
		public get()
		{
			bool value;
			this.GetValue("noerror", value);
			return value;
		}
	}
	
	property any Data
	{
		public set(any data)
		{
			this.SetValue("data", data);
		}
		public get()
		{
			any data;
			this.GetValue("data", data);
			return data;
		}
	}
	
	public void Send()
	{
		if(!RateReset)
		{
			RateReset = new StringMap();
			RateLeft = new StringMap();
			RateLimit = new StringMap();
		}
		
		int time = GetTime();
		if(time != -1)
		{
			int limit = 1;
			this.GetString("route", buffer, sizeof(buffer));
			RateLimit.GetValue(buffer, limit);
			
			int reset;
			if(RateReset.GetValue(buffer, reset))
			{
				if(time > reset)
				{
					RateLimit.SetValue(buffer, limit-1);
				}
				else
				{
					int left;
					RateLeft.GetValue(buffer, left);
					if(left)
					{
						RateLeft.SetValue(buffer, left-1);
					}
					else
					{
						float delay = float(reset) - float(time);
						if(delay > 60.0)
							LogError("Bot is being rate limited to %.1f seconds", delay);
						
						CreateTimer(delay + 1.0, _discordbotretrysend, this);
						return;
					}
				}
			}
			else
			{
				RateReset.SetValue(buffer, time+5);
				RateLimit.SetValue(buffer, limit-1);
			}
		}
		
		EHTTPMethod method;
		this.GetValue("method", method);
		this.GetString("url", buffer, sizeof(buffer));
		Handle request = SteamWorks_CreateHTTPRequest(method, buffer);
		if(request)
		{
			SteamWorks_SetHTTPCallbacks(request, _discordbothttpcompleted, _discordheadersreceived, this.DataCallback);
			Json json = this.JsonBody;
			if(json)
			{
				json.Rewind();
				json.ExportToString(buffer, sizeof(buffer));
				SteamWorks_SetHTTPRequestRawPostBody(request, "application/json; charset=UTF-8", buffer, strlen(buffer));
			}
			else
			{
				SteamWorks_SetHTTPRequestHeaderValue(request, "Content-Length", "0");
			}
			this.GetString("token", buffer, sizeof(buffer));
			SteamWorks_SetHTTPRequestHeaderValue(request, "Authorization", buffer);
			SteamWorks_SetHTTPRequestContextValue(request, this);
			SteamWorks_SendHTTPRequest(request);
		}
		else
		{
			CreateTimer(2.0, _discordbotretrysend, this);
		}
	}
	
	public void Close()
	{
		Handle handle;
		if(this.GetValue("json", handle))
			delete handle;
		
		if(this.GetValue("callback", handle))
			delete handle;
		
		if(this.GetValue("databack", handle))
			delete handle;
		
		delete this;
	}
}

public Action _discordbotretrysend(Handle timer, DiscordRequest request)
{
	request.Send();
	return Plugin_Continue;
}

public int _discordbothttpcompleted(Handle request, bool failure, bool requestSuccessful, EHTTPStatusCode statuscode)
{
}

public void _discordheadersreceived(Handle request, bool failure, DiscordRequest discord)
{
	if(failure)
		return;
	
	int exists;
	char limit[16], left[16];
	if(SteamWorks_GetHTTPResponseHeaderValue(request, "X-RateLimit-Limit", limit, sizeof(limit)))
		exists++;
	
	if(SteamWorks_GetHTTPResponseHeaderValue(request, "X-RateLimit-Remaining", left, sizeof(left)))
		exists++;
	
	if(SteamWorks_GetHTTPResponseHeaderValue(request, "X-RateLimit-Reset", buffer, sizeof(buffer)))
		exists++;
	
	int reset = StringToInt(buffer);
	int time = GetTime() + 3;
	if(reset > time)
		reset = time;
	
	discord.GetRoute(buffer, sizeof(buffer));
	
	if(exists == 3)
	{
		RateReset.SetValue(buffer, reset);
		RateLeft.SetValue(buffer, StringToInt(left));
		RateLimit.SetValue(buffer, StringToInt(limit));
	}
	else
	{
		RateReset.Remove(buffer);
		RateLeft.Remove(buffer);
		RateLimit.Remove(buffer);
	}
}

public void _discordddatareceived(Handle request, bool failure, int offset, int statuscode, DiscordRequest discord)
{
	if(failure || statuscode != 200)
	{
		if(statuscode == 429 || statuscode == 500)
		{
			discord.Send();
		}
		else
		{
			SteamWorksHTTPBodyCallback callback = discord.Callback;
			if(callback != INVALID_FUNCTION)
			{
				Call_StartFunction(null, callback);
				Call_PushNullString();
				Call_PushCell(discord.Data);
				Call_PushCell(0);
				Call_Finish();
			}
			
			if(!discord.SupressError)
			{
				switch(statuscode)
				{
					case 204:
					{
					}
					case 401, 404:
					{
						LogError("Returned %d: Request is Invalid", statuscode);
					}
					case 403:
					{
						LogError("Returned 403: Bot has no such permission to peform this");
					}
					default:
					{
						LogError("Returned %d", statuscode);
					}
				}
			}
			
			discord.Close();
		}
	}
	else
	{
		SteamWorksHTTPBodyCallback callback = discord.Callback;
		if(callback != INVALID_FUNCTION)
		{
			int size;
			if(SteamWorks_GetHTTPResponseBodySize(request, size) && size < 4000)
			{
				SteamWorks_GetHTTPResponseBodyCallback(request, callback, discord.Data);
			}
			else
			{
				Call_StartFunction(null, callback);
				Call_PushNullString();
				Call_PushCell(discord.Data);
				Call_PushCell(0);
				Call_Finish();
			}
		}
		
		discord.Close();
	}
	delete request;
}