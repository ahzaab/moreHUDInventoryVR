#include "skse64/PluginAPI.h"
#include "sksevr/skse64_common/skse_version.h"
#include "AHZScaleform.h"
#include "AHZScaleformHook.h"
#include <shlobj.h>
#include "AHZConsole.h"
#include "skse64/ScaleformCallbacks.h"          // for GFxFunctionHandler
#include <stddef.h>                             // for NULL
#include "AHZConfiguration.h"
#include "AHZPapyrusMoreHudIE.h"
#include "skse64/HashUtil.h"

using namespace std;

#define PLUGIN_NAME  ("AHZmoreHUDInventory") 

IDebugLog	gLog;
PluginHandle	g_pluginHandle = kPluginHandle_Invalid;
CAHZConfiguration g_ahzConfiguration;
CAHZScaleform m_ahzScaleForm;
static UInt32 g_skseVersion = 0;
SKSEScaleformInterface		* g_scaleform = NULL;
SKSEMessagingInterface *g_skseMessaging = NULL;
SKSEPapyrusInterface * g_sksePapyrus = NULL;
AHZEventHandler menuEvent;
static string s_lastIconName;

// Just initialize to start routing to the console window
CAHZDebugConsole theDebugConsole;

/**** scaleform functions ****/

class SKSEScaleform_InstallHooks : public GFxFunctionHandler
{
public:
	virtual void	Invoke(Args * args)
	{
	}
};

class SKSEScaleform_GetCurrentMenu: public GFxFunctionHandler
{
public:
	virtual void	Invoke(Args * args)
	{
		args->result->SetString(g_currentMenu.c_str());
	}
};

class SKSEScaleform_ShowBookRead : public GFxFunctionHandler
{
public:
	virtual void	Invoke(Args * args)
	{
		args->result->SetBool(m_ahzScaleForm.m_showBookRead);
	} 
};


class SKSEScaleform_EnableItemCardResize : public GFxFunctionHandler
{
public:
	virtual void	Invoke(Args * args)
	{
		args->result->SetBool(m_ahzScaleForm.m_enableItemCardResize);
	}
};

class SKSEScaleform_GetWasBookRead : public GFxFunctionHandler
{
public:
	virtual void	Invoke(Args * args)
	{
		if (args && args->args && args->numArgs > 0 && args->args[0].GetType() == GFxValue::kType_Number)
		{
			UInt32 formID = (UInt32)args->args[0].GetNumber();

			TESForm* bookForm = LookupFormByID(formID);
			args->result->SetBool(m_ahzScaleForm.GetWasBookRead(bookForm));
		}
	}
};

class SKSEScaleform_GetIconForItemId : public GFxFunctionHandler
{
public:
	virtual void	Invoke(Args * args) 
	{
		if (args && args->args && args->numArgs > 1 && args->args[0].GetType() == GFxValue::kType_Number && args->args[1].GetType() == GFxValue::kType_String)
		{
			UInt32 formID = (UInt32)args->args[0].GetNumber();
			
			const char * name = args->args[1].GetString();
			
			if (!name)
			{
				return;
			}

			SInt32 itemId = (SInt32)HashUtil::CRC32(name, formID & 0x00FFFFFF);
			s_lastIconName.clear();
			s_lastIconName.append(papyrusMoreHudIE::GetIconName(itemId));
			GFxValue obj;
			args->movie->CreateObject(&obj);
			GFxValue	fxValue;
			fxValue.SetString(s_lastIconName.c_str());
			obj.SetMember("iconName", &fxValue);

			// Add the object to the scaleform function
			args->args[2].SetMember("returnObject", &obj);
		}
	}
};

class SKSEScaleform_HasFormId : public GFxFunctionHandler
{
public:
	virtual void	Invoke(Args* args)
	{
		if (args && args->args && args->numArgs > 1 && args->args[0].GetType() == GFxValue::kType_String && args->args[1].GetType() == GFxValue::kType_Number)
		{
			const char* iconName = args->args[0].GetString();

			UInt32 formId = (UInt32)args->args[1].GetNumber();

			if (!iconName)
			{
				return;
			}

			args->result->SetBool(papyrusMoreHudIE::HasForm(string(iconName), formId));
		}
	}
};

class SKSEScaleform_AHZLog : public GFxFunctionHandler
{
public:
	virtual void	Invoke(Args * args)
	{
#if _DEBUG
		_MESSAGE("%s", args->args[0].GetString());
#else  // Only allow release verbosity for a release build
		if (args && args->args && args->numArgs > 1 && args->args[1].GetType() == GFxValue::kType_Bool && args->args[1].GetBool())
		{
			_MESSAGE("%s", args->args[0].GetString());
		}
#endif
	}
};


