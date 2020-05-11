#pragma once

#include <cstring>
#include <cmath>
#include <vector>
#include <string>

struct Acceleration
{
    std::vector< std::pair<double, double> > points;
    double get(long dx, long dy) const
    {
        if (points.empty()) {
            return 1.;
        }
        const double d = std::sqrt(dx * dx + dy * dy);
        double k = 0;
        for (size_t i = 0; i < points.size() - 1; ++i) {
            if (d < points[i + 1].first) {
                k += points[i].second * (d - points[i].first);
                return k / d;
            } else {
                k += points[i].second * (points[i + 1].first - points[i].first);
            }
        }
        k += points.back().second * (d - points.back().first);
        return k / d;
    }
};

enum class Role
{
    NOCHANGE,
    LEFT,
    RIGHT,
    MIDDLE,
    X1,
    X2,
    SCROLL,
    DISABLE,
};

struct Parameter
{
    std::string device;
    Role right = Role::NOCHANGE;
    Role left = Role::NOCHANGE;
    Role middle = Role::NOCHANGE;
    Role x1 = Role::NOCHANGE;
    Role x2 = Role::NOCHANGE;
    double sensitivity = 8.;
    Acceleration acceleration;
};

struct GlobalParameter
{
    bool help = false;
    bool verbose = false;
    bool kill = false;
    bool pen = false;
};

class Parameters
{
private:
    std::vector<Parameter> parameters_;
    GlobalParameter global_;

    Role getRole_(char c)
    {
        switch (c) {
        case 'L':
        case 'l':
            return Role::LEFT;
        case 'R':
        case 'r':
            return Role::RIGHT;
        case 'M':
        case 'm':
            return Role::MIDDLE;
        case '1':
            return Role::X1;
        case '2':
            return Role::X2;
        case 'S':
        case 's':
            return Role::SCROLL;
        case 'D':
        case 'd':
            return Role::DISABLE;
        default:
            return Role::NOCHANGE;
        }
    }

public:
    void parse(int argc, char** argv)
    {
        if (argc <= 1) {
            global_.help = true;
        }
        Parameter param;
        for (int i = 1; i < argc; i++) {
            if (_strnicmp("/?", argv[i], 2) == 0) {
                global_.help = true;
            } else if (_strnicmp("/V", argv[i], 2) == 0) {
                global_.verbose = true;
            } else if (_strnicmp("/K", argv[i], 2) == 0) {
                global_.kill = true;
            } else if (_strnicmp("/P", argv[i], 2) == 0) {
                global_.pen = true;
            } else if (_strnicmp("/D", argv[i], 2) == 0) {
                parameters_.push_back(param);
                param = Parameter();
                param.device = argv[i] + 2;
            } else if (_strnicmp("/L", argv[i], 2) == 0) {
                param.left = getRole_(argv[i][2]);
            } else if (_strnicmp("/R", argv[i], 2) == 0) {
                param.right = getRole_(argv[i][2]);
            } else if (_strnicmp("/M", argv[i], 2) == 0) {
                param.middle = getRole_(argv[i][2]);
            } else if (_strnicmp("/1", argv[i], 2) == 0) {
                param.x1 = getRole_(argv[i][2]);
            } else if (_strnicmp("/2", argv[i], 2) == 0) {
                param.x2 = getRole_(argv[i][2]);
            } else if (_strnicmp("/A", argv[i], 2) == 0) {
                param.acceleration.points.clear();
                char* p = argv[i] + 2;
                std::pair<double, double> pair;
                pair.first = 0;
                pair.second = strtod(p, &p);
                if (pair.second <= 0) {
                    continue;
                }
                param.acceleration.points.push_back(pair);
                while (*p == ',') {
                    p++;
                    pair.first = strtod(p, &p);
                    if (pair.first <= param.acceleration.points.back().first) {
                        break;
                    }
                    if (*p != ':') {
                        break;
                    }
                    p++;
                    pair.second = strtod(p, &p);
                    if (pair.second <= 0) {
                        break;
                    }
                    param.acceleration.points.push_back(pair);
                }
            } else if (_strnicmp("/S", argv[i], 2) == 0) {
                param.sensitivity = strtod(argv[i] + 2, NULL);
            } else {
                throw std::string("Unknown option \"") + argv[i] + "\"\n";
            }
        }
        parameters_.push_back(param);
    }

    const Parameter& get(const char* device) const
    {
        if (!device) {
            return parameters_.front();
        }
        for (auto& param : parameters_) {
            if (param.device == device) {
                return param;
            }
        }
        return parameters_.front();
    }

    const GlobalParameter& global() const
    {
        return global_;
    }
};
