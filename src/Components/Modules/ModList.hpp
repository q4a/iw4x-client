#pragma once

namespace Components
{
	class ModList : public Component
	{
	public:
		ModList();
		~ModList();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "ModList"; };
#endif

		static void RunMod(std::string mod);

	private:
		static std::vector<std::string> Mods;
		static unsigned int CurrentMod;

		static bool HasMod(std::string modName);

		static unsigned int GetItemCount();
		static const char* GetItemText(unsigned int index, int column);
		static void Select(unsigned int index);
		static void UIScript_LoadMods(UIScript::Token);
		static void UIScript_RunMod(UIScript::Token);
		static void UIScript_ClearMods(UIScript::Token);
	};
}
