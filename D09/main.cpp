#include "TxtFile.h"
#include "SplitIter.h"
#include "StopWatch.h"
#include <print> //<- convenient, but bloats exe-binaries-size, back to printf??
#include <exception>
#define NOMINMAX
#include <Windows.h> //GetModuleFilename, needed to locate Data-Folder relative to executable
#include <span> //used to wrap args

using I = std::int64_t;


void run1(const fs::path& inputFile)
{
	//yet another vec-xy-implementation
	struct Vec2 final{
		std::tuple<I, I> data{};
		Vec2() = default;
		Vec2(const Vec2&) = default;
		Vec2(const I x, const I y) : data{ x,y } {};

		const I& x() const { return std::get<0>(data); }
		const I& y() const { return std::get<1>(data); }
		I& operator[](size_t i) {
			switch (i) {
				case(0): { return std::get<0>(data); }
				case(1): { return std::get<1>(data); }
				default: [[unlikely]] //return dummy or throw?
				{
					constexpr auto Dimensions=std::tuple_size_v<decltype(data)>;
					std::string errMsg(std::format("index '{}' is not in range [0..{}]", i, Dimensions-1));
					throw std::exception( errMsg.c_str() );
				}
			}
		}

		Vec2 operator-(const Vec2& r) const { return {r.x() - x(), r.y() - y() }; }
		Vec2 operator+(const Vec2& r) const { return {r.x() + x(), r.y() + y() }; }

		Vec2& operator-=(const Vec2& r) { 
			std::get<0>(data) -= r.x();
			std::get<1>(data) -= r.y();
			return *this;
		}

		Vec2& operator+=(const Vec2& r) {
			std::get<0>(data) += r.x();
			std::get<1>(data) += r.y();
			return *this;
		}

		explicit Vec2(std::string_view toParse /* x,y */) noexcept
		{
			using CommaSplitter = SplitIter<','>;
			auto current = CommaSplitter{ toParse };
			auto end = CommaSplitter{};
			for (size_t i = 0; i < 2 && current != end; ++i, ++current) {
				const auto& sv = *current;
				std::from_chars(sv.data(), sv.data() + sv.size(), (*this)[i]);
			}
		}
	};

	auto getArea=[](const Vec2& p1, const Vec2& p2) -> I {
		Vec2 dist = p2 - p1;
		I w = std::abs(dist.x()) + 1;
		I h = std::abs(dist.y()) + 1;
		return w * h;
	};

		
	//reading positions
	TxtFile txt(inputFile);
	using Coords=std::vector<Vec2>;
	Coords coords{};
	for ( const auto& line : txt)
	{
		if (line.size() == 0) { break; }
		coords.emplace_back(line);
	}

	std::sort(coords.begin(), coords.end(), [](const auto& p1, const auto& p2) {
		if (p1.x() == p2.x()) { return p1.y() < p2.y(); }
		return p1.x() < p2.x();
		});

	I maxArea{};
	I pairCount{}; //same approach as yesterday, is it a trap?
	for (size_t i = 0; i < coords.size(); ++i) {
		const Vec2& current = coords[i];
		for (size_t j = i; j < coords.size(); ++j) {
			const Vec2& other = coords[j];
			I area = getArea(current, other);
			if (area > maxArea) {
				maxArea = area;
			}
			++pairCount;
		}
	}
	
	std::println("\npart1: area of largest rectancle: {}\n", maxArea);
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