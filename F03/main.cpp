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

using I = std::int64_t;

void run(const fs::path& inputFile)
{
	static const std::array<std::string_view, 5> labels{
		"Red","Green","Blue","Special",""
	};

	using Color = std::tuple<int, int, int,std::string_view>;
	std::map<Color, int> histcolors;
	std::vector<Color> colors;
	TxtFile txt(inputFile);
	for (const auto& line : txt) {
		auto items = line 
			| std::views::chunk_by([](char l,char r) { return isdigit(l)&&isdigit(r); })
			| std::views::transform([](auto&& ch)->std::string_view {return { ch.cbegin(),ch.cend() }; });
			
			Color color{-1,-1,-1,labels[4]};
			for (int idx{}; auto ch : items) {
				int element{};
				auto [ptr, err] = std::from_chars(ch.data(), ch.data() + ch.size(), element);
				switch (idx) {
				case(0):std::get<0>(color) = element; break;
				case(2):std::get<1>(color) = element; break;
				case(4):std::get<2>(color) = element; break;
				}
				++idx;
			}
			if( color!=Color{-1,-1,-1,labels[4]}) {
				++histcolors[color]; //<-similar colors, sort colors-vector??
				colors.emplace_back(color);
			}
	}

	auto m = std::max_element(histcolors.cbegin(), histcolors.cend(), [](const auto& l, const auto& r) {
		const auto& [rgb1, cnt1] = l;
		const auto& [rgb2, cnt2] = r;
		return cnt1 < cnt2;
		});

	

	std::println("\npart1: highest: {} with: {}", m->first, m->second);

	auto mark = [](Color& c) {
		auto& [r, g, b, lbl] = c;
		if (r == g || g == b || b == r) {
			lbl = labels[3]; //special
			return;
		}
		if (r > g && r > b) {
			lbl = labels[0];
			return;
		}
		if (g > r && g > b) {
			lbl = labels[1];
			return;
		}
		if (b > r && b > g) {
			lbl = labels[2];
			return;
		}
		};

	for (Color& c : colors) {
		mark(c);
	}
	I rCount = std::ranges::count_if(colors, [](const Color& c) {return std::get<3>(c) == labels[0]; });
	I gCount = std::ranges::count_if(colors, [](const Color& c) {return std::get<3>(c) == labels[1]; });
	I bCount = std::ranges::count_if(colors, [](const Color& c) {return std::get<3>(c) == labels[2]; });
	I sCount = std::ranges::count_if(colors, [](const Color& c) {return std::get<3>(c) == labels[3]; });
	std::println("\npart2: greens: {}", gCount);

	I prices = rCount * 5 + gCount * 2 + bCount * 4 + sCount * 10;

	std::println("\npart3: pointers: {}", prices);
}

fs::path FindInput() {
	char pFile[MAX_PATH]{};
	::GetModuleFileName(NULL, pFile, MAX_PATH);
	//src/bin/debug/modulefile.exe
	//src/bin/release/modulefile.exe
	//src/inputs.txt
	fs::path inputFile = fs::path(pFile).parent_path().parent_path().parent_path() / "colors.txt";
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