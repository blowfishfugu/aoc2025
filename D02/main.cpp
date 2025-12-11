#include "../common/TxtFile.h"
#include "../common/SplitIter.h"
#include <locale>
#include <print>
#include <span>
#include <string_view>
#include <execution>
#include <ranges>
#include <algorithm>
#define NOMINMAX
#include <Windows.h>

std::locale de_DE( "de_DE" );//locale fuer toUpper/toLower ->zur Performanceoptimierung als auch Umlautbehandlung

void run1( const fs::path& inputFile )
{
	using CommaIter = SplitIter<','>;
	using DashIter = SplitIter<'-'>;

	struct Twice
	{

		std::uint64_t seed{};
		std::uint64_t current{};

		byte countDigits( std::uint64_t input )
		{
			byte digits = 0;
			std::uint64_t tmp = input;
			while( tmp > 0 )
			{
				tmp /= 10;
				++digits;
			}
			return digits;
		}

		std::tuple<std::uint32_t, std::uint32_t, byte> splitSeed()
		{
			byte digits = countDigits( seed );
			byte half = digits / 2;
			if( ( digits % 2 ) == 1 )
			{
				++half;
			}

			std::uint64_t tmp = seed;
			std::uint32_t right{};
			int shift = 1;
			for( byte i = 0; i < half; ++i )
			{
				std::int32_t number = tmp % 10;
				right += number * shift;
				tmp /= 10;
				shift *= 10;
			}

			std::uint32_t left{};
			left = static_cast<std::uint32_t>( tmp );
			return { left,right,digits };
		}

		void initBottom(){
			auto [left,right,digits]=splitSeed();
			if( (digits % 2) == 0 )
			{
				if( left < right ) { current = left + 1; }
				else if( left > right ) { current = left; }
				else { current = left; }
				return;
			}
			//expand
			byte shift = countDigits( right );
			std::int32_t fac = 1;
			for( byte i = 1; i < shift; ++i ) { fac *= 10; }
			current = fac;//123->10 10
		}

		void initTop() {
			auto [left,right,digits]=splitSeed();
			if( ( digits % 2 ) == 0 )
			{
				if( left < right ) { current = left; }
				else if( left > right ) { current = left-1; }
				else { current = left; }
				return;
			}

			//collapse
			byte shift = countDigits( right );
			std::int32_t fac = 1;
			for( byte i = 1; i < shift; ++i ) { fac *= 10; }
			current = fac-1; //100-1 =9 9
		}

		auto operator<=>( const Twice& r ) noexcept { return current <=> r.current; }

		std::uint64_t value()
		{
			std::uint32_t shift = 1;
			std::uint64_t tmp = current;
			while( tmp > 0 )
			{
				tmp /= 10;
				shift *= 10;
			}
			return ( current * shift ) + current;
		}
	};

	TxtFile txt( inputFile );
	std::vector<std::tuple<Twice, Twice>> idRanges{};
	for( auto& line : txt )
	{
		if( line.size() == 0 ) { break; }
		for( CommaIter pairs{ line }; pairs != CommaIter{}; ++pairs )
		{
			Twice startID{};
			Twice endID{};
			DashIter tpl{ *pairs };
			for( int i = 0; i < 2 && tpl != DashIter{}; ++i, ++tpl )
			{
				auto& id = *tpl;
				if( i==0 ){
					std::from_chars( id.data(), id.data() + id.size(), startID.seed );
				}
				else if( i==1 ){
					std::from_chars( id.data(), id.data() + id.size(), endID.seed );
				}
			}
			if( startID.seed != 0ull && endID.seed != 0ull )
			{
				startID.initBottom();
				endID.initTop();
				idRanges.emplace_back( startID, endID );
			}
		}
	}

	std::int64_t chksum{};
	for( auto& [from, to] : idRanges )
	{
		//std::println( "{}-{} -> {}-{}",from.seed, to.seed, from.value(), to.value() );
		std::int64_t current = from.value();
		std::int64_t cap = to.value();
		while (current <= cap) {
			//std::println("{}",current);
			chksum += current;
			++from.current;
			current = from.value();
		}
	}
	
	std::println( "\nchksum1: {}\n", chksum );
}

