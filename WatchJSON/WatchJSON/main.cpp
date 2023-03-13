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
bool g_Changed = false;
LARGE_INTEGER frequency;
LARGE_INTEGER startTick;
LARGE_INTEGER endTick;
double elipseTick;
double deltaTime;

std::wstring ReadSettingFile();
void ReadFiles(std::vector<fs::path>& vec);
void Input();
void Update();
void Render();
void PrintList();
void PrintUI();
bool CheckDiff(int index);
void ApplyDiff(int index);
void ShowFile(int index);
void MeasureTime();

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
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startTick);	// 초기 시작 값

	g_dir = ReadSettingFile();
	ReadFiles(g_jsonPaths);

	while (!g_Exit)
	{
		MeasureTime();
		Input();
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

		std::string pathString = entry.path().string();

		if (std::regex_match(pathString, re))
		{
			g_jsonPaths.push_back(entry.path());
			g_jsonTime.insert({ entry.path(), entry.last_write_time() });
		}

		++iter;
	}
}

void Input()
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
				ShowFile(g_cursorIndex);
				ApplyDiff(g_cursorIndex);
			}
		}
	}

	if (g_currentMode == eStatus::JSON)
	{
		if (CheckDiff(g_cursorIndex))
		{
			g_Changed = true;
		}

		if (g_inputMap[VK_ESCAPE])
		{
			system("cls");
			g_currentMode = eStatus::MENU;
		}

		if (g_inputMap[VK_RETURN])
		{
			if (g_Changed)
			{
				g_Changed = false;
				system("cls");
				ShowFile(g_cursorIndex);
			}
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
	gotoxy(0, 25);

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

	if (g_Changed && g_currentMode == eStatus::JSON)
	{
		gotoxy(0, 24);
		
		std::cout << g_jsonPaths[g_cursorIndex].filename().string() << " 파일이 수정됐습니다. 엔터를 누르면 다시 로드합니다.";
	}
	else
	{
		gotoxy(0, 24);
		printf("                                                                           ");
	}
}

bool CheckDiff(int index)
{
	fs::directory_iterator iter(g_dir);

	fs::path currentPath = g_jsonPaths[g_cursorIndex];

	while (iter != fs::end(iter))
	{
		const fs::directory_entry& entry = *iter;

		std::string pathString = entry.path().string();

		if (pathString.compare(currentPath.string()) == 0)
		{
			if (g_jsonTime.at(currentPath) != entry.last_write_time())
			{
				return true;
			}
		}

		++iter;
	}
	return false;
}

void ApplyDiff(int index)
{
	fs::directory_iterator iter(g_dir);

	fs::path currentPath = g_jsonPaths[g_cursorIndex];

	while (iter != fs::end(iter))
	{
		const fs::directory_entry& entry = *iter;

		std::string pathString = entry.path().string();

		if (pathString.compare(currentPath.string()) == 0)
		{
			g_jsonTime[currentPath] = entry.last_write_time();
			return;
		}

		++iter;
	}
}

void ShowFile(int index)
{
	FILE* fp;
	fopen_s(&fp, g_jsonPaths[g_cursorIndex].string().c_str(), "rb"); // non-Windows use "r"
	const int bufferLength = 65536;
	//char* readBuffer = new char(bufferLength);
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

void MeasureTime()
{
	QueryPerformanceCounter(&endTick);

	elipseTick = (double)(endTick.QuadPart - startTick.QuadPart) /
		(double)(frequency.QuadPart);
	deltaTime = elipseTick * 1000;	// 1000을 곱하면 ms 단위 안곱하면 s단위

	QueryPerformanceCounter(&startTick);
}
