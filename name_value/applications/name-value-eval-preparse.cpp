
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>
#include <comma/application/command_line_options.h>

// name of executable (will be set to argv[0])
static const char *exec_name = "";
static std::string kwd_expect = "expect";

void usage()
{
    std::cerr << "Usage: " << exec_name << " [-h|--help] [-a|--assign] [-t|--test] [-o|--output-variables=<file>] [-d|--demangle] [<input_file>]\n"
"\n"
"Transform expressions containing path variables (i.e. with names like \"a/b[10/c\") to make them suitable for\n"
"input to Python.\n"
"\n"
"If no input file is specified, standard input is used.\n"
"\n"
"Option --assign expects the input to consist of assignments like <name>=<value>. The <value> may optionally\n"
"contain quotes, but they are not required, even if the value contains spaces (e.g. route=Sydney to Perth).\n"
"The generaed Python code only puts quotes around values that do not look like numbers.\n"
"\n"
"Option --test expects the input to consist of boolean expressions, e.g. \"a/b/c < 100\". Note that any \"=\" in the\n"
"input is transformed to \"==\". Multiple expressions may be used (one per line), but it is recommended to only test\n"
"one expression at a time, since Python may stop if there is an error. If the expression evaluates to True, there\n"
"is no output, otherwise the expected and actual values of the variable(s) in the expression are printed.\n"
"\n"
"Without option --assign or --test, the input is expected to be Python code. The output is the same as the input\n"
"(with variable names transformed), along with code to print any variables that are assigned a value (printed as\n"
"<name>=<value>). Option --demangle may be used to return variable names to their original form (in which case\n"
"assigned variables are not printed).\n"
"\n"
"Note that Python uses indentation to indicate blocks; indentation must match exactly. Python normally requires \":\"\n"
"at the end of an \"if\", \"while\" etc., but this app makes it optional (added if missing).\n"
"\n"
"Options:\n"
"\n"
"    -h|--help              Show this help\n"
"    -a|--assign            Expect input of the form <name>=<value> (<value> becomes quoted or not as appropriate)\n"
"    -t|--test              Expect input to be boolean expressions; output will be like \"if not (<expr>): print ...\"\n"
"                           (i.e. prints a message if the expression is not true)\n"
"    -o|--output-variables= Restrict output variables to just the ones in this file (not compatible with --test)\n"
"    -d|--demangle          Transform variable names from \"mangled\" form (no slashes or square brackets) back to\n"
"                           their original form\n"
"\n"
"Predefined Python functions:\n"
"\n"
"   near(x, y, eps)         True if x and y are within eps of each other (i.e. abs(x - y) <= eps)\n"
"   near_percent(x, y, p)   True if x and y are within p% of each other (i.e. abs(x - y) <= x * p / 100)\n"
"   max_index(dict)         Returns the maximum (integer) index in a dictionary (i.e. array)\n"
"   number_of(dict)         Returns max_index(dict) + 1 (i.e. assumes indexes start from 0 and none are missing)\n"
"   starts_with(s, x)       True if string s begins with string x\n"
"   ends_with(s, x)         True if string s ends with string x\n"
"   contains(s, x)          True if string s has a substring x\n"
"   matches(s, r)           Check if s matches regular expression r\n"
"                           See: http://docs.python.org/2/howto/regex.html\n"
"   sphere_distance(lat1,lon1,lat2,lon2)   Great circle distance between two points\n"
"\n"
"Special operators:\n"
"\n"
"    a == b +/- c           Becomes \"near(a, b, c)\" (or with \"!=\", \"not near(a, b, c)\"\n"
"    a == b +/- c%          Becomes \"near(a, b, c / 100.0 * a)\"\n"
"\n"
"Example:\n"
"\n"
"    ( " << exec_name << " --assign fixm-prototype.path-value\n"
"      echo \"route/fix[0]/altitude > 0\" | " << exec_name << " --test ) | python\n"
"\n"
"    ( cat vars.pv | " << exec_name << " --assign; " << exec_name << " rules.txt ) | python\n"
"\n";
}

