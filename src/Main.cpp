#include "STDInclude.hpp"

namespace Main
{
	static Utils::Hook EntryPointHook;

	void Initialize()
	{
		Main::EntryPointHook.Uninstall();

		Utils::Cryptography::Rand::Initialize();
		Components::Loader::Initialize();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		if (Components::Loader::PerformingUnitTests())
		{
			DWORD result = (Components::Loader::PerformUnitTests() ? 0 : -1);
			Components::Loader::Uninitialize();
			ExitProcess(result);
		}
#else
		if (Components::Flags::HasFlag("tests"))
		{
			Components::Logger::Print("Unit tests are disabled outside the debug environment!\n");
		}
#endif
	}

	void Uninitialize()
	{
		Components::Loader::Uninitialize();
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// Ensure we're working with our desired binary
		if (Utils::Hook::Get<DWORD>(0x4C0FFF) != 0x6824748B)
		{
			return FALSE;
		}

		DWORD oldProtect;
		VirtualProtect(GetModuleHandle(NULL), 0x6C73000, PAGE_EXECUTE_READWRITE, &oldProtect);

		Main::EntryPointHook.Initialize(0x6BAC0F, [] ()
		{
			__asm
			{
				// This has to be called, otherwise the hook is not uninstalled and we're looping into infinity
				call Main::Initialize

				mov eax, 6BAC0Fh
				jmp eax
			}

		})->Install();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		Main::Uninitialize();
	}

	return TRUE;
}
