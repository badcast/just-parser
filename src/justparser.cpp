/*
 * ** Just Object Parser ***
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
#include <functional>
#include <sstream>

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

    // Внутреннее хранилище памяти (Storage manager)
    struct just_storage {
        std::vector<void*> raw_blocks;
        std::vector<jtree_t*> trees;
        std::vector<jstruct*> structs;
        std::vector<std::function<void()>> destructors;
        size_t total_allocated = 0;

        ~just_storage() {
            for (auto& dtor : destructors) {
                dtor();
            }
            for (void* blk : raw_blocks) {
                std::free(blk);
            }
            for (jtree_t* t : trees) {
                delete t;
            }
            for (jstruct* s : structs) {
                delete s;
            }
        }
    };

    template <typename T>
    method inline T* jalloc(just_storage* storage = nullptr) {
        T* obj = new T();
        if (storage) {
            storage->total_allocated += sizeof(T);
            storage->destructors.push_back([obj]() { delete obj; });
        }
        return obj;
    }

    template <typename T>
    method inline void jfree(T* ptr) {
        delete ptr;
    }

    method inline just_storage* just_storage_new_init() {
        return new just_storage();
    }

    method inline void just_storage_free(just_storage* storage) {
        delete storage;
    }

    method inline void* just_storage_alloc_field(just_storage** storage, JustType type, size_t custom_size = 0) {
        size_t sz = (type == JustType::JustString) ? (custom_size + 1) : static_cast<size_t>(just_type_size(type));
        if (sz == 0) sz = 8;
        void* mem = std::calloc(1, sz);
        if (storage && *storage) {
            (*storage)->raw_blocks.push_back(mem);
            (*storage)->total_allocated += sz;
        }
        return mem;
    }

    method inline jtree_t* just_storage_alloc_tree(just_storage** storage, jtree_t* owner = nullptr) {
        jtree_t* t = new jtree_t();
        if (storage && *storage) {
            (*storage)->trees.push_back(t);
            (*storage)->total_allocated += sizeof(jtree_t);
        }
        if (owner) {
            owner->push_back(static_cast<int>(reinterpret_cast<std::intptr_t>(t)));
        }
        return t;
    }

    method inline jstruct* just_storage_alloc_struct(just_storage** storage) {
        jstruct* s = new jstruct();
        if (storage && *storage) {
            (*storage)->structs.push_back(s);
            (*storage)->total_allocated += sizeof(jstruct);
        }
        return s;
    }

    method inline JustType just_storage_get_type(just_storage* /*storage*/, const jstring& /*name*/) {
        return JustType::Unknown;
    }

    /*parser*/
    method inline int just_string_to_hash_fast(const char* char_side, int contentLength);
    method inline bool just_is_unsigned_jnumber(const char char_side);
    method inline bool just_is_jnumber(const char* char_side, int* getLength);
    method jbool just_is_jreal(const char* char_side, int* getLength);
    method inline jbool just_is_jbool(const char* char_side, int* getLength);
    method int just_get_format(const char* char_side, just_storage** storage, JustType& containType, jvariant* outValue = nullptr);
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
    method int bug_just_avail(just_object_parser* parser, jstruct* current_entry, just_storage** storage, just_stats& eval, const char* source, int length, int depth);

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
            default:
                break;
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
        unsigned int x = 2166136261u;
        int y = 0;
        while (*(char_side) && y++ < contentLength) {
            x ^= static_cast<unsigned char>(*(char_side++));
            x *= 16777619u;
        }
        return static_cast<int>(x);
    }

    // method for check valid a unsigned number
    method inline bool just_is_unsigned_jnumber(const char char_side) { return std::isdigit(static_cast<unsigned char>(char_side)); }

    // method for check valid a signed number
    method inline bool just_is_jnumber(const char* char_side, int* getLength)
    {
        int x = 0;
        if (*char_side == just_syntax.just_negative_sym || *char_side == just_syntax.just_positive_sym) {
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
        bool has_digits = false;
        int len = 0;

        if (char_side[len] == just_syntax.just_negative_sym || char_side[len] == just_syntax.just_positive_sym) {
            ++len;
        }

        for (;;) {
            if (!just_is_unsigned_jnumber(char_side[len])) {
                if (real)
                    break;

                if (char_side[len] == just_syntax.just_dot && has_digits) {
                    real = true;
                } else {
                    break;
                }
            } else {
                has_digits = true;
            }
            ++len;
        }

        if (real && getLength) {
            *getLength = len;
        }
        return real;
    }

    // method for check is bool ?
    method inline jbool just_is_jbool(const char* char_side, int* getLength)
    {
        const int true_length = sizeof(just_syntax.just_true_string) - 1;
        const int false_length = sizeof(just_syntax.just_false_string) - 1;
        if (!std::strncmp(char_side, just_syntax.just_true_string, true_length)) {
            *getLength = true_length;
            return true;
        }
        if (!std::strncmp(char_side, just_syntax.just_false_string, false_length)) {
            *getLength = false_length;
            return true;
        }
        *getLength = 0;
        return false;
    }

    // method for get format from raw content, also to write in storage pointer
    method int just_get_format(const char* char_side, just_storage** storage, JustType& containType, jvariant* outValue)
    {
        /*
         * Priority:
         *  - JReal
         *  - JNumber
         *  - JBool
         *  - JString
         */

        void* mem = nullptr;
        int offset = 0;

        // Null type
        if (char_side == nullptr || *char_side == '\0') {
            containType = JustType::Null;
        } else if (just_is_jreal(char_side, &offset)) { // Real type -----------------------------------------------------------------------------
            containType = JustType::JustReal;
            if (storage) {
                jreal conv = std::strtod(char_side, nullptr); // use "C++" method
                // Copy to
                mem = std::memcpy(just_storage_alloc_field(storage, containType), &conv, just_type_size(containType));
                if (outValue)
                    *outValue = mem;
            }
        } else if (just_is_jnumber(char_side, &offset)) { // Number type ---------------------------------------------------------------------------------
            jnumber conv = 0;
            containType = JustType::JustNumber;
            long long tmp = 0;
            sscanf(char_side, "%lld", &tmp); // use "C" method
            conv = static_cast<jnumber>(tmp);
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
            containType = JustType::JustString;
            jstring stringSyntax;
            ++offset;
            while (char_side[offset] && char_side[offset] != just_syntax.just_format_string) {
                char ch = char_side[offset++];
                if (ch == just_syntax.just_left_seperator) {
                    if (!char_side[offset])
                        throw std::bad_exception();
                    ch = char_side[offset++];
                }
                if (storage)
                    stringSyntax.push_back(ch);
            }
            if (char_side[offset] != just_syntax.just_format_string)
                throw std::bad_exception();
            ++offset;
            if (storage) {
                void* field = just_storage_alloc_field(storage, containType, stringSyntax.size());
                mem = std::memcpy(field, stringSyntax.data(), stringSyntax.size());
                static_cast<char*>(mem)[stringSyntax.size()] = '\0';
                if (outValue)
                    *outValue = mem;
            }
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
        std::memcpy(skipping, just_syntax.just_trim_segments, sizeof(just_syntax.just_trim_segments));
        std::memcpy(skipping + sizeof(just_syntax.just_trim_segments), just_syntax.just_block_segments, sizeof(just_syntax.just_block_segments));
        for (x = 0; x < length && char_side[x];) {
            for (y = 0; y < static_cast<int>(sizeof(skipping));)
                if (char_side[x] == skipping[y])
                    break;
            else
                ++y;
            if (y == static_cast<int>(sizeof(skipping)))
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
        if (len <= 0) return 0;
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

    just_object_node::just_object_node()
    : _jowner(nullptr), jname(""), flags(0), handle(nullptr)
    {
        judata.v_variant = nullptr;
    }

    just_object_node::just_object_node(just_object_parser* owner, void* handle_ptr)
    : _jowner(owner), flags(0), handle(handle_ptr)
    {
        judata.v_variant = handle_ptr;
        if (handle_ptr) {
            this->jname = static_cast<const char*>(handle_ptr);
        }
    }

    method JustType just_object_node::type() const
    {
        int base_flag = flags & 0x3;
        if (base_flag == Node_TreeFlag) {
            return JustType::JustTree;
        }
        if (base_flag == Node_ValueFlag || base_flag == Node_ArrayFlag) {
            return static_cast<JustType>(flags >> 2);
        }
        if (_jowner && _jowner->_storage) {
            return just_storage_get_type(static_cast<just_storage*>(this->_jowner->_storage), jname);
        }
        return JustType::Unknown;
    }

    method just_object_node* just_object_node::tree(const jstring& child)
    {
        if (!has_tree() || !handle)
            return nullptr;

        jstruct* sub_entry = static_cast<jstruct*>(handle);
        int alpha = 0, beta;
        just_object_node* node = nullptr;

        do {
            size_t pos = child.find(just_syntax.just_tree_pathbrk, alpha);
            beta = (pos == jstring::npos) ? static_cast<int>(child.length()) : static_cast<int>(pos);
            int hash = just_string_to_hash_fast(child.c_str() + alpha, beta - alpha);
            auto iter = sub_entry->find(hash);
            if (iter != sub_entry->end()) {
                if (beta == static_cast<int>(child.length())) {
                    node = &iter->second;
                    break;
                } else if (iter->second.has_tree()) {
                    sub_entry = static_cast<jstruct*>(iter->second.handle);
                } else {
                    break;
                }
            } else {
                break;
            }
            alpha = beta + 1;
        } while (alpha < static_cast<int>(child.length()));

        return node;
    }

    method jbool just_object_node::has_tree() const
    {
        return (flags & 0x3) == Node_TreeFlag && handle != nullptr;
    }

    method jbool just_object_node::has_value() const
    {
        int base_flag = flags & 0x3;
        return (base_flag == Node_ValueFlag || base_flag == Node_ArrayFlag) && handle != nullptr;
    }

    method jbool just_object_node::is_array() const
    {
        return (flags & 0x3) == Node_ArrayFlag;
    }

    method jnumber just_object_node::get_int() const
    {
        if (type() == JustType::JustNumber && handle && !is_array())
            return *static_cast<jnumber*>(handle);
        if (type() == JustType::JustReal && handle && !is_array())
            return static_cast<jnumber>(*static_cast<jreal*>(handle));
        return 0;
    }

    method jbool just_object_node::get_bool() const
    {
        if (type() == JustType::JustBoolean && handle && !is_array())
            return *static_cast<jbool*>(handle);
        return false;
    }

    method jreal just_object_node::get_real() const
    {
        if (type() == JustType::JustReal && handle && !is_array())
            return *static_cast<jreal*>(handle);
        if (type() == JustType::JustNumber && handle && !is_array())
            return static_cast<jreal>(*static_cast<jnumber*>(handle));
        return 0.0;
    }

    method jstring just_object_node::get_str() const
    {
        if (type() == JustType::JustString && handle && !is_array())
            return jstring(static_cast<const char*>(handle));
        return {};
    }

    static jstring just_escape_string(const jstring& value)
    {
        jstring escaped;
        escaped.reserve(value.size());
        for (char ch : value) {
            if (ch == '\\' || ch == just_syntax.just_format_string)
                escaped.push_back(just_syntax.just_left_seperator);
            escaped.push_back(ch);
        }
        return escaped;
    }

    jstring just_object_node::to_string() const
    {
        if (is_array() && handle) {
            std::ostringstream oss;
            oss << "{ ";
            switch (type()) {
                case JustType::JustString: {
                    const auto& vec = *static_cast<std::vector<jstring>*>(handle);
                    for (size_t i = 0; i < vec.size(); ++i)
                        oss << (i ? ", " : "") << "\"" << just_escape_string(vec[i]) << "\"";
                    break;
                }
                case JustType::JustNumber: {
                    const auto& vec = *static_cast<std::vector<jnumber>*>(handle);
                    for (size_t i = 0; i < vec.size(); ++i)
                        oss << (i ? ", " : "") << vec[i];
                    break;
                }
                case JustType::JustReal: {
                    const auto& vec = *static_cast<std::vector<jreal>*>(handle);
                    for (size_t i = 0; i < vec.size(); ++i)
                        oss << (i ? ", " : "") << vec[i];
                    break;
                }
                case JustType::JustBoolean: {
                    const auto& vec = *static_cast<std::vector<jbool>*>(handle);
                    for (size_t i = 0; i < vec.size(); ++i)
                        oss << (i ? ", " : "") << (vec[i] ? "true" : "false");
                    break;
                }
                default:
                    break;
            }
            oss << " }";
            return oss.str();
        }

        switch (type()) {
            case JustType::JustString:
                return get_str();
            case JustType::JustNumber:
                return std::to_string(get_int());
            case JustType::JustBoolean:
                return get_bool() ? jstring(just_syntax.just_true_string) : jstring(just_syntax.just_false_string);
            case JustType::JustReal:
                return std::to_string(get_real());
            case JustType::JustTree:
                return jstring(just_syntax.just_unknown_string);
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

    method const jstring just_object_node::name() const { return jname; }

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
            if (pointer + x >= epointer)
                break;

            if (depth > 0 && pointer[x] == just_syntax.just_block_segments[1]) {
                // TODO: Next Depth (Up)
                ++x;
                --depth;
                if (tree.size() > 1)
                    tree.pop_back();
                x += just_autoskip_comment(pointer + x, length - x);
                if (pointer + x < epointer && pointer[x] == just_syntax.just_obstacle) {
                    ++x;
                }
                continue;
            }
            x += just_skip(pointer + x, length - x);

            // check property name
            if (!just_valid_property_name(pointer + y, x - y))
                throw std::bad_exception();

            // property name
            // prototype_node.propertyName.append(just_source + y, static_cast<size_t>(x - y));

            // std::cout << jstring(pointer + y, static_cast<size_t>(x - y)) << std::endl;

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
                        if (x >= y) break;
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
                                        default:
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
                                    default:
                                        break;
                                }
                            }
                        }
                        x += just_autoskip_comment(pointer + x, y - x);
                    }
                    if (pointer[x] == just_syntax.just_block_segments[1])
                        ++x;
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
                        jstat.jarrstrings_total_bytes += z - 1;
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
                    default:
                        break;
                }

                // prototype_node.set_native_memory(pointer);
                // pointer = nullptr;
                // prototype_node.flags = Node_ValueFlag | valueType << 2;
            }

            // entry.insert(std::make_pair(y = just_string_to_hash(prototype_node.propertyName.c_str()), prototype_node));

            /*
             # if* defined(QDEBUG) || defined(DEBUG)
             // get iter
             auto _curIter = entry.find(j);
             if (_dbgLastNode) _dbgLastNode->nextNode = &_curIter->second;
             _curIter->second.prevNode = _dbgLastNode;
            _dbgLastNode = &_curIter->second;
            #endif
            */
            x += just_autoskip_comment(pointer + x, length - x);
            if (pointer + x < epointer && pointer[x] == just_syntax.just_obstacle) {
                ++x;
                x += just_autoskip_comment(pointer + x, length - x);
            }
        }

        just_storage_free(pstorage);
    }

    method void just_avail(just_storage** /*storage*/, just_stats& jstat, const char* source, int length)
    {
        just_avail_only(jstat, source, length);
    }

    // big algorithm, please free me.
    // divide and conquer method avail
    method int bug_just_avail(just_object_parser* parser, jstruct* current_entry, just_storage** storage, just_stats& eval, const char* source, int length, int depth)
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
            if (x >= length)
                break;
            if (depth > 0 && pointer[x] == just_syntax.just_block_segments[1])
                break;
            x += just_skip(pointer + x, length - x);

            // Preparing, check property name
            if (!just_valid_property_name(pointer + y, x - y))
                throw std::bad_exception();

            // BUG: Set property name
            // prototype_node.propertyName.append(pointer + y, static_cast<size_t>(x - y)); // set property name
            just_object_node node_item;
            node_item._jowner = parser;
            node_item.jname.assign(pointer + y, static_cast<size_t>(x - y));

            // has comment line
            x += just_autoskip_comment(pointer + x, length - x);
            x += just_trim(pointer + x, length - x); // trim string
            // is block or array
            if (pointer[x] == *just_syntax.just_block_segments) {
                if (just_is_array(pointer + x, y, length - x)) {
                    y += x++;
                    current_block_type = JustType::Unknown;
                    node_item.flags = Node_ArrayFlag;

                    // While end of array length
                    while (x < y) {
                        x += just_autoskip_comment(pointer + x, y - x);
                        if (x >= y)
                            break;
                        // next index
                        if (pointer[x] == just_syntax.just_obstacle)
                            ++x;
                        else {
                            jvariant pvalue = nullptr;
                            x += just_get_format(pointer + x, storage, valueType, &pvalue);
                            if (valueType != JustType::Unknown) {
                                if (current_block_type == JustType::Unknown) {
                                    current_block_type = valueType;

                                    switch (current_block_type) {
                                        case JustType::JustBoolean:
                                            node_item.set_native_memory((void*)jalloc<std::vector<jbool>>(*storage));
                                            ++eval.jarrbools;
                                            break;
                                        case JustType::JustNumber:
                                            node_item.set_native_memory((void*)jalloc<std::vector<jnumber>>(*storage));
                                            ++eval.jarrnumbers;
                                            break;
                                        case JustType::JustReal:
                                            node_item.set_native_memory((void*)jalloc<std::vector<jreal>>(*storage));
                                            ++eval.jarrreals;
                                            break;
                                        case JustType::JustString:
                                            node_item.set_native_memory((void*)jalloc<std::vector<jstring>>(*storage));
                                            ++eval.jarrstrings;
                                            break;
                                        default:
                                            break;
                                    }

                                } else if (current_block_type != valueType) {
                                    // convert just numbers as JustReal (для секции mix { 1, 2.0, 3 })
                                    if (current_block_type == JustType::JustNumber && valueType == JustType::JustReal) {
                                        auto* old_vec = static_cast<std::vector<jnumber>*>(node_item.handle);
                                        auto* new_vec = jalloc<std::vector<jreal>>(*storage);
                                        for (jnumber v : *old_vec) {
                                            new_vec->push_back(static_cast<jreal>(v));
                                        }
                                        node_item.set_native_memory((void*)new_vec);
                                        current_block_type = JustType::JustReal;
                                        --eval.jarrnumbers;
                                        ++eval.jarrreals;
                                    } else if (current_block_type == JustType::JustReal && valueType == JustType::JustNumber) {
                                        // Допустимо: целое число внутри массива вещественных чисел
                                    } else {
                                        throw std::runtime_error("Multi type is found.");
                                    }
                                }

                                switch (current_block_type) {
                                    case JustType::JustString: {
                                        auto ref = static_cast<const char*>(pvalue);
                                        ((std::vector<jstring>*)node_item.handle)->emplace_back(ref ? ref : "");
                                        break;
                                    }
                                    case JustType::JustBoolean: {
                                        auto ref = static_cast<jbool*>(pvalue);
                                        ((std::vector<jbool>*)node_item.handle)->emplace_back(*ref);
                                        break;
                                    }
                                    case JustType::JustReal: {
                                        jreal val = 0.0;
                                        if (valueType == JustType::JustNumber) {
                                            val = static_cast<jreal>(*static_cast<jnumber*>(pvalue));
                                        } else {
                                            val = *static_cast<jreal*>(pvalue);
                                        }
                                        ((std::vector<jreal>*)node_item.handle)->emplace_back(val);
                                        break;
                                    }
                                    case JustType::JustNumber: {
                                        auto ref = static_cast<jnumber*>(pvalue);
                                        ((std::vector<jnumber>*)node_item.handle)->emplace_back(*ref);
                                        break;
                                    }
                                    default:
                                        break;
                                }
                            }
                        }
                        x += just_autoskip_comment(pointer + x, y - x);
                    }

                    // BUG: Set node flags
                    node_item.flags |= static_cast<int>(current_block_type) << 2;
                } else { // get the next node
                    auto _nodes = just_storage_alloc_struct(storage);

                    ++x;
                    x += bug_just_avail(parser, _nodes, storage, eval, pointer + x, length - x, depth + 1);
                    node_item.flags = Node_TreeFlag | (static_cast<int>(JustType::JustTree) << 2);

                    // BUG: set node memory pointer (handle)
                    node_item.set_native_memory(_nodes);

                    x += just_autoskip_comment(pointer + x, length - x);
                }
                if (pointer[x] != just_syntax.just_block_segments[1]) {
                    throw std::bad_exception();
                }
                ++x;
            } else { // get also value
                jvariant memory = nullptr;
                x += just_get_format(pointer + x, storage, valueType, &memory);

                // BUG: prototype set memory pointer
                node_item.set_native_memory(memory);

                // BUG: memory ?
                memory = nullptr;
                node_item.flags = Node_ValueFlag | (static_cast<int>(valueType) << 2);
            }

            // BUG: Insert node child
            int hash = just_string_to_hash_fast(node_item.jname.c_str(), static_cast<int>(node_item.jname.size()));
            (*current_entry)[hash] = node_item;

            x += just_autoskip_comment(pointer + x, length - x);
            // Пропуск запятой между блоками (например: human1 { ... }, human2 { ... })
            if (x < length && pointer[x] == just_syntax.just_obstacle) {
                ++x;
                x += just_autoskip_comment(pointer + x, length - x);
            }
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

    just_object_parser::just_object_parser(JustAllocationMethod /*allocationMethod*/)
    : _storage(nullptr)
    {
        // TODO: param allocationMethod is support feature
    }

    just_object_parser::~just_object_parser()
    {
        if (_storage) {
            just_storage_free(static_cast<just_storage*>(_storage));
            _storage = nullptr;
        }
    }

    method void just_object_parser::deserialize_from(const jstring& filename)
    {
        long length;
        std::ifstream file;

        // try open file
        file.open(filename, std::ios::in);

        // has error from open
        if (!file)
            throw std::runtime_error("error open file");

        // read length
        length = file.seekg(0, std::ios::end).tellg();
        file.seekg(0, std::ios::beg);

        if (length < 0)
            throw std::runtime_error("error reading file length");
        std::vector<char> buffer(static_cast<size_t>(length) + 1);

        // read
        file.read(buffer.data(), length);
        length = file.gcount();
        buffer[static_cast<size_t>(length)] = '\0';
        // close file
        file.close();

        // deserialize
        deserialize(buffer.data(), static_cast<int>(length));
    }

    method void just_object_parser::deserialize(const jstring& source) { deserialize(source.data(), static_cast<int>(source.size())); }

    method void just_object_parser::deserialize(const jstring& content, int /*depth*/) { deserialize(content.data(), static_cast<int>(content.size())); }

    method void just_object_parser::deserialize(const char* source, int len)
    {
        if (len < 0 || (!source && len > 0))
            throw std::invalid_argument("invalid source buffer");
        const char* input = source ? source : "";
        jstring terminated_source(input, static_cast<size_t>(len));

        // FIXME: CLEAR FUNCTION IS UPGRADE
        just_stats eval = {};
        entry.clear(); // clears alls

        if (_storage) {
            just_storage_free(static_cast<just_storage*>(_storage));
        }

        // init storage
        just_storage* pstore = just_storage_new_init();
        _storage = pstore;
        bug_just_avail(this, &this->entry, &pstore, eval, terminated_source.data(), len, 0);
    }

    method jstring just_object_parser::serialize(JustSerializeFormat format) const
    {
        jstring data;

        if (format == JustSerializeFormat::JustBeautify) {
            data += "//@Just Node Object Version: 1.0.0\n";
        }

        std::function<void(const jstruct&, int)> dump_struct = [&](const jstruct& current_map, int depth) {
            jstring indent = (format == JustSerializeFormat::JustBeautify) ? jstring(depth * 4, ' ') : "";
            jstring nl = (format == JustSerializeFormat::JustBeautify) ? "\n" : " ";
            for (const auto& kv : current_map) {
                const just_object_node& n = kv.second;
                data += indent + n.name();
                if (n.has_tree()) {
                    data += (format == JustSerializeFormat::JustBeautify) ? " {\n" : "{";
                    const jstruct* sub = static_cast<const jstruct*>(n.handle);
                    if (sub) {
                        dump_struct(*sub, depth + 1);
                    }
                    data += indent + "}" + nl;
                } else if (n.is_array()) {
                    data += " " + n.to_string() + nl;
                } else {
                    data += (format == JustSerializeFormat::JustBeautify) ? " " : " ";
                    if (n.type() == JustType::JustString)
                        data += "\"" + just_escape_string(n.to_string()) + "\"" + nl;
                    else
                        data += n.to_string() + nl;
                }
            }
        };

        dump_struct(this->entry, 0);
        return data;
    }

    method just_object_node* just_object_parser::search(const jstring& pattern)
    {
        int hash = just_string_to_hash_fast(pattern.c_str(), static_cast<int>(pattern.size()));

        std::function<just_object_node*(jstruct&)> search_recursive = [&](jstruct& cur) -> just_object_node* {
            auto iter = cur.find(hash);
            if (iter != cur.end() && iter->second.name() == pattern)
                return &iter->second;
            for (auto& kv : cur) {
                if (kv.second.has_tree() && kv.second.handle) {
                    if (auto* found = search_recursive(*static_cast<jstruct*>(kv.second.handle)))
                        return found;
                }
            }
            return nullptr;
        };

        return search_recursive(this->entry);
    }

    method just_object_node* just_object_parser::tree(const jstring& nodename) { return at(nodename); }

    method just_object_node* just_object_parser::at(const jstring& nodePath)
    {
        just_object_node* node = nullptr;
        std::add_pointer<decltype(this->entry)>::type cur_entry = &this->entry;
        decltype(cur_entry->begin()) iter;
        int alpha = 0, beta;
        int hash;
        int path_len = static_cast<int>(nodePath.length());

        // get splits
        do {
            size_t pos = nodePath.find(just_syntax.just_tree_pathbrk, alpha);
            beta = (pos == jstring::npos) ? path_len : static_cast<int>(pos);
            hash = just_string_to_hash_fast(nodePath.c_str() + alpha, beta - alpha);
            iter = cur_entry->find(hash);
            if (iter != end(*cur_entry)) {
                if (beta == path_len) {
                    node = &iter->second; // get the target section
                    break;
                } else if (iter->second.has_tree()) {
                    cur_entry = static_cast<jstruct*>(iter->second.handle);
                } else {
                    break;
                }
            } else {
                break;
            }
            alpha = beta + 1;
        } while (alpha < path_len && cur_entry != nullptr);

        return node;
    }

    method jbool just_object_parser::contains(const jstring& nodePath) { return at(nodePath) != nullptr; }

    method jnumber just_object_parser::occupied_memory()
    {
        if (!_storage) return 0;
        return static_cast<jnumber>(static_cast<just_storage*>(_storage)->total_allocated);
    }

    method int just_object_parser::treeCount() const
    {
        return static_cast<int>(this->entry.size());
    }

    method just_object_node& operator<<(just_object_node& root, const jstring& nodename)
    {
        just_object_node* child = root.tree(nodename);
        if (!child)
            throw std::runtime_error("Child node not found: " + nodename);
        return *child;
    }

    method just_object_node& operator<<(just_object_parser& root, const jstring& nodename)
    {
        just_object_node* child = root.tree(nodename);
        if (!child)
            throw std::runtime_error("Root node not found: " + nodename);
        return *child;
    }

    method std::ostream& operator<<(std::ostream& out, const just_object_node& node)
    {
        if (node.is_array()) {
            out << node.to_string();
            return out;
        }

        switch (node.type()) {
            case JustType::JustNumber:
                out << static_cast<jnumber>(node);
                break;
            case JustType::JustBoolean:
                out << (static_cast<jbool>(node) ? "true" : "false");
                break;
            case JustType::JustReal:
                out << static_cast<jreal>(node);
                break;
            case JustType::JustString:
                out << static_cast<jstring>(node);
                break;
            case JustType::JustTree:
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
            out << parser.serialize(JustSerializeFormat::JustBeautify);
        return out;
    }

} // namespace just

#undef method
#undef MACROPUT
