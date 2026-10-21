#pragma once

#include <boost/beast/http.hpp>

#include <string>
#include <string_view>

namespace http = boost::beast::http;

namespace dicomserver {

/**
 * DICOMweb (DICOM PS3.18) handlers – Phase 14.
 *
 * Base path: /dicom-web
 */
http::response<http::string_body> HandleDicomWeb(
    const http::request<http::string_body>& req,
    std::string_view path,
    http::verb method);

bool IsDicomWebPath(std::string_view path);

}  // namespace dicomserver
