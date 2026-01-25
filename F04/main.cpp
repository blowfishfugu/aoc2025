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

using I = std::int64_t;

struct V2 {
	I x{};
	I y{};
	auto operator<=>( const V2& r) const noexcept = default;
	V2 operator-(const V2& r) const{
		return { x - r.x, y - r.y };
	}
	V2 operator+(const V2& r) const {
		return { x + r.x, y + r.y };
	}

	I manhattan() const {
		return x + y;
	}
};

void run(const fs::path& inputFile)
{
	TxtFile txt(inputFile);
	size_t linecount = std::count(txt.buf.begin(), txt.buf.end(), '\n');
	
	std::vector<V2> points{ V2{} }; //start
	points.reserve(linecount);

	for (const auto& line : txt) {
		auto items = line 
			| std::views::chunk_by([](char l,char r) { return isdigit(l)&&isdigit(r); })
			| std::views::transform([](auto&& ch)->std::string_view {return { ch.cbegin(),ch.cend() }; });
			
			V2 pt{-1,-1};
			for (int idx{}; auto&& val : items) {
				int element{};
				auto [ptr, err] = std::from_chars(val.data(), val.data() + val.size(), element);
				switch (idx) {
				case(0):pt.x = element; break;
				case(2):pt.y = element; break;
				}
				++idx;
			}
			if (pt != V2{ -1,-1 }) {
				points.emplace_back(pt);
			}
	}

	auto calcDeltas = [](const std::vector<V2>& points) {
		std::vector<V2> deltas;
		deltas.reserve(points.size());
		for (size_t i = 1; i < points.size(); ++i) {
			const V2& l = points[i - 1];
			const V2& r = points[i];
			V2 dist = r - l;
			dist.x = std::abs(dist.x);
			dist.y = std::abs(dist.y);
			deltas.emplace_back(dist);
		}
		return deltas;
		};

	std::vector<V2> deltas = calcDeltas(points);
	I steps = std::accumulate(deltas.begin(), deltas.end(), 0ll, 
		[]( const I& l, const V2& r)->I {
			return l + r.manhattan();
		});
	
	std::println("\npart1: steps: {}", steps);

	auto sumDiagonal= [](const I& l, const V2& r)->I {
		if (r.x == r.y)
		{
			return l + r.x;
		}
		else if (r.x < r.y) {
			//I remainingY = r.y - r.x;
			//return l + r.x + remainingY; //l + r.x + r.y - r.x
			return l + r.y;
		}
		else if (r.y < r.x) {
			return l + r.x;
		}
		};

	I steps2 = std::accumulate(deltas.begin(), deltas.end(), 0ll, sumDiagonal);

	std::println("\npart2: stepsdiagonal: {}", steps2);

	std::sort(points.begin(), points.end(), [](const V2& l, const V2& r){
		return l.manhattan() < r.manhattan();
		});

	std::vector<V2> deltas3 = calcDeltas(points);
	I steps3 = std::accumulate(deltas3.begin(), deltas3.end(), 0ll, sumDiagonal);
	
	std::println("\npart3: stepssorted: {}", steps3);
}

fs::path FindInput() {
	char pFile[MAX_PATH]{};
	::GetModuleFileName(NULL, pFile, MAX_PATH);
	//src/bin/debug/modulefile.exe
	//src/bin/release/modulefile.exe
	//src/inputs.txt
	fs::path inputFile = fs::path(pFile).parent_path().parent_path().parent_path() / "trash.txt";
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