
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>
#include <comma/application/command_line_options.h>

// name of executable (will be set to argv[0])
static const char *exec_name = "";

void usage()
{
    std::cerr << "Usage: " << exec_name << " [-h|--help] [-a|--assign] [-t|--test] [-d|--demangle] [<input_file>]\n"
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
"    -h|--help      Show this help\n"
"    -a|--assign    Expect input of the form <name>=<value> (<value> becomes quoted or not as appropriate)\n"
"    -t|--test      Expect input to be boolean expressions; output will be like \"if not (<expr>): print ...\"\n"
"                   (i.e. prints a message if the expression is not true)\n"
"    -d|--demangle  Transform variable names from \"mangled\" form (no slashes or square brackets) back to\n"
"                   their original form\n"
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
    bool command;   // set to !(assign || test)
    bool demangle;

    Options() : assign(false), test(false), command(false), demangle(false) { }
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
    return is_start_of_id(ch) || std::isdigit(ch) || ch == '/' || ch == '[' || ch == ']';
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
            // escape any quotes of the same type, unless it is already escaped
            if (str[n] == '\\') { result += str[++n]; if (n > last) break; }
            else if (str[n] == quote_char) { result += '\\'; }
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
    for (size_t n = 0;n < id.length();++n)
    {
        char ch = id[n];
        if (ch == '/')      { result += "_S"; }
        else if (ch == '[') { result += "_L"; }
        else if (ch == ']') { result += "_R"; }
        else if (ch == '_') { result += "__"; }
        else { result += ch; }

    }
    return result;
}

// inverse of mangle_id()
std::string demangle_id(const std::string &id)
{
    std::string result;
    size_t len = id.length();

    for (size_t n = 0;n < len;++n)
    {
        char ch = id[n];

        if (ch == '_' && n + 1 < len)
        {
            ch = id[++n];
            if (ch == 'S') { ch = '/'; }
            else if (ch == 'L') { ch = '['; }
            else if (ch == 'R') { ch = ']'; }
            // otherwise ch must be "_'
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
                std::vector<Token> rest(tokens.begin() + n + 2, tokens.end()); // the rest of the tokens
                tokens.erase(tokens.begin() + n - 3, tokens.end());
                tokens.push_back(Token(t_function, "near"));
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

// split a string into individual tokens
void tokenise(const std::string &line, const Options &opt,
    /*out*/ std::vector<Token> &tokens)
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
            while (is_id(char_at(line, pos))) { ++pos; }
            std::string id = line.substr(tok_start, pos - tok_start);
            if (is_keyword(id)) { tok_str = id; type = t_keyword; }
            else if (next_nonblank_char(line, pos) == '(') { tok_str = id; type = t_function; }
            else { tok_str = (opt.demangle ? demangle_id(id) : mangle_id(id)); type = t_id; }
        }
        else
        if (std::isdigit(ch) || ch == '.' || (ch == '-' && (std::isdigit(next_ch) || next_ch == '.')))
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
            if (tok_str == "=") { found_assign_op = true; if (opt.test) tok_str = "=="; }
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

void print_error_prefix(const std::string &filename, int line_num)
{
    std::cerr << exec_name << ": line " << line_num;
    if (!filename.empty()) { std::cerr << " of " << filename; }
    std::cerr << ": ";
}

void process_assign(const std::vector<Token> &tokens, const std::string &input_line,
    const std::string &filename, int line_num)
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
        { std::cout << tokens << '\n'; }
    }
}

void process_test(const std::vector<Token> &tokens, const std::string &original_line)
{
    if (tokens.empty()) { return; }
    std::string input_line = trim_spaces(original_line);

    // create a set of all variables in the expression (so we can print each one just once)
    Varmap vars;
    for (size_t n = 0;n < tokens.size();++n)
    {
        if (tokens[n].type == t_id)
        {
            std::string id = tokens[n].str;
            vars[demangle_id(id)] = id;
        }
    }

    std::cout << "# " << input_line << '\n';
    std::cout << "if not(" << tokens << "):\n";

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

            // i->first is the demangled (original) name, i->second is the mangled name
            // (repr() puts single quotes around strings; replace with double quotes)
            std::cout << "    print '" << i->first << "/expected=" << quote(expr_str, '"') << "'\n";
            std::cout << "    print '" << i->first << "/actual='+repr(" << i->second << ").replace(\"'\", '\"')\n";
        }
    }
    else
    {
        std::cout << "    print 'false=" << quote(input_line, '\"') << "'\n";
    }
}

void process_command(const std::vector<Token> &tokens, Varmap &assigned_vars, bool option_demangle)
{
    std::cout << tokens << '\n';

    if (!option_demangle)
    {
        // add any assigned variables to the Varmap
        for (size_t n = 0;n < tokens.size();++n)
        {
            if (tokens[n].type == t_operator && tokens[n].str == "=" && n > 0 && tokens[n - 1].type == t_id)
            {
                std::string id = tokens[n - 1].str;
                assigned_vars[demangle_id(id)] = id;
            }
        }
    }
}

void print_function_defs()
{
    std::cout
        << "def near(x, y, eps):\n"
        << "    return abs(x - y) <= eps\n";
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
void process(const std::string &filename, const Options &opt)
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

    for (int line_num = 1;std::getline(infile, line);++line_num)
    {
        tokens.resize(0);
        tokenise(line, opt, tokens);
        // debug_tokens(tokens);

        if (opt.assign) { process_assign(tokens, line, filename, line_num); }
        else if (opt.test) { process_test(tokens, line); }
        else { process_command(tokens, assigned_vars, opt.demangle); }
    }

    if (opt.command) { print_assigned_variables(assigned_vars); }
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
    opt.command = !(opt.assign || opt.test);
    opt.demangle = options.exists("-d,--demangle");

    if (opt.assign && opt.test)
    { std::cerr << exec_name << ": cannot have --assign and --test\n"; exit(1); }

    if (opt.demangle && (opt.assign || opt.test))
    { std::cerr << exec_name << ": cannot use --demangle with --assign or --test\n"; exit(1); }

    // get unnamed options
    const char *valueless_options = "-a,--assign,-t,--test,-d,--demangle";
    const char *options_with_values = "";
    std::vector<std::string> unnamed = options.unnamed(valueless_options, options_with_values);
    std::string filename;

    for (size_t i = 0;i < unnamed.size();++i)
    {
        if (unnamed[i][0] == '-') { std::cerr << exec_name << ": unknown option \"" << unnamed[i] << "\"\n"; exit(1); }
        else if (filename.empty()) { filename = unnamed[i]; }
        else { std::cerr << exec_name << ": unexpected argument \"" << unnamed[i] << "\"\n"; exit(1); }
    }

    if (!opt.assign && !opt.demangle) { print_function_defs(); }
    process(filename, opt);
    return 0;
}

