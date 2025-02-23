/*-
 * Copyright 2021 Vsevolod Stakhov
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RSPAMD_UTIL_HXX
#define RSPAMD_UTIL_HXX

#pragma once

#include <memory>
#include <array>
#include <string_view>
#include <optional>
#include <tuple>
#include <algorithm>

/*
 * Common C++ utilities
 */

namespace rspamd {
/*
 * Creates std::array from a standard C style array with automatic size calculation
 */
template <typename... Ts>
constexpr auto array_of(Ts&&... t) -> std::array<typename std::decay_t<typename std::common_type_t<Ts...>>, sizeof...(Ts)>
{
	using T = typename std::decay_t<typename std::common_type_t<Ts...>>;
	return {{ std::forward<T>(t)... }};
}

template<class C, class K, class V = typename C::mapped_type, typename std::enable_if_t<
		std::is_constructible_v<typename C::key_type, K>
		&& std::is_constructible_v<typename C::mapped_type, V>, bool> = false>
constexpr auto find_map(const C &c, const K &k) -> std::optional<std::reference_wrapper<const V>>
{
	auto f = c.find(k);

	if (f != c.end()) {
		return std::cref<V>(f->second);
	}

	return std::nullopt;
}


template <typename _It>
inline constexpr auto make_string_view_from_it(_It begin, _It end)
{
	using result_type = std::string_view;

	return result_type{((begin != end) ? &*begin : nullptr),
					   (typename result_type::size_type)std::max(std::distance(begin, end),
							   (typename result_type::difference_type)0)
	};
}

/**
 * Iterate over lines in a string, newline characters are dropped
 * @tparam S
 * @tparam F
 * @param input
 * @param functor
 * @return
 */
template<class S, class F, typename std::enable_if_t<std::is_invocable_v<F, std::string_view> &&
    std::is_constructible_v<std::string_view, S>, bool> = true>
inline auto string_foreach_line(const S &input, const F &functor)
{
	auto it = input.begin();
	auto end = input.end();

	while (it != end) {
		auto next = std::find(it, end, '\n');
		while (next >= it && (*next == '\n' || *next == '\r')) {
			--next;
		}
		functor(make_string_view_from_it(it, next));
		it = next;

		if (it != end) {
			++it;
		}
	}
}

template<class S, typename std::enable_if_t<std::is_constructible_v<std::string_view, S>, bool> = true>
inline auto string_split_on(const S &input, std::string_view::value_type chr) -> std::pair<std::string_view, std::string_view>
{
	auto pos = std::find(std::begin(input), std::end(input), chr);

	if (pos != input.end()) {
		auto first = std::string_view{std::begin(input), static_cast<std::size_t>(std::distance(std::begin(input), pos))};
		while (*pos == chr && pos != input.end()) {
			++pos;
		}
		auto last = std::string_view{pos,  static_cast<std::size_t>(std::distance(pos, std::end(input)))};

		return {first, last};
	}

	return {std::string_view{input}, std::string_view{}};
}

/**
 * Enumerate for range loop
 */
template <typename T,
		typename TIter = decltype(std::begin(std::declval<T>())),
		typename = decltype(std::end(std::declval<T>()))>
constexpr auto enumerate(T && iterable)
{
	struct iterator
	{
		size_t i;
		TIter iter;
		bool operator != (const iterator & other) const { return iter != other.iter; }
		void operator ++ () { ++i; ++iter; }
		auto operator * () const { return std::tie(i, *iter); }
	};
	struct iterable_wrapper
	{
		T iterable;
		auto begin() { return iterator{ 0, std::begin(iterable) }; }
		auto end() { return iterator{ 0, std::end(iterable) }; }
	};
	return iterable_wrapper{ std::forward<T>(iterable) };
}

/**
 * Allocator that cleans up memory in a secure way on destruction
 * @tparam T
 */
template <class T> class secure_mem_allocator : public std::allocator<T>
{
public:
	using value_type = typename std::allocator<T>::value_type;
	using size_type = typename std::allocator<T>::size_type;
	template<class U> struct rebind { typedef secure_mem_allocator<U> other; };
	secure_mem_allocator() noexcept = default;
	secure_mem_allocator(const secure_mem_allocator &_) noexcept : std::allocator<T>(_) {}
	template <class U> explicit secure_mem_allocator(const secure_mem_allocator<U>&) noexcept {}

	void deallocate(value_type *p, size_type num) noexcept {
		rspamd_explicit_memzero((void *)p, num);
		std::allocator<T>::deallocate(p, num);
	}
};


}

#endif //RSPAMD_UTIL_HXX
