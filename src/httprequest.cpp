#include "httprequest.h"
#include <algorithm>

void HttpRequest::set_method(const char* start, const char* end) {
    string m(start, end);
    auto it = str_to_method_map.find(m);
    method = it == str_to_method_map.end() ?
             HttpMethod::unknown ? it -> second;
