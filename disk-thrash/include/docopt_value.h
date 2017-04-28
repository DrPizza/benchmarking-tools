// from https://github.com/docopt/docopt.cpp, dual licensed under MIT and Boost license

//
//  value.h
//  docopt
//
//  Created by Jared Grubb on 2013-10-14.
//  Copyright (c) 2013 Jared Grubb. All rights reserved.
//

#ifndef docopt__value_h_
#define docopt__value_h_

#include <string>
#include <vector>
#include <functional> // std::hash
#include <iosfwd>
#include <variant>

namespace std
{
	template<> struct hash<std::vector<std::string> > {
		typedef std::vector<std::string> argument_type;
		typedef std::size_t result_type;

		result_type operator()(const argument_type& v) const {
			size_t seed = std::hash<size_t>{}(v.size());

			// stolen from boost::hash_combine
			std::hash<std::string> hasher;
			for(auto const& str : v) {
				seed ^= hasher(str) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}

			return seed;
		}
	};
}

namespace docopt {

	/// A generic type to hold the various types that can be produced by docopt.
	///
	/// This type can be one of: {bool, unsigned int, string, vector<string>}, or empty.

	using value = std::variant<std::monostate, bool, std::string, unsigned int, std::vector<std::string> >;

	inline bool is_empty(const value& v) {
		return std::holds_alternative<std::monostate>(v);
	}
}

namespace std {
	std::ostream& operator<<(std::ostream& os, const docopt::value& v);
}

#endif /* defined(docopt__value_h_) */
