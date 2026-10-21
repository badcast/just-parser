/*
 *** Just Object Parser — Extended Unit Tests ***
 *** File: justmoretest.cpp
 *** Build: g++ -std=c++17 justmoretest.cpp justparser.cpp -o justmoretest && ./justmoretest
 */

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <functional>

#include "justparser"

// ============================================================================
// Легковесный фреймворк для тестирования
// ============================================================================
namespace test_framework {

    static int total_tests = 0;
    static int passed_tests = 0;
    static int failed_tests = 0;

    #define COLOR_GREEN "\033[1;32m"
    #define COLOR_RED   "\033[1;31m"
    #define COLOR_CYAN  "\033[1;36m"
    #define COLOR_RESET "\033[0m"

    #define ASSERT_TRUE(cond) \
        do { \
            if (!(cond)) { \
                throw std::runtime_error(std::string("ASSERT_TRUE failed: ") + #cond + \
                    " (line " + std::to_string(__LINE__) + ")"); \
            } \
        } while (0)

    #define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))

    #define ASSERT_EQ(actual, expected) \
        do { \
            if (!((actual) == (expected))) { \
                std::ostringstream oss; \
                oss << "ASSERT_EQ failed: " << #actual << " != " << #expected \
                    << " (line " << __LINE__ << ")"; \
                throw std::runtime_error(oss.str()); \
            } \
        } while (0)

    #define ASSERT_NEAR(actual, expected, eps) \
        do { \
            if (std::fabs((actual) - (expected)) > (eps)) { \
                std::ostringstream oss; \
                oss << "ASSERT_NEAR failed: " << (actual) << " vs " << (expected) \
                    << " (line " << __LINE__ << ")"; \
                throw std::runtime_error(oss.str()); \
            } \
        } while (0)