// command line options
struct Options
{
    bool assign;
    bool test;
    bool command;           // set to !(assign || test)
    bool restrict_vars;     // whether to restrict the output variables to just the ones in the output_vars set
    bool demangle;
    std::set<std::string> output_vars;

    Options() : assign(false), test(false), command(false), restrict_vars(false), demangle(false) { }
};

// token types
enum token_type
{
    t_id, t_function, t_keyword, t_number, t_string, t_operator, t_undefined
};

// a single input token
struct Token
{
    token_type type;        // type
    std::string str;        // token (note that t_string retains its quotes)
    size_t spaces_before;   // number of whitespace characters before token

    Token()
        : type(t_undefined), spaces_before(0)
    { }

    Token(token_type t, const std::string &s, size_t num_spaces_before = 0)
        : type(t), str(s), spaces_before(num_spaces_before)
    { }
};

// map from demangled to mangled variable names (for storing things like the set of assigned variables)
typedef std::map<std::string, std::string> Varmap;

typedef std::set<std::string> Varset;

// return a string with n spaces
std::string spaces(size_t n)
{
    if (n <= 40) { return std::string("                                        ").substr(0, n); }
    std::string s;
    for ( ;n > 0;--n) { s += ' '; }
    return s;
}

// print a Token
std::ostream &operator << (std::ostream &out, const Token &t)
{
    return out << spaces(t.spaces_before) << t.str;
}

// print a vector of Tokens
std::ostream &operator << (std::ostream &out, const std::vector<Token> &t)
{
    for (size_t n = 0;n < t.size();++n) { out << t[n]; }
    return out;
}

// dump a vector of tokens
void debug_tokens(const std::vector<Token> &tokens)
{
    std::cout << "DEBUG TOKENS\n";
    for (size_t n = 0;n < tokens.size();n++)
    {
        std::cout << "  TOKEN: {" << spaces(tokens[n].spaces_before) << "}{" << tokens[n].str << "} ";
        switch (tokens[n].type)
        {
            case t_id:          std::cout << "id"; break;
            case t_keyword:     std::cout << "keyword"; break;
            case t_function:    std::cout << "function"; break;
            case t_number:      std::cout << "number"; break;
            case t_string:      std::cout << "string"; break;
            case t_operator:    std::cout << "operator"; break;
            case t_undefined:   std::cout << "undefined"; break;
            default:            std::cout << "???";
        }
        std::cout << '\n';
    }
}

// return true if ch can be the first character of an identifier
inline bool is_start_of_id(char ch)
{
    return std::isalpha(ch) || ch == '_';
}

// return true if ch can be a character in an identifier (other than the first character)
inline bool is_id(char ch)
{
    return is_start_of_id(ch) || std::isdigit(ch) || ch == '/' || ch == '.' || ch == '[' || ch == ']';
}

// return str[index] unless index is beyond the end of the string, in which case return '\0'
inline char char_at(const std::string &str, size_t index)
{
    if (index >= str.length()) { return '\0'; }
    return str[index];
}

bool starts_with(const std::string &str, const std::string &other)
{
    for (size_t n = 0;n < other.length();++n) { if (str[n] != other[n]) return false; }
    return true;
}

// get the length of the operator starting at pos (e.g. returns 2 for ">=" or 1 for "+")
size_t operator_length(const std::string &str, size_t pos)
{
    if (str.substr(pos, 3) == "+/-") { return 3; }

    const char *two_char_op[] =
    { "<=", ">=", "==", "!=", "<>", "+=", "-=", "*=", "/=", "%=", ">>", "<<", "**", "//", 0 };

    char ch = char_at(str, pos);
    char next_ch = char_at(str, pos + 1);

    for (size_t n = 0;two_char_op[n];++n)
    {
        if (ch == two_char_op[n][0] && next_ch == two_char_op[n][1]) { return 2; }
    }

    return 1;
}

// remove white space at the start and end of a string
std::string trim_spaces(const std::string &str)
{
    size_t n = 0;
    for ( ;std::isspace(char_at(str, n));++n) { }
    if (n >= str.length()) { return ""; }

    size_t m = str.length() - 1;
    for ( ;std::isspace(char_at(str, m));--m) { }

    return str.substr(n, m - n + 1);
}

