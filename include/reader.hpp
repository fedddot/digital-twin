#ifndef	READER_HPP
#define	READER_HPP

#include <optional>

namespace nanoipc {
	template <typename T>
	class Reader {
	public:
		virtual ~Reader() noexcept = default;
		virtual std::optional<T> read() = 0;
	};
}

#endif // READER_HPP