    #define ASSERT_THROWS(expr, ExceptionType) \
        do { \
            bool caught = false; \
            try { \
                expr; \
            } catch (const ExceptionType&) { \
                caught = true; \
            } catch (...) { \
                throw std::runtime_error(std::string("ASSERT_THROWS caught wrong exception type for: ") + \
                    #expr + " (line " + std::to_string(__LINE__) + ")"); \
            } \
            if (!caught) { \
                throw std::runtime_error(std::string("ASSERT_THROWS expected exception, but none thrown: ") + \
                    #expr + " (line " + std::to_string(__LINE__) + ")"); \
            } \
        } while (0)

    void run_test(const std::string& name, const std::function<void()>& fn) {
        ++total_tests;
        std::cout << COLOR_CYAN << "[ RUN      ] " << COLOR_RESET << name << std::endl;
        try {
            fn();
            ++passed_tests;
            std::cout << COLOR_GREEN << "[       OK ] " << COLOR_RESET << name << std::endl;
        } catch (const std::exception& e) {
            ++failed_tests;
            std::cout << COLOR_RED << "[  FAILED  ] " << COLOR_RESET << name
                      << "\n             Reason: " << e.what() << std::endl;
        } catch (...) {
            ++failed_tests;
            std::cout << COLOR_RED << "[  FAILED  ] " << COLOR_RESET << name
                      << "\n             Reason: Unknown exception" << std::endl;
        }
    }

} // namespace test_framework

using namespace just;
using namespace test_framework;

// ============================================================================
// Исходные данные из syntax.just[cite: 1]
// ============================================================================
static const char* SYNTAX_JUST_SAMPLE = R"(
//Next just_tree_struct
struct_tree{
    humans{
        human1
        {
            id      0
            name    "Alex"
            age     21
            from    "Earth"
        },
        human2
        {
            id      1
            name    "Jessy"
            age     19
            from    "Earth"
        }
    }
    //From ID component
    students {
        0, 1
    }
}

//Next just_tree_array (Single Type)
strings  
{
    "This is text 1"
    "This is text 2"
    "This is text 3"
}

numbers
{
    //There numbers
    1
    2
    3
}

reals
{
    //There floating points
    1.0
    2.0
    3.33333333333333333
}

bools
{
    //There bools
    false
    true
}

mix
{
    //There mix memeber (number and next floating point)
    1
    2.0
    3
}
)";

// ============================================================================
// Тесты
// ============================================================================

// 1. Полный тест оригинального syntax.just[cite: 1]
void test_syntax_just_full() {
    just_object_parser parser;
    parser.deserialize(SYNTAX_JUST_SAMPLE);

    ASSERT_EQ(parser.treeCount(), 6);
    ASSERT_TRUE(parser.contains("struct_tree/humans/human1/name"));
    ASSERT_TRUE(parser.contains("struct_tree/humans/human2/age"));
    ASSERT_TRUE(parser.contains("struct_tree/students"));
    ASSERT_FALSE(parser.contains("struct_tree/humans/human3"));

    // Проверка human1[cite: 1]
    auto* h1_id   = parser.at("struct_tree/humans/human1/id");
    auto* h1_name = parser.at("struct_tree/humans/human1/name");
    auto* h1_age  = parser.at("struct_tree/humans/human1/age");
    auto* h1_from = parser.at("struct_tree/humans/human1/from");

    ASSERT_TRUE(h1_id && h1_name && h1_age && h1_from);
    ASSERT_EQ(h1_id->type(), JustType::JustNumber);
    ASSERT_EQ(h1_id->value<int>(), 0);
    ASSERT_EQ(h1_name->type(), JustType::JustString);
    ASSERT_EQ(h1_name->value<jstring>(), "Alex");
    ASSERT_EQ(h1_age->value<jnumber>(), 21);
    ASSERT_EQ(static_cast<jstring>(*h1_from), "Earth");

    // Проверка human2 через оператор <<[cite: 1, 2, 3]
    just_object_node& h2 = parser << "struct_tree" << "humans" << "human2";
    ASSERT_TRUE(h2.has_tree());
    ASSERT_EQ((h2 << "id").value<int>(), 1);
    ASSERT_EQ((h2 << "name").value<jstring>(), "Jessy");
    ASSERT_EQ((h2 << "age").value<int>(), 19);
    ASSERT_EQ((h2 << "from").value<jstring>(), "Earth");
}

// 2. Тест всех типов массивов и автоконвертации mix (JustNumber -> JustReal)[cite: 1, 2]
void test_arrays_and_mix_promotion() {
    just_object_parser parser;
    parser.deserialize(SYNTAX_JUST_SAMPLE);

    // Массив с разделителями-запятыми: students { 0, 1 }[cite: 1]
    auto* students = parser.at("struct_tree/students");
    ASSERT_TRUE(students != nullptr);
    ASSERT_TRUE(students->is_array());
    ASSERT_EQ(students->type(), JustType::JustNumber);
    const auto& st_vec = students->get_array<jnumber>();
    ASSERT_EQ(st_vec.size(), 2u);
    ASSERT_EQ(st_vec[0], 0);
    ASSERT_EQ(st_vec[1], 1);

    // Массив строк без запятых[cite: 1]
    auto* strings = parser.at("strings");
    ASSERT_TRUE(strings && strings->is_array());
    ASSERT_EQ(strings->type(), JustType::JustString);
    const auto& str_vec = strings->get_array<jstring>();
    ASSERT_EQ(str_vec.size(), 3u);
    ASSERT_EQ(str_vec[0], "This is text 1");
    ASSERT_EQ(str_vec[1], "This is text 2");
    ASSERT_EQ(str_vec[2], "This is text 3");

    // Массив целых чисел[cite: 1]
    const auto& num_vec = parser.at("numbers")->get_array<jnumber>();
    ASSERT_EQ(num_vec.size(), 3u);
    ASSERT_EQ(num_vec[0], 1);
    ASSERT_EQ(num_vec[1], 2);
    ASSERT_EQ(num_vec[2], 3);

    // Массив вещественных чисел[cite: 1]
    const auto& real_vec = parser.at("reals")->get_array<jreal>();
    ASSERT_EQ(real_vec.size(), 3u);
    ASSERT_NEAR(real_vec[0], 1.0, 1e-9);
    ASSERT_NEAR(real_vec[1], 2.0, 1e-9);
    ASSERT_NEAR(real_vec[2], 3.33333333333333333, 1e-15);

    // Массив булевых значений[cite: 1]
    const auto& bool_vec = parser.at("bools")->get_array<jbool>();
    ASSERT_EQ(bool_vec.size(), 2u);
    ASSERT_EQ(bool_vec[0], false);
    ASSERT_EQ(bool_vec[1], true);

    // Смешанный массив mix { 1, 2.0, 3 } -> должен повыситься до JustReal[cite: 1, 2]
    auto* mix = parser.at("mix");
    ASSERT_TRUE(mix && mix->is_array());
    ASSERT_EQ(mix->type(), JustType::JustReal);
    const auto& mix_vec = mix->get_array<jreal>();
    ASSERT_EQ(mix_vec.size(), 3u);
    ASSERT_NEAR(mix_vec[0], 1.0, 1e-9);
    ASSERT_NEAR(mix_vec[1], 2.0, 1e-9);
    ASSERT_NEAR(mix_vec[2], 3.0, 1e-9);
}

// 3. Тест знаковых чисел, экранированных кавычек и граничных примитивов[cite: 2]
void test_primitives_and_escaping() {
    const char* src = R"(
        neg_int     -9876543210
        pos_int     +42
        zero_int    0
        neg_real    -123.456
        pos_real    +0.005
        flag_true   true
        flag_false  false
        escaped_str "Hello \"Just\" World"
    )";