// put quotes around a string (unless it is already quoted)
std::string quote(const std::string &str, char quote_char)
{
    static const char squote = '\'';
    static const char dquote = '\"';

    std::string result;
    result += quote_char;

    if (!str.empty())
    {
        // check for existing quotes
        size_t first = 0;
        size_t last = str.length() - 1;

        if ((str[0] == squote || str[0] == dquote) && str[0] == str[last])
        { ++first; --last; }

        for (size_t n = first;n <= last;++n)
        {
            // escape any quotes
            //if (str[n] == '\\') { result += str[++n]; if (n > last) break; }
            if (str[n] == '\\') { result += str[n]; }
            else if (str[n] == squote || str[n] == dquote) { result += '\\'; }
            result += str[n];
        }
    }

    result += quote_char;
    return result;
}

char next_nonblank_char(const std::string &line, size_t pos)
{
    while (pos < line.length() && isspace(line[pos])) { ++pos; }
    return (pos < line.length() ? line[pos] : '\0');
}

// check if a string looks like a number (with no trailing characters)
bool is_number(const std::string &str)
{
    size_t pos = 0;
    bool any_digits = false;

    if (str[0] == '-') { ++pos; }
    while (std::isdigit(char_at(str, pos))) { ++pos; any_digits = true; }
    if (char_at(str, pos) == '.')
    {
        ++pos;
        while (std::isdigit(char_at(str, pos))) { ++pos; any_digits = true; }
    }

    return (any_digits && pos >= str.length());
}

// check if a string is a Python keyword
bool is_keyword(const std::string &str)
{
    const char *python_keyword[] =
    {
        "and", "assert", "break", "class", "continue", "def", "del", "elif", "else", "except", "exec",
        "finally", "for", "from", "global", "if", "import", "in", "is", "lambda", "not", "or", "pass",
        "print", "raise", "return", "try", "while", "yield", 0
    };

    for (size_t n = 0;python_keyword[n];++n) { if (str == python_keyword[n]) return true; }
    return false;
}

// transform an id so that it has no special characters ("/" "[" or "]")
std::string mangle_id(const std::string &id)
{
    std::string result;
    int last_start_pos = 0;

    for (size_t n = 0;n < id.length();++n)
    {
        char ch = id[n];
        if (ch == '/') { ch = '.'; }

        if (ch == '.')
        {
            std::string sub_id = id.substr(last_start_pos, n - last_start_pos);
            if (is_keyword(sub_id)) { result += '_'; }
            last_start_pos = n + 1;
        }

        result += ch;

        /*
        if (ch == '/')      { result += "_S"; }
        else if (ch == '[') { result += "_L"; }
        else if (ch == ']') { result += "_R"; }
        else if (ch == '_') { result += "__"; }
        else { result += ch; }
        */

    }

    std::string sub_id = id.substr(last_start_pos, std::string::npos);
    if (is_keyword(sub_id)) { result += '_'; }

    return result;
}

// inverse of mangle_id()
std::string demangle_id(const std::string &id, bool restore_slashes)
{
    std::string result;
    size_t len = id.length();
    int start_pos = 0;

    for (size_t n = 0;n < len;++n)
    {
        char ch = id[n];
        if (ch == '.' || ch == '/') { start_pos = n + 1; }

        if (ch == '.' && restore_slashes) { ch = '/'; }
        else if (ch == '_' && (n + 1 == len || id[n + 1] == '.' || id[n + 1] == '/'))
        {
            std::string sub_id = id.substr(start_pos, n - start_pos);
            if (is_keyword(sub_id)) { continue; }  // exclude the final underscore
        }

        result += ch;
    }
    return result;
}

