#include "..\..\STDInclude.hpp"

using namespace std::literals;

namespace Components
{
	ConnectProtocol::Container ConnectProtocol::ConnectContainer = { false, false, "" };

	bool ConnectProtocol::Evaluated()
	{
		return ConnectProtocol::ConnectContainer.Evaluated;
	}

	bool ConnectProtocol::Used()
	{
		if (!ConnectProtocol::Evaluated())
		{
			ConnectProtocol::EvaluateProtocol();
		}

		return (ConnectProtocol::ConnectContainer.ConnectString.size() > 0);
	}

	bool ConnectProtocol::InstallProtocol()
	{
		HKEY hKey = NULL;
		std::string data;

		char ownPth[MAX_PATH] = { 0 };
		char workdir[MAX_PATH] = { 0 };
		char regred[MAX_PATH] = { 0 };

		DWORD dwsize = MAX_PATH;
		HMODULE hModule = GetModuleHandle(NULL);

		if (hModule != NULL)
		{
			if (GetModuleFileName(hModule, ownPth, MAX_PATH) == ERROR)
			{
				return false;
			}

			if (GetModuleFileName(hModule, workdir, MAX_PATH) == ERROR)
			{
				return false;
			}
			else
			{
				char* endPtr = strstr(workdir, "iw4x.exe");
				if (endPtr != NULL)
				{
					*endPtr = 0;
				}
				else
				{
					return false;
				}
			}
		}
		else
		{
			return false;
		}

		SetCurrentDirectory(workdir);

		LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Classes\\iw4x\\shell\\open\\command", 0, KEY_ALL_ACCESS, &hKey);
		if (openRes == ERROR_SUCCESS)
		{
			// Check if the game has been moved.
			openRes = RegQueryValueEx(hKey, 0, 0, 0, reinterpret_cast<BYTE*>(regred), &dwsize);
			if (openRes == ERROR_SUCCESS)
			{
				char* endPtr = strstr(regred, "\" \"%1\"");
				if (endPtr != NULL)
				{
					*endPtr = 0;
				}
				else
				{
					return false;
				}

				RegCloseKey(hKey);
				if (strcmp(regred + 1, ownPth))
				{
					openRes = RegDeleteKey(HKEY_CURRENT_USER, "SOFTWARE\\Classes\\iw4x");
				}
				else
				{
					return true;
				}
			}
			else
			{
				openRes = RegDeleteKey(HKEY_CURRENT_USER, "SOFTWARE\\Classes\\iw4x");
			}
		}
		else
		{
			openRes = RegDeleteKey(HKEY_CURRENT_USER, "SOFTWARE\\Classes\\iw4x");
		}

		// Open SOFTWARE\\Classes
		openRes = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Classes", 0, KEY_ALL_ACCESS, &hKey);

		if (openRes != ERROR_SUCCESS)
		{
			return false;
		}

		// Create SOFTWARE\\Classes\\iw4x
		openRes = RegCreateKeyEx(hKey, "iw4x", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, 0);

		if (openRes != ERROR_SUCCESS)
		{
			return false;
		}

		// Write URL:iw4x Protocol
		data = "URL:iw4x Protocol";
		openRes = RegSetValueEx(hKey, "URL Protocol", 0, REG_SZ, reinterpret_cast<const BYTE*>(data.data()), data.size() + 1);

		if (openRes != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return false;
		}

		// Create SOFTWARE\\Classes\\iw4x\\DefaultIcon
		openRes = RegCreateKeyEx(hKey, "DefaultIcon", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, 0);

		if (openRes != ERROR_SUCCESS)
		{
			return false;
		}

		data = Utils::VA("%s,1", ownPth);
		openRes = RegSetValueEx(hKey, 0, 0, REG_SZ, reinterpret_cast<const BYTE*>(data.data()), data.size() + 1);
		RegCloseKey(hKey);

		if (openRes != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return false;
		}

		openRes = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Classes\\iw4x", 0, KEY_ALL_ACCESS, &hKey);

		if (openRes != ERROR_SUCCESS)
		{
			return false;
		}

		openRes = RegCreateKeyEx(hKey, "shell\\open\\command", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, 0);

		if (openRes != ERROR_SUCCESS)
		{
			return false;
		}

		data = Utils::VA("\"%s\" \"%s\"", ownPth, "%1");
		openRes = RegSetValueEx(hKey, 0, 0, REG_SZ, reinterpret_cast<const BYTE*>(data.data()), data.size() + 1);
		RegCloseKey(hKey);

		if (openRes != ERROR_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void ConnectProtocol::EvaluateProtocol()
	{
		if (ConnectProtocol::ConnectContainer.Evaluated) return;
		ConnectProtocol::ConnectContainer.Evaluated = true;

		std::string cmdLine = GetCommandLine();

		auto pos = cmdLine.find("iw4x://");

		if (pos != std::string::npos)
		{
			cmdLine = cmdLine.substr(pos + 7);
			pos = cmdLine.find_first_of("/");

			if (pos != std::string::npos)
			{
				cmdLine = cmdLine.substr(0, pos);
			}

			ConnectProtocol::ConnectContainer.ConnectString = cmdLine;
		}
	}

	ConnectProtocol::ConnectProtocol()
	{
		// IPC handler
		IPCPipe::On("connect", [] (std::string data)
		{
			Command::Execute(Utils::VA("connect %s", data.data()), false);
		});

		// Invocation handler
		// TODO: Don't call it every frame, once is enough!
		Renderer::OnFrame([] ()
		{
			if (!ConnectProtocol::ConnectContainer.Invoked && ConnectProtocol::Used())
			{
				ConnectProtocol::ConnectContainer.Invoked = true;
				Command::Execute(Utils::VA("connect %s", ConnectProtocol::ConnectContainer.ConnectString.data()), false);
			}
		});

		ConnectProtocol::InstallProtocol();
		ConnectProtocol::EvaluateProtocol();

		// Fire protocol handlers
		// Make sure this happens after the pipe-initialization!
		if (!Singleton::IsFirstInstance() && ConnectProtocol::Used())
		{
			IPCPipe::Write("connect", ConnectProtocol::ConnectContainer.ConnectString);
			ExitProcess(0);
		}
		else
		{
			// Only skip intro here, invocation will be done later.
			Utils::Hook::Set<BYTE>(0x60BECF, 0xEB);
		}
	}
}
