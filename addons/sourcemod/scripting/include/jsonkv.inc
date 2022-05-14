#if defined _jsonkv_included
	#endinput
#endif
#define _jsonkv_included

#include <string>
#include <keyvalues>

#define MAKEKEYSTRING	int size = strlen(key)+3; char[] key2 = new char[size]; if(key[0] == '"' || IsNullString(key)) strcopy(key2, size, key); else Format(key2, size, "\"%s\"", key);

static stock void ExportToStringSub(KeyValues kv, char[] json, int length, bool array)
{
	bool first = true;
	do
	{
		static char buffer1[512];
		if(kv.GetSectionName(buffer1, sizeof(buffer1)) && !StrEqual(buffer1, "_array"))
		{
			bool newarray = view_as<bool>(KvGetNum(kv, "_array"));
			if(kv.GotoFirstSubKey(false))
			{
				if(newarray)
				{
					if(array)
					{
						if(first)	// [[
						{
							Format(json, length, "%s[", json);
						}
						else	// [{},[
						{
							Format(json, length, "%s,[", json);
						}
					}
					else if(first)	// {"section": [
					{
						Format(json, length, "%s%s:[", json, buffer1);
					}
					else	// {"key": "value", "section": [
					{
						Format(json, length, "%s,%s:[", json, buffer1);
					}
						
					ExportToStringSub(kv, json, length, true);
					first = false;
					continue;
				}
				
				if(array)
				{
					if(first)	// [{
					{
						Format(json, length, "%s{", json);
					}
					else	// [{},{
					{
						Format(json, length, "%s,{", json);
					}
				}
				else if(first)	// {"section": {
				{
					Format(json, length, "%s%s:{", json, buffer1);
				}
				else	// {"key": "value", "section": {
				{
					Format(json, length, "%s,%s:{", json, buffer1);
				}
				
				ExportToStringSub(kv, json, length, false);
				first = false;
				continue;
			}
			
			static char buffer2[512];
			kv.GetString(NULL_STRING, buffer2, sizeof(buffer2));
			if(array)
			{
				if(first)	// ["value"
				{
					Format(json, length, "%s%s", json, buffer2);
				}
				else	// ["value", "value"
				{
					Format(json, length, "%s,%s", json, buffer2);
				}
			}
			else if(first)	// {"key": "value"
			{
				Format(json, length, "%s%s:%s", json, buffer1, buffer2);
			}
			else	// {"section": {}, "key": "value"
			{
				Format(json, length, "%s,%s:%s", json, buffer1, buffer2);
			}
			first = false;
		}
	} while(kv.GotoNextKey(false));
	kv.GoBack();

	if(array)
	{
		Format(json, length, "%s]", json);
	}
	else
	{
		Format(json, length, "%s}", json);
	}
}

