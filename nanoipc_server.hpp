#ifndef	NANOIPC_HPP
#define	NANOIPC_HPP

#include <cstddef>
#include <cstdint>
#include <functional>

#include "nanoipc_read_buffer.hpp"

namespace nanoipc {
	template <typename IpcRequest, typename IpcResponse>
	class NanoIpcServer {
	public:
		using RequestHandler = std::function<IpcResponse(const IpcRequest&)>;
		using RequestParser = std::function<IpcRequest(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;
		using ResponseSerializer = std::function<std::size_t(const IpcResponse& response, std::uint8_t *dest_buff, const std::size_t dest_buff_capacity)>;
		using SerialDataWriter = std::function<void(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;

		NanoIpcServer(
			const RequestHandler& request_handler,
			const RequestParser& request_parser,
			const ResponseSerializer& response_serializer,
			const SerialDataWriter& serial_data_writer,
			ReadBuffer *read_buffer
		);
		NanoIpcServer(const NanoIpcServer&) = delete;
		NanoIpcServer& operator=(const NanoIpcServer&) = delete;

		virtual ~NanoIpcServer() noexcept = default;
		void process();
	private:
		ReadBuffer *m_read_buffer;
	};
}

#endif // NANOIPC_HPP