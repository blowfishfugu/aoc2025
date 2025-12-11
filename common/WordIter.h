#pragma once
#include <string>
#include <string_view>
#include <optional>

//Zieht "Woerter". Ein Wort beginnt mit einem Buchstaben,
//und darf dann weitere Buchstaben, Unterstriche und Zahlen enthalten
//ein Wort muss mehr als ein Zeichen haben
struct WordIter final
{
	using iterator_category = std::forward_iterator_tag;
	using value_type = std::string_view;
	using difference_type = std::ptrdiff_t;
	using reference_type = value_type const&;
	using pointer_type = const value_type*;


	value_type value;
	size_t start_pos{};
	size_t end_pos{};
	bool reachedEnd = true;
	std::optional<value_type> fulltext{};

	//end()
	WordIter() = default;

	//begin()
	explicit WordIter( reference_type _buf ) noexcept
		: fulltext( _buf )
	{
		++*this; //erstes item -> operator++()
	};
	
	reference_type operator*() const { return value; }
	pointer_type operator->() const { return &value; }

	WordIter& operator=( reference_type _buf ) noexcept
	{
		fulltext = _buf;
		start_pos = 0LL;
		end_pos = 0LL;
		++*this;
		return *this;
	}

	WordIter& operator=( const WordIter& ref )
	{
		fulltext = ref.fulltext;
		value = ref.value;
		start_pos = ref.start_pos;
		end_pos = ref.end_pos;
		reachedEnd = false;
		return *this;
	}

	void findStartOfWord()
	{
		//gültige Startzeichen eines Wortes
		for( ; start_pos < fulltext->length(); ++start_pos )
		{
			const char c = fulltext.value()[ start_pos ];
			if( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' )
				|| ( c == '_' )
				|| ( c == 'ä' ) || ( c == 'ö' ) || ( c == 'ü' )
				|| ( c == 'Ä' ) || ( c == 'Ö' ) || ( c == 'Ü' )
				|| ( c == 'ß' ) )
			{
				end_pos = start_pos + 1;
				break;
			}
		}
	}

	void findEndOfWord()
	{
		//gültige weitere Zeichen eines Wortes
		//bis zum ersten !nicht gültigen der Weiteren
		for( ; end_pos < fulltext->length(); ++end_pos )
		{
			const char c = fulltext.value()[ end_pos ];
			if( !( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' )
				|| ( c == '_' )
				|| ( c == 'ä' ) || ( c == 'ö' ) || ( c == 'ü' )
				|| ( c == 'Ä' ) || ( c == 'Ö' ) || ( c == 'Ü' )
				|| ( c == 'ß' )
				|| ( c >= '0' && c <= '9' ) ) )
			{
				break;
			}
		}
	}

	// ++it;
	WordIter& operator++()
	{
		value = {};
		reachedEnd = true;
		if( fulltext && fulltext->length() > 0 ) //optional
		{
			reachedEnd = false;
			while( true )
			{
				end_pos = start_pos;

				findStartOfWord();//<-inkrement start_pos
				findEndOfWord();//<-inkrement end_pos
				//hiernach endpos==startpos, oder endpos>startpos, subtraktion ok.
				size_t txtLen = end_pos - start_pos;
				if( txtLen != 1 ) //leer(also am ende) oder mehr als 1 zeichen
				{
					break;
				}
				else //zu kurz, ab endpos weitersuchen
				{
					start_pos = end_pos;
				}
			}

			if( start_pos < end_pos )
			{
				value = fulltext->substr( start_pos, end_pos - start_pos );
				start_pos = end_pos + 1;
			}
			else
			{
				value = {};
				reachedEnd = true;
			}
		}
		return *this;
	}

	//it++;
	WordIter operator++( int )
	{
		WordIter current( *this );
		++*this;
		return current;
	}

	friend bool operator==( const WordIter& lhs, const WordIter& rhs )
	{
		return lhs.value == rhs.value//<-wird bei doppelten leerzeilen abbrechen
			&& lhs.reachedEnd == rhs.reachedEnd;
	}
};
