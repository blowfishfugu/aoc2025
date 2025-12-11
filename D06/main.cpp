#include "../common/TxtFile.h"
#include "../common/SplitIter.h"
#include "../common/StopWatch.h"
#include <locale>
#include <print>
#include <span>
#include <string_view>
#include <execution>
#include <ranges>
#include <algorithm>
#include <functional>
#include <mdspan>
#define NOMINMAX
#include <Windows.h>

std::locale de_DE("de_DE");//locale fuer toUpper/toLower ->zur Performanceoptimierung als auch Umlautbehandlung

using I = std::int64_t;
static I plus(const I& l, const I& r) { return l + r; };
static I mul (const I& l, const I& r) { return l * r; };

//TODO: try same using mdspan and/or valarray
void run(const fs::path& inputFile)
{
	
	TxtFile txt(inputFile);
	std::vector<std::string_view> lines;
	for ( auto& line : txt)
	{
		if (line.size() == 0) { break; }
		lines.emplace_back(line);
	}

	if (lines.size() < 2) {
		std::println("nothing to compute");
		return;
	}


	struct BlockDescriptor {
		std::function<I(const I&,const I&)> operand=plus;
		size_t start{};
		I width{};
		I result{};
		I result2{};
	};
	std::vector<BlockDescriptor> blocks{};
	std::string_view operands = lines[lines.size() - 1];

	I width{};
	char op{'\0'};
	I start{};
	for (size_t pos{}; pos < operands.size(); ++pos) {
		char c = operands[pos];
		if (c == ' ') { ++width; }
		else {
			if (op != '\0') {
				blocks.emplace_back((op=='+')?plus:mul, start, width - 1);//without space
			}
			op = c;
			start = pos;
			width = 1ll;
		}
	}
	if (op != '\0') {
		blocks.emplace_back((op=='+')?plus:mul, start, width);
	}
	//std::vector<std::vector<I>> numbers{};
	//numbers.resize(blocks.size());

	//part1 by line
	for (size_t i{}; i < lines.size() - 1; ++i) {
		std::string_view line = lines[i];
		for (size_t col{}; col < blocks.size(); ++col) {
			BlockDescriptor& info = blocks[col];
			std::string_view strNum = line.substr(info.start, info.width);
			//left to right per line + trim-leading spaces
			while (strNum.size() > 0 && std::isblank(strNum.front())) { strNum.remove_prefix(1); }
			I number{};
			std::from_chars(strNum.data(), strNum.data() + strNum.size(), number);
			//numbers[col].emplace_back(number);
			if (i == 0ull) {
				info.result = number;
			}
			else {
				info.result = info.operand(info.result, number);
			}
		}
	}

	//part2 transposed
	for (size_t col{}; col < blocks.size(); ++col) {
		BlockDescriptor& info = blocks[col];
		std::vector<std::string> byCol;
		byCol.resize(info.width);
		for (size_t i{}; i < lines.size() - 1; ++i) {
			std::string_view line = lines[i];
			std::string_view strNum = line.substr(info.start, info.width);
			for (I c = static_cast<I>(strNum.size()) - 1; c >= 0ll; --c) {
				char item = strNum[c];
				if (!std::isblank(item)) {
					byCol[c].push_back(item);
				}
			}
		}
		for (size_t i{}; const std::string& str : byCol) {
			I number{};
			std::from_chars(str.data(), str.data()+str.size(), number);
			if (i == 0ull) { info.result2 = number; }
			else { info.result2 = info.operand(info.result2, number); }
			++i;
		}
	}

	I sum1 = std::accumulate(blocks.begin(), blocks.end(), 0ull,
		[](const I& c, const BlockDescriptor& c2) { 
			return c + c2.result;
		});
	I sum2 = std::accumulate(blocks.begin(), blocks.end(), 0ull,
		[](const I& c, const BlockDescriptor& c2) {
			return c + c2.result2;
		});
	std::println("\nsum1: {}\n", sum1);
	std::println("\nsum2: {}\n", sum2);

	/*
	sum1: 6299564383938
	sum2: 11950004808442
	*/
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
	run(inputFile);

	return 0;
}