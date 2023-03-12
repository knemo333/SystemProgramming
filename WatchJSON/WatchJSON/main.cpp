#include <iostream>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <vector>
#include <unordered_map>
#include <regex>

#include "rapidjson/document.h"

namespace fs = std::filesystem;

constexpr int BUFFERSIZE = 256;
constexpr int CURSOROFFSET_X = 20;

enum class eMenu
{
	LIST,
};

std::wstring g_dir;
std::vector<fs::path> g_jsonPaths;
std::unordered_map<fs::path, fs::file_time_type> g_jsonTime;
bool g_Exit = false;
std::unordered_map<int, bool> g_inputMap;
eMenu g_currentMode = eMenu::LIST;
int g_cursorIndex = 0;

std::wstring ReadSettingFile();
void ReadFiles(std::vector<fs::path>& vec);
void CheckInput();
void Update();
void Render();
void PrintList();

void gotoxy(int x, int y)
{
	COORD Cur;
	Cur.X = x;
	Cur.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Cur);
}

int main()
{
	CONSOLE_CURSOR_INFO ConsoleCursor;
	ConsoleCursor.bVisible = false;
	ConsoleCursor.dwSize = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ConsoleCursor);

	g_dir = ReadSettingFile();
	ReadFiles(g_jsonPaths);

	while (!g_Exit)
	{
		CheckInput();
		Update();
		Render();
	}

	return 0;
}

std::wstring ReadSettingFile()
{
	WCHAR* directory;
	directory = new WCHAR[BUFFERSIZE];
	memset(directory, 0x00, BUFFERSIZE);

	GetPrivateProfileString(L"JSON", L"src", L"NOTFOUND", directory, BUFFERSIZE, L".\\setting.ini");

	return directory;
}

void ReadFiles(std::vector<fs::path>& vec)
{
	std::regex re("^\\S+(.json)$");
	fs::directory_iterator iter(g_dir);

	while (iter != fs::end(iter))
	{
		const fs::directory_entry& entry = *iter;
		
		std::string temp = entry.path().string();

		if (std::regex_match(temp, re))
		{
			g_jsonPaths.push_back(entry.path().filename());
			g_jsonTime.insert({ entry.path(), entry.last_write_time()});
		}

		++iter;
	}
}

void CheckInput()
{
	for (auto& e : g_inputMap)
	{
		e.second = false;
	}

	if (GetAsyncKeyState(VK_UP) & 1)
	{
		g_inputMap[VK_UP] = true;
	}

	if (GetAsyncKeyState(VK_DOWN) & 1)
	{
		g_inputMap[VK_DOWN] = true;
	}

	if (GetAsyncKeyState(VK_RETURN) & 1)
	{
		g_inputMap[VK_RETURN] = true;
	}
}

void Update()
{
	if (g_inputMap[VK_UP])
	{
		--g_cursorIndex;
		if (g_cursorIndex < 0)
		{
			g_cursorIndex = g_jsonPaths.size();
		}
	}

	if (g_inputMap[VK_DOWN])
	{
		++g_cursorIndex;
		g_cursorIndex %= g_jsonPaths.size() + 1;
	}

	if (g_inputMap[VK_RETURN])
	{
		if (g_cursorIndex == g_jsonPaths.size())
		{
			g_Exit = true;
		}



	}
}

void Render()
{
	gotoxy(0, 0);

	switch (g_currentMode)
	{
		case eMenu::LIST:
			PrintList();
			break;
		default:
			break;
	}

	for (int i = 0; i < g_jsonPaths.size() + 1; ++i)
	{
		if (i == g_cursorIndex)
		{
			continue;
		}

		gotoxy(CURSOROFFSET_X, i);
		std::cout << "  ";
	}
	gotoxy(CURSOROFFSET_X, g_cursorIndex);
	std::cout << "¡ç";
}

void PrintList()
{
	for (auto e : g_jsonPaths)
	{
		std::cout << e.string() << std::endl;
	}

	std::cout << "Á¾·á" << std::endl;
}