void run2(const fs::path& inputFile)
{
	using CommaIter = SplitIter<','>;
	using DashIter = SplitIter<'-'>;

	struct ID {
		std::int64_t seed{};
	};
	
	TxtFile txt(inputFile);
	std::vector<std::tuple<ID, ID>> idRanges{};
	for (auto& line : txt)
	{
		if (line.size() == 0) { break; }
		for (CommaIter pairs{ line }; pairs != CommaIter{}; ++pairs)
		{
			ID startID{};
			ID endID{};
			DashIter tpl{ *pairs };
			for (int i = 0; i < 2 && tpl != DashIter{}; ++i, ++tpl)
			{
				auto& id = *tpl;
				if (i == 0) {
					std::from_chars(id.data(), id.data() + id.size(), startID.seed);
				}
				else if (i == 1) {
					std::from_chars(id.data(), id.data() + id.size(), endID.seed);
				}
			}
			if (startID.seed != 0ull && endID.seed != 0ull)
			{
				idRanges.emplace_back(startID, endID);
			}
		}
	}

	std::atomic<std::int64_t> chksum{};
	std::for_each(std::execution::par,
		idRanges.begin(), idRanges.end(), [&chksum](auto& tpl)
		{
			auto containsPattern = [](std::int64_t seed) {
				std::vector<std::uint8_t> pattern{};
				byte digits{};
				std::uint64_t tmp = seed;
				while (tmp > 0)
				{
					pattern.emplace_back(static_cast<std::uint8_t>(tmp % 10));
					tmp /= 10;
					++digits;
				}

				byte half = digits / 2;
				for (byte chunksize = 1; chunksize <= half; ++chunksize) {
					if ((digits % chunksize) != 0) {
						continue; //to note: following chunk may not create equal sized packages at the end
					}
					bool allsame = true;
					const auto& subranges = pattern | std::views::chunk(chunksize);
					// | transform | adjacent, könnte das alles mit filter klappen?
					std::int64_t prevhash{-1ll};
					for (const auto& chunk : subranges) {
						std::int64_t curhash{};
						for (int pos = 1; std::uint8_t c : chunk) {
							curhash += static_cast<std::uint64_t>(c) * pos;
							pos *= 10;
						}
						if (prevhash == -1ll) { 
							prevhash = curhash; 
						}
						else if (prevhash != curhash ) {
							allsame = false;
							break;
						}
					}
					if (allsame) { 
						return true; 
					}
				}

			return false;
			};

			auto& [from, to] = tpl;
			//std::println("{}-{}", from.seed, to.seed);
			while (from.seed <= to.seed) {
				if (containsPattern(from.seed)) {
					//std::println("{}", from.seed);
					chksum.fetch_add( from.seed );
				}
				++from.seed;
			}
		});

	std::println("\nchksum2: {}\n", chksum.load());
}

int main( int argc, const char* argv[] )
{
	SetConsoleOutputCP( 1252 ); //latin1

	auto args = std::span{ argv,static_cast<size_t>( argc ) };
	if( args.size() < 2 )
	{
		std::println( "no input specified" );
		return 0;
	}

	char pFile[ MAX_PATH ]{};
	::GetModuleFileName( NULL, pFile, MAX_PATH );
	fs::path root = fs::path( pFile ).parent_path().parent_path().parent_path().parent_path() / "Data";
	if( !fs::exists(root) && !fs::is_directory(root) )
	{
		std::println( "folder not found: {}", root.string() );
		return 0;
	}

	fs::path inputFile = root / args[ 1 ];
	if( !fs::exists( inputFile ) )
	{
		std::println( "input not found: {}", inputFile.string() );
		return 0;
	}

	run1( inputFile );
	run2( inputFile );
	
	return 0;
}