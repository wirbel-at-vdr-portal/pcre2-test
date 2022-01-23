/*******************************************************************************
 * a bare minimum example how to include PCRE2 in your projects.
 * Winfried Koehler <nvdec A.T. quantentunnel D.O.T. de>
 *
 * NOTE: this example uses librepfunc,
 *       https://github.com/wirbel-at-vdr-portal/librepfunc
 ******************************************************************************/
#include <string>
#include <iostream>
#include <algorithm>
#include <repfunc.h>


/* we use UTF-8 here, so each code unit is 8 bits.
 * NOTE:
 *   --> define PCRE2_CODE_UNIT_WIDTH before #include <pcre2.h>
 */
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>


/*******************************************************************************
 * class Pcre2Match, encapsulates PCRE2.
 ******************************************************************************/

class Pcre2Match {
private:
  pcre2_code* re;
  pcre2_match_data* match_data;
  std::string errmsg;
public:
  Pcre2Match(std::string Pattern);
  ~Pcre2Match();

  bool Matches(std::string s);
  std::string ErrorMessage(void);
};


Pcre2Match::Pcre2Match(std::string Pattern) {
  int errorcode;
  PCRE2_SIZE erroroffset;

  re = pcre2_compile(
         (PCRE2_SPTR) Pattern.c_str(),
         PCRE2_ZERO_TERMINATED,
         0
          // | PCRE2_ANCHORED           // Force pattern anchoring
          // | PCRE2_ALLOW_EMPTY_CLASS  // Allow empty classes
          // | PCRE2_ALT_BSUX           // Alternative handling of \u, \U, and \x
          // | PCRE2_ALT_CIRCUMFLEX     // Alternative handling of ^ in multiline mode
          // | PCRE2_ALT_VERBNAMES      // Process backslashes in verb names
          // | PCRE2_AUTO_CALLOUT       // Compile automatic callouts
         | PCRE2_CASELESS               // Do caseless matching
          // | PCRE2_DOLLAR_ENDONLY     // $ not to match newline at end
          // | PCRE2_DOTALL             // . matches anything including NL
         | PCRE2_DUPNAMES               // Allow duplicate names for subpatterns
          // | PCRE2_ENDANCHORED        // Pattern can match only at end of subject
          // | PCRE2_EXTENDED           // Ignore white space and # comments
          // | PCRE2_FIRSTLINE          // Force matching to be before newline
          // | PCRE2_LITERAL            // Pattern characters are all literal
          // | PCRE2_MATCH_INVALID_UTF  // Enable support for matching invalid UTF
          // | PCRE2_MATCH_UNSET_BACKREF// Match unset backreferences
          // | PCRE2_MULTILINE          // ^ and $ match newlines within data
         | PCRE2_NEVER_BACKSLASH_C      // Lock out the use of \C in patterns
         | PCRE2_NEVER_UCP              // Lock out PCRE2_UCP, e.g. via (*UCP)
          // | PCRE2_NEVER_UTF          // Lock out PCRE2_UTF, e.g. via (*UTF)
          // | PCRE2_NO_AUTO_CAPTURE    // Disable numbered capturing parentheses (named ones available)
          // | PCRE2_NO_AUTO_POSSESS    // Disable auto-possessification
          // | PCRE2_NO_DOTSTAR_ANCHOR  // Disable automatic anchoring for .*
          // | PCRE2_NO_START_OPTIMIZE  // Disable match-time start optimizations
         | PCRE2_NO_UTF_CHECK           // Do not check the pattern for UTF validity (only relevant if PCRE2_UTF is set)
          // | PCRE2_UCP                // Use Unicode properties for \d, \w, etc.
          // | PCRE2_UNGREEDY           // Invert greediness of quantifiers
          // | PCRE2_USE_OFFSET_LIMIT   // Enable offset limit for unanchored matching
         | PCRE2_UTF                    // Treat pattern and subjects as UTF strings
         ,
         &errorcode,
         &erroroffset,
         nullptr);

  if (re == nullptr) {
     PCRE2_UCHAR buf[256];
     pcre2_get_error_message(errorcode, buf, sizeof(buf));
     errmsg = (char*) buf;
     match_data = nullptr;
     return;
     }

  match_data = pcre2_match_data_create_from_pattern(re, nullptr);
}


Pcre2Match::~Pcre2Match() {
  pcre2_match_data_free(match_data);
  pcre2_code_free(re);
}


bool Pcre2Match::Matches(std::string s) {
  if ((re == nullptr) or (match_data == nullptr))
     return false;

  int rc = pcre2_match(
              re,                    /* the compiled pattern */
              (PCRE2_SPTR) s.c_str(),/* the subject string */
              s.size(),              /* the length of the subject */
              0,                     /* start at offset 0 in the subject */
              0,                     /* default options */
              match_data,            /* block for storing the result */
              nullptr);              /* use default match context */

  if (rc < 0) {
     PCRE2_UCHAR buf[256];
     pcre2_get_error_message(rc, buf, sizeof(buf));
     errmsg = (char*) buf;
     return false;
     }

  return true;
}


std::string Pcre2Match::ErrorMessage(void) {
  return errmsg;
}




/*******************************************************************************
 * the program: List all files in folder '/video', which matches the pattern
 * given as arg.
 ******************************************************************************/

int main(int argc, char** argp) {
  if (argc < 2) {
     std::cout << "usage: " << argp[0] << " <pcre pattern>" << std::endl;
     return 0;
     }
  cFileList files("/video");
  auto list = files.List();
  std::sort(list.begin(), list.end());
  Pcre2Match pm(argp[1]);

  for(auto f:list) {
     if (pm.Matches(f))
        std::cout << f << std::endl;
     }

  return 0;
}
