#include "TxtFile.h"
#include "SplitIter.h"
#include "StopWatch.h"
#include <print> //<- convenient, but bloats exe-binaries-size, back to printf??

#define NOMINMAX
#include <Windows.h> //GetModuleFilename, needed to locate Data-Folder relative to executable
#include <span> //used to wrap args

using I = std::int64_t;


void run1(const fs::path& inputFile)
{
	//yet another vec-xyz-implementation
	struct Vec3 {
		std::tuple<I, I, I> data{};
		//named accessors
		const I& x() const { return std::get<0>(data); }
		const I& y() const { return std::get<1>(data); }
		const I& z() const { return std::get<2>(data); }
		//indexed setters
		I& operator[](size_t i) {
			switch (i) {
				case(0): { return std::get<0>(data); }
				case(1): { return std::get<1>(data); }
				case(2): { return std::get<2>(data); }
				default: [[unlikely]] //return dummy or throw?
				{
					thread_local I empty{};
					return empty;
				}
			}
		}
		
		//math needed for puzzle
		
		//direction of vec3s
		Vec3 operator-(const Vec3& r) const {
			return { {r.x() - x(), r.y() - y(), r.z() - z()} };
		}

		//euclidian distance unsquared, exact length not needed
		I sqLen() const {
			return x() * x() + y() * y() + z() * z();
		}
	};

	//Items to read, have a position, and a circuit-ID
	struct Junction {
		Vec3 pos{};
		size_t circuitID{};
		explicit Junction(std::string_view toParse /* x,y,z */ ) noexcept
		{
			using CommaSplitter = SplitIter<','>;
			auto current = CommaSplitter{ toParse };
			auto end = CommaSplitter{};
			for (size_t i = 0; i < 3 && current != end;++i, ++current) {
				const auto& sv = *current;
				std::from_chars(sv.data(), sv.data()+sv.size(), pos[i]);
			}
		}
	};

	using Junctions = std::vector<Junction>;
	
	//reading positions
	TxtFile txt(inputFile);
	Junctions juncs;
	for ( const auto& line : txt)
	{
		if (line.size() == 0) { break; }
		juncs.emplace_back(line);
	}

	//example 20 juncs and limit 10
	//in puzzle 1000 juncs and limit=1000!
	int cap = (juncs.size() == 20) ? 10 : 1000;

	//length,direction,start,end
	using Distance = std::tuple<I, Vec3, Junction*, Junction*>;
	using Distances = std::vector<Distance>;
	Distances dists{};
	//pairing each-by-remaining
	for (size_t i = 0; i < juncs.size(); ++i) {
		Junction* current = &(juncs[i]);
		for (size_t j = i + 1; j < juncs.size(); ++j) {
			Junction* other = &(juncs[j]);
			Vec3 direction = other->pos - current->pos;
			I len = direction.sqLen();
			dists.emplace_back(len, direction, current, other);
		}
	}

	//sort-by-lengths
	std::sort(dists.begin(), dists.end(), [](const auto& l, const auto& r)
		{
			return std::get<0>(l) < std::get<0>(r);
		});

	using Circuit = std::vector<Junction*>; //possible optimization, just count per circuit
	std::vector<Circuit> circuits;
	std::vector<size_t> sizesOnCap; //copy of sizes to not disturb part2

	for (int i = 0; Distance& d:dists) {
		auto& [len, dir, j1, j2] = d;
		if (j1->circuitID == 0 && j2->circuitID == 0) { //both unassigned, new circuit
			circuits.emplace_back(Circuit{});
			j1->circuitID = circuits.size();
			j2->circuitID = circuits.size();
			circuits[j1->circuitID - 1].emplace_back(j1);
			circuits[j2->circuitID - 1].emplace_back(j2);
		}
		else if (j1->circuitID == 0) { //attach junc1 to junc2s circuit
			j1->circuitID = j2->circuitID;
			Circuit& c1 = circuits[j1->circuitID - 1];
			c1.emplace_back(j1);
			if (c1.size() == juncs.size()) { //all juncs are in one circuit
				std::println(std::cout, "last connection {} x {} = {}",
					j1->pos.x(), j2->pos.x(), j1->pos.x() * j2->pos.x());
				break;
			}
		}
		else if (j2->circuitID == 0) { //attach junc2 to junc1s circuit
			j2->circuitID = j1->circuitID;
			Circuit& c2 = circuits[j2->circuitID - 1];
			c2.emplace_back(j2);
			if (c2.size() == juncs.size()) { //all juncs are in one circuit
				std::println(std::cout, "last connection {} x {} = {}",
					j1->pos.x(), j2->pos.x(), j1->pos.x()* j2->pos.x());
				break;
			}
		}
		else {
			//both in same circuit, nothing happens, different circuits->join.
			if (j1->circuitID != j2->circuitID) { //otherwise merge circuits
				//std::println(std::cout, "merging circuit {} and {} ?", j1->circuitID, j2->circuitID);
				Circuit& c1 = circuits[j1->circuitID- 1];
				Circuit& c2 = circuits[j2->circuitID- 1];
				
				for (Junction* j : c2) { //this copy is ugly, room for improvement
					j->circuitID = j1->circuitID;
					c1.emplace_back(j);
				}
				c2.clear();

				if (c1.size() == juncs.size()) { //all juncs are in one circuit
					std::println(std::cout, "last connection {} x {} = {}",
						j1->pos.x(), j2->pos.x(), j1->pos.x()* j2->pos.x());
					break;
				}
			}
		}

		++i;
		if (i == cap) { //on first attempt (part2), sorted here, and made a mess
			for (Circuit& c : circuits) { sizesOnCap.emplace_back(c.size()); }
		}
	}

	//result of part1
	std::sort(sizesOnCap.begin(), sizesOnCap.end(), [](const size_t& l, const size_t& r) {return l > r; });

	I chksum{ 1ll };
	for (size_t i = 0ull; i < 3ull; ++i) {
		chksum *= sizesOnCap[i];
	}
	std::println("\npart1: product of three largest circuit-sizes: {}\n", chksum);;
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