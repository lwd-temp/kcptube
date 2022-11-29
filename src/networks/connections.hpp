#pragma once

#ifndef __CONNECTIONS__
#define __CONNECTIONS__

#include <functional>
#include <memory>
#include <map>
#include <array>
#include <atomic>
#include <set>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <tuple>
#include <asio.hpp>

#include "../shares/share_defines.hpp"
#include "stun.hpp"
#include "kcp.hpp"

constexpr uint8_t TIME_GAP = std::numeric_limits<uint8_t>::max();	//seconds
constexpr size_t BUFFER_SIZE = 4096u;
constexpr size_t BUFFER_EXPAND_SIZE = 128u;
constexpr size_t RETRY_TIMES = 5u;
constexpr size_t RETRY_WAITS = 3u;
constexpr size_t CLEANUP_WAITS = 10;	// second
constexpr size_t KCP_WINDOW_SIZE = 4096;
constexpr auto KCP_UPDATE_INTERVAL = std::chrono::milliseconds(2);
constexpr auto STUN_RESEND = std::chrono::seconds(30);
constexpr auto CHANGEPORT_UPDATE_INTERVAL = std::chrono::seconds(1);
constexpr auto FINDER_EXPIRES_INTERVAL = std::chrono::seconds(1);
constexpr auto EXPRING_UPDATE_INTERVAL = std::chrono::milliseconds(50);
const asio::ip::udp::endpoint local_empty_target(asio::ip::make_address_v6("::1"), 70);

enum class feature : uint8_t
{
	initialise,
	failure,
	disconnect,
	data
};

enum class protocol_type : uint8_t { tcp = 0, udp };

uint32_t time_now_for_kcp();
std::string_view feature_to_string(feature ftr);
std::string protocol_type_to_string(protocol_type prtcl);
std::string debug_data_to_string(const uint8_t *data, size_t len);
void debug_print_data(const uint8_t *data, size_t len);

namespace packet
{
	constexpr size_t empty_data_size = 8;

	int64_t right_now();

	std::vector<uint8_t> create_packet(feature ftr, protocol_type prtcl, const std::vector<uint8_t> &data);
	size_t create_packet(feature ftr, protocol_type prtcl, uint8_t *input_data, size_t data_size);

	std::tuple<int64_t, feature, protocol_type, std::vector<uint8_t>> unpack(const std::vector<uint8_t> &data);
	std::tuple<int64_t, feature, protocol_type, uint8_t*, size_t> unpack(uint8_t *data, size_t length);

	std::tuple<uint32_t, uint16_t, uint16_t> get_initialise_details_from_unpacked_data(const std::vector<uint8_t> &data);
	std::tuple<uint32_t, uint16_t, uint16_t> get_initialise_details_from_unpacked_data(const uint8_t *data);

	std::vector<uint8_t> request_initialise_packet(protocol_type prtcl);

	std::vector<uint8_t> response_initialise_packet(protocol_type prtcl, uint32_t uid, uint16_t port_start, uint16_t port_end);

	std::vector<uint8_t> inform_disconnect_packet(protocol_type prtcl);

	std::vector<uint8_t> inform_error_packet(protocol_type prtcl, const std::string &error_msg);

	std::vector<uint8_t> create_data_packet(protocol_type prtcl, const std::vector<uint8_t> &custom_data);
	size_t create_data_packet(protocol_type prtcl, uint8_t *custom_data, size_t length);

	std::string get_error_message_from_unpacked_data(const std::vector<uint8_t> &data);
	std::string get_error_message_from_unpacked_data(uint8_t *data, size_t length);

	std::tuple<uint32_t, std::vector<uint8_t>> get_confirm_data_from_unpacked_data(const std::vector<uint8_t> &data);
	std::tuple<uint32_t, uint8_t*, size_t> get_confirm_data_from_unpacked_data(uint8_t *data, size_t length);
}	// namespace packet


using asio::ip::tcp;
using asio::ip::udp;

class tcp_session;

using tcp_callback_t = std::function<void(std::shared_ptr<uint8_t[]>, size_t, tcp_session*)>;
using udp_callback_t = std::function<void(std::shared_ptr<uint8_t[]>, size_t, udp::endpoint&&, asio::ip::port_type)>;

