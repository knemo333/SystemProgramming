#include <iostream>
#include <filesystem>
#include <windows.h>
#include <vector>
#include <regex>

namespace fs = std::filesystem;

constexpr int BUFFERSIZE = 256;

std::wstring ReadDirectory();

int main()
{
	std::wstring dir;
	dir = ReadDirectory();

	fs::path jsonPath(dir);
	fs::directory_iterator iter(dir);
	while (iter != fs::end(iter))
	{
		const fs::directory_entry& entry = *iter;
		std::cout << entry.path() << std::endl;
		++iter;
	}

	Sleep(2000);
	return 0;
}

std::wstring ReadDirectory()
{
	WCHAR* directory;
	directory = new WCHAR[BUFFERSIZE];
	memset(directory, 0x00, BUFFERSIZE);

	GetPrivateProfileString(L"JSON", L"src", L"babo", directory, BUFFERSIZE, L".\\setting.ini");

	return directory;
}
