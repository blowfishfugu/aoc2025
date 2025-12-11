#include "../common/TxtFile.h"
#include "../common/SplitIter.h"
#include "../common/StopWatch.h"
#include <print> //<- convenient, but bloats exe-binaries-size, back to printf??
#include <exception>
#define NOMINMAX
#include <Windows.h> //GetModuleFilename, needed to locate Data-Folder relative to executable
#include <span> //used to wrap args

using I = std::int64_t;

//yet another vec-xy-implementation
struct Vec2 final {
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
			constexpr auto Dimensions = std::tuple_size_v<decltype(data)>;
			std::string errMsg(std::format("index '{}' is not in range [0..{}]", i, Dimensions - 1));
			throw std::exception(errMsg.c_str());
		}
		}
	}

	Vec2 operator-(const Vec2& r) const { return { r.x() - x(), r.y() - y() }; }
	Vec2 operator+(const Vec2& r) const { return { r.x() + x(), r.y() + y() }; }

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

using Edge = Vec2;
using Edges = std::vector<Edge>;

auto getArea = [](const Vec2& p1, const Vec2& p2) -> I {
	Vec2 dist = p2 - p1;
	I w = std::abs(dist.x()) + 1;
	I h = std::abs(dist.y()) + 1;
	return w * h;
	};


enum Layout {
	H = 0, V = 1
};

struct Line final {
	Line() = delete;
	explicit Line(const Edge* from, const Edge* to) : edges{ from,to } {
		recalc();
	}
	std::tuple<const Edge*, const Edge*> edges{};
	Vec2 dir{};
	Layout layout{ Layout::H };
	Vec2 toInner{}; //direction this line is allowed to expand to
	//sidenote: detect mergable zigzag-s ? 
	// #X##    #XX#
	// XXXX -> XXXX
	// #X##    #XX#
	Line* prev{}; //plan expand rect along toInner until next line
	//having opposite toInner, being in range and at least one opposing "red-stone"
	Line* next{};
	I _cnt{};

	void recalc() {
		if (edges == std::tuple<Edge*, Edge*>{}) { return; } //empty init, return.

		auto [from, to] = edges;
		dir = *from - *to;
		if (dir.x() == 0)
		{
			_cnt = dir.y() + 1;
			layout = Layout::V;
		}
		else {
			_cnt = dir.x() + 1;
			layout = Layout::H;
		}
	}
};

using Lines = std::vector<Line>;

Lines buildBoundingBox( Edges& edges) {
	Lines bounding{};
	for (size_t i = 1; i < edges.size(); ++i) {
		const Edge& e1 = edges[i - 1];
		const Edge& e2 = edges[i];
		bounding.emplace_back(&e1, &e2);
	}
	bounding.emplace_back(&edges[edges.size() - 1], &edges[0]);
	return bounding;
}

void run1(const fs::path& inputFile)
{
		
	//reading positions
	TxtFile txt(inputFile);
	using Coords=std::vector<Vec2>;
	Coords coords{};
	for ( const auto& line : txt)
	{
		if (line.size() == 0) { break; }
		coords.emplace_back(line);
	}

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
	
	std::println("\npart1: area of largest rectangle: {}\n", maxArea);
}

Edges readEdges(const fs::path& inputFile) {
	TxtFile txt(inputFile);
	Edges edges{};
	for (const auto& str : txt)
	{
		if (str.size() == 0) { break; }
		edges.emplace_back(str);
	}
	return std::move(edges);
}

void run2b(const fs::path& inputFile)
{

	//reading positions
	Edges edges = readEdges(inputFile);
	
	Lines bounding = buildBoundingBox(edges);

	I boundingLength{};
	for (const Line& line : bounding) {
		boundingLength += std::abs(line._cnt);
	}

	using MappedLines = std::vector < Line*>;
	MappedLines byX{};
	for (Line& line : bounding) { if (line.layout == Layout::V) { byX.emplace_back(&line); } }
	std::sort(byX.begin(), byX.end(), [](Line* l,Line* r) {
		return std::get<0>(l->edges)->x() < std::get<0>(r->edges)->x();
		});

	MappedLines byY{};
	for (Line& line : bounding) { if (line.layout == Layout::H) { byY.emplace_back(&line); } }
	std::sort(byY.begin(), byY.end(), [](Line* l, Line* r) {
		return std::get<0>(l->edges)->y() < std::get<0>(r->edges)->y();
		});

	Line* xFirst = byX[0];
	Line* yFirst = byY[0];
	Vec2 vLeftTop{ std::get<0>(xFirst->edges)->x(), std::get<0>(yFirst->edges)->y()};
	for ( auto& e : edges) {
		e -= vLeftTop;
	}

	//properties of a rectangle, two points on border, reflected-points at least in-side region

	I maxArea{};
	

	std::println("\npart2: area of largest rectangle in boundary: {}\n", maxArea);
}

//get reflected edgepoints, and check if those are on the boundary,
//or intersect point "outside", being on the rim is okay
bool linesLeaveBoundary(const Vec2& p1, const Vec2& p2, const Lines& boundingBox) {
	
	return false;
}

void run2(const fs::path& inputFile)
{
	//applicable?: https://en.wikipedia.org/wiki/Jordan_curve_theorem
	/*
	If the initial point (pa) of a ray lies outside a simple polygon (region A),
	the number of intersections of the ray and the polygon is 
	even.
	If the initial point (pb) of a ray lies inside the polygon (region B),
	the number of intersections is 
	odd.
	->"our" lines are horizontal or vertical, shooting in 4 directions might be enough?
	*/

	//reading positions
	TxtFile txt(inputFile);
	Edges edges{};
	for (const auto& str : txt)
	{
		if (str.size() == 0) { break; }
		edges.emplace_back(str);
	}
	Lines boundingBox = buildBoundingBox(edges);

	I maxArea{};
	I pairCount{}; //same approach as yesterday, is it a trap?
	for (size_t i = 0; i < edges.size(); ++i) {
		const Vec2& current = edges[i];
		for (size_t j = i; j < edges.size(); ++j) {
			const Vec2& other = edges[j];
			if (linesLeaveBoundary(current, other, boundingBox)) {
				continue;
			}
			I area = getArea(current, other);
			if (area > maxArea) {
				maxArea = area;
			}
			++pairCount;
		}
	}

	std::println("\npart1: area of largest rectangle: {}\n", maxArea);
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
	//run2b(inputFile);
	run2(inputFile);

	return 0;
}