// check for special token sequences such as "a == b +/- c" and replace them with the required Python code
void transform_special_tokens(/*out*/ std::vector<Token> &tokens)
{
    for (size_t n = 0;n < tokens.size();++n)
    {
        if (tokens[n].type == t_operator && tokens[n].str == "+/-" && n >= 3 && n < tokens.size() - 1 &&
            tokens[n - 2].type == t_operator)
        {
            std::string op = tokens[n - 2].str;
            if (op == "==" || op == "!=")
            {
                Token lhs = tokens[n - 3];
                // (tokens[n-2] is op)
                Token rhs = tokens[n - 1];
                // (tokens[n] is "+/-")
                Token eps = tokens[n + 1];

                // check for percentage (e.g. "500 +/- 10%")
                bool is_percent = (n + 2 < tokens.size() && tokens[n + 2].type == t_operator && tokens[n + 2].str == "%");

                int start_of_rest = n + 2;
                if (is_percent) { ++start_of_rest; }

                std::vector<Token> rest(tokens.begin() + start_of_rest, tokens.end()); // the rest of the tokens
                tokens.erase(tokens.begin() + n - 3, tokens.end());

                if (op == "!=") { tokens.push_back(Token(t_keyword, "not", 1)); }

                tokens.push_back(Token(t_function, (is_percent ? "near_percent" : "near"), 1));
                tokens.push_back(Token(t_operator, "("));
                tokens.push_back(lhs);
                tokens.push_back(Token(t_operator, ","));
                tokens.push_back(rhs);
                tokens.push_back(Token(t_operator, ","));
                tokens.push_back(eps);
                tokens.push_back(Token(t_operator, ")"));
                tokens.insert(tokens.end(), rest.begin(), rest.end());  // append rest of tokens
            }
        }
    }
}

// get the index of the first token with a particular type and value
// Returns -1 if the token is not in the vector
int find_token(const std::vector<Token> &tokens, token_type type, const std::string &val)
{
    for (size_t n = 0;n < tokens.size();++n) { if (tokens[n].type == type && tokens[n].str == val) return n; }
    return -1;
}

void check_transform_id(std::string &id)
{
    while (true)
    {
        size_t pos = id.find("[MAX]");
        if (pos == std::string::npos) { break; }

        std::string replacement = "max_index(";
        replacement += id.substr(0, pos);
        replacement += ")";
        id.replace(pos + 1, 3, replacement);
    }
}

