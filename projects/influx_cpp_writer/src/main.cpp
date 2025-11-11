#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <curl/curl.h>

namespace {

std::string get_env(const char *name)
{
    if (const char *value = std::getenv(name)) {
        return value;
    }
    std::ostringstream oss;
    oss << "Environment variable '" << name << "' is required";
    throw std::runtime_error(oss.str());
}

std::string build_write_url(CURL *curl,
                            const std::string &base_url,
                            const std::string &org,
                            const std::string &bucket)
{
    std::ostringstream oss;
    oss << base_url;
    if (!base_url.empty() && base_url.back() == '/') {
        oss.seekp(-1, std::ios_base::end);
    }
    char *encoded_org = curl_easy_escape(curl, org.c_str(), static_cast<int>(org.length()));
    char *encoded_bucket = curl_easy_escape(curl, bucket.c_str(), static_cast<int>(bucket.length()));
    if (!encoded_org || !encoded_bucket) {
        if (encoded_org) {
            curl_free(encoded_org);
        }
        if (encoded_bucket) {
            curl_free(encoded_bucket);
        }
        throw std::runtime_error("Failed to encode URL parameters");
    }

    oss << "/api/v2/write?org=" << encoded_org
        << "&bucket=" << encoded_bucket
        << "&precision=ns";

    curl_free(encoded_org);
    curl_free(encoded_bucket);
    return oss.str();
}

std::string current_timestamp_ns()
{
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto epoch = now.time_since_epoch();
    return std::to_string(duration_cast<nanoseconds>(epoch).count());
}

void check_curl(CURLcode code)
{
    if (code != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(code));
    }
}

} // namespace

int main()
{
    try {
        const std::string influx_url = get_env("INFLUX_URL");
        const std::string influx_token = get_env("INFLUX_TOKEN");
        const std::string influx_org = get_env("INFLUX_ORG");
        const std::string influx_bucket = get_env("INFLUX_BUCKET");

        const std::string measurement = "sensor_data";
        const std::string tag_set = "host=example-host";

        const double value = 42.0;
        const std::string timestamp = current_timestamp_ns();

        std::ostringstream line_protocol;
        line_protocol << measurement << ',' << tag_set << ' ' << "value=" << value << ' ' << timestamp;

        CURL *curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize libcurl");
        }

        const std::string write_url = build_write_url(curl, influx_url, influx_org, influx_bucket);

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: text/plain; charset=utf-8");
        const std::string auth_header = "Authorization: Token " + influx_token;
        headers = curl_slist_append(headers, auth_header.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, write_url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, line_protocol.str().c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, line_protocol.str().size());

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            throw std::runtime_error(std::string("Failed to write to InfluxDB: ") + curl_easy_strerror(res));
        }

        long response_code = 0;
        check_curl(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code));

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (response_code != 204) {
            std::cerr << "Unexpected response code: " << response_code << '\n';
            return 1;
        }

        std::cout << "Successfully wrote point to InfluxDB." << std::endl;
        return 0;
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}