void empty_tcp_callback(std::shared_ptr<uint8_t[]> tmp1, size_t tmps, tcp_session *tmp2);
void empty_udp_callback(std::shared_ptr<uint8_t[]> tmp1, size_t tmps, udp::endpoint &&tmp2, asio::ip::port_type tmp3);
void empty_tcp_disconnect(tcp_session *tmp);
int empty_kcp_output(const char *, int, void *);

class tcp_session
{
public:

	tcp_session(asio::io_context &net_io, asio::strand<asio::io_context::executor_type> &asio_strand, tcp_callback_t callback_func)
		: network_io(net_io), task_assigner(asio_strand), connection_socket(network_io),
		callback(callback_func), callback_for_disconnect(empty_tcp_disconnect),
		last_receive_time(packet::right_now()), last_send_time(packet::right_now()),
		paused(false), stopped(false) {}

	void start();

	void session_is_ending(bool set_ending);
	bool session_is_ending();

	void pause(bool set_as_pause);
	void stop();
	bool is_pause();
	bool is_stop();
	bool is_open();

	void disconnect();

	void async_read_data();

	size_t send_data(const std::vector<uint8_t> &buffer_data);
	size_t send_data(const uint8_t *buffer_data, size_t size_in_bytes);
	size_t send_data(const uint8_t *buffer_data, size_t size_in_bytes, asio::error_code &ec);

	void async_send_data(std::shared_ptr<std::vector<uint8_t>> data);
	void async_send_data(std::vector<uint8_t> &&data);
	void async_send_data(std::shared_ptr<uint8_t[]> buffer_data, size_t size_in_bytes);
	void async_send_data(std::shared_ptr<uint8_t[]> buffer_data, uint8_t *start_pos, size_t size_in_bytes);
	void async_send_data(const uint8_t *buffer_data, size_t size_in_bytes);

	void when_disconnect(std::function<void(tcp_session*)> callback_before_disconnect);

	void replace_callback(tcp_callback_t callback_func);

	tcp::socket& socket();

	int64_t time_gap_of_receive();

	int64_t time_gap_of_send();

private:
	void after_write_completed(const asio::error_code &error, size_t bytes_transferred);

	void after_read_completed(std::shared_ptr<uint8_t[]> buffer_cache, const asio::error_code &error, size_t bytes_transferred);

	asio::io_context &network_io;
	asio::strand<asio::io_context::executor_type> &task_assigner;
	tcp::socket connection_socket;
	tcp_callback_t callback;
	std::function<void(tcp_session*)> callback_for_disconnect;
	std::atomic<int64_t> last_receive_time;
	std::atomic<int64_t> last_send_time;
	std::atomic<bool> paused;
	std::atomic<bool> stopped;
	std::atomic<bool> session_ending;
};

class tcp_server
{
public:
	using acceptor_callback_t = std::function<void(std::unique_ptr<tcp_session>&&)>;
	tcp_server() = delete;
	tcp_server(asio::io_context &io_context, asio::strand<asio::io_context::executor_type> &asio_strand,
		const tcp::endpoint &ep, acceptor_callback_t acceptor_callback_func, tcp_callback_t callback_func)
		: internal_io_context(io_context), task_assigner(asio_strand), tcp_acceptor(io_context),
		acceptor_callback(acceptor_callback_func), session_callback(callback_func)
	{
		acceptor_initialise(ep);
		start_accept();
	}

private:
	void acceptor_initialise(const tcp::endpoint &ep);
	void start_accept();
	void handle_accept(std::unique_ptr<tcp_session> &&new_connection, const asio::error_code &error_code);

	asio::io_context &internal_io_context;
	asio::strand<asio::io_context::executor_type> &task_assigner;
	tcp::acceptor tcp_acceptor;
	acceptor_callback_t acceptor_callback;
	tcp_callback_t session_callback;
	bool paused;
};

class tcp_client
{
public:
	tcp_client() = delete;
	tcp_client(asio::io_context &io_context, asio::strand<asio::io_context::executor_type> &asio_strand)
		: internal_io_context(io_context), resolver(internal_io_context), task_assigner(asio_strand){}

	std::unique_ptr<tcp_session> connect(tcp_callback_t callback_func, asio::error_code &ec);

	bool set_remote_hostname(const std::string &remote_address, asio::ip::port_type port_num, asio::error_code &ec);
	bool set_remote_hostname(const std::string &remote_address, const std::string &port_num, asio::error_code &ec);

private:

