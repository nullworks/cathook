#pragma once

#include <unordered_map>
//#include <cstring>
#include <string.h>
#include <memory>
#include "core/logging.hpp"

// this and the cpp are creds to "Altimor"

class RecvTable;
class RecvProp;

class equal_char
{
public:
    bool operator()(const char *const &v1, const char *const &v2) const
    {
        return !strcmp(v1, v2);
    }
};

struct hash_char
{
public:
    size_t operator()(const char *obj) const
    {
        size_t res         = 0;
        const size_t prime = 31;
        for (size_t i = 0; i < strlen(obj); ++i)
        {
            res = obj[i] + (res * prime);
        }
        return res;
    }
};

class netvar_tree
{
    struct node;
    using map_type = boost::unordered_flat_map<const char *, std::shared_ptr<node>, hash_char, equal_char>;

    struct node
    {
        node(int offset, RecvProp *p) : offset(offset), prop(p)
        {
        }

        map_type nodes;
        int offset;
        RecvProp *prop;
    };

    map_type nodes;

public:
    // netvar_tree ( );

    void init();

private:
    void populate_nodes(class RecvTable *recv_table, map_type *map);

    /**
     * get_offset_recursive - Return the offset of the final node
     * @map:	Node map to scan
     * @acc:	Offset accumulator
     * @name:	Netvar name to search for
     *
     * Get the offset of the last netvar from map and return the sum of it and
     * accum
     */
    int get_offset_recursive(map_type &map, int acc, const char *name)
    {
        if (!map.count(name))
        {
            logging::Info("can't find %s!", name);
            return 0;
        }
        return acc + map[name]->offset;
    }

    /**
     * get_offset_recursive - Recursively grab an offset from the tree
     * @map:	Node map to scan
     * @acc:	Offset accumulator
     * @name:	Netvar name to search for
     * @args:	Remaining netvar names
     *
     * Perform tail recursion with the nodes of the specified branch of the tree
     * passed for map and the offset of that branch added to acc
     */
    template <typename... args_t> int get_offset_recursive(map_type &map, int acc, const char *name, args_t... args)
    {
        if (!map.count(name))
        {
            logging::Info("can't find %s!", name);
            return 0;
        }
        const auto &node = map[name];
        return get_offset_recursive(node->nodes, acc + node->offset, args...);
    }

    RecvProp *get_prop_recursive(map_type &map, const char *name)
    {
        return map[name]->prop;
    }

    template <typename... args_t> RecvProp *get_prop_recursive(map_type &map, const char *name, args_t... args)
    {
        const auto &node = map[name];
        return get_prop_recursive(node->nodes, args...);
    }

public:
    /**
     * get_offset - Get the offset of a netvar given a list of branch names
     * @name:	Top level datatable name
     * @args:	Remaining netvar names
     *
     * Initiate a recursive search down the branch corresponding to the
     * specified datable name
     */
    template <typename... args_t> int get_offset(const char *name, args_t... args)
    {
        const auto &node = nodes[name];
        if (node == 0)
        {
            logging::Info("Invalid NetVar node: %s", name);
            return 0;
        }
        int offset = get_offset_recursive(node->nodes, node->offset, args...);
        return offset;
    }

    template <typename... args_t> RecvProp *get_prop(const char *name, args_t... args)
    {
        const auto &node = nodes[name];
        return get_prop_recursive(node->nodes, args...);
    }

    void dump();
};

extern netvar_tree gNetvars;