methodmap Json < KeyValues
{
	public Json()
	{
		return view_as<Json>(new KeyValues("json"));
	}
	
	public int ExportToString(char[] buffer, int maxlength)
	{
		if(!KvGotoFirstSubKey(this, false))
			return 0;
		
		strcopy(buffer, maxlength, "{");
		ExportToStringSub(view_as<KeyValues>(this), buffer, maxlength, false);
		return 1;
	}
	
	public bool ImportFromString(const char[] buffer, const char[] resourceName="StringToKeyValues")
	{
		static char buffer1[512], buffer2[512];
		bool inString, isValue, isNum, isSlash;
		int length, stored;
		
		//DeleteFile("lastdiscordset.txt");
		int size = strlen(buffer)-1;
		for(int i=1; i<=size; i++)
		{
			switch(buffer[i])
			{
				case '{':	// Add section
				{
					if(inString)
					{
						if(isSlash)
						{
							isSlash = false;
							
							if(length < 510)
							{
								if(isValue)
								{
									buffer2[length++] = '\\';
								}
								else
								{
									buffer1[length++] = '\\';
								}
							}
						}
						
						if(length < 510)
						{
							if(isValue)
							{
								buffer2[length++] = buffer[i];
							}
							else
							{
								buffer1[length++] = buffer[i];
							}
						}
					}
					else
					{
						if(stored)
						{
							stored = 0;
						}
						else
						{
							IntToString(i, buffer1, sizeof(buffer1));
						}
						
						if(!KvJumpToKey(this, buffer1, true))
						{
							KvRewind(this);
							return false;
						}
						
						//LogToFileEx("lastdiscordset.txt", "161 | %s: {\n", buffer1);
						isValue = false;
					}
				}
				case '[':	// Add array
				{
					if(inString)
					{
						if(isSlash)
						{
							isSlash = false;
							if(length < 510)
							{
								if(isValue)
								{
									buffer2[length++] = '\\';
								}
								else
								{
									buffer1[length++] = '\\';
								}
							}
						}
						
						if(length < 510)
						{
							if(isValue)
							{
								buffer2[length++] = buffer[i];
							}
							else
							{
								buffer1[length++] = buffer[i];
							}
						}
					}
					else
					{
						if(stored)
						{
							stored = 0;
						}
						else
						{
							IntToString(i, buffer1, sizeof(buffer1));
						}
						
						if(!KvJumpToKey(this, buffer1, true))
						{
							KvRewind(this);
							return false;
						}
						
						//LogToFileEx("lastdiscordset.txt", "192 | %s: [\n", buffer1);
						KvSetNum(this, "_array", true);
						isValue = false;
						stored = 0;
					}
				}
				case '}', ']':	// Back section/array
				{
					if(inString)
					{
						if(isSlash)
						{
							isSlash = false;
							if(length < 510)
							{
								if(isValue)
								{
									buffer2[length++] = '\\';
								}
								else
								{
									buffer1[length++] = '\\';
								}
							}
						}
						
						if(length < 510)
						{
							if(isValue)
							{
								buffer2[length++] = buffer[i];
							}
							else
							{
								buffer1[length++] = buffer[i];
							}
						}
					}
					else
					{
						if(isNum)
						{
							if(isValue)
							{
								buffer2[length] = '\0';
								isValue = false;
							}
							else
							{
								buffer1[length] = '\0';
							}
							isNum = false;
							stored++;
						}
						
						if(stored == 1)
						{
							KvSetString(this, buffer1, buffer1);
							//LogToFileEx("lastdiscordset.txt", "231 | %s", buffer1);
							stored = 0;
						}
						else if(stored)
						{
							KvSetString(this, buffer1, buffer2);
							//LogToFileEx("lastdiscordset.txt", "237 | %s: %s", buffer1, buffer2);
							stored = 0;
						}
							
						if(i == size)
							return true;
						
						if(!KvGoBack(this))
						{
							KvRewind(this);
							return false;
						}
						
						//LogToFileEx("lastdiscordset.txt", "247 | %c", buffer[i]);
					}
				}
				case '"':	// Start/end string
				{
					if(isSlash)
					{
						if(length < 510)
						{
							if(isValue)
							{
								buffer2[length++] = buffer[i];
							}
							else
							{
								buffer1[length++] = buffer[i];
							}
						}
						isSlash = false;
					}
					else if(inString)	// End
					{
						if(isValue)
						{
							buffer2[length] = '"';
							buffer2[length+1] = '\0';
							inString = false;
							isValue = false;
							stored++;
						}
						else
						{
							buffer1[length] = '"';
							buffer1[length+1] = '\0';
							inString = false;
							stored++;
						}
					}
					else	// Start
					{
						if(isValue)
						{
							buffer2[0] = '"';
						}
						else
						{
							buffer1[0] = '"';
						}
						
						length = 1;
						inString = true;
					}
				}
				case ':':	// Key to value
				{
					if(inString)
					{
						if(isSlash)
						{
							isSlash = false;
							if(length < 510)
							{
								if(isValue)
								{
									buffer2[length++] = '\\';
								}
								else
								{
									buffer1[length++] = '\\';
								}
							}
						}
						
						if(length < 510)
						{
							if(isValue)
							{
								buffer2[length++] = buffer[i];
							}
							else
							{
								buffer1[length++] = buffer[i];
							}
						}
					}
					else
					{
						isValue = true;
					}
				}
				case ',':	// Next key
				{
					if(inString)
					{
						if(isSlash)
						{
							isSlash = false;
							if(length < 510)
							{
								if(isValue)
								{
									buffer2[length++] = '\\';
								}
								else
								{
									buffer1[length++] = '\\';
								}
							}
						}
						
						if(length < 510)
						{
							if(isValue)
							{
								buffer2[length++] = buffer[i];
							}
							else
							{
								buffer1[length++] = buffer[i];
							}
						}
					}
					else if(stored || isNum)
					{
						if(isNum)
						{
							stored++;
							isNum = false;
							if(isValue)
							{
								buffer2[length] = '\0';
								isValue = false;
							}
							else
							{
								buffer1[length] = '\0';
							}
						}
						
						if(stored == 1)
						{
							KvSetString(this, buffer1, buffer1);
							//LogToFileEx("lastdiscordset.txt", "348 | %s", buffer1);
						}
						else
						{
							KvSetString(this, buffer1, buffer2);
							//LogToFileEx("lastdiscordset.txt", "353 | %s: %s", buffer1, buffer2);
						}
						
						stored = 0;
					}
				}
				case '\\':	// Special
				{
					if(isSlash)
					{
						if(length < 510)
						{
							if(isValue)
							{
								buffer2[length++] = buffer[i];
							}
							else
							{
								buffer1[length++] = buffer[i];
							}
						}
						
						isSlash = false;
					}
					else
					{
						isSlash = true;
					}
				}
				case ' ':	// Space
				{
					if(inString)
					{
						if(isSlash)
						{
							isSlash = false;
							if(length < 510)
							{
								if(isValue)
								{
									buffer2[length++] = '\\';
								}
								else
								{
									buffer1[length++] = '\\';
								}
							}
						}
						
						if(length < 510)
						{
							if(isValue)
							{
								buffer2[length++] = buffer[i];
							}
							else
							{
								buffer1[length++] = buffer[i];
							}
						}
					}
				}
				default:	// Letter/Number
				{
					if(!inString && !isNum)
					{
						isNum = true;
						length = 0;
					}
					
					if(isSlash)
					{
						isSlash = false;
						if(length < 510)
						{
							if(isValue)
							{
								buffer2[length++] = '\\';
							}
							else
							{
								buffer1[length++] = '\\';
							}
						}
					}
						
					if(length < 510)
					{
						if(isValue)
						{
							buffer2[length++] = buffer[i];
						}
						else
						{
							buffer1[length++] = buffer[i];
						}
					}
				}
			}
		}
		KvRewind(this);
		return true;
	}
	
	public bool ImportFromFile(const char[] file)
	{
		return false;
	}
	
	public void Import(Json other)
	{
		KvCopySubkeys(other, this);
	}
	
	public void SetString(const char[] key, const char[] value)
	{
		MAKEKEYSTRING
		
		if(StrEqual(value, "null"))
		{
			KvSetString(this, key2, value);
		}
		else
		{
			size = strlen(value)+3;
			char[] value2 = new char[size];
			Format(value2, size, "\"%s\"", value);
			KvSetString(this, key2, value2);
		}
	}
	
	public void SetNum(const char[] key, int value)
	{
		MAKEKEYSTRING
		
		KvSetNum(this, key2, value);
	}
	
	public void SetUInt64(const char[] key, const int value[2])
	{
		MAKEKEYSTRING
		
		if(value[0])
		{
			char buffer[24];
			FormatEx(buffer, sizeof(buffer), "\"%d%d\"", value[0], value[1]);
			KvSetString(this, key2, buffer);
		}
		else
		{
			char buffer[14];
			FormatEx(buffer, sizeof(buffer), "\"%d\"", buffer);
			KvSetString(this, key2, buffer);
		}
	}
	
	public void SetFloat(const char[] key, float value)
	{
		MAKEKEYSTRING
		
		char buffer[18];
		FormatEx(buffer, sizeof(buffer), "\"%f\"", value);
		KvSetString(this, key2, buffer);
	}
	
	public void SetColor(const char[] key, int r, int g, int b, int a=0)
	{
		MAKEKEYSTRING
		
		KvSetNum(this, key2, r * 65536 + g * 256 + b);
	}
	
	public void SetColor4(const char[] key, const int color[4])
	{
		MAKEKEYSTRING
		
		KvSetNum(this, key2, color[0] * 65536 + color[1] * 256 + color[2]);
	}
	
	public void SetVector(const char[] key, const float vec[3])
	{
		MAKEKEYSTRING
		
		char buffer[48];
		FormatEx(buffer, sizeof(buffer), "[\"%f\" \"%f\" \"%f\"]", vec[0], vec[1], vec[2]);
		KvSetString(this, key2, buffer);
	}
	
	public void SetBool(const char[] key, bool value)
	{
		MAKEKEYSTRING
		
		KvSetString(this, key2, value ? "true" : "false");
	}
	
	public void GetString(const char[] key, char[] value, int maxlength, const char[] defvalue="")
	{
		MAKEKEYSTRING
		
		KvGetString(this, key2, value, maxlength, defvalue);
		if(value[0] == '"')
		{
			size = strlen(value)-1;
			if(size > 0 && value[size] == '"')
			{
				value[size] = '\0';
				strcopy(value, maxlength, value[1]);
			}
		}
	}
	
	public int GetNum(const char[] key, int defvalue=0)
	{
		MAKEKEYSTRING
		
		return KvGetNum(this, key2, defvalue);
	}
	
	public float GetFloat(const char[] key, float defvalue=0.0)
	{
		MAKEKEYSTRING
		
		char buffer[18];
		FloatToString(defvalue, buffer, sizeof(buffer));
		KvGetString(this, key2, buffer, sizeof(buffer), buffer);
		if(buffer[0] != '"')
			return StringToFloat(buffer);
		
		size = strlen(buffer)-1;
		if(size > 0 && buffer[size] == '"')
			buffer[size] = '\0';
		
		return StringToFloat(buffer[1]);
	}
	
	public void GetColor(const char[] key, int &r=0, int &g=0, int &b=0, int &a=0)
	{
		MAKEKEYSTRING
		
		int value = KvGetNum(this, key2);
		r = value / 65536;
		g = (value % 65536) / 256;
		b = value % 256;
		a = 255;
	}
	
	public void GetColor4(const char[] key, int color[4])
	{
		MAKEKEYSTRING
		
		int value = KvGetNum(this, key2);
		color[0] = value / 65536;
		color[1] = (value % 65536) / 256;
		color[2] = value % 256;
		color[3] = 255;
	}
	
	public void GetUInt64(const char[] key, int value[2], int defvalue[2]={0,0})
	{
		MAKEKEYSTRING
		
		char buffer[26];
		KvGetString(this, key2, buffer, sizeof(buffer));
		if(buffer[0] == '"')
		{
			size = strlen(buffer)-1;
			if(size > 0 && buffer[size] == '"')
			{
				buffer[size] = '\0';
				KvSetString(this, key2, buffer[1]);
				
				KvGetUInt64(this, key2, value, defvalue);
				
				buffer[size] = '"';
				KvSetString(this, key2, buffer);
			}
			else
			{
				KvSetString(this, key2, buffer[1]);
				
				KvGetUInt64(this, key2, value, defvalue);
				
				KvSetString(this, key2, buffer);
			}
		}
		else
		{
			KvGetUInt64(this, key2, value, defvalue);
		}
	}
	
	public void GetVector(const char[] key, float vec[3], const float defvalue[3]={0.0, 0.0, 0.0})
	{
		MAKEKEYSTRING
		
		char buffer[48];
		KvGetString(this, key2, buffer, sizeof(buffer));
		if(buffer[0] == '"')
		{
			size = strlen(buffer)-1;
			if(size > 0 && buffer[size] == '"')
			{
				buffer[size] = '\0';
				KvSetString(this, key2, buffer[1]);
				
				KvGetVector(this, key2, vec, defvalue);
				
				buffer[size] = '"';
				KvSetString(this, key2, buffer);
			}
			else
			{
				KvSetString(this, key2, buffer[1]);
				
				KvGetVector(this, key2, vec, defvalue);
				
				KvSetString(this, key2, buffer);
			}
		}
		else
		{
			KvGetVector(this, key2, vec, defvalue);
		}
	}
	
	public bool GetBool(const char[] key)
	{
		MAKEKEYSTRING
		
		char buffer[6];
		KvGetString(this, key2, buffer, sizeof(buffer));
		return !StrContains(buffer, "true", false);
	}
	
	public bool JumpToKey(const char[] key, bool create=false, bool array=false)
	{
		MAKEKEYSTRING
		
		bool result = KvJumpToKey(this, key2, create);
		if(result && create && array)
			KvSetNum(this, "_array", true);
		
		return result;
	}
}