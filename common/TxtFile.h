#pragma once
#include "LineIter.h"
#include <string>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

struct TxtFile
{
	std::string buf{};
	explicit TxtFile( fs::path input )
	{
		auto sz = fs::file_size( input );
		if( sz > 0 ) {
			buf.resize( sz, 0 );
			std::ifstream inFile( input );
			inFile.read( buf.data(), sz );
			inFile.close();

			size_t lastChar = buf.find_last_not_of( '\0' );
			if( ( lastChar + 1 ) < buf.size() )
			{
				buf.erase( ( lastChar + 1 ), buf.size() - lastChar );
			}
		}
	}

	LineIter begin() { return LineIter{ buf }; }
	LineIter end() { return LineIter{}; }
};