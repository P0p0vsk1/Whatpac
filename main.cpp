#include <iostream>
#include <string>
#include <sstream>
#include <curl/curl.h>
#include <json/json.h>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string fetch_package_info(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

std::string join_json_array(const Json::Value& array) {
    std::ostringstream oss;
    for (Json::Value::ArrayIndex i = 0; i < array.size(); ++i) {
        if (i > 0) {
            oss << ", ";
        }
        oss << array[i].asString();
    }
    return oss.str();
}

std::string safe_as_string(const Json::Value& value) {
    return value.isString() ? value.asString() : "";
}

void parse_and_print_package_info(const std::string& json_data, bool is_official, bool& found) {
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;
    std::istringstream s(json_data);

    if (Json::parseFromStream(builder, s, &root, &errs)) {
        if (is_official) {
            if (!root["results"].empty()) {
                found = true;
                const auto& pkg = root["results"][0];
                std::cout << "Package Name: " << safe_as_string(pkg["pkgname"]) << std::endl;
                std::cout << "Description: " << safe_as_string(pkg["pkgdesc"]) << std::endl;
                std::cout << "Repository: " << safe_as_string(pkg["repo"]) << std::endl;
                std::cout << "Version: " << safe_as_string(pkg["pkgver"]) << std::endl;
                std::cout << "URL: " << safe_as_string(pkg["url"]) << std::endl;
                std::cout << "License: " << (pkg["licenses"].isArray() ? join_json_array(pkg["licenses"]) : safe_as_string(pkg["licenses"])) << std::endl;
                std::cout << "Dependencies: " << (pkg["depends"].isArray() ? join_json_array(pkg["depends"]) : safe_as_string(pkg["depends"])) << std::endl;
            }
        } else {
            if (!root["results"].empty()) {
                found = true;
                const auto& pkg = root["results"][0];
                std::cout << "Package Name: " << safe_as_string(pkg["Name"]) << std::endl;
                std::cout << "Description: " << safe_as_string(pkg["Description"]) << std::endl;
                std::cout << "Repository: AUR" << std::endl;
                std::cout << "Version: " << safe_as_string(pkg["Version"]) << std::endl;
                std::cout << "URL: " << safe_as_string(pkg["URL"]) << std::endl;
                std::cout << "License: " << safe_as_string(pkg["License"]) << std::endl;
                std::cout << "Dependencies: " << (pkg["Depends"].isArray() ? join_json_array(pkg["Depends"]) : safe_as_string(pkg["Depends"])) << std::endl;
            }
        }
    } else {
        std::cerr << "Failed to parse JSON: " << errs << std::endl;
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <package_name>" << std::endl;
        return 1;
    }

    std::string package_name = argv[1];
    std::string official_url = "https://archlinux.org/packages/search/json/?name=" + package_name;
    std::string aur_url = "https://aur.archlinux.org/rpc/?v=5&type=info&arg[]=" + package_name;

    bool found = false;

    std::string official_data = fetch_package_info(official_url);
    parse_and_print_package_info(official_data, true, found);

    std::string aur_data = fetch_package_info(aur_url);
    parse_and_print_package_info(aur_data, false, found);

    if (!found) {
        std::cout << "Package not found in either official repositories or AUR." << std::endl;
    }

    return 0;
}
