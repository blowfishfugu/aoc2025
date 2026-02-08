#include "../common/TxtFile.h"
#include "../common/SplitIter.h"
#include "../common/StopWatch.h"
#include <print> //<- convenient, but bloats exe-binaries-size, back to printf??

#define NOMINMAX
#include <Windows.h> //GetModuleFilename, needed to locate Data-Folder relative to executable


#include <ranges>
#include <iomanip>
#include <map>
#include <array>
#include <numeric>
#include <set>

using I = std::int64_t;

void run(const fs::path& inputFile)
{
	struct tunnel {
		char name{};
		I start{};
		I end{};
		I length{};
		I poweredlength{};
	};
	std::map<char, tunnel> tunnels;
	TxtFile txt(inputFile);
	std::string_view line = *txt.begin();
	for (I pos{}; pos < line.size();++pos) {
		char c = line[pos];
			auto found = tunnels.find(c);
			if (found == tunnels.end()) {
				tunnels.emplace(c, tunnel{ c,pos,{},{} });
			}
			else {
				tunnel& t = found->second;
				t.end = pos;
				t.length = t.end - t.start;
				t.poweredlength = t.length;
				if (c >= 'A' && c <= 'Z') {
					t.poweredlength = -t.length;
				}
			}
	}

	std::set<char> visited;
	I travelled{};
	I powertravel{};
	for (I pos = 0ll; pos>=0ll && pos < line.size();) {
		char c = line[pos];
		visited.emplace(c);
		tunnel& t = tunnels[c];
		if (pos == t.start) {
			pos = t.end;
		}
		else {
			pos = t.start;
		}
		travelled += t.length;
		powertravel += t.poweredlength;
		++pos;
	}

	std::println("\npart1: tunnelsum: {}", travelled);
	std::println("\npart3: powsersum: {}", powertravel);

	std::string inOrder;
	for (const char& c : line) {
		if( !visited.contains(c)){
			inOrder.push_back(c);
			visited.emplace(c);
		}
	}
	std::println("\npart2: unvisited: {}", inOrder);

}

fs::path FindInput() {
	char pFile[MAX_PATH]{};
	::GetModuleFileName(NULL, pFile, MAX_PATH);
	//src/bin/debug/modulefile.exe
	//src/bin/release/modulefile.exe
	//src/inputs.txt
	fs::path inputFile = fs::path(pFile).parent_path().parent_path().parent_path() / "tunnels.txt";
	if (!fs::exists(inputFile))
	{
		std::println("input not found: {}", inputFile.string());
		return {};
	}
	return inputFile;
}


int main(int argc, const char* argv[])
{
	StopWatch timeIt(std::cout);
	SetConsoleOutputCP(1252); //latin1

	auto inputFile = FindInput();
	run(inputFile);
	return 0;
}