// split a string into individual tokens
void tokenise(const std::string &line, const Options &opt,
    /*out*/ std::vector<Token> &tokens,
    bool test_is_raw_python = false   // whether --test input is raw python code (vs just a list of boolean expressions)
)
{
    size_t pos = 0;
    size_t len = line.length();
    bool found_assign_op = false;

    while (pos < len)
    {
        size_t space_start = pos;

        // skip whitespace (and finish if end of line or comment)
        while (std::isspace(char_at(line, pos))) { ++pos; }
        if (pos >= len || line[pos] == '#') { break; }

        size_t num_spaces = pos - space_start;
        size_t tok_start = pos;
        char ch = line[pos];
        char next_ch = char_at(line, pos + 1);
        token_type type = t_undefined;
        std::string tok_str;

        if (opt.assign && found_assign_op)
        {
            // treat the rest of the line as one long string
            pos = len;
            tok_str = quote(trim_spaces(line.substr(tok_start, pos - tok_start)), '\'');
            std::string unquoted_tok_str = tok_str.substr(1, tok_str.length() - 2);
            if (is_number(unquoted_tok_str)) { tok_str = unquoted_tok_str; }
            type = t_string;
        }
        else
        if (is_start_of_id(ch))
        {
            // TODO: maybe allow spaces around array indexes , e.g. "a/b[ 10 ]/c"
            while (is_id(char_at(line, pos))) { ++pos; }
            std::string id = line.substr(tok_start, pos - tok_start);
            check_transform_id(id);
            if (is_keyword(id)) { tok_str = id; type = t_keyword; }
            else if (next_nonblank_char(line, pos) == '(') { tok_str = id; type = t_function; }
            else { tok_str = (opt.demangle ? demangle_id(id, true) : mangle_id(id)); type = t_id; }
        }
        else
        if (std::isdigit(ch) ||
            (ch == '.' && std::isdigit(next_ch)) ||
            (ch == '-' && (std::isdigit(next_ch) || next_ch == '.')))
        {
            bool any_digits = false;

            if (ch == '-') { ++pos; }
            while (std::isdigit(char_at(line, pos))) { ++pos; any_digits = true; }

            if (char_at(line, pos) == '.')
            {
                ++pos;
                while (std::isdigit(char_at(line, pos))) { ++pos; any_digits = true; }
            }

            if (any_digits) { type = t_number; }
            else { type = t_operator; pos = tok_start + 1; }  // token is a single character ("-" or ".")

            tok_str = line.substr(tok_start, pos - tok_start);
        }
        else
        if (ch == '\"' || ch == '\'')
        {
            for (++pos;pos < len && line[pos] != ch;++pos)
            {
                // check for escape character
                if (line[pos] == '\\') { if (++pos >= len) break; }
            }
            if (pos < len) { ++pos; }
            tok_str = line.substr(tok_start, pos - tok_start);
            type = t_string;
        }
        else
        {
            pos += operator_length(line, pos);
            tok_str = line.substr(tok_start, pos - tok_start);
            type = t_operator;
            if (tok_str == "=")
            {
                found_assign_op = true;
                if (opt.test && (!test_is_raw_python || 
                    (tokens.size() > 0 && tokens[0].type == t_id && tokens[0].str == kwd_expect)))
                { tok_str = "=="; }
            }
        }

        tokens.push_back(Token(type, tok_str, num_spaces));

        if (!opt.command && tokens.size() == 1)
        {
            // Python will complain if the line is indented
            tokens[0].spaces_before = 0;
        }
    }

    transform_special_tokens(tokens);

    if (opt.assign)
    {
        // transform var= into var=''
        if (tokens.size() == 2 && tokens[1].str == "=") { tokens.push_back(Token(t_string, "''", 0)); }
    }

    if (opt.command)
    {
        // add any missing ":" at the end of the line
        if (tokens.size() > 0 && tokens[0].type == t_keyword)
        {
            // check if there is a ":" anywhere in the line
            if (find_token(tokens, t_operator, ":") == -1)
            {
                if (tokens[0].str == "if" || tokens[0].str == "else" || tokens[0].str == "elif" ||
                    tokens[0].str == "while" || tokens[0].str == "for" || tokens[0].str == "try" ||
                    tokens[0].str == "except" || tokens[0].str == "finally")
                { tokens.push_back(Token(t_operator, ":", 0)); }
            }
        }
    }
}

// output the definition of an empty Python class
void define_python_class(const char *class_name)
{
    std::cout << "class " << class_name << " : pass\n";
}

// output the appropriate Python statements to initialise a variable such as "a.b[10].c"
//
// Each level of the hierarchy is checked to see if it has already been initialised; if not,
// the appropriate initialisation is performed. For arrays (treated as Python dictionaries),
// the statement "whatever={}" is output if the array has not been seen before;
// for everything else, a new level in the hierarchy is initialised using "whatever=OBJ()"
// (other than the last one, e.g. c in a.b.c, since it will be assigned a value directly)

void init_variable_hierarchy(const std::string &var_name, Varset &variable_hierarchy)
{
    // an empty class
    const char *python_class = "OBJ";

    int last_pos = -1;
    std::string hierarchy;

    while (true)
    {
        size_t pos = var_name.find_first_of('.', last_pos + 1);
        int next_pos = (pos == std::string::npos ? var_name.length() : (int) pos);

        // path up to this point ("a", then "a.b", then "a.b.c", etc.)
        std::string hierarchy(var_name, 0, next_pos);

        size_t bracket_pos = hierarchy.find_first_of('[', last_pos + 1);
        bool is_array = (bracket_pos != std::string::npos);

        if (is_array)
        {
            std::string upto_bracket(var_name, 0, bracket_pos + 1);  // include final "["
            Varset::iterator i = variable_hierarchy.find(upto_bracket);

            if (i == variable_hierarchy.end())
            {
                if (variable_hierarchy.size() == 0) { define_python_class(python_class); }
                variable_hierarchy.insert(upto_bracket);
                std::string before_bracket(var_name, 0, bracket_pos);  // exclude final "["
                std::cout << before_bracket << "={}\n";
            }
        }

        if (next_pos == (int) var_name.length()) { break; }

        Varset::iterator i = variable_hierarchy.find(hierarchy);

        if (i == variable_hierarchy.end())
        {
            if (variable_hierarchy.size() == 0) { define_python_class(python_class); }
            variable_hierarchy.insert(hierarchy);
            std::cout << hierarchy << "=" << python_class << "()\n";
        }

        last_pos = next_pos;
    }
}

