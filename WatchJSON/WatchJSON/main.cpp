#include <iostream>
#include <windows.h>
#include <vector>
#include <io.h>

constexpr int BUFFERSIZE = 256;

std::wstring ReadDirectory();
std::vector<std::string> get_files_inDirectory(const std::string& _path, const std::string& _filter);

int main()
{
	std::wstring dir;
	dir = ReadDirectory();

	std::wcout << dir << std::endl;

	std::string dir_a;
	dir_a.assign(dir.begin(), dir.end());

	std::vector<std::string> files = get_files_inDirectory(dir_a, "*.json");

	for (auto& file : files)
	{
		std::cout << file << std::endl;
	}

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

// find all matched file in specified directory
// [INPUT]
//   >> const std::string& _path        Search path        ex) c:/directory/
//   >> const std::string& _filter        Search filter    ex) *.exe or *.*
// [RETURN]
//   >> std::vector<std::string>        All matched file name & extension
std::vector<std::string> get_files_inDirectory(const std::string& _path, const std::string& _filter)
{
	std::string searching = _path + _filter;

	std::vector<std::string> return_;

	_finddata_t fd;
	long handle = _findfirst(searching.c_str(), &fd);  //현재 폴더 내 모든 파일을 찾는다.

	if (handle == -1)    return return_;

	int result = 0;
	do
	{
		return_.push_back(fd.name);
		result = _findnext(handle, &fd);
	} while (result != -1);

	_findclose(handle);

	return return_;
}
