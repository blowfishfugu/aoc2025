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
	auto print = [](std::vector<std::string>& lines) {
#ifndef NDEBUG
		for (const auto& l : lines) {
			std::println("{}", l);
		}
		std::println("");
#endif
		};
	
	TxtFile txt(inputFile);
	std::vector<std::string> lines;
	for ( const auto& line : txt)
	{
		if (line.size() == 0) { break; }
		lines.emplace_back(line.begin(), line.end());
	}

	print(lines);

	I splitCount{};
	for (size_t i = 1; i < lines.size(); ++i) {
		
		const std::string& prev = lines[i - 1];
		std::string& current = lines[i];

		for (size_t c = 0; c < prev.size() && c < current.size(); ++c) {
			char top = prev[c];
			char now = current[c];
			if (top == 'S' || top=='|') {
				if (now == '^') {
					current[c - 1] = '|';
					current[c + 1] = '|';
					current[c] = 'x';
					++splitCount;
				}
				else {
					current[c] = '|';
				}
			}
		}
	}
	print(lines);

	std::println("\nsplitCount: {}\n", splitCount);
}


void run2(const fs::path& inputFile)
{
	struct Cell {
		char cellType{ '.' };
		I visited{};

		explicit Cell(char c) noexcept //marked explicit, Cell c=(char)'c' denied.
		{
			cellType = c;
			if (cellType=='S') {
				visited = 1ll; //first ray
			}
		}
	};

	using Row = std::vector<Cell>;
	auto print = [](std::vector<Row>& lines) {
#ifndef NDEBUG
		for (const auto& l : lines) {
			for (const Cell& c : l) {
				if (c.cellType == '|') {
					std::print("{} ", c.visited);
				}
				else {
					std::print("{} ", c.cellType);
				}
			}
			std::println("");
		}
#endif
		};

	TxtFile txt(inputFile);
	std::vector<Row> lines;
	for (int i = 0; const auto& line : txt)
	{
		if (line.size() == 0) { break; }
		if ((i % 2) == 0) //<-skipping lines for speed
			lines.emplace_back(line.begin(), line.end()); //char-to-cell, only works because of suitable cell-constructor
		++i;
	}

	print(lines);

	I splitCount{};
	for (size_t i = 1; i < lines.size(); ++i) {
		const Row& prev = lines[i - 1];
		Row& current = lines[i];
		for (size_t c = 0; c < prev.size() && c < current.size(); ++c) { //we know its a tree, might inc-loop from center
			const Cell& prevCell = prev[c];
			Cell& currentCell = current[c];
			//same as before, but inherit visitCount
			if ( prevCell.cellType == '|' || prevCell.cellType == 'S') {
				if (currentCell.cellType == '^') {
					currentCell.cellType = 'x';
					currentCell.visited += prevCell.visited;

					current[c - 1].cellType = '|';
					current[c - 1].visited += currentCell.visited;
					current[c + 1].cellType = '|';
					current[c + 1].visited += currentCell.visited;
					++splitCount;
				}
				else {
					currentCell.cellType = '|';
					currentCell.visited += prevCell.visited; //<- += because rays can intersect rays
				}
			}
		}
	}
	print(lines);

	if (lines.size() > 0) {
		I visitCount{};
		const Row& lastRow = lines.back();
		for (const Cell& cell : lastRow) {
			if (cell.cellType == '|') {
#ifndef NDEBUG
				std::print("{} ", cell.visited);
#endif
				visitCount += cell.visited;
			}
		}
		std::println("part2: visitCount: {}", visitCount);
	}

	std::println("\npart1: splitCount: {}\n", splitCount);
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
#ifndef NDEBUG
	run1(inputFile); //part2 contains part1, walks combineable
#endif
	run2(inputFile);

	return 0;
}