void print_error_prefix(const std::string &filename, int line_num)
{
    std::cerr << exec_name << ": line " << line_num;
    if (!filename.empty()) { std::cerr << " of " << filename; }
    std::cerr << ": ";
}

void process_assign(const std::vector<Token> &tokens, const std::string &input_line,
    const std::string &filename, int line_num, Varset &variable_hierarchy)
{
    if (tokens.size() != 0)
    {
        if (tokens.size() != 3 || tokens[1].str != "=" || tokens[0].type != t_id)
        {
            print_error_prefix(filename, line_num);
            std::cerr << "expected \"name=value\"; got: \"" << input_line << "\"\n";
        }
        else
        if (tokens[0].type == t_keyword)
        {
            print_error_prefix(filename, line_num);
            std::cerr << "illegal name \"" << tokens[0].str << "\" (Python keyword)\n";
        }
        else
        {
            init_variable_hierarchy(tokens[0].str, variable_hierarchy);
            std::cout << tokens << '\n';
        }
    }
}

size_t count_starting_spaces(const std::string &s)
{
    for (size_t n = 0;n < s.size();++n)
    {
        if (s[n] != ' ' && s[n] != '\t') { return n; }
    }
    return s.size();
}

void process_test(std::vector<Token> &tokens, const std::string &original_line,
    const std::string &filename, int line_num, bool *raw_python)
{
    std::string input_line;
    size_t leading_spaces = 0;

    if (line_num == 1)
    {
        if (starts_with(original_line, "#python"))
        {
            std::cout << "# raw python\n";
            *raw_python = true;
            return;
        }
        std::cout << "# not raw python\n";
    }

    if (tokens.empty()) { return; }

    if (*raw_python)
    {
        leading_spaces = count_starting_spaces(original_line);

        if (tokens[0].type == t_id && tokens[0].str == kwd_expect)
        {
            tokens.erase(tokens.begin());
            if (tokens.empty()) { return; }  // really an error to have nothing after "expect", but minor
            tokens[0].spaces_before = 0;
            // (by this stage, tokens[0].spaces_before has been set to 0; get leading_spaces from original line)
            input_line = trim_spaces(original_line.substr(leading_spaces + kwd_expect.length()));
            // continue to parse as normal
        }
        else
        {
            std::cout << "# SRCLINE " << line_num << ' ' << original_line << '\n'
                << spaces(leading_spaces) << tokens << '\n';
            // can't just output original_line, since variable names need to be transformed
            // (e.g. from "a/b/c" to "a.b.c")
            return;
        }
    }
    else
    {
        input_line = trim_spaces(original_line);
    }

    // create a set of all variables in the expression (so we can print each one just once)
    Varmap vars;
    for (size_t n = 0;n < tokens.size();++n)
    {
        if (tokens[n].type == t_id)
        {
            std::string id = tokens[n].str;
            vars[demangle_id(id, true)] = id;
        }
    }

    std::cout << "# SRCLINE " << line_num << " " << input_line << '\n'
        << spaces(leading_spaces) << "_result_ = (" << tokens << ")\n"
        << spaces(leading_spaces) << "if type(_result_) != bool: err_expr_not_bool()\n"
        << spaces(leading_spaces) << "elif not _result_:\n";

    if (vars.size() != 0)
    {
        Varmap::const_iterator i = vars.begin();
        Varmap::const_iterator end = vars.end();

        for ( ;i != end;++i)
        {
            std::string expr_str = input_line;

            // transform "var==value" to "value", "var < value" to "< value" and leave everything else unchanged
            if (starts_with(expr_str, i->first))
            {
                expr_str = trim_spaces(expr_str.substr(i->first.length()));
                if (starts_with(expr_str, "==")) { expr_str = trim_spaces(expr_str.substr(2)); }
                else if (starts_with(expr_str, "=")) { expr_str = trim_spaces(expr_str.substr(1)); }
            }

            std::cout << spaces(leading_spaces)
                << "    print '" << i->first << "/expected=" << quote(expr_str, '"') << "'\n";

            // use a Python trick to force repr() to use double quotes instead of single
            // (for an explanation, see: http://www.gossamer-threads.com/lists/python/python/157285
            // -- search that page for "Python delimits a string it by single quotes preferably")
            std::cout << spaces(leading_spaces)
                << "    print '" << i->first << "/actual=\"'+repr(\"'\\0\"+str(" << i->second << "))[6:]\n";
        }
    }
    else
    {
        std::cout << spaces(leading_spaces)
            << "    print 'false=" << quote(input_line, '\"') << "'\n";
    }
}

