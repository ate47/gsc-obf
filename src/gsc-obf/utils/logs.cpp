#include <includes.hpp>
#include <utils/logs.hpp>
#include <utils/utils.hpp>

namespace utils::logs {

    static struct {
        struct {
            loglevel loglevel = utils::logs::LVL_INFO;
            const char* logfile{};
            bool basiclog{};
            bool noStdLogs{};
            std::ostream* outStream{};
            std::ostream* cout{};
            std::ostream* cerr{};
            utils::logs::logcallback callback{};

            std::vector<std::string> paths{};
        } log;
    } cfg;

    void addlogpath(const std::string& path) {
        size_t start{};
        while (start < path.size()) {
            size_t idx{ path.find(';', start) };
            if (idx == std::string::npos) {
                idx = path.size();
            }

            cfg.log.paths.push_back(path.substr(start, idx));

            start = idx + 1;
        }
    }
    void cleanuplogpaths() { cfg.log.paths.clear(); }

    struct {
        const char* TRACE = actssec("TRACE");
        const char* ERROR = actssec("ERROR");
        const char* WARN = actssec("WARN.");
        const char* INFO = actssec("INFO.");
        const char* DEBUG = actssec("DEBUG");
        const char* UKN = actssec("UKN..");
    } _s;

    const char* name(loglevel lvl) {
        switch (lvl) {
        case LVL_TRACE:
        case LVL_TRACE_PATH:
            return _s.TRACE;
        case LVL_ERROR:
            return _s.ERROR;
        case LVL_WARNING:
            return _s.WARN;
        case LVL_INFO:
            return _s.INFO;
        case LVL_DEBUG:
            return _s.DEBUG;
        default:
            return _s.UKN;
        }
    }

    void setlevel(loglevel lvl) { cfg.log.loglevel = lvl; }
    loglevel getlevel() { return cfg.log.loglevel; }
    void setbasiclog(bool basiclog) { cfg.log.basiclog = basiclog; }

    void setfile(const char* filename) { cfg.log.logfile = filename; }
    void addoutstream(std::ostream* outStream) { cfg.log.outStream = outStream; }
    void addoutcallback(logcallback callback) { cfg.log.callback = callback; }
    void disablestdout(bool disabled) { cfg.log.noStdLogs = disabled; }
    void setstrstreams(std::ostream* cout, std::ostream* cerr) {
        auto& logs{ cfg.log };
        logs.cerr = cerr;
        logs.cout = cout;
    }
    const char* logfile() { return cfg.log.logfile; }

    inline std::tm localtime_xp(std::time_t timer) {
        // https://stackoverflow.com/questions/38034033/c-localtime-this-function-or-variable-may-be-unsafe
        std::tm bt{};
#if defined(__unix__)
        localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
        localtime_s(&bt, &timer);
#else
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        bt = *std::localtime(&timer);
#endif
        return bt;
    }
    void log(loglevel level, const char* file, size_t line, const char* str, bool endl) {
        if (getlevel() > level) {
            return;
        }
        log(level, name(level), file, line, str, endl);
    }

    void log(loglevel level, const char* header, const char* file, size_t line, const char* str, bool endl) {
        if (file && cfg.log.paths.size()) {
            // check if the file can be handled
            std::string_view fsw{ file };
            bool match{};
            for (const std::string& p : cfg.log.paths) {
                if (fsw.starts_with(p)) {
                    match = true;
                    break;
                }
            }
            if (!match)
                return; // not matching our pattern
        }

        if (cfg.log.callback) {
            cfg.log.callback(level, header, file, line, str, endl);
        }

        auto f = [&](std::ostream& out) {
            bool locked{};
            if (!cfg.log.basiclog) {
                std::tm tm = localtime_xp(std::time(nullptr));
                out << "[" << std::put_time(&tm, "%H:%M:%S") << "]";
            }

            if (!cfg.log.basiclog) {
                if (header) {
                    out << '[' << header << ']';
                }
                if (file && cfg.log.loglevel == LVL_TRACE_PATH) {
                    out << '[' << file;
                    if (line) {
                        out << '@' << std::dec << line;
                    }
                    out << ']';
                }
                out << " ";
            }
            out << str;

            if (endl)
                out << "\n";
        };

        auto* lf = logfile();
        if (lf) {
            std::ofstream out{ lf, std::ios::app };

            f(out);

            out.close();
        } else {
            auto& logs{ cfg.log };

            if (level < LVL_WARNING) {
                if (!logs.noStdLogs || logs.cout) {
                    f(logs.cout ? *logs.cout : std::cout);
                }
            } else {
                if (!logs.noStdLogs || logs.cerr) {
                    f(logs.cerr ? *logs.cerr : std::cerr);
                }
            }
        }
        if (cfg.log.outStream) {
            f(*cfg.log.outStream);
        }
    }
} // namespace utils::logs