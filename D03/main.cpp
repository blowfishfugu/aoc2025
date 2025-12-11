#include "../common/TxtFile.h"
#include "../common/SplitIter.h"
#include "../common/StopWatch.h"
#include <print> //<- convenient, but bloats exe-binaries-size, back to printf??

#define NOMINMAX
#include <Windows.h> //GetModuleFilename, needed to locate Data-Folder relative to executable
#include <span> //used to wrap args

using I = std::int64_t;


void run1(const fs::path& inputFile)
{
	struct Val {
		char number{ '\0' };
		I pos{};
		I spaceLeft{};
	};

	auto buildMax = [](const std::vector<Val>& jolts, I maxLen) {
		const I joltsSize = static_cast<I>(jolts.size());
		if (joltsSize < maxLen || joltsSize==0ull) {
			return I{}; 
		}
		
		std::string result{};
		I lastAdded = -1;
		while (static_cast<I>(result.size()) < maxLen && lastAdded!=joltsSize-1 ) {
			for (const Val& jolt:jolts ) {
				if ( jolt.pos>lastAdded //jolt after last added jolt
					&& jolt.spaceLeft >= (maxLen-static_cast<I>(result.size())) //having enough digits left for completion
					) {
					result.push_back(jolt.number);
					lastAdded = jolt.pos;
					break;
				}
			}
		}
		I num{};
		std::from_chars(result.data(), result.data() + result.size(), num);
		return num;
	};

	TxtFile txt(inputFile);
	I chksum1{};
	I chksum2{};
	for ( const auto& line : txt)
	{
		if (line.size() == 0) { break; }
		std::vector<Val> jolts{};
		for (int pos{}; char c:line) {
			I left = (static_cast<I>(line.size()) - pos);
			jolts.emplace_back(c, pos, left);
			++pos;
		}

		std::sort(jolts.begin(), jolts.end(), [](const auto& j1, const auto& j2) {
			if (j1.number == j2.number) { return j1.spaceLeft > j2.spaceLeft; }
			return j1.number > j2.number;
		});
	
		chksum1 += buildMax(jolts, 2);
		chksum2 += buildMax(jolts, 12);
	}

	
	
	std::println("\npart1: sum of biggest 2-size-joltages: {}\n", chksum1);
	std::println("\npart2: sum of biggest 12-size-joltages: {}\n", chksum2);
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
	fs::path root = fs::path(pFile).parent_path().parent_path().parent_path().parent_path() / "Data";
	if (!fs::exists(root) && !fs::is_directory(root))
	{
		std::println("folder not found: {}", root.string());
		return 0;
	}

	fs::path inputFile = root / args[1];
	if (!fs::exists(inputFile))
	{
		std::println("input not found: {}", inputFile.string());
		return 0;
	}

	run1(inputFile);

	return 0;
}