void process_command(const std::vector<Token> &tokens, Varmap &assigned_vars, const Options &opt,   
    const std::set<std::string> &restrict_vars)
{
    std::cout << tokens << '\n';

    if (!opt.demangle)
    {
        // add any assigned variables to the Varmap
        for (size_t n = 0;n < tokens.size();++n)
        {
            if (tokens[n].type == t_operator && tokens[n].str == "=" && n > 0 && tokens[n - 1].type == t_id)
            {
                std::string id = tokens[n - 1].str;
                if (!(opt.restrict_vars && restrict_vars.find(id) == restrict_vars.end()))
                { assigned_vars[demangle_id(id, false)] = id; }
            }
        }
    }
}

void print_header()
{
    std::cout
        << "import sys, re, inspect, math\n"
        << "def near(x, y, eps): return abs(x - y) <= eps\n"
        << "def near_percent(x, y, percent): return abs(x - y) <= x * percent * 0.01\n"
        << "def max_index(dict) : return max(dict.keys())\n"
        << "def number_of(dict) : return len(dict.keys())\n"
        << "def starts_with(s, x): return s.find(x) == 0\n"
        << "def ends_with(s, x): return s.rfind(x) == len(s) - len(x)\n"
        << "def contains(s, x): return s.find(x) != -1\n"
        << "def matches(s, p): return re.search(p, s) != None\n"
        << "def deg_to_rad(d): return (d / 180.0) * 3.14159265359\n"
        << "def rad_to_deg(r): return (r / 3.14159265359) * 180.0\n"
        << "def km_to_nm(k): return k / 1.852\n"
        << "def nm_to_km(n): return n * 1.852\n"
        << "def sphere_distance_km(lat1, lon1, lat2, lon2):\n"
        << "    phi1 = deg_to_rad(lat1)\n"
        << "    phi2 = deg_to_rad(lat2)\n"
        << "    lat_delta = deg_to_rad(lat2 - lat1)\n"
        << "    lon_delta = deg_to_rad(lon2 - lon1)\n"
        << "    res_val = math.sin(lat_delta / 2.0) * math.sin(lat_delta / 2.0) + math.cos(phi1) * math.cos(phi2) * math.sin(lon_delta / 2.0) * math.sin(lon_delta / 2.0)\n"
        << "    return 6366.70702 * 2.0 * math.atan2(math.sqrt(res_val), math.sqrt(1.0 - res_val))\n"
        << "def sphere_distance_nm(lat1, lon1, lat2, lon2): return km_to_nm(sphere_distance_km(lat1, lon1, lat2, lon2))\n"
        << "def err_expr_not_bool(): print >> sys.stderr, 'File \"?\", line ' + str(inspect.currentframe().f_back.f_lineno) + '\\nTypeError: expected a true or false expression'\n";
        // note: err_expr_not_bool() imitates standard Python error printing:
        // 'File "name", line n' on one line, followed by the error message
}

void print_assigned_variables(const Varmap &assigned_vars)
{
    Varmap::const_iterator i = assigned_vars.begin();
    Varmap::const_iterator end = assigned_vars.end();

    for ( ;i != end;++i)
    {
        // i->first is the demangled (original) name, i->second is the mangled name
        // (repr() puts single quotes around strings; replace with double quotes)
        std::cout << "print '" << i->first << "='+repr(" << i->second << ").replace(\"'\", '\"')\n";
    }
}

