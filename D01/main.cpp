#include "../common/TxtFile.h"
#include <locale>
#include <print>
#include <span>
#include <string_view>

#include <Windows.h>

std::locale de_DE( "de_DE" );//locale fuer toUpper/toLower ->zur Performanceoptimierung als auch Umlautbehandlung

void run( const fs::path& inputFile )
{
	TxtFile txt( inputFile );

	int pos = 50;
	int slowpos = 50;
	int zeroHit{};
	int everyzeroHit{};
	for( std::string_view val : txt )
	{
		if( val.size() == 0ull ) { continue; }
		int rot = 0;
		if( val[ 0 ] == 'L' ) { rot = -1; }
		else if( val[ 0 ] == 'R' ) { rot = +1; }

		val.remove_prefix( 1 );
		int steps{};
		const auto& [ptr, ec] = std::from_chars( val.data(), val.data() + val.size(), steps );

		pos = ( pos + ( steps * rot ) ) % 100;
		if( pos == 0 )
		{
			++zeroHit;
		}

		for( int i = 0; i < steps; ++i )
		{
			slowpos += rot;
			if( slowpos == 100 ) slowpos = 0;
			if( slowpos == -1 ) slowpos = 99;
			if( slowpos == 0 ) ++everyzeroHit;
		}
	}
	std::println( "zerohits: {}", zeroHit );
	std::println( "everyzerohits: {}", everyzeroHit );


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

	run( inputFile );
	return 0;
}