    just_object_parser parser;
    parser.deserialize(src);

    ASSERT_EQ(parser.at("neg_int")->value<jnumber>(), -9876543210LL);
    ASSERT_EQ(parser.at("pos_int")->value<jnumber>(), 42LL);
    ASSERT_EQ(parser.at("zero_int")->value<jnumber>(), 0LL);

    ASSERT_NEAR(parser.at("neg_real")->value<jreal>(), -123.456, 1e-6);
    ASSERT_NEAR(parser.at("pos_real")->value<jreal>(), 0.005, 1e-6);

    ASSERT_EQ(parser.at("flag_true")->value<bool>(), true);
    ASSERT_EQ(parser.at("flag_false")->value<bool>(), false);

    ASSERT_EQ(parser.at("escaped_str")->value<jstring>(), "Hello \"Just\" World");
}

// 4. Тест на отсутствие коллизий хеша для анаграмм (ab vs ba, id_12 vs id_21)[cite: 2]
void test_anagram_hash_collisions() {
    const char* src = R"(
        ab      10
        ba      20
        abc     30
        cba     40
        cab     50
        node_12 "first"
        node_21 "second"
    )";

    just_object_parser parser;
    parser.deserialize(src);

    ASSERT_EQ(parser.at("ab")->value<int>(), 10);
    ASSERT_EQ(parser.at("ba")->value<int>(), 20);
    ASSERT_EQ(parser.at("abc")->value<int>(), 30);
    ASSERT_EQ(parser.at("cba")->value<int>(), 40);
    ASSERT_EQ(parser.at("cab")->value<int>(), 50);
    ASSERT_EQ(parser.at("node_12")->value<jstring>(), "first");
    ASSERT_EQ(parser.at("node_21")->value<jstring>(), "second");
}

// 5. Тест глубокого поиска search() и приведения типов (operators)[cite: 2, 3]
void test_deep_search_and_operators() {
    const char* src = R"(
        level1 {
            level2 {
                level3 {
                    level4 {
                        secret_key "found_me_deep_inside"
                        secret_num 777
                    }
                }
            }
        }
    )";

    just_object_parser parser;
    parser.deserialize(src);

    just_object_node* found = parser.search("secret_key");
    ASSERT_TRUE(found != nullptr);
    ASSERT_EQ(found->name(), "secret_key");
    ASSERT_EQ(static_cast<jstring>(*found), "found_me_deep_inside");

    just_object_node* num_node = parser.search("secret_num");
    ASSERT_TRUE(num_node != nullptr);
    ASSERT_EQ(static_cast<jnumber>(*num_node), 777);
    ASSERT_EQ(num_node->to_string(), "777");

    ASSERT_TRUE(parser.search("non_existent_node") == nullptr);
}

// 6. Тест сериализации и повторного чтения (Round-Trip)[cite: 2, 3]
void test_serialize_roundtrip() {
    just_object_parser parser1;
    parser1.deserialize(SYNTAX_JUST_SAMPLE);

    jstring dumped_beautify = parser1.serialize(JustSerializeFormat::JustBeautify);
    ASSERT_FALSE(dumped_beautify.empty());

    // Десериализуем сериализованную строку во второй парсер
    just_object_parser parser2;
    parser2.deserialize(dumped_beautify);

    ASSERT_EQ(parser2.at("struct_tree/humans/human1/name")->value<jstring>(), "Alex");
    ASSERT_EQ(parser2.at("struct_tree/humans/human2/age")->value<int>(), 19);
    ASSERT_EQ(parser2.at("numbers")->get_array<jnumber>().size(), 3u);
    ASSERT_EQ(parser2.at("bools")->get_array<jbool>()[1], true);

    just_object_parser escaped_parser;
    escaped_parser.deserialize(R"(scalar "quote: \" and slash: \\" values { "x\"y", "p\\q" })");
    just_object_parser escaped_roundtrip;
    escaped_roundtrip.deserialize(escaped_parser.serialize(JustSerializeFormat::JustBeautify));
    ASSERT_EQ(escaped_roundtrip.at("scalar")->value<jstring>(), "quote: \" and slash: \\");
    const auto& escaped_values = escaped_roundtrip.at("values")->get_array<jstring>();
    ASSERT_EQ(escaped_values[0], "x\"y");
    ASSERT_EQ(escaped_values[1], "p\\q");
}

