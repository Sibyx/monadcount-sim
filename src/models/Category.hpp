//
// Created by Jakub Dubec on 27/03/2025.
//

#ifndef MONADCOUNT_SIM_CATEGORY_HPP
#define MONADCOUNT_SIM_CATEGORY_HPP

#include <string>

class Category {
public:
    enum Type {
        ROOM,
        WALL,
        DOOR,
        SEAT,
        TABLE,
        SNIFFER,
        ACCESS_POINT,
        TERMINAL,
        UNKNOWN
    };

private:
    Type type;

public:
    explicit Category(Type type) : type(type) {}

    static Category fromString(const std::string &str) {
        if (str == "room") return Category(ROOM);
        if (str == "wall") return Category(WALL);
        if (str == "door") return Category(DOOR);
        if (str == "seat") return Category(SEAT);
        if (str == "table") return Category(TABLE);
        if (str == "sniffer") return Category(SNIFFER);
        if (str == "access_point") return Category(ACCESS_POINT);
        if (str == "terminal") return Category(TERMINAL);
        return Category(UNKNOWN);
    }

    std::string toString() const {
        switch (type) {
            case ROOM:
                return "room";
            case WALL:
                return "wall";
            case DOOR:
                return "door";
            case SEAT:
                return "seat";
            case TABLE:
                return "table";
            case SNIFFER:
                return "sniffer";
            case ACCESS_POINT:
                return "access_point";
            case TERMINAL:
                return "terminal";
            default:
                return "unknown";
        }
    }

    Type getType() const { return type; }
};

#endif //MONADCOUNT_SIM_CATEGORY_HPP
