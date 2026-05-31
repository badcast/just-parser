/*
 *** Just Object Parser ***
 *** Project by badcast <lmecomposer@gmail.com>
 *** Project url: https://github.com/badcast/just-parser
 *** Project Date: 31.12.2022
 ***
 *** jnode_t = is Node for object
 *** jtree_t = is Multi jnode_t | Descriptions:  jtree_t have a jnode_t (int) data
 ***
 *** jtree_t have a one or multi nodes
 */

#include <cstdlib>
#include <vector>
#include <tuple>
#include <climits>
#include <stack>
#include <iostream>
#include <set>

// import header
#include "justparser"

#if __unix__ || __linux__
#include <unistd.h>
#elif WIN32
#include <windows.h>
#endif

#define method

#define MACROPUT(base) #base

namespace just
{
typedef jvariant jnode_t;
typedef std::vector<int> jtree_t;

enum { Node_ValueFlag = 1, Node_ArrayFlag = 2, Node_TreeFlag = 3 };

static const struct {
    // member for use floating point (delimeter)
    char just_dot = '.';
    // member for use seperate array data
    char just_obstacle = ',';
    // member for use path breaker in tree. see. just_object_node::at()
    char just_tree_pathbrk = '/';
    // member for use comment breaker
    char just_commentLine[3] = "//";
    // member for use Screening character symbol (\n, \r, \m, etc.)
    char just_left_seperator = '\\';
    // member for use end of line (EOF).
    char just_eol_segment = '\n';
    // member for use string compact
    char just_format_string = '\"';
    // member for use boolean data (true)
    char just_true_string[5] = MACROPUT(true);
    // member for use boolean data (false)
    char just_false_string[6] = MACROPUT(false);
    // member for use null data (not defined type)
    char just_null_string[5] = MACROPUT(null);
    // member for use unknown type (use as string member)
    char just_unknown_string[6] { "{...}" };
    // member for use block or array first/end segments
    char just_block_segments[2] { '{', '}' };
    // member for use trimming characters
    char just_trim_segments[5] { ' ', '\t', '\n', '\r', '\v' }; // TODO: TRIM - replace to std::isspace
    // member for use valid property name (not use "_" as first word)
    char just_valid_property_name[3] = { 'A', 'z', '_' };
    // member for use negative number character
    char just_negative_sym = '-';
    // member for use positive number character
    char just_positive_sym = '+';
} just_syntax;


method inline int system_get_page_size();

method inline int just_type_size(const JustType type);

/*parser*/
method inline int just_string_to_hash_fast(const char* char_side, int contentLength);
method inline bool just_is_unsigned_jnumber(const char char_side);
method inline bool just_is_jnumber(const char* char_side, int* getLength);
method jbool just_is_jreal(const char* char_side, int* getLength);
method inline jbool just_is_jbool(const char* char_side, int* getLength);
method int just_get_format(const char* char_side, just_storage** storage, JustType& containType, jvariant* outValue);
method inline jbool just_has_datatype(const char* char_side);
method int just_trim(const char* char_side, int contentLength);
method int just_skip(const char* char_side, int length);
method inline jbool just_valid_property_name(const char* char_side, int len);
method inline jbool just_is_comment_line(const char* char_side, int len);
method inline int just_has_eol(const char* pointer, int len);
method int just_autoskip_comment(const char* char_side, int len);
method inline jbool just_is_space(const char char_side);
method jbool just_is_array(const char* char_side, int& endpoint, int contentLength);
method void just_avail_only(just_stats& jstat, const char* source, int length);
method void just_avail(just_storage** pstore, just_stats& jstat, const char* source, int length);
method int bug_just_avail(just_storage** storage, just_stats& eval, const char* source, int length, int depth);

method inline int just_type_size(const JustType type)
{
    switch (type) {
    case JustType::JustBoolean:
        return sizeof(jbool);
    case JustType::JustNumber:
        return sizeof(jnumber);
    case JustType::JustReal:
        return sizeof(jreal);
    case JustType::JustTree:
        return sizeof(jtree_t);
    }
    return 0;
}

method inline int system_get_page_size()
{
    int _PAGE_SIZE;
#if __unix__ || __linux__
    _PAGE_SIZE = sysconf(_SC_PAGE_SIZE);
#elif WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    _PAGE_SIZE = static_cast<int>(sysInfo.dwPageSize);
#else
    _PAGE_SIZE = 4096;
#endif
    return _PAGE_SIZE;
}

// method for fast get hash from string
method inline int just_string_to_hash_fast(const char* char_side, int contentLength = INT32_MAX)
{
    int x = 1, y = 0;
    while (*(char_side) && y++ < contentLength)
        x *= *(char_side++);
    return x;
}

// method for check valid a unsigned number
method inline bool just_is_unsigned_jnumber(const char char_side) { return std::isdigit(char_side); }

// method for check valid a signed number
method inline bool just_is_jnumber(const char* char_side, int* getLength)
{

    int x = 0;
    if (*char_side == just_syntax.just_negative_sym) {
        ++x;
        ++char_side;
    }
    const char* pointer = char_side;
    for (; just_is_unsigned_jnumber(*pointer); ++x, ++pointer) { }
    if (getLength) {
        *getLength = x;
    }

    return pointer - char_side > 0;
}

// method for check is real ?
method jbool just_is_jreal(const char* char_side, int* getLength)
{
    bool real = false;
    for (;;) {
        if (!just_is_unsigned_jnumber(char_side[*getLength])) {
            if (real)
                break;

            real = (char_side[*getLength] == just_syntax.just_dot);

            if (!real)
                break;
        }
        ++*getLength;
    }
    return real;
}

// method for check is bool ?
method inline jbool just_is_jbool(const char* char_side, int* getLength)
{
    if (!std::memcmp(char_side, just_syntax.just_true_string, *getLength = sizeof(just_syntax.just_true_string) - 1))
        return true;
    if (!std::memcmp(char_side, just_syntax.just_false_string, *getLength = sizeof(just_syntax.just_false_string) - 1))
        return true;
    *getLength = 0;
    return false;
}

// method for get format from raw content, also to write in storage pointer
method int just_get_format(const char* char_side, JustType& containType, jvariant* outValue = nullptr)
{
    /*
         * Priority:
         *  - JReal
         *  - JNumber
         *  - JBool
         *  - JString
         */

    void* mem;
    int offset = 0;

    // Null type
    if (char_side == nullptr || *char_side == '\0') {
        containType = JustType::Null;
    } else if (just_is_jreal(char_side, &offset)) { // Real type -----------------------------------------------------------------------------
        containType = JustType::JustReal;
        if (storage) {
            jreal conv = std::atof(char_side); // use "C++" method
            // Copy to
            mem = std::memcpy(just_storage_alloc_field(storage, containType), &conv, just_type_size(containType));
            if (outValue)
                *outValue = mem;
        }
    } else if (just_is_jnumber(char_side, &offset)) { // Number type ---------------------------------------------------------------------------------
        jnumber conv;
        containType = JustType::JustNumber;
        sscanf(char_side, "%lld", &conv); // use "C" method
        if (storage) {
            // Copy to
            mem = std::memcpy(just_storage_alloc_field(storage, containType), &conv, just_type_size(containType));
            if (outValue)
                *outValue = mem;
        }
    } else if (just_is_jbool(char_side, &offset)) { // Bool type -----------------------------------------------------------------------------
        containType = JustType::JustBoolean;
        if (storage) {
            jbool conv = (offset == sizeof(just_syntax.just_true_string) - 1);
            // Copy to
            mem = std::memcpy(just_storage_alloc_field(storage, containType), &conv, just_type_size(containType));
            if (outValue)
                *outValue = mem;
        }
    } else if (*char_side == just_syntax.just_format_string) { // String type ----------------------------------------------------------------
        ++offset;
        for (; char_side[offset] != just_syntax.just_format_string; ++offset)
            if (char_side[offset] == just_syntax.just_left_seperator && char_side[offset + 1] == just_syntax.just_format_string)
                ++offset;
        containType = JustType::JustString;
        if (--offset) {
            if (storage) {
                // TODO: Go support next for string. '\n' '\r' .etc.
                jstring stringSyntax;
                stringSyntax.reserve(offset);
                for (int x = 0; x < offset; ++x) {
                    if (char_side[x + 1] == just_syntax.just_left_seperator)
                        ++x;
                    stringSyntax.push_back(char_side[x + 1]);
                }

                // Copy to
                mem = std::memcpy(just_storage_alloc_field(storage, containType, stringSyntax.size()), stringSyntax.data(), stringSyntax.size());
                if (outValue)
                    *outValue = mem;
            }
        }
        offset += 2;
    } else // another type
        containType = JustType::Unknown;

    return offset;
}

// method for check. has type in value
method inline jbool just_has_datatype(const char* char_side)
{
    JustType type;
    just_get_format(const_cast<char*>(char_side), nullptr, type);
    return type > JustType::Null;
}

// method for trim tabs, space, EOL (etc) to skip.
method int just_trim(const char* char_side, int contentLength = INT_MAX)
{
    int x = 0;
    if (char_side != nullptr)
        for (int y; x < contentLength && *char_side;) {
            for (y = 0; y < static_cast<int>(sizeof(just_syntax.just_trim_segments));)
                if (*char_side == just_syntax.just_trim_segments[y])
                    break;
                else
                    ++y;
            if (y != sizeof(just_syntax.just_trim_segments)) {
                ++x;
                ++char_side;
            } else
                break;
        }
    return x;
}

method int just_skip(const char* char_side, int length = INT_MAX)
{
    int x, y;
    char skipping[sizeof(just_syntax.just_trim_segments) + sizeof(just_syntax.just_block_segments)];
    strncpy(skipping, just_syntax.just_trim_segments, sizeof(just_syntax.just_trim_segments));
    strncpy(skipping + sizeof just_syntax.just_trim_segments, just_syntax.just_block_segments, sizeof(just_syntax.just_block_segments));
    for (x = 0; x < length;) {
        for (y = 0; y < sizeof(skipping);)
            if (char_side[x] == skipping[y])
                break;
            else
                ++y;
        if (y == sizeof(skipping))
            ++x;
        else
            break;
    }
    return x;
}

method inline jbool just_valid_property_name(const char* char_side, int len)
{
    int x;
    for (x = 0; x < len; ++x, ++char_side)
        if (!((*char_side >= *just_syntax.just_valid_property_name && *char_side <= just_syntax.just_valid_property_name[1]) || (x && just_is_unsigned_jnumber(*char_side)) || *char_side == just_syntax.just_valid_property_name[2]))
            return false;
    return len != 0;
}

method inline jbool just_is_comment_line(const char* char_side, int len) { return len > 0 && !std::strncmp(char_side, just_syntax.just_commentLine, std::min(2, len)); }

method inline int just_has_eol(const char* pointer, int len = INT_MAX)
{
    const char* chars = pointer;
    const char* endChars = pointer + len;
    while (*chars && chars < endChars && *chars != just_syntax.just_eol_segment)
        ++chars;
    return static_cast<int>(chars - pointer);
}

// method for skip comment's
method int just_autoskip_comment(const char* char_side, int len)
{
    int offset;
    const char* pointer = char_side;
    pointer += offset = just_trim(pointer, len);
    while (just_is_comment_line(pointer, len - offset)) {
        // skip to EOL
        int skipped;
        pointer += skipped = just_has_eol(pointer, len - offset);
        offset += skipped;
        // trimming
        pointer += skipped = just_trim(pointer, len - offset);
        offset += skipped;
    }
    return offset;
}

method inline jbool just_is_space(const char char_side)
{
    const char* left = just_syntax.just_trim_segments;
    const char* right = left + sizeof(just_syntax.just_trim_segments);
    for (; left < right; ++left)
        if (*left == char_side)
            return true;
    return false;
}

method jbool just_is_array(const char* char_side, int& endpoint, int contentLength = INT_MAX)
{
    jbool result = false;

    if (*char_side == *just_syntax.just_block_segments) {
        int x = 1;
        x += just_autoskip_comment(char_side + x, contentLength - x);
        if (just_has_datatype(char_side + x) || char_side[x] == just_syntax.just_block_segments[1]) {
            for (; char_side[x] && x < contentLength; ++x) {
                if (char_side[x] == just_syntax.just_block_segments[0])
                    break;
                else if (char_side[x] == just_syntax.just_block_segments[1]) {
                    endpoint = x;
                    result = true;
                    break;
                }
            }
        }
    }
    return result;
}

// Just Object Node

just_object_node::just_object_node(just_object_parser* owner, void* handle)
{
    this->_jowner = owner;
    this->jname = handle;
}

method JustType just_object_node::type() const { just_storage_get_type(static_cast<just_storage*>(this->_jowner->_storage), jname); }

method just_object_node* just_object_node::tree(const jstring& child) { return nullptr; }

method jbool just_object_node::has_tree() const { throw std::runtime_error("This is method not implemented"); }

jstring just_object_node::to_string() const
{
    switch (type()) {
    case JustType::JustString:
        return get_str();
    case JustType::JustNumber:
        return std::to_string(get_int());
    case JustType::JustBoolean:
        return std::to_string(get_bool());
    case JustType::JustReal:
        return std::to_string(get_real());
    case JustType::Unknown:
        return jstring(just_syntax.just_unknown_string);
    case JustType::Null:
        return jstring(just_syntax.just_null_string);
    }
    return {};
}

just_object_node::operator jnumber() const { return get_int(); }

just_object_node::operator jbool() const { return get_bool(); }

just_object_node::operator jreal() const { return get_real(); }

just_object_node::operator jstring() const { return get_str(); }

method const jstring just_object_node::name() const { return jstring(static_cast<char*>(jname)); }

method void just_avail_only(just_stats& jstat, const char* source, int length)
{
    int x, z;
    int depth; // depths
    JustType valueType;
    JustType current_block_type;
    just_storage* pstorage;
    std::vector<jtree_t*> tree;
    const char* pointer = source;
    const char* epointer = source + length;

    // TODO: Linear load

    pstorage = just_storage_new_init();

    tree.emplace_back(just_storage_alloc_tree(&pstorage));

    for (depth = x = 0; pointer + x < epointer;) {
        // has comment line
        int y = x += just_autoskip_comment(pointer + x, length - x);
        if (depth > 0 && pointer[x] == just_syntax.just_block_segments[1])
            // TODO: Next Depth (Up)
            break;
        x += just_skip(pointer + x, length - x);

        // check property name
        if (!just_valid_property_name(pointer + y, x - y))
            throw std::bad_exception();

        // property name
        // prototype_node.propertyName.append(just_source + y, static_cast<size_t>(x - y));

        std::cout << jstring(pointer + y, static_cast<size_t>(x - y)) << std::endl;

        // has comment line
        x += just_autoskip_comment(pointer + x, length - x);
        // x += just_trim(pointer + x, length - x); // trim string
        // is block or array
        if (pointer[x] == *just_syntax.just_block_segments) {
            if (just_is_array(pointer + x, y, length - x)) {
                y += x++;
                current_block_type = JustType::Unknown;
                // prototype_node.flags = Node_ArrayFlag;
                for (; x < y;) {
                    x += just_autoskip_comment(pointer + x, y - x);
                    // next index
                    if (pointer[x] == just_syntax.just_obstacle)
                        ++x;
                    else {
                        x += z = just_get_format(pointer + x, nullptr, valueType);

                        // Just Node Object current value Type

                        if (valueType != JustType::Unknown) {
                            if (current_block_type == JustType::Unknown) {
                                current_block_type = valueType;
                                switch (current_block_type) {
                                case JustType::JustString:
                                    ++jstat.jarrstrings;
                                    break;
                                case JustType::JustBoolean:
                                    ++jstat.jarrbools;
                                    break;
                                case JustType::JustReal:
                                    ++jstat.jarrreals;
                                    break;
                                case JustType::JustNumber:
                                    ++jstat.jarrnumbers;
                                    break;
                                }
                            }
                            // can't convert just numbers as JustReal
                            else if (false && current_block_type != valueType)
                                throw std::runtime_error("Multi type is found.");

                            switch (current_block_type) {
                            case JustType::JustString:
                                jstat.jarrstrings_total_bytes += z - 1;
                                // prototype_node.set_native_memory((void*)jalloc<std::vector<jstring>>());
                                break;
                            case JustType::JustBoolean:
                                jstat.jarrbools_total_bytes += z;
                                // prototype_node.set_native_memory((void*)jalloc<std::vector<jbool>>());
                                break;
                            case JustType::JustReal:
                                jstat.jarrreals_total_bytes += z;
                                // prototype_node.set_native_memory((void*)jalloc<std::vector<jreal>>());
                                break;
                            case JustType::JustNumber:
                                jstat.jarrnumbers_total_bytes += z;
                                // prototype_node.set_native_memory((void*)jalloc<std::vector<jnumber>>());
                                break;
                            }
                        }
                    }
                    x += just_autoskip_comment(pointer + x, y - x);
                }

            } else { // enter the next node
                // up next node
                // x += just_avail_only(jstat, pointer + x, length - x);
                tree.emplace_back(just_storage_alloc_tree(&pstorage, tree.back())); // alloc tree on owner

                ++x;
                ++depth;
                // x += just_autoskip_comment(pointer + x, length - x);
            }

            if (false && pointer[x] != just_syntax.just_block_segments[1]) {
                // Error: Line in require end depth
                throw std::bad_exception();
            }
        } else { // get also value
            x += z = just_get_format(pointer + x, nullptr, valueType);

            switch (valueType) {
            case JustType::JustString:
                ++jstat.jstrings;
                jstat.jarrnumbers_total_bytes += z - 1;
                break;
            case JustType::JustBoolean:
                ++jstat.jbools;
                jstat.jarrbools_total_bytes += z;
                break;
            case JustType::JustReal:
                ++jstat.jreals;
                jstat.jarrreals_total_bytes += z;
                break;
            case JustType::JustNumber:
                ++jstat.jnumbers;
                jstat.jarrnumbers_total_bytes += z;
                break;
            }

            // prototype_node.set_native_memory(pointer);
            // pointer = nullptr;
            // prototype_node.flags = Node_ValueFlag | valueType << 2;
        }

        // entry.insert(std::make_pair(y = just_string_to_hash(prototype_node.propertyName.c_str()), prototype_node));

        /*
            #if defined(QDEBUG) || defined(DEBUG)
                    // get iter
                    auto _curIter = entry.find(j);
                    if (_dbgLastNode) _dbgLastNode->nextNode = &_curIter->second;
                    _curIter->second.prevNode = _dbgLastNode;
                    _dbgLastNode = &_curIter->second;
            #endif
            */
        x += just_autoskip_comment(pointer + x, length - x);
    }
}

method void just_avail(just_storage** storage, just_stats& jstat, const char* source, int length)
{
    int x, z;
    int depth; // depths
    JustType valueType;
    JustType current_block_type;
    just_storage* pstorage;
    std::vector<jtree_t*> stack;
    const char* pointer = source;
    const char* epointer = source + length;

    // TODO: Linear load

    pstorage = just_storage_new_init();

    stack.emplace_back(just_storage_alloc_tree(&pstorage));

    for (depth = x = 0; pointer + x < epointer;) {
        // has comment line
        int y = x += just_autoskip_comment(pointer + x, length - x);
        if (depth > 0 && pointer[x] == just_syntax.just_block_segments[1])
            break;
        x += just_skip(pointer + x, length - x);

        // Preparing, check property name
        if (!just_valid_property_name(pointer + y, x - y))
            throw std::bad_exception();

        // property name
        // prototype_node.propertyName.append(just_source + y, static_cast<size_t>(x - y));

        std::cout << jstring(pointer + y, static_cast<size_t>(x - y)) << std::endl;

        //  has comment line
        x += just_autoskip_comment(pointer + x, length - x);
        // is block or array
        if (pointer[x] == *just_syntax.just_block_segments) {
            if (just_is_array(pointer + x, y, length - x)) {
                y += x++;
                current_block_type = JustType::Unknown;
                // prototype_node.flags = Node_ArrayFlag;
                for (; x < y;) {
                    x += just_autoskip_comment(pointer + x, y - x);
                    // next index
                    if (pointer[x] == just_syntax.just_obstacle)
                        ++x;
                    else {
                        x += z = just_get_format(pointer + x, nullptr, valueType);

                        // Just Node Object current value Type

                        if (valueType != JustType::Unknown) {
                            if (current_block_type == JustType::Unknown) {
                                current_block_type = valueType;
                                switch (current_block_type) {
                                case JustType::JustString:
                                    ++jstat.jarrstrings;
                                    break;
                                case JustType::JustBoolean:
                                    ++jstat.jarrbools;
                                    break;
                                case JustType::JustReal:
                                    ++jstat.jarrreals;
                                    break;
                                case JustType::JustNumber:
                                    ++jstat.jarrnumbers;
                                    break;
                                }
                            }
                            // convert just numbers as JustReal
                            else if (false && current_block_type != valueType)
                                throw std::runtime_error("Multi type is found.");

                            switch (current_block_type) {
                            case JustType::JustString:
                                jstat.jarrstrings_total_bytes += z - 1;
                                // prototype_node.set_native_memory((void*)jalloc<std::vector<jstring>>());
                                break;
                            case JustType::JustBoolean:
                                jstat.jarrbools_total_bytes += z;
                                // prototype_node.set_native_memory((void*)jalloc<std::vector<jbool>>());
                                break;
                            case JustType::JustReal:
                                jstat.jarrreals_total_bytes += z;
                                // prototype_node.set_native_memory((void*)jalloc<std::vector<jreal>>());
                                break;
                            case JustType::JustNumber:
                                jstat.jarrnumbers_total_bytes += z;
                                // prototype_node.set_native_memory((void*)jalloc<std::vector<jnumber>>());
                                break;
                            }
                        }
                    }
                    x += just_autoskip_comment(pointer + x, y - x);
                }

            } else { // enter the next node
                // up next node
                // x += just_avail_only(jstat, pointer + x, length - x);

                stack.emplace_back(just_storage_alloc_tree(&pstorage, stack.back())); // alloc tree on owner
                ++x;
                ++depth;
                // x += just_autoskip_comment(pointer + x, length - x);
            }

            if (false && pointer[x] != just_syntax.just_block_segments[1]) {
                // Error: Line in require end depth
                throw std::bad_exception();
            }
            ++x;
        } else { // get also value
            x += z = just_get_format(pointer + x, nullptr, valueType);

            switch (valueType) {
            case JustType::JustString:
                ++jstat.jstrings;
                jstat.jarrnumbers_total_bytes += z - 1;
                break;
            case JustType::JustBoolean:
                ++jstat.jbools;
                jstat.jarrbools_total_bytes += z;
                break;
            case JustType::JustReal:
                ++jstat.jreals;
                jstat.jarrreals_total_bytes += z;
                break;
            case JustType::JustNumber:
                ++jstat.jnumbers;
                jstat.jarrnumbers_total_bytes += z;
                break;
            }

            // prototype_node.set_native_memory(pointer);
            // pointer = nullptr;
            // prototype_node.flags = Node_ValueFlag | valueType << 2;
        }

        // entry.insert(std::make_pair(y = just_string_to_hash(prototype_node.propertyName.c_str()), prototype_node));

        /*
            #if defined(QDEBUG) || defined(DEBUG)
                    // get iter
                    auto _curIter = entry.find(j);
                    if (_dbgLastNode) _dbgLastNode->nextNode = &_curIter->second;
                    _curIter->second.prevNode = _dbgLastNode;
                    _dbgLastNode = &_curIter->second;
            #endif
            */
        x += just_autoskip_comment(pointer + x, length - x);
    }
}

// big algorithm, please free me.
// divide and conquer method avail
method int bug_just_avail(just_storage** storage, just_stats& eval, const char* source, int length, int depth)
{
    int x, y;
    const char* pointer;
    JustType valueType;
    JustType current_block_type;

    std::vector<jtree_t*> _stacks;

    _stacks.emplace_back(just_storage_alloc_tree(storage)); // push head tree

#define prototype_node (_stacks.back()) // set a macro name
#define push(object) (_stacks.push_back(object))
#define pop() (_stacks.pop_back())

    pointer = source;

    for (x = 0; x < length;) {
        // skip and trimming
        y = x += just_autoskip_comment(pointer + x, length - x);
        if (depth > 0 && pointer[x] == just_syntax.just_block_segments[1])
            break;
        x += just_skip(pointer + x, length - x);

        // Preparing, check property name
        if (!just_valid_property_name(pointer + y, x - y))
            throw std::bad_exception();

        // BUG: Set property name
        // prototype_node.propertyName.append(pointer + y, static_cast<size_t>(x - y)); // set property name

        // has comment line
        x += just_autoskip_comment(pointer + x, length - x);
        x += just_trim(pointer + x, length - x); // trim string
        // is block or array
        if (pointer[x] == *just_syntax.just_block_segments) {
            if (just_is_array(pointer + x, y, length - x)) {
                y += x++;
                current_block_type = JustType::Unknown;
                // prototype_node.flags = Node_ArrayFlag;

                // While end of array length
                while (x < y) {
                    x += just_autoskip_comment(pointer + x, y - x);
                    // next index
                    if (pointer[x] == just_syntax.just_obstacle)
                        ++x;
                    else {
                        jvariant pvalue;
                        x += just_get_format(pointer + x, storage, valueType, &pvalue);
                        if (valueType != JustType::Unknown) {
                            if (current_block_type == JustType::Unknown) {
                                current_block_type = valueType;

                                switch (current_block_type) {
                                case JustType::JustBoolean:
                                    // prototype_node.set_native_memory((void*)jalloc<std::vector<jbool>>());
                                    break;
                                case JustType::JustNumber:
                                    // prototype_node.set_native_memory((void*)jalloc<std::vector<jnumber>>());
                                    break;
                                case JustType::JustReal:
                                    // prototype_node.set_native_memory((void*)jalloc<std::vector<jreal>>());
                                    break;
                                case JustType::JustString:
                                    // prototype_node.set_native_memory((void*)jalloc<std::vector<jstring>>());
                                    break;
                                }

                            } else if (current_block_type != valueType && current_block_type == JustType::JustReal && valueType != JustType::JustNumber) {
                                throw std::runtime_error("Multi type is found.");
                            }

                            switch (current_block_type) {
                            case JustType::JustString: {
                                /*auto ref = (jstring*)memory;
                                    ((std::vector<jstring>*)prototype_node.handle)->emplace_back(*ref);
                                    jfree(ref);*/
                                break;
                            }
                            case JustType::JustBoolean: {
                                //                                    auto ref = (jbool*)memory;
                                //                                    ((std::vector<jbool>*)prototype_node.handle)->emplace_back(*ref);
                                //                                    jfree(ref);
                                break;
                            }
                            case JustType::JustReal: {
                                //                                    jreal* ref;
                                //                                    if (valueType == JustType::JustNumber) {
                                //                                        ref = new jreal(static_cast<jreal>(*(jnumber*)memory));
                                //                                        jfree((jnumber*)pointer);
                                //                                    } else
                                //                                        ref = (jreal*)memory;

                                //                                    ((std::vector<jreal>*)prototype_node.handle)->emplace_back(*ref);
                                //                                    jfree(ref);
                                break;
                            }
                            case JustType::JustNumber: {
                                //                                    auto ref = (jnumber*)memory;
                                //                                    ((std::vector<jnumber>*)prototype_node.handle)->emplace_back(*ref);
                                //                                    jfree(ref);
                                break;
                            }
                            }
                        }
                    }
                    x += just_autoskip_comment(pointer + x, y - x);
                }

                // BUG: Set node flags
                // prototype_node.flags |= (current_block_type) << 2;
            } else { // get the next node
                auto _tree = just_storage_alloc_tree(storage);

                ++x;
                x += bug_just_avail(storage, eval, pointer + x, length - x, depth + 1);
                // prototype_node.flags = Node_StructFlag;

                // BUG: set node memory pointer (handle)
                //  prototype_node.set_native_memory(_nodes);

                x += just_autoskip_comment(pointer + x, length - x);
            }
            if (pointer[x] != just_syntax.just_block_segments[1]) {
                throw std::bad_exception();
            }
            ++x;
        } else { // get also value
            x += just_get_format(pointer + x, storage, valueType);

            // BUG: prototype set memory pointer
            // prototype_node.set_native_memory(memory);

            // BUG: memory ?
            // memory = nullptr;
            //  prototype_node.flags = Node_ValueFlag | valueType << 2;
        }

        // BUG: Insert node child
        // entry.insert(std::make_pair(y = just_string_to_hash_fast(prototype_node.propertyName.c_str()), prototype_node));

        x += just_autoskip_comment(pointer + x, length - x);
    }

#undef prototype_node // undefine a macro name
#undef push
#undef pop

    return x;
}

just_object_parser::just_object_parser()
    : just_object_parser::just_object_parser(JustAllocationMethod::dynamic_allocation)
{
}

just_object_parser::just_object_parser(JustAllocationMethod allocationMethod)
    : _storage(nullptr)
{
    // TODO: param allocationMethod is support feature
}

just_object_parser::~just_object_parser() { }

method void just_object_parser::deserialize_from(const jstring& filename)
{
    long length;
    char* buffer;
    std::ifstream file;

    // try open file
    file.open(filename);

    // has error from open
    if (!file)
        throw std::runtime_error("error open file");

    // read length
    length = file.seekg(0, std::ios::end).tellg();
    file.seekg(0, std::ios::beg);

    // check buffer
    if ((buffer = (char*)malloc(length)) == nullptr)
        throw std::bad_alloc();

    // read
    length = file.readsome(buffer, length);
    // close file
    file.close();

    // deserialize
    deserialize((char*)buffer, length);
    // free buffer
    free(buffer);
}
method void just_object_parser::deserialize(const jstring& source) { deserialize(source.data(), source.size()); }

method void just_object_parser::deserialize(const char* source, int len)
{
    // FIXME: CLEAR FUNCTION IS UPGRADE
    just_stats eval = {};
    entry.clear(); // clears alls

    if (_storage) {
        std::free(_storage);
    }

    // init storage
    _storage = just_storage_new_init();
    just_avail_only(eval, source, len);
}
method jstring just_object_parser::serialize(JustSerializeFormat format) const
{
    jstring data;
    throw std::runtime_error("no complete");

    if (format == JustSerializeFormat::JustBeautify) {
        data += "//@Just Node Object Version: 1.0.0\n";
    }

    // jstruct* entry;

    // Write structure block

    // TODO: Go here write (serialize)

    return data;
}
method just_object_node* just_object_parser::search(const jstring& pattern)
{
    just_object_node* node;
    int hash = just_string_to_hash_fast(pattern.c_str());

    auto iter = this->entry.find(hash);

    if (iter != std::end(this->entry))
        node = &iter->second;
    else
        node = nullptr;
    return node;
}

method just_object_node* just_object_parser::tree(const jstring& nodename) { return at(nodename); }

method just_object_node* just_object_parser::at(const jstring& nodePath)
{
    just_object_node* node = nullptr;
    std::add_pointer<decltype(this->entry)>::type entry = &this->entry;
    decltype(entry->begin()) iter;
    int alpha = 0, beta;
    int hash;
    // get splits
    do {
        if ((beta = nodePath.find(just_syntax.just_tree_pathbrk, alpha)) == ~0)
            beta = static_cast<int>(nodePath.length());
        hash = just_string_to_hash_fast(nodePath.c_str() + alpha, beta - alpha);
        iter = entry->find(hash);
        if (iter != end(*entry)) {
            if (iter->second.has_tree())
                entry = nullptr; // decltype(entry)(iter->second._bits);
            else {
                if (beta == nodePath.length())
                    node = &iter->second; // get the next section
                break;
            }
        }
        alpha = ++beta;
        beta = nodePath.length();

    } while (alpha < beta);
    return node;
}

method jbool just_object_parser::contains(const jstring& nodePath) { return at(nodePath) != nullptr; }

method just_object_node& operator<<(just_object_node& root, const jstring& nodename) { return *root.tree(nodename); }

method just_object_node& operator<<(just_object_parser& root, const jstring& nodename) { return *root.tree(nodename); }

method std::ostream& operator<<(std::ostream& out, const just_object_node& node)
{
    switch (node.type()) {
    case JustType::JustNumber:
        out << static_cast<jnumber>(node);
        break;
    case JustType::JustBoolean:
        out << static_cast<jbool>(node);
        break;
    case JustType::JustReal:
        out << static_cast<jreal>(node);
        break;
    case JustType::JustString:
        out << static_cast<jstring>(node);
        break;
    case JustType::Unknown:
        out << just_syntax.just_unknown_string;
        break;
    case JustType::Null:
        out << just_syntax.just_null_string;
        break;
    }

    return out;
}

method std::ostream& operator<<(std::ostream& out, const just_object_parser& parser)
{
    if (false)
        out << just_syntax.just_unknown_string;
    else
        out << parser.serialize();
    return out;
}

} // namespace just

#undef method
#undef MACROPUT
