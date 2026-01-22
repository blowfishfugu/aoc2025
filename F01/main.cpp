#include "../common/TxtFile.h"
#include "../common/SplitIter.h"
#include "../common/StopWatch.h"
#include <print> //<- convenient, but bloats exe-binaries-size, back to printf??

#define NOMINMAX
#include <Windows.h> //GetModuleFilename, needed to locate Data-Folder relative to executable
#include <span> //used to wrap args

using I = std::int64_t;


void run2(const fs::path& inputFile)
{
	TxtFile txt(inputFile);
	I justCount{};
	I evenCount{};
	I noneCount{};
	for (const auto& line : txt) {
		I nana = line.size() / 2;
		justCount += nana;
		if ((nana % 2) == 0) {
			evenCount += nana;
		}
		if (!line.contains("e")) {
			noneCount += nana;
		}
	}
	std::println("\npart1: justCount: {}", justCount);
	std::println("\npart2: evenCount: {}", evenCount);
	std::println("\npart3: noneCount: {}", noneCount);
}


int main(int argc, const char* argv[])
{
	StopWatch timeIt(std::cout);
	SetConsoleOutputCP(1252); //latin1

	auto args = std::span{ argv,static_cast<size_t>(argc) };
	if (args.size() < 2)
	{
		std::println("no input specified");
		return 0;
	}

	char pFile[MAX_PATH]{};
	::GetModuleFileName(NULL, pFile, MAX_PATH);
	fs::path inputFile = fs::path(pFile).parent_path().parent_path().parent_path() / "bananas.txt";
	if (!fs::exists(inputFile))
	{
		std::println("input not found: {}", inputFile.string());
		return 0;
	}
	run2(inputFile);

	return 0;
}