// tokenise the input file (or stdin if filename is empty), outputing the appropriate Python code
void process(const std::string &filename, const Options &opt, const std::set<std::string> &restrict_vars)
{
    std::ifstream file_stream;
    
    if (!filename.empty())
    {
        file_stream.open(filename.c_str());
        if (!file_stream) { std::cerr << exec_name << ": cannot open " << filename << '\n'; exit(1); }
    }

    std::istream &infile = (filename.empty() ? std::cin : file_stream);

    std::string line;
    std::vector<Token> tokens;

    // map from demangled (original) variable name to mangled name for variables on lhs of an assignment statement;
    // not used for --assign or --test
    Varmap assigned_vars;

    // set of "seen" variable names (e.g. "a", "a.b", "a.b["); only used for --assign
    Varset variable_hierarchy;

    // with the input to --test is raw python code (vs just being a list of boolean expressions, one per line)
    bool test_is_raw_python = false;

    for (int line_num = 1;std::getline(infile, line);++line_num)
    {
        tokens.resize(0);
        tokenise(line, opt, tokens, test_is_raw_python);
        // debug_tokens(tokens);

        if (opt.assign) { process_assign(tokens, line, filename, line_num, variable_hierarchy); }
        else if (opt.test) { process_test(tokens, line, filename, line_num, &test_is_raw_python); }
        else { process_command(tokens, assigned_vars, opt, restrict_vars); }
    }

    if (opt.command) { print_assigned_variables(assigned_vars); }
}

void read_restrict_vars(const std::string &filename, std::set<std::string> &restrict_vars)
{
    std::ifstream file(filename.c_str());

    if (!file)
    {
        std::cerr << exec_name << ": cannot open " << filename << '\n';
        exit(1);
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::string var_name = trim_spaces(line);
        if (!var_name.empty()) { restrict_vars.insert(mangle_id(var_name)); }
    }

    if (restrict_vars.size() == 0)
    {
        std::cerr << exec_name << ": empty --output-variables file: " << filename << '\n';
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    exec_name = argv[0];
    comma::command_line_options options(argc, argv);
    if (options.exists("-h,--help")) { usage(); return 0; }

    // get flags
    Options opt;
    opt.assign = options.exists("-a,--assign");
    opt.test = options.exists("-t,--test");
    opt.restrict_vars = options.exists("-o,--output-variables");
    opt.command = !(opt.assign || opt.test);
    opt.demangle = options.exists("-d,--demangle");

    if (opt.test)
    {
        if (opt.assign) { std::cerr << exec_name << ": cannot have --assign and --test\n"; exit(1); }
        if (opt.restrict_vars) { std::cerr << exec_name << ": cannot have --output-variables and --test\n"; exit(1); }
    }

    if (opt.demangle && (opt.assign || opt.test))
    { std::cerr << exec_name << ": cannot use --demangle with --assign or --test\n"; exit(1); }

    // get unnamed options
    const char *valueless_options = "-a,--assign,-t,--test,-d,--demangle";
    const char *options_with_values = "-o,--output-variables";
    std::vector<std::string> unnamed = options.unnamed(valueless_options, options_with_values);
    std::set<std::string> restrict_vars;
    std::string filename;

    for (size_t i = 0;i < unnamed.size();++i)
    {
        if (unnamed[i][0] == '-') { std::cerr << exec_name << ": unknown option \"" << unnamed[i] << "\"\n"; exit(1); }
        else if (filename.empty()) { filename = unnamed[i]; }
        else { std::cerr << exec_name << ": unexpected argument \"" << unnamed[i] << "\"\n"; exit(1); }
    }

    if (opt.restrict_vars)
    {
        std::string restrict_filename = options.value<std::string> ("-o,--output-variables");
        if (restrict_filename.empty()) { std::cerr << exec_name << ": expected filename for --output-variables\n"; exit(1); }
        read_restrict_vars(restrict_filename, restrict_vars);
    }

    if (!opt.assign && !opt.demangle) { print_header(); }
    process(filename, opt, restrict_vars);
    return 0;
}

