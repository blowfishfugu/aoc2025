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

		Vec3 operator-(const Vec3& r) const {
			return { {r.x() - x(), r.y() - y(), r.z() - z()} };
		}

		I sqLen() const {
			return x() * x() + y() * y() + z() * z();
		}
	};
	struct Junction {
		Vec3 pos{};
		size_t circuitID{};
		explicit Junction(std::string_view toParse) noexcept
		{
			using CommaIter = SplitIter<','>;
			auto it = CommaIter{ toParse };
			for (size_t i = 0; i < 3 && it != CommaIter{};++i, ++it) {
				std::from_chars((*it).data(), (*it).data() + (*it).size(), pos[i]);
			}
		}
	};

	using Junctions = std::vector<Junction>;
	
	TxtFile txt(inputFile);
	Junctions juncs;
	for ( const auto& line : txt)
	{
		if (line.size() == 0) { break; }
		juncs.emplace_back(line);
	}

	//length,direction,start,end
	using Distance = std::tuple<I, Vec3, Junction*, Junction*>;
	using Distances = std::vector<Distance>;
	Distances dists{};
	for (size_t i = 0; i < juncs.size(); ++i) {
		Junction* current = &(juncs[i]);
		for (size_t j = i + 1; j < juncs.size(); ++j) {
			Junction* other = &(juncs[j]);
			Vec3 direction = other->pos - current->pos;
			I len = direction.sqLen();
			dists.emplace_back(len, direction, current, other);
		}
	}

	std::sort(dists.begin(), dists.end(), [](const auto& l, const auto& r)
		{
			return std::get<0>(l) < std::get<0>(r);
		});
	using Circuit = std::vector<Junction*>;
	std::vector<Circuit> circuits;
	int cap = juncs.size() / 2;
	for (int i = 0; Distance& d:dists) {
		auto& [len, dir, first, second] = d;
		if (first->circuitID == 0 && second->circuitID == 0) {
			circuits.emplace_back(Circuit{});
			first->circuitID = circuits.size();
			second->circuitID = circuits.size();
			circuits[first->circuitID - 1].emplace_back(first);
			circuits[second->circuitID - 1].emplace_back(second);
		}
		else if (first->circuitID == 0) {
			first->circuitID = second->circuitID;
			circuits[first->circuitID - 1].emplace_back(first);
		}
		else if (second->circuitID == 0) {
			second->circuitID = first->circuitID;
			circuits[second->circuitID - 1].emplace_back(second);
		}
		else {
			//std::println(std::cerr, "should we merge circuit {} and {} ?", first->circuitID, second->circuitID);
			if (first->circuitID != second->circuitID) {
				Circuit& c1 = circuits[first->circuitID- 1];
				Circuit& c2 = circuits[second->circuitID- 1];
				for (Junction* j : c2) {
					j->circuitID = first->circuitID;
					c1.emplace_back(j);
				}
				c2.clear();
			}
		}
		++i;
		if (i >= cap) {
			break;
		}
	}
	
	std::sort(circuits.begin(), circuits.end(), 
		[](const auto& l,const auto& r) {
		return l.size() > r.size();
		});

	I chksum{1ll};
	for (size_t i = 0ull; i < 3ull; ++i) {
		chksum *= circuits[i].size();
	}
	std::println("\npart1: product of three largest circuit-sizes: {}\n", chksum);
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