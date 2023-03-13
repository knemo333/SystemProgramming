#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <vector>
#include <unordered_map>
#include <regex>

namespace fs = std::filesystem;

constexpr int BUFFERSIZE = 256;
constexpr int CURSOROFFSET_X = 20;

enum class eStatus
{
	MENU,
	JSON
};

std::wstring g_dir;
std::vector<fs::path> g_jsonPaths;
std::unordered_map<fs::path, fs::file_time_type> g_jsonTime;
bool g_Exit = false;
std::unordered_map<int, bool> g_inputMap;
eStatus g_currentMode = eStatus::MENU;
int g_cursorIndex = 0;

std::wstring ReadSettingFile();
void ReadFiles(std::vector<fs::path>& vec);
void CheckInput();
void Update();
void Render();
void PrintList();
void PrintUI();
void ShowFile(int index);

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
			g_jsonPaths.push_back(entry.path());
			g_jsonTime.insert({ entry.path(), entry.last_write_time() });
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

	if (GetAsyncKeyState(VK_ESCAPE) & 1)
	{
		g_inputMap[VK_ESCAPE] = true;
	}
}

void Update()
{
	if (g_currentMode == eStatus::MENU)
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

			if (g_cursorIndex < g_jsonPaths.size())
			{
				system("cls");
				g_currentMode = eStatus::JSON;
			}
		}
	}

	if (g_currentMode == eStatus::JSON)
	{
		if (g_inputMap[VK_ESCAPE])
		{
			system("cls");
			g_currentMode = eStatus::MENU;
		}

		if (g_inputMap[VK_RETURN])
		{

		}
	}


}

void Render()
{
	gotoxy(0, 0);

	switch (g_currentMode)
	{
		case eStatus::MENU:
			PrintList();
			for (int i = 0; i < g_jsonPaths.size() + 1; ++i)
			{
				if (i == g_cursorIndex)
				{
					continue;
				}

				gotoxy(CURSOROFFSET_X, i);
				printf("  ");
			}
			gotoxy(CURSOROFFSET_X, g_cursorIndex);
			printf("←");
			break;
		case eStatus::JSON:
			ShowFile(g_cursorIndex);
			break;
		default:
			break;
	}

	PrintUI();
}

void PrintList()
{
	for (auto& e : g_jsonPaths)
	{
		std::cout << e.filename().string() << std::endl;
	}

	printf("종료\n");
}

void PrintUI()
{
	gotoxy(0, 20);

	switch (g_currentMode)
	{
		case eStatus::MENU:
			printf("[↓] : 아래 , [↑] : 위 , [Enter] : 선택");
			break;
		case eStatus::JSON:
			printf("[ESC] : 나가기");
			break;
		default:
			break;
	}
}

void ShowFile(int index)
{
	FILE* fp;
	fopen_s(&fp, g_jsonPaths[g_cursorIndex].string().c_str(), "rb"); // non-Windows use "r"
	const int bufferLength = 65536;
	char readBuffer[bufferLength] = { 0, };
#if 0
	//RapidJson의 FileReadStream을 이용해서 버퍼를 읽는다.
	FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	Document doc;
	doc.ParseStream(is);
	fclose(fp);
#else
	size_t fileSize = fread(readBuffer, 1, bufferLength, fp);
	fclose(fp);
	rapidjson::Document doc;
	doc.Parse(readBuffer);
#endif

	if (doc.HasMember("value"))
	{
		rapidjson::Value& v = doc["value"];
		std::cout << "value: " << v.GetString() << std::endl;
//		std::cout << "age: " << doc["age"].GetInt() << std::endl;
// 
// 		std::cout << "arr: ";
// 		const rapidjson::Value& arr = doc["arr"];
// 		for (rapidjson::SizeType i = 0; i < arr.Size(); i++)
// 		{
// 			std::cout << arr[i].GetInt() << "  ";
// 		}
		std::cout << std::endl;
	}
}
