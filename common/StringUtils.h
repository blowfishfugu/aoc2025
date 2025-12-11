#pragma once
#ifndef STRING_UTILS_H
#define STRING_UTILS_H
#include <string>
#include <string_view>
#include <concepts>
#include <type_traits>
#include <locale>

extern std::locale de_DE; //main.cpp, std::locale de_DE( "de_DE" );

template<class T>
concept StringViewLike = std::is_convertible_v<T, std::string_view>;

template<class T>
concept StringLike = std::is_convertible_v<T, std::string>;

template<StringViewLike SV = std::string_view>
std::string asString( const SV& v )
{
	return { v.data(), v.data() + v.length() };
}

/// <summary>
/// Ersetzung inplace auf string-referenz
/// </summary>
/// <typeparam name="S"></typeparam>
/// <param name="bufAdd"></param>
/// <param name="pattern"></param>
/// <param name="replacement"></param>
/// <returns></returns>
template<StringLike S = std::string>
std::string replace( S& bufAdd, const std::string_view pattern, const std::string_view replacement )
{
	const size_t szPattern = pattern.length();
	auto offset = bufAdd.find( pattern, 0 );
	while( offset != S::npos )
	{
		bufAdd.replace( offset, szPattern, replacement.data() );
		offset = bufAdd.find( pattern, offset + replacement.size() );
	}
	return bufAdd;
}

template<StringLike S = std::string>
void trim( S& str )
{
	std::string_view v = str;
	while( v.starts_with( " " ) ) { v.remove_prefix( 1 ); }
	while( v.ends_with( " " ) ) { v.remove_suffix( 1 ); }
	if( v.size() != str.size() )
	{
		str = asString( v );
	}
}

template<StringLike S = std::string >
std::string replace_const( const S& bufAdd, const std::string_view pattern, const std::string_view replacement )
{
	std::string tmp{ bufAdd };
	return replace<S>( tmp, pattern, replacement );
}

template<StringLike S = std::string >
S asLower( const S& str )
{
	if( str.length() == 0 ) { return str; }
	S lower;
	lower.reserve( str.size() );

	for( const char& c : str ) { lower.push_back( std::tolower( c, de_DE ) ); }
	return lower;
}

template<StringLike S = std::string >
S asUpper( const S& str )
{
	if( str.length() == 0 ) { return str; }
	S upper;
	upper.reserve( str.size() );

	for( const char& c : str ) { upper.push_back( std::toupper( c, de_DE ) ); }
	return upper;
}

/// <summary>
/// inplace-ersetzung lower-case
/// </summary>
/// <typeparam name="S"></typeparam>
/// <param name="str"></param>
template<StringLike S = std::string >
void toLower( S& str )
{
	if( str.length() == 0 ) { return; }
	for( char& c : str ) { c = std::tolower( c, de_DE ); }
}

/// <summary>
/// inplace-ersetzung UPPERCASE
/// </summary>
/// <typeparam name="S"></typeparam>
/// <param name="str"></param>
template<StringLike S = std::string >
void toUpper( S& str )
{
	if( str.length() == 0 ) { return; }
	for( char& c : str ) { c = std::toupper( c, de_DE ); }
}


#endif