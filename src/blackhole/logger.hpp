#pragma once

#include <memory>
#include <vector>

#include "attribute.hpp"
#include "common.hpp"
#include "filter.hpp"
#include "frontend.hpp"
#include "keyword.hpp"
#include "keyword/message.hpp"
#include "keyword/severity.hpp"
#include "keyword/timestamp.hpp"
#include "utils/unique.hpp"

namespace blackhole {

namespace log {

typedef std::function<void()> exception_handler_t;

class default_exception_handler_t {
public:
    void operator()() const {
#ifdef BLACKHOLE_DEBUG
        throw;
#else
        try {
            throw;
        } catch (const std::exception& err) {
            std::cout << "logging core error occurred: " << err.what() << std::endl;
        } catch (...) {
            std::cout << "logging core error occurred: unknown" << std::endl;
        }
#endif
    }
};

} // namespace log

class logger_base_t {
    bool m_enabled;

protected:
    filter_t m_filter;
    log::exception_handler_t m_exception_handler;

    std::vector<std::unique_ptr<base_frontend_t>> m_frontends;

    log::attributes_t m_global_attributes;

public:
    logger_base_t() :
        m_enabled(true),
        m_filter(default_filter_t::instance()),
        m_exception_handler(log::default_exception_handler_t())
    {}

    bool enabled() const {
        return m_enabled;
    }

    void enable() {
        m_enabled = true;
    }

    void disable() {
        m_enabled = false;
    }

    void set_filter(filter_t&& filter) {
        m_filter = std::move(filter);
    }

    void add_attribute(const log::attribute_pair_t& attr) {
        m_global_attributes.insert(attr);
    }

    void add_frontend(std::unique_ptr<base_frontend_t> frontend) {
        m_frontends.push_back(std::move(frontend));
    }

    void set_exception_handler(log::exception_handler_t&& handler) {
        m_exception_handler = std::move(handler);
    }

    log::record_t open_record() const {
        return open_record(log::attributes_t());
    }

    log::record_t open_record(log::attribute_pair_t&& local_attribute) const {
        return open_record(log::attributes_t({ std::move(local_attribute) }));
    }

    log::record_t open_record(log::attributes_t&& local_attributes) const {
        if (enabled() && !m_frontends.empty()) {
            log::attributes_t attributes = merge({
                // universe_attributes          // Program global.
                // thread_attributes            // Thread local.
                m_global_attributes,            // Logger object specific.
                get_event_attributes(),         // Event specific, e.g. timestamp.
                std::move(local_attributes)     // Any user attributes.
            });

            if (m_filter(attributes)) {
                log::record_t record;
                record.attributes = std::move(attributes);
                return record;
            }
        }

        return log::record_t();
    }

    void push(log::record_t&& record) const {
        for (auto it = m_frontends.begin(); it != m_frontends.end(); ++it) {
            try {
                const std::unique_ptr<base_frontend_t>& frontend = *it;
                frontend->handle(record);
            } catch (...) {
                m_exception_handler();
            }
        }
    }

private:
    log::attributes_t get_event_attributes() const {
        log::attributes_t attributes = {
            keyword::timestamp() = std::time(nullptr)
        };
        return attributes;
    }
};

template<typename Level>
class verbose_logger_t : public logger_base_t {
    typedef typename aux::underlying_type<Level>::type level_type;

public:
    log::record_t open_record(Level level) const {
        log::attributes_t attributes = { keyword::severity<Level>() = level };
        return logger_base_t::open_record(std::move(attributes));
    }
};

} // namespace blackhole

//!@todo: Make fallback logger. Make it configurable.
//!@todo: Make json formatter. Try to implement logstash formatter.
//!@todo: Make msgpack formatter.
