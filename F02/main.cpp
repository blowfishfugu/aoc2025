#include "../common/TxtFile.h"
#include "../common/SplitIter.h"
#include "../common/StopWatch.h"
#include <print> //<- convenient, but bloats exe-binaries-size, back to printf??

#define NOMINMAX
#include <Windows.h> //GetModuleFilename, needed to locate Data-Folder relative to executable
#include <span> //used to wrap args
#include <algorithm>
#include <numeric>
using I = std::int64_t;


void run(const fs::path& inputFile)
{
	TxtFile txt(inputFile);
	std::vector<I> coaster;
	for (const auto& line : txt) {
		for(char c:line){
			if(c=='^'){coaster.emplace_back(1ll);}
			if(c=='v'){coaster.emplace_back(-1ll);}
		}
	}
	std::partial_sum(coaster.cbegin(), coaster.cend(),
                     coaster.begin(), std::plus<>());
	std::println("\npart1: highest: {}", *std::max_element(coaster.cbegin(),coaster.cend()));
}

void run2(const fs::path& inputFile)
{
	TxtFile txt(inputFile);
	std::vector<I> coaster;
	for (const auto& line : txt) {
		for (char c : line) {
			if (c == '^') { coaster.emplace_back(1ll); }
			if (c == 'v') { coaster.emplace_back(-1ll); }
		}
	}
	
		I inc{ 1 };
		for (size_t i = 1ull; i < coaster.size(); ++i) {
			I& prev = coaster[i - 1];
			const I& next = coaster[i];
			if (prev == next) {
				prev = inc * prev;
				++inc;
			}
			else { 
				prev = inc * prev;
				inc = 1ll; 
			}
		}
		coaster[coaster.size() - 1] *= inc;

		std::partial_sum(coaster.cbegin(), coaster.cend(),
			coaster.begin(), std::plus<>());

	std::println("\npart2: highest: {}", *std::max_element(coaster.cbegin(), coaster.cend()));
}


void run3(const fs::path& inputFile)
{
	TxtFile txt(inputFile);
	using rle = std::tuple<I, I>;
	std::vector<rle> coaster;
	for (const auto& line : txt) 
	{
		for (char c : line) {
			I num{};
			if (c == '^') { num=1ll; }
			if (c == 'v') { num=-1ll; }
			size_t pos = coaster.size();
			if (pos == 0ull) {
				coaster.emplace_back(num, 1ll);
				continue;
			}
			if (std::get<0>(coaster[pos-1]) != num) {
				coaster.emplace_back(num, 1ll);
			}
			else {
				++std::get<1>(coaster[pos-1]);
			}
		}
	}
	
	const I maxSeq = std::get<1>( *std::max_element(coaster.cbegin(), coaster.cend()
		, [](const rle& l, const rle& r) { return std::get<1>(l) < std::get<1>(r); }
	));
	std::println("\npart3: longestsequence: {}", maxSeq);

	std::vector<I> fibs{ 0ll,1ll };
	for (I i = 2ll; i <= maxSeq; ++i) {
		I f= fibs[i - 1] + fibs[i-2];
		fibs.emplace_back(f);
	}

	I maxheight{};
	I height{};
	for (const auto& [s, v] : coaster) {
		height += s * fibs[v];
		maxheight = std::max(height, maxheight);
	}
	std::println("part3: maxheight: {}", maxheight);
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
	fs::path inputFile = fs::path(pFile).parent_path().parent_path().parent_path() / "coaster.txt";
	if (!fs::exists(inputFile))
	{
		std::println("input not found: {}", inputFile.string());
		return 0;
	}
	run(inputFile);
	run2(inputFile);
	run3(inputFile);

	return 0;
}