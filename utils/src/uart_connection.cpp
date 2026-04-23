#include "uart_connection.hpp"
#include "serialib.h"

namespace nanoipc {

// Helper function to convert unsigned int parity to SerialParity enum
static SerialParity convert_parity(unsigned int parity) {
	switch (parity) {
		case 0: return SERIAL_PARITY_NONE;
		case 1: return SERIAL_PARITY_ODD;
		case 2: return SERIAL_PARITY_EVEN;
		default: throw std::invalid_argument("Invalid parity value");
	}
}

// Helper function to convert unsigned int stop_bits to SerialStopBits enum
static SerialStopBits convert_stop_bits(unsigned int stop_bits) {
	switch (stop_bits) {
		case 1: return SERIAL_STOPBITS_1;
		case 2: return SERIAL_STOPBITS_2;
		default: throw std::invalid_argument("Invalid stop_bits value");
	}
}

// Helper function to convert unsigned int data_bits to SerialDataBits enum
static SerialDataBits convert_data_bits(unsigned int data_bits) {
	switch (data_bits) {
		case 5: return SERIAL_DATABITS_5;
		case 6: return SERIAL_DATABITS_6;
		case 7: return SERIAL_DATABITS_7;
		case 8: return SERIAL_DATABITS_8;
		default: throw std::invalid_argument("Invalid data_bits value");
	}
}

UartConnection::UartConnection(
	const std::string& device_path,
	const unsigned int baudrate,
	const unsigned int parity,
	const unsigned int stop_bits,
	const unsigned int data_bits,
	OnCharReceivedCallback on_char_received
)
	: m_device_path(device_path),
	  m_baudrate(baudrate),
	  m_parity(parity),
	  m_stop_bits(stop_bits),
	  m_data_bits(data_bits),
	  m_on_char_received(on_char_received),
	  m_is_open(false),
	  m_listening(false),
	  m_serial_port(nullptr)
{
	if (device_path.empty()) {
		throw std::invalid_argument("device_path cannot be empty");
	}
	if (!on_char_received) {
		throw std::invalid_argument("on_char_received callback cannot be null");
	}
	if (baudrate == 0) {
		throw std::invalid_argument("baudrate must be non-zero");
	}
	
	// Validate parity, stop_bits, and data_bits by attempting to convert them
	convert_parity(parity);
	convert_stop_bits(stop_bits);
	convert_data_bits(data_bits);
}

UartConnection::~UartConnection()
{
	try {
		if (is_open()) {
			close();
		}
	} catch (...) {
		// Suppress exceptions in destructor
	}
}

void UartConnection::open()
{
	if (is_open()) {
		throw std::logic_error("UartConnection is already open");
	}

	// Create serialib instance
	serialib *serial_port = new serialib();
	if (!serial_port) {
		throw std::runtime_error("Failed to allocate serialib instance");
	}

	// Convert parameters to serialib enum types
	SerialDataBits db = convert_data_bits(m_data_bits);
	SerialParity p = convert_parity(m_parity);
	SerialStopBits sb = convert_stop_bits(m_stop_bits);

	// Open the serial port
	// serialib::openDevice returns 1 on success, 0 on failure
	if (serial_port->openDevice(
		m_device_path.c_str(),
		m_baudrate,
		db,
		p,
		sb) != 1)
	{
		delete serial_port;
		throw std::runtime_error("Failed to open UART device: " + m_device_path);
	}

	m_serial_port = serial_port;
	m_is_open.store(true);
	m_listening.store(true);

	// Start listening thread
	try {
		m_listen_thread = std::make_unique<std::thread>(
			&UartConnection::listen_thread_routine,
			this
		);
	} catch (const std::exception& e) {
		m_listening.store(false);
		m_is_open.store(false);
		serial_port->closeDevice();
		delete serial_port;
		m_serial_port = nullptr;
		throw std::runtime_error(std::string("Failed to start listening thread: ") + e.what());
	}
}

void UartConnection::close()
{
	if (!is_open()) {
		throw std::logic_error("UartConnection is not open");
	}

	// Signal listening thread to stop
	m_listening.store(false);

	// Wait for listening thread to finish
	if (m_listen_thread && m_listen_thread->joinable()) {
		m_listen_thread->join();
		m_listen_thread.reset();
	}

	// Close the serial port
	serialib *serial_port = static_cast<serialib*>(m_serial_port);
	if (serial_port) {
		serial_port->closeDevice();
		delete serial_port;
		m_serial_port = nullptr;
	}

	m_is_open.store(false);
}

bool UartConnection::is_open() const
{
	return m_is_open.load();
}

void UartConnection::write(const std::uint8_t *data, std::size_t data_size)
{
	if (!is_open()) {
		throw std::runtime_error("UartConnection is not open");
	}

	if (!data && data_size > 0) {
		throw std::invalid_argument("data pointer cannot be null when data_size > 0");
	}

	if (data_size == 0) {
		return;  // Nothing to write
	}

	serialib *serial_port = static_cast<serialib*>(m_serial_port);
	if (!serial_port) {
		throw std::runtime_error("Serial port handle is invalid");
	}

	// Write each byte
	for (std::size_t i = 0; i < data_size; ++i) {
		if (serial_port->writeChar(data[i]) == -1) {
			throw std::runtime_error("Failed to write data to UART device");
		}
	}
}

void UartConnection::listen_thread_routine()
{
	serialib *serial_port = static_cast<serialib*>(m_serial_port);
	if (!serial_port) {
		return;
	}

	char byte_buffer = 0;
	while (m_listening.load()) {
		// Read one character with timeout (100ms)
		// readChar returns 1 on success, 0 on timeout, -1 on error
		int result = serial_port->readChar(&byte_buffer, 100);
		if (result == 1) {
			m_on_char_received(static_cast<uint8_t>(byte_buffer));
		}
	}
}

}  // namespace nanoipc
