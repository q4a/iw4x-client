#pragma once

namespace Components
{
	class ServerInfo : public Component
	{
	public:
		ServerInfo();
		~ServerInfo();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "ServerInfo"; };
#endif

		static Utils::InfoString GetInfo();

	private:
		class Container
		{
		public:
			class Player
			{
			public:
				int ping;
				int score;
				std::string name;
			};

			unsigned int currentPlayer;
			std::vector<Player> playerList;
			Network::Address target;
		};

		static Container PlayerContainer;

		static void ServerStatus(UIScript::Token);

		static unsigned int GetPlayerCount();
		static const char* GetPlayerText(unsigned int index, int column);
		static void SelectPlayer(unsigned int index);

		static void DrawScoreboardInfo(void* a1);
		static void DrawScoreboardStub();
	};
}
