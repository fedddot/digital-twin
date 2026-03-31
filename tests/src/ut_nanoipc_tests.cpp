#include "gtest/gtest.h"
#include <string>

#include "nanoipc_server.hpp"

using namespace nanoipc;

using ApiRequest = std::string;
using ApiResponse = int;
using TestNanoIpc = NanoIpcServer<ApiRequest, ApiResponse, 10UL>;

TEST(ut_nanoipc_server, sanity) {
	// GIVEN
	// WHEN

	// THEN
}