#include "rofl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Defines.
#define HINT_CHAR ':'
#define ASSIGN_CHAR '='
#define OBJECT_CHAR '#'
#define COMMENT_CHAR ';'
#define STRLIT_ESC_CHAR '\\'
#define MAX_OBJ_NESTING 100
#define MAX_ARRAY_LENGTH 100

// Global constants.
static const char indent_chars[] = {'\t', ' '};
static const char strlit_chars[] = {'\'', '"'};

// Enums.
enum mode {
    READ_INDENT,
    READ_OBJECT,
    READ_KEY,
    READ_HINT, 
    READ_VALUE
};

// Routines
// Parse rofl.
enum rofl_error parse_rofl(
    const char* p_string, 
    rofl_callback_t p_callback,
    bool p_quiet
) {
    // Setup.
    enum mode mode = READ_INDENT;
    struct {
        bool defined;
        bool word_completed;
        struct rofl_slice names[MAX_OBJ_NESTING];
        unsigned int nest_count;
    } object = {0};
    struct {
        struct{
            bool word_completed;
            struct rofl_slice slice;
        } name;
        struct {
            bool word_completed;
            struct rofl_slice slice;
        } hint;
        struct {
            struct {
                double values[MAX_ARRAY_LENGTH];
                unsigned int count;
            } number_value_pool;
            struct {
                struct rofl_slice values[MAX_ARRAY_LENGTH];
                unsigned int count;
            } string_value_pool;
        } values;
    } member = {0};
    struct {
        unsigned int current; 
        unsigned int previous;
        unsigned int factor; // Whitespace count per indent.
        int c;
    } indent = {0, 0, 0, '\0'};
    int strlit_char = '\0';
    unsigned int row = 1;
    unsigned int col = 1;
    int c = *p_string;

    // Local macros.
#define TARGET_OBJECT object.names[object.nest_count]
#define FINISH_LINE() { \
    if(strlit_char != '\0') { \
        PRINT_ERROR("Unterminated string literal."); \
        return ROFL_ERROR_SYNTAX; \
    } \
    switch (mode) { \
        case READ_VALUE: { \
            struct rofl_result result = p_callback( \
                object.names, \
                object.nest_count, \
                member.name.slice, \
                member.hint.slice, \
                member.values.number_value_pool.values, \
                member.values.number_value_pool.count, \
                member.values.string_value_pool.values, \
                member.values.string_value_pool.count \
            ); \
            if(result.error != ROFL_ERROR_OK) { \
                PRINT_RESULT_ERROR(result); \
                return result.error; \
            }\
            break;  \
         } \
        case READ_OBJECT: \
            if(TARGET_OBJECT.length == 0) { \
                PRINT_ERROR("Object name is left blank.");\
                return ROFL_ERROR_SYNTAX; \
            } \
            object.defined = true; \
            break; \
        case READ_INDENT: \
            indent.current = 0; \
            break; \
        default: \
            PRINT_ERROR("Member without value is forbidden."); \
            return ROFL_ERROR_SYNTAX; \
    } \
}
#define SETUP_NEXT_LINE() { \
    indent.previous = indent.current; \
    indent.current = 0; \
    mode = READ_INDENT; \
    object.word_completed = false; \
    member.name.word_completed = false; \
    member.name.slice.length = 0; \
    member.hint.word_completed = false; \
    member.hint.slice.length = 0; \
    member.values.number_value_pool.count = 0; \
    member.values.string_value_pool.count = 0; \
    row++; \
    col = 1; \
    c = *++p_string; \
    continue; \
}
#define PRINT_ERROR(mp_message) { \
    if(!p_quiet) printf("ROFL Error: " mp_message \
            " [row %u, col %u]\n", row, \
            col); \
}
#define PRINT_RESULT_ERROR(mp_result) {\
    if(!p_quiet) printf("ROFL Error: %s [row %u, col %u]\n", (mp_result).message, row, col); \
}
#define NEXT_CHAR() { \
    c = *++p_string; \
    col++; \
    continue; \
}


    // Loop through the texts.
    while(c) {
        // Meet comment.
        if(c == COMMENT_CHAR && strlit_char == '\0') {
            // Skip to new line.
            p_string = strchr(p_string, '\n');
            
            // Stop the text if there is no new line character.
            // In other word, the text has reached the end.
            if(p_string == NULL) break;

            FINISH_LINE();
            SETUP_NEXT_LINE();
        }
        // Meet new line.
        else if(c == '\n') {
            FINISH_LINE();
            SETUP_NEXT_LINE();
        }
        // Expecting indentation.
        else if(mode == READ_INDENT) {
            // Current char is not an indent char.
            if(!strchr(indent_chars, c)) {
                // If indent factor is yet to setup,
                // then this is (probably) the first 
                // indentation, set indent factor.
                if(indent.factor == 0) indent.factor = indent.current;

                // Check if correct indentation.
                if((indent.current % indent.factor) != 0) {
                    PRINT_ERROR("Invalid indentation.");
                    return ROFL_ERROR_SYNTAX;
                }

                // Calculate nest count.
                if(indent.current > indent.previous) {
                    int indent_difference = (indent.current - indent.previous) / indent.factor;
                    object.nest_count += indent_difference;
                    object.defined = false; // New scope = request object to be
                                            // defined.
                } else if (indent.current < indent.previous) {
                    int indent_difference = (indent.previous - indent.current) / indent.factor;
                    object.nest_count -= indent_difference;
                }

                // Check if nest count is too big.
                if(object.nest_count >= MAX_OBJ_NESTING) {
                    PRINT_ERROR("Too many nestings.");
                    return ROFL_ERROR_INTERNAL;
                }

                // Setup for reading object.
                if(c == OBJECT_CHAR) {
                    mode = READ_OBJECT;
                    TARGET_OBJECT.begin = p_string + 1;
                    TARGET_OBJECT.length = 0;
                    NEXT_CHAR();
                }
                // Make sure target object is defined.
                else if(!object.defined) {
                    PRINT_ERROR("Object is yet to be defined.");
                    return ROFL_ERROR_SYNTAX;
                }

                // Read member.
                mode = READ_KEY;
                member.name.slice.begin = p_string;
                member.name.slice.length = 0;
                continue; // Reparse the character again.
            }

            // Setup indent_char.
            if(indent.c == '\0')
                indent.c = c;

            // Check if indent is mixed.
            if(indent.c != c) {
                if(!p_quiet)
                    PRINT_ERROR( "Mix indentation.");
                return ROFL_ERROR_SYNTAX;
            }

            // Calculate indentation.
            indent.current++;
            NEXT_CHAR();
        } 

        // Complete calculating indentation.
        // So start parsing.
        // Expecting object name.
        else if(mode == READ_OBJECT) {
            // Handle whitespace.
            if(isspace(c)) {
                if(TARGET_OBJECT.length != 0) object.word_completed = true;
                else TARGET_OBJECT.begin++; // Haven't meet object name.
                NEXT_CHAR();
            }

            // Check if it is a valid char for name.
            if(!(isalnum(c) || c == '_')) {
                PRINT_ERROR("Invalid character for an object name.");
                return ROFL_ERROR_SYNTAX;
            }
            
            // Check if the word already completed.
            if (object.word_completed){
                PRINT_ERROR("There can only be one object name");
                return ROFL_ERROR_SYNTAX;
            }

            // Include current char as object name.
            TARGET_OBJECT.length++;
            NEXT_CHAR();
        }

        // Expecting member name.
        else if(mode == READ_KEY) {
            // Check if start hinting value.
            if(c == HINT_CHAR) {
               // Check if name is empty.
               if(member.name.slice.length == 0) {
                   PRINT_ERROR("Expecting a member name.");
                   return ROFL_ERROR_SYNTAX;
               }
               member.hint.slice.begin = p_string + 1;
               mode = READ_HINT;
               NEXT_CHAR();
            }
            
            // Check if start assigning value.
            if(c == ASSIGN_CHAR) {
               // Check if name is empty.
               if(member.name.slice.length == 0) {
                   PRINT_ERROR("Expecting a member name.");
                   return ROFL_ERROR_SYNTAX;
               }
               mode = READ_VALUE;
               NEXT_CHAR();
            }

            // Handle whitespace.
            if(isspace(c)) {
                if(member.name.slice.length != 0) member.name.word_completed = true;
                else member.name.slice.begin++;
                NEXT_CHAR();
            }

            // Check if it is a valid char for name.
            if(!(isalnum(c) || c == '_')) {
               PRINT_ERROR("Invalid character for a member name.");
               return ROFL_ERROR_SYNTAX;
            }

            // Check if the member name already completed.
            if (member.name.word_completed){
                PRINT_ERROR("There can only be one member name");
                return ROFL_ERROR_SYNTAX;
            }

            // Include current char as member name.
            member.name.slice.length++;
            NEXT_CHAR();
        }

        // Expecting member hint.
        else if(mode == READ_HINT) {
            // Check if start assigning value.
            if(c == ASSIGN_CHAR) {
               // Check if name is empty.
               if(member.hint.slice.length == 0) {
                   PRINT_ERROR("Expecting a hint.");
                   return ROFL_ERROR_SYNTAX;
               }
               mode = READ_VALUE;
               NEXT_CHAR();
            }

            // Handle whitespace.
            if(isspace(c)) {
                if(member.hint.slice.length != 0) member.hint.word_completed = true;
                else member.hint.slice.begin = p_string + 1;
                NEXT_CHAR();
            }
            else if (member.hint.word_completed){
                PRINT_ERROR("There can only be one hint");
                return ROFL_ERROR_SYNTAX;
            }

            // Include current char as hint.
            member.hint.slice.length++;
            NEXT_CHAR();
        }

        // Expecting member values.
        else if(mode == READ_VALUE) {

            // Handle strlit char.
            if(strchr(strlit_chars, c)) {
                // Setup strlit.
                if(strlit_char == '\0') {
                    // Check if member is occupied by string.
                    if(member.values.number_value_pool.count != 0) {
                        PRINT_ERROR("Member already assigned with number(s).");
                        return ROFL_ERROR_SYNTAX;
                    }

                    // Check if there is too much values.
                    if(member.values.string_value_pool.count >= MAX_ARRAY_LENGTH) {
                        PRINT_ERROR("Too many values.");
                        return ROFL_ERROR_INTERNAL;
                    }

                    member.values.string_value_pool.values[
                        member.values.string_value_pool.count].begin = p_string + 1;
                    member.values.string_value_pool.values[
                        member.values.string_value_pool.count].length = 0;
                    member.values.string_value_pool.count++;
                    strlit_char = c;
                }

                // Terminate string literal.
                else strlit_char = '\0';

                NEXT_CHAR();
            }
            // If in string literal.
            else if(strlit_char != '\0') {
                member.values.string_value_pool.values[
                    member.values.string_value_pool.count - 1].length++;
                NEXT_CHAR();
            }

            // If met number.
            if(isdigit(c)) {
                // Check if member is occupied by string.
                if(member.values.string_value_pool.count != 0) {
                    PRINT_ERROR("Member already assigned with string(s).");
                    return ROFL_ERROR_SYNTAX;
                }

                // Check if have space.
                if(member.values.number_value_pool.count >= MAX_ARRAY_LENGTH) {
                    PRINT_ERROR("Too many values.");
                    return ROFL_ERROR_INTERNAL;
                }

                // Convert to number immediately.
                double result;
                int count;
                sscanf(p_string, "%lf%n", &result, &count);
                member.values.number_value_pool.values[
                    member.values.number_value_pool.count++] = result;

                // Update position manually.
                p_string += count;
                col += count;
                c = *p_string;
                continue;
            }
            
            // Skip whitespace.
            if(isspace(c)) NEXT_CHAR();

            // Oh no, it is not a string or a number charater.
            // Objection!!!
            PRINT_ERROR("Unexpected character.");
            return ROFL_ERROR_SYNTAX;
        }

        // Shouldn't reach this part.
        PRINT_ERROR("This error shouldn't be raised.");
        return ROFL_ERROR_INTERNAL;
    };

    // Finish the final line that is not ended with new line character.
    FINISH_LINE();

    // End local macros.
#undef FINISH_LINE
#undef SETUP_NEXT_LINE
#undef PRINT_ERROR
#undef PRINT_RESULT_ERROR
#undef REGISTER_VALUE
#undef NEXT_CHAR
#undef TARGET_OBJECT 

    return  ROFL_ERROR_OK;
}
