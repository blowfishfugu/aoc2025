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
#define NOMINMAX
#include <Windows.h>

std::locale de_DE("de_DE");//locale fuer toUpper/toLower ->zur Performanceoptimierung als auch Umlautbehandlung

void run(const fs::path& inputFile)
{
	using I = std::int64_t;
	using Range = std::tuple<I, I>; //[start,end]
	using Item = std::tuple<I, I>; //itemid, occurrence -> struct?
	using DashIter = SplitIter<'-'>;

	TxtFile txt(inputFile);
	std::vector<Range> idRanges{};
	std::vector<Item> items{};
	for (int stage = 0; const auto& line : txt)
	{
		if (line.size() == 0) { ++stage; continue; }
		if (stage == 0) //10-14 -> tuple<start,end>
		{
			Range lr{};
			DashIter tpl{ line };
			if (tpl != DashIter{})
			{
				std::from_chars((*tpl).data(), (*tpl).data() + (*tpl).size(), std::get<0>(lr));
				++tpl;
				if (tpl != DashIter{})
				{
					std::from_chars((*tpl).data(), (*tpl).data() + (*tpl).size(), std::get<1>(lr));
					idRanges.emplace_back(lr);
				}
			}
		}
		else if (stage == 1) //1234 -> tuple<id,hitcount>
		{
			I id{};
			//TODO: in StringUtils ein Conv<integral|floating>( string_view ) hinterlegen
			std::from_chars(line.data(), line.data() + line.size(), id);
			items.emplace_back(id, 0ll);
		}

	}

	std::sort(idRanges.begin(), idRanges.end(), [](const auto& l, const auto& r) {
		return std::get<0>(l) < std::get<0>(r);
		});

	for (auto& Item : items) {
		for (const Range& rng : idRanges) {
			const I& id = std::get<0>(Item);
			const auto& [l, r] = rng;
			if (id >= l && id <= r) {
				++std::get<1>(Item);
			}
		}
	}

	const I part1 = std::count_if(items.begin(), items.end(), [](const Item& item) {
		return std::get<1>(item) > 0;
		});
	std::println("\nsum1: {}\n", part1);

	for (size_t i = 1; i < idRanges.size(); ++i) //ranges vorsortiert, cut/cap or merge?
	{
		auto& [lc, rc] = idRanges[i - 1];
		auto& [ln, rn] = idRanges[i];
		if (ln > rc) { //gap
		}
		else if (lc == ln) { //same start,skip current,expand next
			rn = std::max(rc, rn);
			lc = -1; rc = -1;
		}
		else //inner, ln>lc && ln<=rc
		{
			rn = std::max(rc, rn);
			rc = ln - 1;
		}
	}

	I chksum{};
	for (const auto& lr : idRanges) {
		const auto& [l, r] = lr;
		if (l >= 0) {
			chksum += ((r - l) + 1);
		}
	}
	std::println("\nsum2: {}\n", chksum);
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