	asio::io_context &internal_io_context;
	asio::strand<asio::io_context::executor_type> &task_assigner;
	tcp::resolver resolver;
	asio::ip::basic_resolver_results<asio::ip::tcp> remote_endpoints;
};



class udp_server
{
public:
	udp_server() = delete;
	udp_server(asio::io_context &io_context, asio::strand<asio::io_context::executor_type> &asio_strand, const udp::endpoint &ep, udp_callback_t callback_func)
		: task_assigner(asio_strand), port_number(ep.port()), resolver(io_context), connection_socket(io_context), callback(callback_func)
	{
		initialise(ep);
		start_receive();
	}

	void continue_receive();

	void async_send_out(std::shared_ptr<std::vector<uint8_t>> data, const udp::endpoint &client_endpoint);
	void async_send_out(std::shared_ptr<uint8_t[]> data, size_t data_size, const udp::endpoint &client_endpoint);
	void async_send_out(std::shared_ptr<uint8_t[]> data, uint8_t *start_pos , size_t data_size, const udp::endpoint &client_endpoint);
	void async_send_out(std::vector<uint8_t> &&data, const udp::endpoint &client_endpoint);
	udp::resolver& get_resolver() { return resolver; }

private:
	void initialise(const udp::endpoint &ep);
	void start_receive();
	void handle_receive(std::shared_ptr<uint8_t[]> buffer_cache, const asio::error_code &error, std::size_t bytes_transferred);

	asio::ip::port_type get_port_number();

	asio::strand<asio::io_context::executor_type> &task_assigner;
	asio::ip::port_type port_number;
	udp::resolver resolver;
	udp::socket connection_socket;
	udp::endpoint incoming_endpoint;
	udp_callback_t callback;
};

class udp_client
{
public:
	udp_client() = delete;
	udp_client(asio::io_context &io_context, asio::strand<asio::io_context::executor_type> &asio_strand, udp_callback_t callback_func)
		: task_assigner(asio_strand), connection_socket(io_context), resolver(io_context), callback(callback_func),
		last_receive_time(packet::right_now()), last_send_time(packet::right_now()),
		paused(false), stopped(false)
	{
		initialise();
	}

	void pause(bool set_as_pause);
	void stop();
	bool is_pause();
	bool is_stop();

	udp::resolver::results_type get_remote_hostname(const std::string &remote_address, asio::ip::port_type port_num, asio::error_code &ec);
	udp::resolver::results_type get_remote_hostname(const std::string &remote_address, const std::string &port_num, asio::error_code &ec);

	void disconnect();

	void async_receive();

	size_t send_out(const std::vector<uint8_t> &data, const udp::endpoint &peer_endpoint, asio::error_code &ec);
	size_t send_out(const uint8_t *data, size_t size, const udp::endpoint &peer_endpoint, asio::error_code &ec);

	void async_send_out(std::shared_ptr<std::vector<uint8_t>> data, const udp::endpoint &peer_endpoint);
	void async_send_out(std::shared_ptr<uint8_t[]> data, size_t data_size, const udp::endpoint &peer_endpoint);
	void async_send_out(std::shared_ptr<uint8_t[]> data, uint8_t *start_pos, size_t data_size, const udp::endpoint &peer_endpoint);
	void async_send_out(std::vector<uint8_t> &&data, const udp::endpoint &peer_endpoint);

	int64_t time_gap_of_receive();
	int64_t time_gap_of_send();

protected:
	void initialise();

	void start_receive();

	void handle_receive(std::shared_ptr<uint8_t[]> buffer_cache, const asio::error_code &error, std::size_t bytes_transferred);

	asio::strand<asio::io_context::executor_type> &task_assigner;
	udp::socket connection_socket;
	udp::resolver resolver;
	udp::endpoint incoming_endpoint;
	udp_callback_t callback;
	std::atomic<int64_t> last_receive_time;
	std::atomic<int64_t> last_send_time;
	std::atomic<bool> paused;
	std::atomic<bool> stopped;
};

std::unique_ptr<rfc3489::stun_header> send_stun_3489_request(udp_server &sender, const std::string &stun_host);
std::unique_ptr<rfc8489::stun_header> send_stun_8489_request(udp_server &sender, const std::string &stun_host);
void resend_stun_8489_request(udp_server &sender, const std::string &stun_host, rfc8489::stun_header *header);

#endif // !__CONNECTIONS__