void test_non_terminated_input_buffer() {
    const char source[] = { 'f', 'l', 'a', 'g', ' ', 't', 'r', 'u', 'e' };
    just_object_parser parser;
    parser.deserialize(source, sizeof(source));
    ASSERT_EQ(parser.at("flag")->value<bool>(), true);
}

// 7. Тест записи и чтения с диска через deserialize_from()[cite: 2, 3]
void test_file_deserialization() {
    const std::string tmp_filename = "tmp_test_syntax.just";
    {
        std::ofstream out(tmp_filename, std::ios::binary);
        out << SYNTAX_JUST_SAMPLE;
    }

    just_object_parser parser;
    parser.deserialize_from(tmp_filename);

    ASSERT_TRUE(parser.contains("struct_tree/humans/human1/name"));
    ASSERT_EQ(parser.at("struct_tree/humans/human1/name")->value<jstring>(), "Alex");
    ASSERT_TRUE(parser.occupied_memory() > 0);

    std::remove(tmp_filename.c_str());

    // Проверка ошибки при открытии несуществующего файла[cite: 2]
    ASSERT_THROWS(parser.deserialize_from("no_such_file_99999.just"), std::runtime_error);
}

// 8. Тест обработки ошибок и невалидного синтаксиса[cite: 2]
void test_error_handling() {
    just_object_parser parser;

    // 1. Недопустимое имя свойства (начинается с цифры)[cite: 2]
    ASSERT_THROWS(parser.deserialize("123invalid { a 1 }"), std::bad_exception);

    // 2. Незакрытая фигурная скобка блока[cite: 2]
    ASSERT_THROWS(parser.deserialize("broken_block { a 1 "), std::bad_exception);

    // 3. Несовместимые типы в массиве (например, строка и число)[cite: 2]
    ASSERT_THROWS(parser.deserialize("bad_array { \"text\", 123 }"), std::runtime_error);

    // 4. Обращение к несуществующему узлу через оператор <<[cite: 2]
    parser.deserialize("root { a 1 }");
    ASSERT_THROWS(parser << "root" << "missing_child", std::runtime_error);
}

// 9. Тест многократного переиспользования объекта парсера (очистка памяти)[cite: 2]
void test_parser_reuse_memory_reset() {
    just_object_parser parser;

    parser.deserialize("first_run { val 100 }");
    ASSERT_TRUE(parser.contains("first_run/val"));
    ASSERT_EQ(parser.at("first_run/val")->value<int>(), 100);

    // Повторный вызов deserialize должен очистить старое дерево и _storage[cite: 2]
    parser.deserialize("second_run { val 200 }");
    ASSERT_FALSE(parser.contains("first_run/val"));
    ASSERT_TRUE(parser.contains("second_run/val"));
    ASSERT_EQ(parser.at("second_run/val")->value<int>(), 200);
}

// ============================================================================
// Точка входа
// ============================================================================
int main() {
    std::cout << "==========================================\n";
    std::cout << "   Running Just Object Parser Test Suite  \n";
    std::cout << "==========================================\n";

    run_test("1. Full syntax.just parsing & navigation", test_syntax_just_full);
    run_test("2. Arrays & mix (Number -> Real) promotion", test_arrays_and_mix_promotion);
    run_test("3. Signed primitives & string escaping", test_primitives_and_escaping);
    run_test("4. Hash collision resistance on anagrams", test_anagram_hash_collisions);
    run_test("5. Deep tree search() & cast operators", test_deep_search_and_operators);
    run_test("6. Serialize & Deserialize round-trip", test_serialize_roundtrip);
    run_test("6a. Non-terminated input buffer", test_non_terminated_input_buffer);
    run_test("7. File I/O (deserialize_from)", test_file_deserialization);
    run_test("8. Invalid syntax & exception handling", test_error_handling);
    run_test("9. Parser reuse & storage cleanup", test_parser_reuse_memory_reset);

    std::cout << "==========================================\n";
    std::cout << "Total:  " << total_tests << "\n";
    std::cout << COLOR_GREEN << "Passed: " << passed_tests << COLOR_RESET << "\n";
    if (failed_tests > 0) {
        std::cout << COLOR_RED << "Failed: " << failed_tests << COLOR_RESET << "\n";
        return 1;
    }
    std::cout << "All tests passed successfully!\n";
    return 0;
}