bool RegisterScaleform(GFxMovieView * view, GFxValue * root)
{
	RegisterFunction <SKSEScaleform_InstallHooks>(root, view, "InstallHooks");
	RegisterFunction <SKSEScaleform_ShowBookRead>(root, view, "ShowBookRead");
	RegisterFunction <SKSEScaleform_AHZLog>(root, view, "AHZLog");
	RegisterFunction <SKSEScaleform_GetCurrentMenu>(root, view, "GetCurrentMenu");
	RegisterFunction <SKSEScaleform_EnableItemCardResize>(root, view, "EnableItemCardResize");
	RegisterFunction <SKSEScaleform_GetWasBookRead>(root, view, "GetWasBookRead");
	RegisterFunction <SKSEScaleform_GetIconForItemId>(root, view, "GetIconForItemId");
	RegisterFunction <SKSEScaleform_HasFormId>(root, view, "HasFormId");
	


	MenuManager::GetSingleton()->MenuOpenCloseEventDispatcher()->AddEventSink(&menuEvent);
	return true;
}

// Listens to events dispatched by SKSE
void EventListener(SKSEMessagingInterface::Message* msg)
{
	if (!msg)
	{
		return;
	}

	if (string(msg->sender) == "SKSE" && msg->type == SKSEMessagingInterface::kMessage_DataLoaded)
	{
  
	}
}

extern "C"
{
	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{
		string logPath = "\\My Games\\Skyrim Special Edition\\SKSE\\";
		logPath.append(PLUGIN_NAME);
		logPath.append(".log");

		gLog.OpenRelative(CSIDL_MYDOCUMENTS, logPath.c_str());
		gLog.SetPrintLevel(IDebugLog::kLevel_VerboseMessage);
		gLog.SetLogLevel(IDebugLog::kLevel_Message);

		// populate info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "Ahzaab's moreHUD Inventory Plugin";
		info->version = PLUGIN_VERSION;

		// store plugin handle so we can identify ourselves later
		g_pluginHandle = skse->GetPluginHandle();

		if (skse->isEditor)
		{
			_ERROR("loaded in editor, marking as incompatible");

			return false;
		}
		else if (skse->runtimeVersion < (MAKE_EXE_VERSION(1, 5, 97)))
		{
			_ERROR("unsupported runtime version %08X", skse->runtimeVersion);

			return false;
		}
		else if (SKSE_VERSION_RELEASEIDX < 53)
		{
			_ERROR("unsupported skse release index %08X", SKSE_VERSION_RELEASEIDX);

			return false;
		}

		// get the scaleform interface and query its version
		g_scaleform = (SKSEScaleformInterface *)skse->QueryInterface(kInterface_Scaleform);
		if (!g_scaleform)
		{
			_ERROR("couldn't get scaleform interface");
			return false;
		}

		if (g_scaleform->interfaceVersion < SKSEScaleformInterface::kInterfaceVersion)
		{
			_ERROR("scaleform interface too old (%d expected %d)", g_scaleform->interfaceVersion, SKSEScaleformInterface::kInterfaceVersion);
			return false;
		}

		// ### do not do anything else in this callback
		// ### only fill out PluginInfo and return true/false

		g_skseMessaging = (SKSEMessagingInterface *)skse->QueryInterface(kInterface_Messaging);
		if (!g_skseMessaging)
		{
			_ERROR("couldn't get messaging interface");
			return false;
		}

	  g_sksePapyrus = (SKSEPapyrusInterface *)skse->QueryInterface(kInterface_Papyrus);
	  if (!g_skseMessaging)
	  {
		  _ERROR("couldn't get Papyrus interface");
		  return false;
	  }

		// supported runtime version
		return true;
	}

	void RegisterForIventory(GFxMovieView * view, GFxValue * object, InventoryEntryData * item)
	{
		m_ahzScaleForm.ExtendItemCard(view, object, item);
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse)
	{
		//while (!IsDebuggerPresent())
		//{
		//   Sleep(10);
		//}

		//Sleep(1000 * 2);

		g_ahzConfiguration.Initialize(PLUGIN_NAME);
		m_ahzScaleForm.Initialize();

		// register scaleform callbacks
		g_scaleform->Register("AHZmoreHUDInventory", RegisterScaleform);

		g_scaleform->RegisterForInventory(RegisterForIventory);

		// Register listener for the gme loaded event
		g_skseMessaging->RegisterListener(skse->GetPluginHandle(), "SKSE", EventListener);

	  // Register Papyrus functions
	  g_sksePapyrus->Register(papyrusMoreHudIE::RegisterFuncs);

		_MESSAGE("%s -v%d Loaded", PLUGIN_NAME, PLUGIN_VERSION);
		return true;
	}
};