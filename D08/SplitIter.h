#pragma once
#include <string>
#include <string_view>
#include <optional>

template< char splitChar>
struct SplitIter final
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
	SplitIter() = default;

	//begin()
	explicit SplitIter( reference_type _buf ) noexcept
		: fulltext( _buf )
	{
		++*this; //erstes item -> operator++()
	};

	reference_type operator*() const { return value; }
	pointer_type operator->() const { return &value; }

	SplitIter& operator=( reference_type _buf ) noexcept
	{
		fulltext = _buf;
		start_pos = 0LL;
		end_pos = 0LL;
		++*this;
		return *this;
	}

	SplitIter& operator=( const SplitIter& ref )
	{
		fulltext = ref.fulltext;
		value = ref.value;
		start_pos = ref.start_pos;
		end_pos = ref.end_pos;
		reachedEnd = false;
		return *this;
	}

	// ++it;
	SplitIter& operator++()
	{
		value = {};
		reachedEnd = true;
		if( fulltext && fulltext->length() > 0 ) //optional
		{
			reachedEnd = false;
			end_pos = fulltext->find( splitChar, start_pos );
			if( end_pos != std::string_view::npos )
			{
				value = fulltext->substr( start_pos, end_pos - start_pos );
				start_pos = end_pos + 1;
			}
			else
			{
				end_pos = fulltext->length();
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
		}
		return *this;
	}

	//it++;
	SplitIter operator++( int )
	{
		SplitIter current( *this );
		++*this;
		return current;
	}

	friend bool operator==( const SplitIter& lhs, const SplitIter& rhs )
	{
		return lhs.value == rhs.value//<-wird bei doppelten leerzeilen abbrechen
			&& lhs.reachedEnd == rhs.reachedEnd; //<-daher zusaetzlich ein reachedEnd
	}
};

using TabIter = SplitIter<'\t'>;